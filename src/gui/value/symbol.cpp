//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/symbol.hpp>

// ----------------------------------------------------------------------------- : SymbolValueEditor

// MSE Slim: display only. The edit button, the "double click to edit symbol" hint and the
// double click handler were removed together with the symbol editor (gui/symbol).
IMPLEMENT_VALUE_EDITOR(Symbol) {}

void SymbolValueEditor::determineSize(bool) {
  if (style().height == 0) style().height = 50;
  bounding_box.height = 50;
}
