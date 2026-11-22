//+----------------------------------------------------------------------------+
//| Description:  Setsmith - a toolbar with pill-shaped buttons                |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/themed_toolbar.hpp>
#include <gui/theme.hpp>
#include <wx/graphics.h>
#ifdef __WXMSW__
  #include <windows.h>
  #include <commctrl.h>
#endif

// ----------------------------------------------------------------------------- : ThemedToolBar

ThemedToolBar::ThemedToolBar(wxWindow* parent, wxWindowID id, long style)
  : wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style)
{
  SetBackgroundColour(theme::header());
  SetForegroundColour(theme::text());
  SetToolBitmapSize(wxSize(20, 20));
  SetToolPacking(4);
  SetToolSeparation(14);
}

#ifdef __WXMSW__
bool ThemedToolBar::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM* result) {
  NMHDR* hdr = (NMHDR*)lParam;
  if (hdr->code != NM_CUSTOMDRAW || hdr->hwndFrom != (HWND)GetHWND()) {
    return wxToolBar::MSWOnNotify(idCtrl, lParam, result);
  }
  NMTBCUSTOMDRAW* cd = (NMTBCUSTOMDRAW*)lParam;
  switch (cd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *result = CDRF_NOTIFYITEMDRAW;
      return true;
    case CDDS_ITEMPREPAINT: {
      UINT state = cd->nmcd.uItemState;
      bool disabled = state & CDIS_DISABLED;
      bool checked  = state & CDIS_CHECKED;
      bool hot      = (state & CDIS_HOT) && !disabled;
      bool pressed  = (state & CDIS_SELECTED) && !disabled;
      if (hot || pressed || checked) {
        // Paint the pill with GDI+ straight onto the toolbar's device context
        unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::CreateFromNativeHDC(cd->nmcd.hdc));
        if (gc) {
          RECT r = cd->nmcd.rc;
          wxRect rect(r.left + 1, r.top + 2, r.right - r.left - 2, r.bottom - r.top - 4);
          // a checked tool (bold, italic, ...) gets a soft gold pill, not the full gold of an active tab
          if (checked) {
            gc->SetPen(wxPen(wxColour(0xe0, 0xb0, 0x3a, 150)));
            gc->SetBrush(wxBrush(wxColour(0xe0, 0xb0, 0x3a, 60)));
            gc->DrawRoundedRectangle(rect.x + 0.5, rect.y + 0.5, rect.width - 1, rect.height - 1, 7);
          } else {
            theme::draw_pill(*gc, rect, hot ? 1.0 : 0.0, false, pressed);
          }
        }
      }
      cd->clrText = (COLORREF)(0xec | (0xe8 << 8) | (0xdf << 16));
      cd->clrTextHighlight = (COLORREF)(0xec | (0xe8 << 8) | (0xdf << 16));
      cd->clrBtnFace = (COLORREF)(0x09 | (0x0b << 8) | (0x11 << 16));
      *result = TBCDRF_NOBACKGROUND | TBCDRF_NOEDGES | TBCDRF_NOOFFSET | TBCDRF_NOMARK | TBCDRF_NOETCHEDEFFECT | CDRF_DODEFAULT;
      return true;
    }
    default:
      *result = CDRF_DODEFAULT;
      return true;
  }
}
#endif
