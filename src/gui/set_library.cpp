//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the sets shown on the dashboard                   |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set_library.hpp>
#include <data/settings.hpp>
#include <data/set.hpp>
#include <util/io/package.hpp>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/mstream.h>
#include <set>

// ----------------------------------------------------------------------------- : Finding sets

String default_watched_folder() {
  return wxStandardPaths::Get().GetDocumentsDir() + _("\\Setsmith Cards");
}

// A path in one standard form, so the same set found twice is shown once.
static String normalized(const String& path) {
  wxFileName fn(path);
  fn.Normalize(wxPATH_NORM_ABSOLUTE | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE);
  return fn.GetFullPath();
}

// Adds the sets directly inside one folder: .mse-set files and .mse-set folders.
static void add_sets_in(const String& folder, vector<String>& out) {
  if (!wxDir::Exists(folder)) return;
  wxDir dir(folder);
  if (!dir.IsOpened()) return;
  String name;
  for (bool ok = dir.GetFirst(&name, _("*.mse-set"), wxDIR_FILES | wxDIR_DIRS); ok; ok = dir.GetNext(&name)) {
    out.push_back(folder + _("\\") + name);
  }
}

vector<String> find_library_sets() {
  vector<String> found;
  add_sets_in(default_watched_folder(), found);
  for (const String& folder : settings.watched_folders) add_sets_in(folder, found);
  // Recent sets, except scratch sets in the temp folder (made by tools and tests)
  String temp = normalized(wxFileName::GetTempDir()).Lower();
  for (const String& file : settings.recent_sets) {
    if (normalized(file).Lower().StartsWith(temp)) continue;
    if (wxFileName::FileExists(file) || wxFileName::DirExists(file)) found.push_back(file);
  }
  vector<String> out;
  std::set<String> seen;
  for (const String& p : found) {
    String key = normalized(p).Lower();
    if (seen.insert(key).second) out.push_back(normalized(p));
  }
  return out;
}

// ----------------------------------------------------------------------------- : Reading a set

// Reads a whole file from a package as text (UTF-8, byte order mark removed).
static String read_text(Package& pkg, const String& file) {
  auto in = pkg.openIn(file);
  std::string bytes;
  char buf[65536];
  while (in && !in->Eof()) {
    in->Read(buf, sizeof(buf));
    size_t n = in->LastRead();
    if (n == 0) break;
    bytes.append(buf, n);
  }
  if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF) {
    bytes.erase(0, 3);
  }
  return String::FromUTF8(bytes.data(), bytes.size());
}

// Removes formatting tags such as <i> from a card name.
static String strip_tags(const String& s) {
  String out;
  bool in_tag = false;
  for (wxUniChar c : s) {
    if (c == '<') in_tag = true;
    else if (c == '>') in_tag = false;
    else if (!in_tag) out += c;
  }
  return out.Trim().Trim(false);
}

// The value of a "key: value" line, if the line (after `tabs` tabs) starts with that key.
static bool key_value(const String& line, int tabs, const String& key, String& value) {
  String prefix = String(_('\t'), tabs) + key + _(":");
  if (!line.StartsWith(prefix)) return false;
  value = line.Mid(prefix.size()).Trim().Trim(false);
  return true;
}

// Card names and image files from set text, in order. Folder sets keep each card in its own file
// ("include file: card X"), which is read the same way.
static void read_cards(Package& pkg, const String& text, vector<String>& names, vector<String>& images) {
  wxArrayString lines = wxSplit(text, '\n', '\0');
  bool in_card = false;
  for (String line : lines) {
    line.Replace(_("\r"), _(""));
    if (line.empty()) continue;
    if (line[0] != '\t') {
      in_card = line.StartsWith(_("card:"));
      if (in_card) { names.push_back(_("")); images.push_back(_("")); }
      String include;
      if (key_value(line, 0, _("include file"), include) && pkg.existsIn(include)) {
        read_cards(pkg, read_text(pkg, include), names, images);
      }
      continue;
    }
    if (!in_card) continue;
    String value;
    if (key_value(line, 1, _("name"), value) && names.back().empty()) names.back() = strip_tags(value);
    else if (key_value(line, 1, _("image"), value) && images.back().empty()) images.back() = value;
  }
}

bool read_set_summary(const String& path, SetSummary& out) {
  try {
    Package pkg;
    pkg.open(path, true);
    if (!pkg.existsIn(_("set"))) return false;
    String text = read_text(pkg, _("set"));
    out = SetSummary();
    out.path = path;
    out.modified = wxFileName(path).GetModificationTime();
    // Header: top-level "game:" and "tile card:", and "title:" / "set code:" inside "set info:".
    wxArrayString lines = wxSplit(text, '\n', '\0');
    bool in_info = false;
    for (String line : lines) {
      line.Replace(_("\r"), _(""));
      if (line.empty()) continue;
      if (line[0] != '\t') {
        in_info = line.StartsWith(_("set info:"));
        key_value(line, 0, _("game"), out.game);
        // written as "tile card"; the reader also accepts the underscore form
        if (!key_value(line, 0, _("tile card"), out.tile_card)) key_value(line, 0, _("tile_card"), out.tile_card);
        continue;
      }
      if (in_info) {
        String value;
        if (key_value(line, 1, _("title"), value)) out.title = strip_tags(value);
        else if (key_value(line, 1, _("set code"), value)) out.code = value;
      }
    }
    if (out.title.empty()) out.title = wxFileName(path).GetName();
    vector<String> images;
    read_cards(pkg, text, out.card_names, images);
    // The tile picture: the picked card's image, else the first card that has one.
    for (size_t i = 0; i < out.card_names.size() && out.tile_image.empty(); ++i) {
      if (!out.tile_card.empty() && out.card_names[i] == out.tile_card) out.tile_image = images[i];
    }
    for (size_t i = 0; i < images.size() && out.tile_image.empty(); ++i) out.tile_image = images[i];
    return true;
  } catch (...) {
    return false;
  }
}

void load_tile_image(SetSummary& out) {
  out.image_loaded = true;
  if (out.tile_image.empty()) return;
  try {
    Package pkg;
    pkg.open(out.path, true);
    if (!pkg.existsIn(out.tile_image)) return;
    // Copy into memory first: image decoders need a stream they can seek, and zip streams cannot.
    auto in = pkg.openIn(out.tile_image);
    wxMemoryOutputStream mem;
    in->Read(mem);
    wxMemoryInputStream img(mem);
    wxLogNull quiet; // a broken image just leaves the tile without a picture
    out.image.LoadFile(img, wxBITMAP_TYPE_ANY);
  } catch (...) {}
}

void save_tile_card(const String& path, const String& card_name) {
  SetP set = open_package<Set>(path);
  set->tile_card = card_name;
  set->save();
}
