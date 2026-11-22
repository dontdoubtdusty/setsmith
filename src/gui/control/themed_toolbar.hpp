//+----------------------------------------------------------------------------+
//| Description:  Setsmith - a toolbar with pill-shaped buttons                |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <util/prec.hpp>
#include <wx/toolbar.h>

/// Setsmith: the native toolbar, but each button gets a dark pill on hover and a gold one when checked.
/** On Windows the toolbar asks its owner to help draw each button (custom draw); we paint the pill
 *  and let Windows draw the icon and label on top, so dropdowns, tooltips and controls keep working.
 */
class ThemedToolBar : public wxToolBar {
public:
  ThemedToolBar(wxWindow* parent, wxWindowID id, long style = wxTB_FLAT | wxNO_BORDER | wxTB_HORIZONTAL | wxTB_NODIVIDER | wxTB_NOALIGN);

  #ifdef __WXMSW__
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM* result) override;
  #endif
};
