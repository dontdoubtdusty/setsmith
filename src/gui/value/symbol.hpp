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

/// Shows a SymbolValue (such as the set symbol) in the editor.
/** MSE Slim: the symbol editor was removed, so symbols are display-only.
 *  Symbols already stored in a set file still show and render on cards.
 */
class SymbolValueEditor : public SymbolValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Symbol);

  void determineSize(bool) override;
};

