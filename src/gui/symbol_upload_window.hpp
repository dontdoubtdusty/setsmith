//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/symbol.hpp>

class SymbolUploadPreview;

// ----------------------------------------------------------------------------- : SymbolUploadWindow

/// Dialog for picking an image as the set symbol (MSE Slim; replaces the symbol editor).
/** Lists best practices for set symbols, lets the user choose an image file, previews it in each
 *  rarity before it is used, and warns about images that won't print well.
 */
class SymbolUploadWindow : public wxDialog {
public:
  SymbolUploadWindow(Window* parent, const SymbolStyle& style);

  /// The chosen image, scaled down to at most 1024 px on its longest side. Only valid after OK.
  Image getImage() const;

private:
  DECLARE_EVENT_TABLE();

  Image image;
  SymbolUploadPreview* preview;
  wxStaticText* file_label;
  wxStaticText* notes;

  void onChoose(wxCommandEvent&);
  /// Check an image and say what might go wrong with it; false if it can't be used at all
  bool checkImage(const Image& img, const String& filename);
};
