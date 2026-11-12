//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the dark workshop theme                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// Setsmith: the look of the app. One place for the palette, so a color is changed once.
// No animation and no custom drawing loops: colors are set once per window.

#include <util/prec.hpp>

namespace theme {

// ----------------------------------------------------------------------------- : Palette

wxColour backdrop();   ///< darkest: behind the card preview, workspace areas
wxColour panel();      ///< panels, toolbars, dialogs
wxColour field();      ///< lists and text boxes
wxColour border();     ///< thin lines
wxColour text();
wxColour text_dim();   ///< disabled and secondary text
wxColour accent();     ///< logo gold: selection, focus
wxColour accent_text();///< text on top of the accent

/// Dark replacement for wxSystemSettings::GetColour
wxColour colour(wxSystemColour c);

// ----------------------------------------------------------------------------- : Setup

/// Register the bundled Figtree fonts and, on Windows 10 or later, ask for dark scrollbars and menus.
/** Call once at startup, before any window is made. */
void init();

/// The UI font (Figtree if it was found, else the system font)
wxFont font(int point_size = 0, bool bold = false);

/// Color a window and everything inside it. Safe to call again.
void apply(wxWindow* window);

}
