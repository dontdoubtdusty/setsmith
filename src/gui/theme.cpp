//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the dark workshop theme                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/theme.hpp>
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
#ifdef __WXMSW__
  #include <windows.h>
  #include <commctrl.h>
  #include <uxtheme.h>
#endif

namespace theme {

// ----------------------------------------------------------------------------- : Palette

// MSE Slim: charcoal with a gold accent (the gold of the Setsmith diamond).
wxColour backdrop()    { return wxColour(0x16, 0x17, 0x1a); }
wxColour panel()       { return wxColour(0x26, 0x28, 0x2d); }
wxColour field()       { return wxColour(0x1d, 0x1f, 0x23); }
wxColour border()      { return wxColour(0x3b, 0x3e, 0x45); }
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
  if (c == wxSYS_COLOUR_3DDKSHADOW)                                 return wxColour(0x0e, 0x0f, 0x11);
  if (c == wxSYS_COLOUR_BTNHIGHLIGHT)                               return wxColour(0x4d, 0x51, 0x59);
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
  w->SetBackgroundColour(input ? field() : panel());
  w->SetForegroundColour(text());
  if (input || dynamic_cast<wxButton*>(w) || dynamic_cast<wxCheckBox*>(w)
      || dynamic_cast<wxStaticText*>(w)) {
    w->SetFont(font());
  }
  #ifdef __WXMSW__
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

}
