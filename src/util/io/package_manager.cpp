//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <util/error.hpp>
#include <util/file_utils.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <data/locale.hpp>
#include <data/export_template.hpp>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

// ----------------------------------------------------------------------------- : PackageManager : in memory

PackageManager package_manager;

// The name templates use to depend on the program itself (an internal identifier, kept for compatibility).
static const String mse_package = _("magicseteditor.exe");


void PackageManager::init() {
  local.init(true);
  global.init(false);
  if (!(local.valid() || global.valid()))
    throw Error(_("The MSE data files can not be found, there should be a directory called 'data' with these files. ")
                _("The expected place to find it in was either ") + wxStandardPaths::Get().GetDataDir() + _(" or ") +
                wxStandardPaths::Get().GetUserDataDir());
}
void PackageManager::destroy() {
  loaded_packages.clear();
}
void PackageManager::reset() {
  loaded_packages.clear();
}

PackagedP PackageManager::openAny(const String& name_, bool just_header) {
  String name = trim(name_);
  if (starts_with(name,_("/"))) name = name.substr(1);
  if (starts_with(name,_(":NO-WARN-DEP:"))) name = name.substr(13);
  // Attempt to load local data first.
  String filename;
  if (wxFileName(name).IsRelative()) {
    // local data dir?
    filename = normalize_filename(local.name(name));
    if (!wxFileExists(filename) && !wxDirExists(filename)) {
      // global data dir
      filename = normalize_filename(global.name(name));
    }
  } else { // Absolute filename
    filename = normalize_filename(name);
  }

  // Is this package already loaded?
  PackagedP& p = loaded_packages[filename];
  if (!p) {
    // load with the right type, based on extension
    wxFileName fn(filename);
    if      (fn.GetExt() == _("mse-game"))            p = make_intrusive<Game>();
    else if (fn.GetExt() == _("mse-style"))           p = make_intrusive<StyleSheet>();
    else if (fn.GetExt() == _("mse-locale"))          p = make_intrusive<Locale>();
    else if (fn.GetExt() == _("mse-include"))         p = make_intrusive<IncludePackage>();
    else if (fn.GetExt() == _("mse-symbol-font"))     p = make_intrusive<SymbolFont>();
    else if (fn.GetExt() == _("mse-export-template")) p = make_intrusive<ExportTemplate>();
    else {
      throw PackageError(_("Unrecognized package type: '") + fn.GetExt() + _("'\nwhile trying to open: ") + name);
    }
    p->open(filename, just_header);
  } else if (!just_header) {
    p->loadFully();
  }
  return p;
}

void PackageManager::findMatching(const String& pattern, vector<PackagedP>& out) {
  // first find local packages
  String file = local.findFirstMatching(pattern);
  while (!file.empty()) {
    out.push_back(openAny(file, true));
    file = wxFindNextFile();
  }
  // then global packages not already in the list
  file = global.findFirstMatching(pattern);
  while (!file.empty()) {
    PackagedP p = openAny(file, true);
    if (find(out.begin(), out.end(), p) == out.end()) {
      out.push_back(p);
    }
    file = wxFindNextFile();
  }
}

bool PackageManager::existsInPackage(const String& name) {
  if (!name.empty() && name.GetChar(0) == _('/')) {
    // break name
    size_t start = name.find_first_not_of(_("/\\"), 1); // allow "//package/name" from incorrect scripts
    size_t pos = name.find_first_of(_("/\\"), start);
    if (start < pos && pos != String::npos) {
      // open package
      PackagedP p = openAny(name.substr(start, pos - start));
      return p->existsIn(name.substr(pos + 1));
    }
  }
  throw FileNotFoundError(name, _("No package name specified, use '/package/filename'"));
}

