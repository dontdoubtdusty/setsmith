//+----------------------------------------------------------------------------+
//| Description:  Setsmith - flat line icons drawn in code                     |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// Setsmith: toolbar, tab and menu icons are drawn as vector lines (Stage 5 of the new look, 2026-11-22),
// so they stay sharp at any size and can be recolored (gold tab, dim when disabled).
// Names match the old PNGs in resource/tool; a name without a drawing falls back to the PNG.

#include <util/prec.hpp>

/// Draw the named icon at size x size pixels in one color. Returns an invalid image for unknown names.
wxImage draw_icon(const String& name, int size, const wxColour& colour);
