//+----------------------------------------------------------------------------+
//| Description:  Setsmith - the editor's header band and status strip        |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/header_bar.hpp>
#include <gui/control/themed_toolbar.hpp>
#include <gui/theme.hpp>
#include <gui/icons.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#ifdef __WXMSW__
  #include <windows.h>
#endif

// ----------------------------------------------------------------------------- : HeaderBar

HeaderBar::HeaderBar(wxWindow* parent)
  : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetBackgroundColour(theme::header());
  row_height = FromDIP(56);
  toolbar = new ThemedToolBar(this, wxID_ANY);
  toolbar->SetExtraStyle(wxWS_EX_PROCESS_UI_UPDATES);
  SetMinSize(wxSize(-1, row_height));
}

void HeaderBar::addTab(int id, const String& label, const String& icon, const String& help) {
  tabs.push_back({id, label, icon, help, wxRect()});
  Refresh(false);
}

void HeaderBar::selectTab(int id) {
  selected = id;
  Refresh(false);
}

void HeaderBar::setFullscreen(bool on) {
  fullscreen = on;
  arrange();
  Refresh(false);
}

// ----------------------------------------------------------------------------- : Layout

int HeaderBar::tabsRight() const {
  // Measure with a screen DC; the tabs are laid out the same way in onPaint
  wxClientDC dc(const_cast<HeaderBar*>(this));
  dc.SetFont(theme::font(10, true));
  int x = logoLeft() + logoWidth() + FromDIP(30);
  for (const Tab& t : tabs) x += dc.GetTextExtent(t.label).x + FromDIP(18 + 8 + 30) + FromDIP(6);
  return x;
}

int HeaderBar::logoLeft() const {
  // in full screen the menu button comes first
  return fullscreen ? FromDIP(12 + 34 + 12) : FromDIP(18);
}

int HeaderBar::logoWidth() const {
  wxClientDC dc(const_cast<HeaderBar*>(this));
  dc.SetFont(theme::font(14, true));
  return FromDIP(20 + 10) + dc.GetTextExtent(_("Setsmith")).x;
}

int HeaderBar::buttonsLeft() const {
  int buttons = fullscreen ? 3 : 1;
  return GetClientSize().x - FromDIP(12) - buttons * FromDIP(38);
}

void HeaderBar::arrange() {
  // Realize sends a size event to this window, which would arrange again
  if (arranging) return;
  arranging = true;
  toolbar->Realize();
  arranging = false;
  wxSize tb = toolbar->GetBestSize();
  int left = tabsRight() + FromDIP(16);
  bool rows = left + tb.x > buttonsLeft() - FromDIP(12);
  if (rows) {
    toolbar->SetSize(FromDIP(12), row_height + (FromDIP(44) - tb.y) / 2, tb.x, tb.y);
  } else {
    toolbar->SetSize(left, (row_height - tb.y) / 2, tb.x, tb.y);
  }
  if (rows != two_rows) {
    two_rows = rows;
    SetMinSize(wxSize(-1, row_height + (rows ? FromDIP(44) : 0)));
    if (GetParent()) GetParent()->Layout();
  }
  Refresh(false);
}

void HeaderBar::onSize(wxSizeEvent& ev) {
  // only when the band itself changed width (the toolbar also sends size events here)
  if (GetClientSize().x != last_width) {
    last_width = GetClientSize().x;
    arrange();
  }
  ev.Skip();
}

// ----------------------------------------------------------------------------- : Drawing

