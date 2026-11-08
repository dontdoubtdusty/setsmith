//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/symbol.hpp>
#include <gui/symbol_upload_window.hpp>
#include <gui/util.hpp>
#include <data/action/value.hpp>

// ----------------------------------------------------------------------------- : SymbolValueEditor

// MSE Slim: an Upload button (and double click) open SymbolUploadWindow instead of the symbol editor.
IMPLEMENT_VALUE_EDITOR(Symbol)
  , button_down(false)
{
  button_image = Bitmap(load_resource_image(_("edit_symbol")));
}

void SymbolValueEditor::draw(RotatedDC& dc) {
  SymbolValueViewer::draw(dc);
  // helper text if there is no symbol yet
  if (symbols.empty()) {
    dc.SetFont(wxFont(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(*wxBLACK);
    String hint = _("Double click to upload a symbol");
    RealSize text_size = dc.GetTextExtent(hint);
    dc.DrawText(hint, align_in_rect(ALIGN_MIDDLE_CENTER, text_size, dc.getInternalRect()));
  }
  if (nativeLook()) {
    // the Upload button, at the right edge
    double height = bounding_box.height;
    double width  = bounding_box.height + 2;
    RealRect rect(bounding_box.width - width, 0, width, height);
    draw_button(&editor(), dc.getDC(), dc.trRectToBB(rect), false, button_down, true);
    dc.SetFont(*wxNORMAL_FONT);
    String label = _("Upload");
    RealSize text_size = dc.GetTextExtent(label);
    dc.DrawText(label, align_in_rect((Alignment)(ALIGN_BOTTOM | ALIGN_CENTER), text_size, RealRect(rect.x, 0, width, height * 0.9)));
    RealSize image_size(button_image.GetWidth(), button_image.GetHeight());
    dc.DrawBitmap(button_image, align_in_rect(ALIGN_MIDDLE_CENTER, image_size, RealRect(rect.x, 0, width, height * 0.8)));
  }
}

bool SymbolValueEditor::onButton(const RealPoint& pos) const {
  return pos.y >= 0 && pos.y < bounding_box.height && pos.x >= bounding_box.width - (bounding_box.height + 2);
}

bool SymbolValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent&) {
  if (!nativeLook() || !onButton(pos)) return false;
  button_down = true;
  parent.redraw(*this);
  return true;
}

bool SymbolValueEditor::onLeftUp(const RealPoint& pos, wxMouseEvent&) {
  if (!button_down) return false;
  button_down = false;
  parent.redraw(*this);
  if (onButton(pos)) uploadSymbol();
  return true;
}

bool SymbolValueEditor::onLeftDClick(const RealPoint&, wxMouseEvent&) {
  uploadSymbol();
  return true;
}

void SymbolValueEditor::determineSize(bool) {
  if (style().height == 0) style().height = 50;
  bounding_box.height = 50;
}

void SymbolValueEditor::uploadSymbol() {
  SymbolUploadWindow wnd(wxGetTopLevelParent(&editor()), style());
  if (wnd.ShowModal() != wxID_OK) return;
  // store the image in the set as a new .png, so undo can go back to the previous symbol
  LocalFileName new_file = getLocalPackage().newFileName(field().name, _(".png"));
  wnd.getImage().SaveFile(getLocalPackage().nameOut(new_file), wxBITMAP_TYPE_PNG);
  addAction(value_action(valueP(), new_file));
}
