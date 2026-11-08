//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/symbol.hpp>

// ----------------------------------------------------------------------------- : SymbolValueEditor

/// Shows a SymbolValue (such as the set symbol) and lets the user upload an image for it.
/** MSE Slim: the vector symbol editor was removed. An uploaded image is stored in the set as a
 *  .png and colored per rarity (render_image_symbol). Older vector symbols still show and render.
 */
class SymbolValueEditor : public SymbolValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Symbol);

  void draw(RotatedDC& dc) override;
  bool onLeftDown  (const RealPoint& pos, wxMouseEvent&) override;
  bool onLeftUp    (const RealPoint& pos, wxMouseEvent&) override;
  bool onLeftDClick(const RealPoint& pos, wxMouseEvent&) override;
  void determineSize(bool) override;
private:
  /// Is the position on the Upload button?
  bool onButton(const RealPoint& pos) const;
  /// Show the upload dialog and store the chosen image in the set
  void uploadSymbol();

  bool button_down;
  Bitmap button_image;
};