void HeaderBar::onPaint(wxPaintEvent&) {
  wxAutoBufferedPaintDC dc(this);
  unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
  if (!gc) return;
  hits.clear();
  wxSize cs = GetClientSize();
  gc->SetPen(*wxTRANSPARENT_PEN);
  gc->SetBrush(wxBrush(theme::header()));
  gc->DrawRectangle(0, 0, cs.x, cs.y);
  theme::draw_gold_rule(*gc, 0, cs.y - 1, cs.x);

  // Menu button (full screen only, where the menu bar is hidden)
  if (fullscreen) {
    int bs = FromDIP(34);
    wxRect r(FromDIP(12), (row_height - bs) / 2, bs, bs);
    theme::draw_pill(*gc, r, hover == BTN_MENU ? 1.0 : 0.0, false, pressed == BTN_MENU);
    int is = FromDIP(18);
    wxImage img = draw_icon(_("menu"), is, hover == BTN_MENU ? theme::text() : wxColour(0xc4, 0xc6, 0xcc));
    if (img.Ok()) gc->DrawBitmap(wxBitmap(img), r.x + (bs - is) / 2, r.y + (bs - is) / 2, is, is);
    hits.push_back({BTN_MENU, r});
  }

  // Logo: the gold Setsmith diamond and the name
  int x = logoLeft();
  {
    double d = FromDIP(10), cx = x + d, cy = row_height / 2.0;
    wxGraphicsPath diamond = gc->CreatePath();
    diamond.MoveToPoint(cx, cy - d);
    diamond.AddLineToPoint(cx + d * 0.8, cy);
    diamond.AddLineToPoint(cx, cy + d);
    diamond.AddLineToPoint(cx - d * 0.8, cy);
    diamond.CloseSubpath();
    gc->SetBrush(gc->CreateLinearGradientBrush(cx, cy - d, cx, cy + d, wxColour(0xf6, 0xd6, 0x7c), wxColour(0xb0, 0x7a, 0x1a)));
    gc->FillPath(diamond);
    gc->SetFont(gc->CreateFont(theme::font(14, true), theme::text()));
    double tw, th;
    gc->GetTextExtent(_("Setsmith"), &tw, &th);
    gc->DrawText(_("Setsmith"), x + 2 * d + FromDIP(10), cy - th / 2);
    x += logoWidth() + FromDIP(30);
  }

  // Tabs as pills: icon and label, gold when selected
  int ph = FromDIP(36), py = (row_height - ph) / 2, icon = FromDIP(18);
  for (Tab& t : tabs) {
    gc->SetFont(gc->CreateFont(theme::font(10, true), theme::text()));
    double tw, th;
    gc->GetTextExtent(t.label, &tw, &th);
    t.rect = wxRect(x, py, (int)tw + icon + FromDIP(8 + 30), ph);
    bool active = t.id == selected;
    theme::draw_pill(*gc, t.rect, hover == t.id ? 1.0 : 0.0, active, pressed == t.id);
    wxColour ink = active ? theme::accent_text() : (hover == t.id ? theme::text() : wxColour(0xc4, 0xc6, 0xcc));
    wxImage img = draw_icon(t.icon, icon, ink);
    if (img.Ok()) gc->DrawBitmap(wxBitmap(img), t.rect.x + FromDIP(15), t.rect.y + (ph - icon) / 2, icon, icon);
    gc->SetFont(gc->CreateFont(theme::font(10, true), ink));
    gc->DrawText(t.label, t.rect.x + FromDIP(15) + icon + FromDIP(8), t.rect.y + (ph - th) / 2);
    hits.push_back({t.id, t.rect});
    x = t.rect.GetRight() + FromDIP(6);
  }

  // Window buttons on the right
  int bs = FromDIP(34), bx = buttonsLeft() + FromDIP(4), by = (row_height - bs) / 2;
  auto button = [&](int id, const String& name) {
    wxRect r(bx, by, bs, bs);
    bool close = id == BTN_CLOSE;
    if (hover == id || pressed == id) {
      if (close) {
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(wxColour(0xc4, 0x2b, 0x1c)));
        gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, r.height / 2.0);
      } else {
        theme::draw_pill(*gc, r, 1.0, false, pressed == id);
      }
    }
    int is = FromDIP(16);
    wxImage img = draw_icon(name, is, hover == id ? theme::text() : wxColour(0xb0, 0xb4, 0xbe));
    if (img.Ok()) gc->DrawBitmap(wxBitmap(img), r.x + (bs - is) / 2, r.y + (bs - is) / 2, is, is);
    hits.push_back({id, r});
    bx += FromDIP(38);
  };
  if (fullscreen) button(BTN_MINIMIZE, _("minimize"));
  button(BTN_FULLSCREEN, fullscreen ? _("fullscreen_exit") : _("fullscreen"));
  if (fullscreen) button(BTN_CLOSE, _("close"));

  // Second row: a faint divider above the toolbar
  if (two_rows) {
    gc->SetBrush(wxBrush(wxColour(255, 255, 255, 12)));
    gc->DrawRectangle(0, row_height, cs.x, 1);
  }
}

// ----------------------------------------------------------------------------- : Mouse

int HeaderBar::hitAt(const wxPoint& p) const {
  for (const Hit& h : hits) if (h.rect.Contains(p)) return h.id;
  return INT_MIN;
}

void HeaderBar::onMotion(wxMouseEvent& ev) {
  int h = hitAt(ev.GetPosition());
  if (h != hover) {
    hover = h;
    String tip;
    for (const Tab& t : tabs) if (t.id == h) tip = t.help;
    if (h == BTN_FULLSCREEN) tip = fullscreen ? _("Leave full screen (F11)") : _("Full screen (F11)");
    if (h == BTN_MENU)       tip = _("Menu (File, Edit, Cards, ...)");
    if (h == BTN_MINIMIZE)   tip = _("Minimize");
    if (h == BTN_CLOSE)      tip = _("Close this set");
    SetToolTip(tip);
    Refresh(false);
  }
}