pair<unique_ptr<wxInputStream>,Packaged*> PackageManager::openFileFromPackage(Packaged* package, const String& name) {
  if (!name.empty() && name.GetChar(0) == _('/')) {
    // absolute name; break name
    size_t start = name.find_first_not_of(_("/\\"), 1); // allow "//package/name" from incorrect scripts
    size_t pos   = name.find_first_of(_("/\\"), start);
    if (start < pos && pos != String::npos) {
      // open package
      PackagedP p = openAny(name.substr(start, pos-start));
      if (package && !is_substr(name,start,_(":NO-WARN-DEP:"))) {
        package->requireDependency(p.get());
      }
      return {p->openIn(name.substr(pos + 1)), p.get()};
    }
  } else if (package) {
    // relative name
    return {package->openIn(name), package};
  }
  throw FileNotFoundError(name, _("No package name specified, use '/package/filename'"));
}
pair<unique_ptr<wxInputStream>, Packaged*> openFileFromPackage(Packaged* package, const String& name) {
  return package_manager.openFileFromPackage(package, name);
}

String PackageManager::openFilenameFromPackage(Packaged* package, const String& name) {
  if (!name.empty() && name.GetChar(0) == _('/')) {
    // absolute name; break name
    size_t start = name.find_first_not_of(_("/\\"), 1); // allow "//package/name" from incorrect scripts
    size_t pos   = name.find_first_of(_("/\\"), start);
    if (start < pos && pos != String::npos) {
      // open package
      PackagedP p = openAny(name.substr(start, pos-start));
      if (package && !is_substr(name,start,_(":NO-WARN-DEP:"))) {
        package->requireDependency(p.get());
      }
      return p->absoluteFilename() + _("/") + name.substr(pos + 1);
    }
  } else if (package) {
    // relative name
    return package->absoluteFilename() + _("/") + name;
  }
  throw FileNotFoundError(name, _("No package name specified, use '/package/filename'"));
}

String PackageManager::getDictionaryDir(bool l) const {
  String dir = (l ? local : global).getDirectory();
  if (dir.empty()) return wxEmptyString;
  else             return dir + _("/dictionaries/");
}

// ----------------------------------------------------------------------------- : PackageManager : on disk

bool PackageManager::checkDependency(const PackageDependency& dep, bool report_errors) {
  // mse package?
  if (dep.package == mse_package) {
    if (app_version < dep.version) {
      queue_message(MESSAGE_WARNING, _ERROR_3_("package out of date", _("Setsmith"), app_version.toString(), dep.version.toString()));
    }
    return true;
  }
  // does the package exist?
  if (!local.exists(dep.package) && !global.exists(dep.package)) {
    if (report_errors)
      queue_message(MESSAGE_WARNING, _ERROR_1_("package not found", dep.package));
    return false;
  }
  PackagedP package = openAny(dep.package, true);
  if (package->version < dep.version) {
    if (report_errors)
      queue_message(MESSAGE_WARNING, _ERROR_3_("package out of date", dep.package, package->version.toString(), dep.version.toString()));
    return false;
  }
  return true;
}
// ----------------------------------------------------------------------------- : PackageDirectory

void PackageDirectory::init(bool local) {
  is_local = local;
  if (local) {
    init(wxStandardPaths::Get().GetUserDataDir() + _("/data"));
  } else {
    // determine data directory
    String dir = wxStandardPaths::Get().GetDataDir();
    // check if this is the actual data directory, especially during debugging,
    // the data may be higher up:
    //  exe path  = mse/build/debug/mse.exe
    //  data path = mse/data
    while (!wxDirExists(dir + _("/data"))) {
      String d = dir;
      dir = wxPathOnly(dir);
      if (d == dir) {
        // we are at the root -> 'data' not found anywhere in the path
        dir = wxStandardPaths::Get().GetDataDir();
        break;
      }
    }
    init(dir + _("/data"));
  }
}
void PackageDirectory::init(const String& dir) {
  if (wxDirExists(dir))
    directory = dir;
  else
    directory.clear();
}

String PackageDirectory::name(const String& name) const {
  return directory + _("/") + name;
}
bool PackageDirectory::exists(const String& filename) const {
  String fn = name(filename);
  return wxFileExists(fn) || wxDirExists(fn);
}

String PackageDirectory::findFirstMatching(const String& pattern) const {
  if (!wxDirExists(directory)) return String();
  return wxFindFirstFile(directory + _("/") + pattern, 0);
}

