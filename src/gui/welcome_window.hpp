//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/about_window.hpp> // for HoverButton
#include <gui/set_library.hpp>
#include <wx/timer.h>
#include <climits>

DECLARE_POINTER_TYPE(Set);
class wxGraphicsContext;

// ----------------------------------------------------------------------------- : WelcomeWindow

/// Setsmith: the dashboard, the first window you see at startup.
/** A grid of set tiles (sets in the watched folders and recent sets), a New set tile, game filters,
 *  Open and Folders buttons, and a Continue button for the last opened set. Everything is drawn by
 *  the window itself so tiles can animate on hover.
 */
class WelcomeWindow : public wxFrame {
public:
  WelcomeWindow();

private:
  DECLARE_EVENT_TABLE();

  // One clickable thing on the page and where it was last drawn
  struct Hit { int id; wxRect rect; };
  // Ids of the fixed buttons; tiles use 0 (New set) and 1.. (set index + 1); filter pills use HIT_FILTER - n
  enum { HIT_OPEN = -1, HIT_FOLDERS = -2, HIT_CONTINUE = -3, HIT_FILTER = -100 };

  Bitmap logo;
  vector<SetSummary> sets;      ///< every set found, newest first
  vector<size_t> shown;         ///< indices into sets that pass the game filter
  vector<String> games;         ///< games present, for the filter pills
  String game_filter;           ///< empty for all games
  vector<Hit> hits;             ///< rebuilt on every paint
  map<int, double> glow;        ///< hover animation per hit id, 0 to 1
  map<String, double> fade_in;  ///< picture fade-in per set path, 0 to 1
  map<String, wxBitmap> tile_bitmaps; ///< pictures cut to tile shape, per set path
  int hover = INT_MIN;          ///< hit id under the mouse
  double scroll = 0, scroll_target = 0, content_height = 0;
  wxTimer timer;

  void rescan();
  void refilter();
  int hitAt(const wxPoint& pos) const;
  int columns() const;
  wxRect tileRect(size_t slot) const;

  void onPaint(wxPaintEvent&);
  void draw(wxGraphicsContext& gc);
  void drawTile(wxGraphicsContext& gc, const SetSummary& s, const wxRect& r, double h);
  void drawNewTile(wxGraphicsContext& gc, const wxRect& r, double h);
  void drawButton(wxGraphicsContext& gc, int id, const wxRect& r, const String& label, bool primary, const String& sub = String());
  wxBitmap tileBitmap(const SetSummary& s, const wxSize& size);

  void onMouseMove (wxMouseEvent&);
  void onMouseLeave(wxMouseEvent&);
  void onLeftUp    (wxMouseEvent&);
  void onRightUp   (wxMouseEvent&);
  void onWheel     (wxMouseEvent&);
  void onSize      (wxSizeEvent&);
  void onTimer     (wxTimerEvent&);
  void onActivate  (wxActivateEvent&);
  void onIdle      (wxIdleEvent& ev);
  void onKey       (wxCommandEvent& ev); ///< Ctrl+O and Ctrl+N
  bool step();      ///< advances the animations one frame; true while anything still moves
  void startAnimation();

  void openSet(const String& path);
  void onOpenSet();
  void onNewSet();
  void showFoldersMenu(const wxRect& below);
  void showTileMenu(size_t set_index);

  /// Close the welcome window, and show the given set
  void close(const SetP& set);
};

// ----------------------------------------------------------------------------- : Hover button with label

/// An extended hover button, not only has base images, but also has two labels
class HoverButtonExt : public HoverButton {
public:
  HoverButtonExt(Window* parent, int id, const wxImage& icon_name, const String& label, const String& sub_label);

private:
  Bitmap icon;
  String label, sub_label;
  wxFont font_large, font_small;

protected:
  void draw(DC& dc) override;
};
