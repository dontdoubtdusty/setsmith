//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the sets shown on the dashboard                   |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// Setsmith: finds sets in the watched folders and the recent list, and reads just enough of each one for a
// dashboard tile (title, code, game, card names, one image). Never loads the game or the whole set, so a folder
// full of sets shows up quickly.

#include <util/prec.hpp>

/// What a dashboard tile shows for one set
struct SetSummary {
  String path;                ///< full path of the .mse-set file or folder
  String title, code, game;   ///< from the set header; title falls back to the file name
  vector<String> card_names;  ///< card names in file order, tags removed
  String tile_card;           ///< the card picked for the tile picture, empty for the first card
  String tile_image;          ///< file inside the package holding that card's image, empty if none
  wxDateTime modified;        ///< last change of the file
  wxImage image;              ///< decoded tile picture, filled in later by load_tile_image
  bool image_loaded = false;  ///< load_tile_image has run (the image may still be empty)
};

/// The folder that is always watched: Documents\Setsmith Cards
String default_watched_folder();

/// Paths of every set to show: sets in the watched folders plus the recent sets that still exist, without repeats
vector<String> find_library_sets();

/// Reads a set's header and card list. Returns false if the file is not a readable set.
bool read_set_summary(const String& path, SetSummary& out);

/// Decodes the summary's tile picture from the package (into out.image) and marks it loaded
void load_tile_image(SetSummary& out);

/// Stores the card picked for a set's tile picture in the set file (opens and saves the whole set)
void save_tile_card(const String& path, const String& card_name);
