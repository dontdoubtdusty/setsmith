//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the dark workshop theme                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// Setsmith: the look of the app. One place for the palette, so a color is changed once.
// Colors are set once per window. The dashboard (welcome_window.cpp) is the one place with its own drawing
// and light hover animation (owner's choice, 2026-11-22).

#include <util/prec.hpp>

class wxGraphicsContext;

namespace theme {

// ----------------------------------------------------------------------------- : Palette

wxColour backdrop();   ///< darkest: behind the card preview, workspace areas
wxColour header();     ///< the header band, menu bar and status strip of the editor
wxColour panel();      ///< panels, dialogs
wxColour field();      ///< lists and text boxes
wxColour border();     ///< thin lines
wxColour raised();     ///< hovered buttons and menu items
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

// ----------------------------------------------------------------------------- : Drawing

/// A rounded "pill" button background: hover 0..1 lifts it, active fills it with gold.
void draw_pill(wxGraphicsContext& gc, const wxRect& r, double hover, bool active, bool pressed = false);

/// The thin gold line under a header band, fading out at both ends
void draw_gold_rule(wxGraphicsContext& gc, double x, double y, double width);

/// Full screen as Setsmith uses it: no title bar, border or menu bar (the header has a menu button)
long fullscreen_style();

// ----------------------------------------------------------------------------- : Windows only

/// Paint the menu bar of a frame dark. Call first thing in the frame's MSWWindowProc;
/// returns true when the message was handled and *result must be returned.
bool menu_bar_message(wxWindow* frame, WXUINT msg, WXWPARAM wp, WXLPARAM lp, WXLRESULT* result);

/// Paint over the light line Windows draws under the menu bar. Call after the default
/// MSWWindowProc for WM_NCPAINT and WM_NCACTIVATE.
void cover_menu_bar_line(wxWindow* frame, WXUINT msg);

}
