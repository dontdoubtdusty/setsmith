//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the editor's header band and status strip        |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <util/prec.hpp>
#include <climits>

class ThemedToolBar;
class wxGraphicsContext;

// ----------------------------------------------------------------------------- : HeaderBar

/// Setsmith: the band across the top of the set window, in the dashboard's style (Stage 5, 2026-11-22).
/** Logo, the panel tabs as pills, the toolbar, and window buttons (full screen, and minimize and
 *  close while full screen, when there is no title bar). The toolbar moves to a second row when
 *  the window is too narrow for one.
 *  Tab clicks are sent to the parent as tool events with the tab's id; the full screen button sends
 *  ID_WINDOW_FULLSCREEN.
 */
class HeaderBar : public wxWindow {
public:
  HeaderBar(wxWindow* parent);

  ThemedToolBar* tools() const { return toolbar; }

  void addTab(int id, const String& label, const String& icon, const String& help);
  void selectTab(int id);
  void setFullscreen(bool on);
  /// Place the toolbar again; call after tools were added or removed
  void arrange();

private:
  DECLARE_EVENT_TABLE();

  struct Tab { int id; String label, icon, help; wxRect rect; };
  enum { BTN_FULLSCREEN = -1, BTN_MINIMIZE = -2, BTN_CLOSE = -3, BTN_MENU = -4 };
  struct Hit { int id; wxRect rect; };

  ThemedToolBar* toolbar;
  vector<Tab> tabs;
  vector<Hit> hits;
  int selected = INT_MIN, hover = INT_MIN, pressed = INT_MIN;
  bool fullscreen = false;
  bool two_rows = false;
  bool arranging = false;
  int last_width = -1;
  int row_height;

  int logoLeft() const;      ///< x where the logo starts
  int logoWidth() const;     ///< width of the diamond and the name
  void showMenu();           ///< the menu bar's menus as a popup (full screen)
  int tabsRight() const;     ///< x where the tabs end
  int buttonsLeft() const;   ///< x where the window buttons start
  int hitAt(const wxPoint& p) const;

  void onPaint(wxPaintEvent&);
  void onSize(wxSizeEvent&);
  void onMotion(wxMouseEvent&);
  void onLeave(wxMouseEvent&);
  void onDown(wxMouseEvent&);
  void onUp(wxMouseEvent&);
  void onDoubleClick(wxMouseEvent&);
};

// ----------------------------------------------------------------------------- : StatusStrip

/// Setsmith: a dark status line at the bottom of the set window (the native status bar can't be colored).
class StatusStrip : public wxWindow {
public:
  StatusStrip(wxWindow* parent);
  void setText(const String& text);

private:
  DECLARE_EVENT_TABLE();
  String text;
  void onPaint(wxPaintEvent&);
};