void HeaderBar::onLeave(wxMouseEvent&) {
  hover = pressed = INT_MIN;
  Refresh(false);
}

void HeaderBar::onDown(wxMouseEvent& ev) {
  pressed = hitAt(ev.GetPosition());
  Refresh(false);
}

void HeaderBar::onUp(wxMouseEvent& ev) {
  int h = hitAt(ev.GetPosition());
  bool click = h != INT_MIN && h == pressed;
  pressed = INT_MIN;
  Refresh(false);
  if (!click) return;
  wxWindow* frame = wxGetTopLevelParent(this);
  if (h == BTN_MENU) {
    showMenu();
  } else if (h == BTN_MINIMIZE) {
    static_cast<wxTopLevelWindow*>(frame)->Iconize();
  } else if (h == BTN_CLOSE) {
    frame->Close();
  } else {
    wxCommandEvent cmd(wxEVT_TOOL, h == BTN_FULLSCREEN ? (int)ID_WINDOW_FULLSCREEN : h);
    cmd.SetEventObject(this);
    frame->GetEventHandler()->ProcessEvent(cmd);
  }
}

// The menu bar's menus as one popup. In full screen the menu bar is hidden but still owned by the frame
// (so its shortcuts keep working); wx won't pop up a menu that belongs to a menu bar, so the popup is
// built from the bar's Windows menus directly and taken apart again without destroying them.
void HeaderBar::showMenu() {
  #ifdef __WXMSW__
    wxFrame* frame = dynamic_cast<wxFrame*>(wxGetTopLevelParent(this));
    if (!frame || !frame->GetMenuBar()) return;
    wxMenuBar* bar = frame->GetMenuBar();
    HMENU popup = CreatePopupMenu();
    for (size_t i = 0; i < bar->GetMenuCount(); ++i) {
      AppendMenuW(popup, MF_POPUP | MF_STRING, (UINT_PTR)(HMENU)bar->GetMenu(i)->GetHMenu(), bar->GetMenuLabel(i).wc_str());
    }
    wxRect r;
    for (const Hit& hit : hits) if (hit.id == BTN_MENU) r = hit.rect;
    wxPoint p = ClientToScreen(wxPoint(r.x, r.GetBottom() + FromDIP(4)));
    TrackPopupMenu(popup, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, (HWND)frame->GetHWND(), nullptr);
    while (GetMenuItemCount(popup) > 0) RemoveMenu(popup, 0, MF_BYPOSITION);
    DestroyMenu(popup);
  #endif
}

// Double clicking the empty band toggles full screen, like double clicking a title bar
void HeaderBar::onDoubleClick(wxMouseEvent& ev) {
  if (hitAt(ev.GetPosition()) != INT_MIN || ev.GetPosition().y > row_height) return;
  wxCommandEvent cmd(wxEVT_TOOL, ID_WINDOW_FULLSCREEN);
  cmd.SetEventObject(this);
  wxGetTopLevelParent(this)->GetEventHandler()->ProcessEvent(cmd);
}

BEGIN_EVENT_TABLE(HeaderBar, wxWindow)
  EVT_PAINT        (HeaderBar::onPaint)
  EVT_SIZE         (HeaderBar::onSize)
  EVT_MOTION       (HeaderBar::onMotion)
  EVT_LEAVE_WINDOW (HeaderBar::onLeave)
  EVT_LEFT_DOWN    (HeaderBar::onDown)
  EVT_LEFT_UP      (HeaderBar::onUp)
  EVT_LEFT_DCLICK  (HeaderBar::onDoubleClick)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : StatusStrip

StatusStrip::StatusStrip(wxWindow* parent)
  : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetMinSize(wxSize(-1, FromDIP(24)));
}

void StatusStrip::setText(const String& t) {
  if (t == text) return;
  text = t;
  Refresh(false);
}

void StatusStrip::onPaint(wxPaintEvent&) {
  wxAutoBufferedPaintDC dc(this);
  wxSize cs = GetClientSize();
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(theme::header());
  dc.DrawRectangle(0, 0, cs.x, cs.y);
  dc.SetPen(wxPen(theme::border()));
  dc.DrawLine(0, 0, cs.x, 0);
  dc.SetFont(theme::font(9));
  dc.SetTextForeground(theme::text_dim());
  wxSize ts = dc.GetTextExtent(text);
  dc.DrawText(text, FromDIP(14), (cs.y - ts.y) / 2 + 1);
}

BEGIN_EVENT_TABLE(StatusStrip, wxWindow)
  EVT_PAINT(StatusStrip::onPaint)
END_EVENT_TABLE()
