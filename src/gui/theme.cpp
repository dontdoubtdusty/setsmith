//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the dark workshop theme                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/theme.hpp>
#include <gui/control/themed_toolbar.hpp>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/toolbar.h>
#include <wx/statline.h>
#include <wx/graphics.h>
#include <algorithm>
#ifdef __WXMSW__
  #include <windows.h>
  #include <commctrl.h>
  #include <uxtheme.h>
#endif

namespace theme {

// ----------------------------------------------------------------------------- : Palette

// Setsmith: blue-black surfaces with a gold accent, matching the dashboard (2026-11-22).
wxColour backdrop()    { return wxColour(0x0b, 0x0d, 0x14); }
wxColour header()      { return wxColour(0x09, 0x0b, 0x11); }
wxColour panel()       { return wxColour(0x13, 0x17, 0x22); }
wxColour field()       { return wxColour(0x0f, 0x12, 0x1b); }
wxColour border()      { return wxColour(0x25, 0x2b, 0x3a); }
wxColour raised()      { return wxColour(0x1e, 0x24, 0x33); }
wxColour text()        { return wxColour(0xe8, 0xe6, 0xe1); }
wxColour text_dim()    { return wxColour(0x8f, 0x92, 0x99); }
wxColour accent()      { return wxColour(0xe0, 0xb0, 0x3a); }
wxColour accent_text() { return wxColour(0x17, 0x15, 0x0e); }

wxColour colour(wxSystemColour c) {
  // An if-chain, because several wx names are aliases of the same value.
  if (c == wxSYS_COLOUR_WINDOW || c == wxSYS_COLOUR_LISTBOX)       return field();
  if (c == wxSYS_COLOUR_WINDOWTEXT || c == wxSYS_COLOUR_BTNTEXT
   || c == wxSYS_COLOUR_LISTBOXTEXT || c == wxSYS_COLOUR_MENUTEXT)  return text();
  if (c == wxSYS_COLOUR_BTNFACE || c == wxSYS_COLOUR_MENU
   || c == wxSYS_COLOUR_FRAMEBK)                                    return panel();
  if (c == wxSYS_COLOUR_BTNSHADOW || c == wxSYS_COLOUR_3DLIGHT)    return border();
  if (c == wxSYS_COLOUR_3DDKSHADOW)                                 return wxColour(0x05, 0x06, 0x0a);
  if (c == wxSYS_COLOUR_BTNHIGHLIGHT)                               return raised();
  if (c == wxSYS_COLOUR_GRAYTEXT)                                   return text_dim();
  if (c == wxSYS_COLOUR_HIGHLIGHT || c == wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT) return accent();
  if (c == wxSYS_COLOUR_HIGHLIGHTTEXT)                              return accent_text();
  if (c == wxSYS_COLOUR_APPWORKSPACE)                               return backdrop();
  return wxSystemSettings::GetColour(c);
}

// ----------------------------------------------------------------------------- : Fonts

static bool have_figtree = false;

wxFont font(int point_size, bool bold) {
  if (point_size <= 0) point_size = 9;
  if (!have_figtree) {
    wxFont f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    f.SetPointSize(point_size);
    if (bold) f.SetWeight(wxFONTWEIGHT_BOLD);
    return f;
  }
  wxFontInfo info(point_size);
  info.FaceName(_("Figtree"));
  if (bold) info.Bold();
  return wxFont(info);
}

// ----------------------------------------------------------------------------- : Setup

#ifdef __WXMSW__
// Ask Windows 10 (1903 and later) for dark scrollbars and context menus. These two entry points
// are in uxtheme.dll by ordinal only. If they are missing, the app just looks a little lighter.
static void enable_dark_mode() {
  HMODULE ux = LoadLibraryW(L"uxtheme.dll");
  if (!ux) return;
  typedef int  (WINAPI *SetPreferredAppModeFn)(int);
  typedef void (WINAPI *FlushMenuThemesFn)();
  SetPreferredAppModeFn set_mode = (SetPreferredAppModeFn)GetProcAddress(ux, MAKEINTRESOURCEA(135));
  FlushMenuThemesFn     flush    = (FlushMenuThemesFn)    GetProcAddress(ux, MAKEINTRESOURCEA(136));
  if (set_mode) set_mode(2); // ForceDark
  if (flush)    flush();
}

// Dark title bar for a top-level window (Windows 10 20H1 and later; attribute 19 on older builds).
// Loaded by name from dwmapi.dll, so no extra link library is needed and older Windows simply keeps the light bar.
static void set_dark_title_bar(wxWindow* w) {
  HWND h = (HWND)w->GetHWND();
  HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
  if (!h || !dwm) return;
  typedef HRESULT (WINAPI *SetAttrFn)(HWND, DWORD, LPCVOID, DWORD);
  SetAttrFn set_attr = (SetAttrFn)GetProcAddress(dwm, "DwmSetWindowAttribute");
  if (!set_attr) return;
  // With the Windows 10 manifest, uxtheme repaints the frame light unless the window opts in to dark mode
  // (AllowDarkModeForWindow, uxtheme ordinal 133).
  typedef BOOL (WINAPI *AllowDarkFn)(HWND, BOOL);
  HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
  AllowDarkFn allow = ux ? (AllowDarkFn)GetProcAddress(ux, MAKEINTRESOURCEA(133)) : nullptr;
  if (allow) allow(h, TRUE);
  BOOL dark = TRUE;
  if (FAILED(set_attr(h, 20, &dark, sizeof(dark)))) set_attr(h, 19, &dark, sizeof(dark));
}

static void set_native_theme(wxWindow* w, const wchar_t* name) {
  HWND h = (HWND)w->GetHWND();
  if (h) SetWindowTheme(h, name, nullptr);
}
#endif

void init() {
  wxString dir = wxStandardPaths::Get().GetDataDir() + _("/resource/fonts/");
  wxArrayString files;
  wxDir::GetAllFiles(dir, &files, _("Figtree-*.ttf"), wxDIR_FILES);
  for (size_t i = 0; i < files.size(); ++i) {
    if (wxFont::AddPrivateFont(files[i])) have_figtree = true;
  }
  #ifdef __WXMSW__
    enable_dark_mode();
  #endif
}

// ----------------------------------------------------------------------------- : Applying

static bool is_input(wxWindow* w) {
  return dynamic_cast<wxListCtrl*>(w) || dynamic_cast<wxTextCtrl*>(w)
      || dynamic_cast<wxChoice*>(w)   || dynamic_cast<wxComboBox*>(w);
}

static void apply_one(wxWindow* w) {
  // Our own custom-drawn controls (card viewer, value editors) draw themselves with theme::colour,
  // so only the standard controls need their colors set here.
  bool input = is_input(w);
  // the editor's header toolbar sits on the darker header band
  bool header_tools = dynamic_cast<ThemedToolBar*>(w) != nullptr;
  w->SetBackgroundColour(input ? field() : header_tools ? header() : panel());
  w->SetForegroundColour(text());
  if (input || dynamic_cast<wxButton*>(w) || dynamic_cast<wxCheckBox*>(w)
      || dynamic_cast<wxStaticText*>(w)) {
    w->SetFont(font());
  }
  #ifdef __WXMSW__
    if (w->IsTopLevel()) set_dark_title_bar(w);
    if (dynamic_cast<wxListCtrl*>(w)) {
      set_native_theme(w, L"DarkMode_Explorer");
      HWND header = ListView_GetHeader((HWND)w->GetHWND());
      if (header) SetWindowTheme(header, L"DarkMode_ItemsView", nullptr);
    } else if (dynamic_cast<wxTextCtrl*>(w) || dynamic_cast<wxChoice*>(w) || dynamic_cast<wxComboBox*>(w)) {
      set_native_theme(w, L"DarkMode_CFD");
    } else if (dynamic_cast<wxButton*>(w) || dynamic_cast<wxCheckBox*>(w) || dynamic_cast<wxToolBar*>(w)) {
      set_native_theme(w, L"DarkMode_Explorer");
    }
  #endif
}

void apply(wxWindow* window) {
  if (!window) return;
  apply_one(window);
  const wxWindowList& kids = window->GetChildren();
  for (wxWindowList::const_iterator it = kids.begin(); it != kids.end(); ++it) {
    apply(*it);
  }
  window->Refresh();
}

// ----------------------------------------------------------------------------- : Drawing

static wxColour with_alpha(const wxColour& c, double a) {
  return wxColour(c.Red(), c.Green(), c.Blue(), (unsigned char)std::max(0.0, std::min(255.0, a * 255)));
}

void draw_pill(wxGraphicsContext& gc, const wxRect& r, double hover, bool active, bool pressed) {
  double radius = r.height / 2.0;
  if (active) {
    gc.SetPen(wxPen(with_alpha(wxColour(0xff, 0xf0, 0xc0), 0.45)));
    gc.SetBrush(gc.CreateLinearGradientBrush(r.x, r.y, r.x, r.GetBottom(), wxColour(0xf0, 0xc8, 0x60), wxColour(0xc8, 0x92, 0x26)));
  } else {
    double a = pressed ? 0.16 : 0.03 + 0.08 * hover;
    gc.SetPen(hover > 0.01 || pressed ? wxPen(with_alpha(wxColour(0xf6, 0xd6, 0x7c), 0.15 + 0.35 * hover)) : *wxTRANSPARENT_PEN);
    gc.SetBrush(wxBrush(with_alpha(*wxWHITE, a)));
  }
  gc.DrawRoundedRectangle(r.x + 0.5, r.y + 0.5, r.width - 1, r.height - 1, radius);
}

void draw_gold_rule(wxGraphicsContext& gc, double x, double y, double width) {
  wxGraphicsGradientStops line(with_alpha(accent(), 0), with_alpha(accent(), 0));
  line.Add(with_alpha(accent(), 0.55), 0.5);
  gc.SetPen(*wxTRANSPARENT_PEN);
  gc.SetBrush(gc.CreateLinearGradientBrush(x, 0, x + width, 0, line));
  gc.DrawRectangle(x, y, width, 1);
}

long fullscreen_style() {
  return wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION | wxFULLSCREEN_NOMENUBAR;
}

// ----------------------------------------------------------------------------- : Dark menu bar

#ifdef __WXMSW__
// Windows has no supported way to color a menu bar. These undocumented messages (used by Windows'
// own dark apps, and by Notepad++ and others) let the window paint the bar and its items itself.
namespace {
  const UINT WM_UAHDRAWMENU     = 0x0091;
  const UINT WM_UAHDRAWMENUITEM = 0x0092;
  struct UAHMENU { HMENU hmenu; HDC hdc; DWORD dwFlags; };
  union  UAHMENUITEMMETRICS { struct { DWORD cx, cy; } rgsizeBar[2]; struct { DWORD cx, cy; } rgsizePopup[4]; };
  struct UAHMENUPOPUPMETRICS { DWORD rgcx[4]; DWORD fUpdateMaxWidths : 2; };
  struct UAHMENUITEM { int iPosition; UAHMENUITEMMETRICS umim; UAHMENUPOPUPMETRICS umpm; };
  struct UAHDRAWMENUITEM { DRAWITEMSTRUCT dis; UAHMENU um; UAHMENUITEM umi; };

