//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <wx/filename.h>

DECLARE_POINTER_TYPE(Packaged);
class PackageDependency;

// Setsmith: the installer, the installed-package database and the update data were removed.
// Packages are plain folders in the data directory next to the exe.

// ----------------------------------------------------------------------------- : PackageDirectory

/// A directory for packages
class PackageDirectory {
public:
  void init(bool local);
  void init(const String& dir);

  bool valid() const { return !directory.empty(); }

  /// Name of a package in this directory
  String name(const String& name) const;
  /// Does a package with the given name exist?
  bool exists(const String& name) const;

  /// Get the package directory
  inline String getDirectory() const { return directory; }

  /// Find all packages that match a filename pattern (using wxFindFirst)
  String findFirstMatching(const String& pattern) const;

private:
  bool   is_local;
  String directory;
};

// ----------------------------------------------------------------------------- : PackageManager

/// Package manager, loads data files from the default data directory.
/** The PackageManager ensures that each package is only loaded once.
 *  There is a single global instance of the PackageManager, called packages
 */
class PackageManager {
public:
  /// Initialize the package manager
  void init();
  /// Empty the list of packages.
  /** This function MUST be called before the program terminates, otherwise
   *  we could get into fights with pool allocators used by ScriptValues */
  void destroy();
  /// Empty the list of packages, they will all be reloaded
  void reset();

  // --------------------------------------------------- : Packages in memory

  /// Open a package with the specified name (including extension)
  template <typename T>
  intrusive_ptr<T> open(const String& name) {
    PackagedP p = openAny(name);
    intrusive_ptr<T> typedP = dynamic_pointer_cast<T>(p);
    if (typedP) {
      return typedP;
    } else {
      throw InternalError(format_string(_("Package %s loaded as wrong type"),name));
    }
  }

  /// Open a package with the specified name, the type of package is determined by its extension!
  /** @param if just_header is true, then the package is not fully parsed.
   */
  PackagedP openAny(const String& name, bool just_header = false);

  /// Find all packages that match a filename pattern, store them in out
  /** Only reads the package headers */
  void findMatching(const String& pattern, vector<PackagedP>& out);

  /// Check if a file exists in a package
  bool existsInPackage(const String& name);

  /// Open a file from a package, with a name encoded as "/package/file"
  /** If 'package' is set then:
   *    - tries to open a relative file from the package if the name is "file"
   *    - verifies a dependency from that package if an absolute filename is used
   *      this is to force people to fill in the dependencies
   *  Returns the opened file and the package it is in
   */
  pair<unique_ptr<wxInputStream>,Packaged*> openFileFromPackage(Packaged* package, const String& name);

  /// Get a filename to open from a package
  /** WARNING: this is a bit of a hack, since not all package types support names in this way.
   *  It is needed for third party libraries (i.e. hunspell) that load stuff from files.
   */
  String openFilenameFromPackage(Packaged* package, const String& name);

  // --------------------------------------------------- : Packages on disk

  /// Check if the given dependency is currently available
  bool checkDependency(const PackageDependency& dep, bool report_errors = true);

  // --------------------------------------------------- : Other package like things

  /// Get the directory for dictionary files
  String getDictionaryDir(bool local) const;

private:
  map<String, PackagedP> loaded_packages;
  PackageDirectory local, global;
};

/// The global PackageManager instance
extern PackageManager package_manager;