  COLORREF ref(const wxColour& c) { return (COLORREF)(c.Red() | (c.Green() << 8) | (c.Blue() << 16)); }

  void fill(HDC dc, const RECT& r, const wxColour& c) {
    HBRUSH b = CreateSolidBrush(ref(c));
    FillRect(dc, &r, b);
    DeleteObject(b);
  }

  // The menu bar's rectangle in window coordinates
  bool menu_bar_rect(HWND h, RECT& out) {
    MENUBARINFO mbi = {};
    mbi.cbSize = sizeof(mbi);
    if (!GetMenuBarInfo(h, OBJID_MENU, 0, &mbi)) return false;
    RECT wr;
    GetWindowRect(h, &wr);
    out = mbi.rcBar;
    OffsetRect(&out, -wr.left, -wr.top);
    return true;
  }

  HFONT menu_font() {
    static HFONT f = nullptr;
    if (!f) {
      NONCLIENTMETRICSW m = {};
      m.cbSize = sizeof(m);
      SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(m), &m, 0);
      f = CreateFontIndirectW(&m.lfMenuFont);
    }
    return f;
  }
}

bool menu_bar_message(wxWindow* frame, WXUINT msg, WXWPARAM, WXLPARAM lp, WXLRESULT* result) {
  HWND h = (HWND)frame->GetHWND();
  if (msg == WM_UAHDRAWMENU) {
    UAHMENU* um = (UAHMENU*)lp;
    RECT r;
    if (!menu_bar_rect(h, r)) return false;
    fill(um->hdc, r, header());
    *result = TRUE;
    return true;
  }
  if (msg == WM_UAHDRAWMENUITEM) {
    UAHDRAWMENUITEM* d = (UAHDRAWMENUITEM*)lp;
    wchar_t label[256] = {};
    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = label;
    mii.cch = 255;
    GetMenuItemInfoW(d->um.hmenu, d->umi.iPosition, TRUE, &mii);
    UINT state = d->dis.itemState;
    bool hot  = state & (ODS_HOTLIGHT | ODS_SELECTED);
    bool dim  = state & (ODS_GRAYED | ODS_DISABLED | ODS_INACTIVE);
    fill(d->um.hdc, d->dis.rcItem, hot ? raised() : header());
    HGDIOBJ old = SelectObject(d->um.hdc, menu_font());
    SetBkMode(d->um.hdc, TRANSPARENT);
    SetTextColor(d->um.hdc, ref(dim ? text_dim() : text()));
    UINT flags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
    if (state & ODS_NOACCEL) flags |= DT_HIDEPREFIX;
    DrawTextW(d->um.hdc, label, -1, &d->dis.rcItem, flags);
    SelectObject(d->um.hdc, old);
    *result = TRUE;
    return true;
  }
  return false;
}

void cover_menu_bar_line(wxWindow* frame, WXUINT msg) {
  if (msg != WM_NCPAINT && msg != WM_NCACTIVATE) return;
  HWND h = (HWND)frame->GetHWND();
  RECT bar;
  if (!menu_bar_rect(h, bar)) return;
  RECT line = bar;
  line.top = bar.bottom;
  line.bottom = bar.bottom + 1;
  HDC dc = GetWindowDC(h);
  fill(dc, line, header());
  ReleaseDC(h, dc);
}
#else
bool menu_bar_message(wxWindow*, WXUINT, WXWPARAM, WXLPARAM, WXLRESULT*) { return false; }
void cover_menu_bar_line(wxWindow*, WXUINT) {}
#endif

}
