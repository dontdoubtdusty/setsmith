//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/welcome_window.hpp>
#include <gui/util.hpp>
#include <gui/theme.hpp>
#include <gui/new_window.hpp>
#include <gui/set/window.hpp>
#include <util/window_id.hpp>
#include <data/settings.hpp>
#include <data/format/formats.hpp>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/filename.h>
#include <wx/dirdlg.h>
#include <wx/choicdlg.h>
#include <wx/menu.h>
#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------------- : Look

// Setsmith: the dashboard look (2026-11-22): a dark arena with a warm glow, set tiles like deck boxes,
// a gold Continue button. Colors that only the dashboard uses live here; the shared ones come from theme.
namespace {
  const wxColour gold      (0xe0, 0xb0, 0x3a);
  const wxColour gold_light(0xf6, 0xd6, 0x7c);
  const wxColour gold_dark (0xb0, 0x7a, 0x1a);
  const wxColour ink       (0xec, 0xe8, 0xdf);
  const wxColour ink_dim   (0x8c, 0x92, 0xa2);

  wxColour with_alpha(const wxColour& c, double a) {
    return wxColour(c.Red(), c.Green(), c.Blue(), (unsigned char)std::max(0.0, std::min(255.0, a * 255)));
  }

  // The color of a game's tag on a tile
  wxColour game_colour(const String& game) {
    if (game == _("magic")) return wxColour(0xe8, 0xa3, 0x3d);
    if (game == _("uvs"))   return wxColour(0x3c, 0xc0, 0xb0);
    return wxColour(0x7a, 0x9c, 0xff);
  }

  String game_label(const String& game) {
    if (game == _("magic")) return _("MAGIC");
    if (game == _("uvs"))   return _("UNIVERSUS");
    return game.Upper();
  }

  // "today", "yesterday", "5 days ago", else the date
  String when(const wxDateTime& t) {
    if (!t.IsValid()) return String();
    int days = (wxDateTime::Today() - wxDateTime(t).ResetTime()).GetDays();
    if (days <= 0) return _("Edited today");
    if (days == 1) return _("Edited yesterday");
    if (days < 14) return wxString::Format(_("Edited %d days ago"), days);
    return _("Edited ") + t.Format(_("%b %d, %Y"));
  }

  // Shortens text with an ellipsis until it fits the width
  String fit(wxGraphicsContext& gc, String text, double max_width) {
    double w, h;
    gc.GetTextExtent(text, &w, &h);
    if (w <= max_width) return text;
    while (!text.empty()) {
      text.RemoveLast();
      gc.GetTextExtent(text + _("..."), &w, &h);
      if (w <= max_width) return text.Trim() + _("...");
    }
    return text;
  }

  void set_font(wxGraphicsContext& gc, int size, bool bold, const wxColour& c) {
    gc.SetFont(gc.CreateFont(theme::font(size, bold), c));
  }

  // Ease toward a target: true while still moving
  bool approach(double& value, double target, double rate) {
    double d = target - value;
    if (std::fabs(d) < 0.01) { value = target; return false; }
    value += d * rate;
    return true;
  }
}

// ----------------------------------------------------------------------------- : WelcomeWindow

WelcomeWindow::WelcomeWindow()
  : wxFrame(nullptr, wxID_ANY, _TITLE_("magic set editor"), wxDefaultPosition, wxSize(1180, 760), wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
  , logo(load_resource_image(_("about")))
  , timer(this)
{
  SetIcon(load_resource_icon(_("app")));
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetClientSize(FromDIP(wxSize(1180, 760)));
  SetMinClientSize(FromDIP(wxSize(760, 520)));
  CentreOnScreen();
  wxAcceleratorEntry keys[2];
  keys[0].Set(wxACCEL_CTRL, (int)'O', ID_FILE_OPEN);
  keys[1].Set(wxACCEL_CTRL, (int)'N', ID_FILE_NEW);
  SetAcceleratorTable(wxAcceleratorTable(2, keys));
  rescan();
}

// ----------------------------------------------------------------------------- : Data

void WelcomeWindow::rescan() {
  // Keep what is already known about a set that has not changed (its picture above all).
  map<String, SetSummary> old;
  for (SetSummary& s : sets) old[s.path] = std::move(s);
  sets.clear();
  for (const String& path : find_library_sets()) {
    auto it = old.find(path);
    wxDateTime modified = wxFileName(path).GetModificationTime();
    if (it != old.end() && it->second.modified.IsValid() && modified.IsValid() && it->second.modified == modified) {
      sets.push_back(std::move(it->second));
      continue;
    }
    SetSummary s;
    if (read_set_summary(path, s)) {
      tile_bitmaps.erase(path);
      sets.push_back(std::move(s));
    }
  }
  std::stable_sort(sets.begin(), sets.end(), [](const SetSummary& a, const SetSummary& b) {
    if (!a.modified.IsValid() || !b.modified.IsValid()) return a.modified.IsValid();
    return a.modified.IsLaterThan(b.modified);
  });
  games.clear();
  for (const SetSummary& s : sets) {
    if (!s.game.empty() && std::find(games.begin(), games.end(), s.game) == games.end()) games.push_back(s.game);
  }
  if (!game_filter.empty() && std::find(games.begin(), games.end(), game_filter) == games.end()) game_filter.clear();
  refilter();
}

void WelcomeWindow::refilter() {
  shown.clear();
  for (size_t i = 0; i < sets.size(); ++i) {
    if (game_filter.empty() || sets[i].game == game_filter) shown.push_back(i);
  }
  Refresh(false);
}

// ----------------------------------------------------------------------------- : Layout

int WelcomeWindow::columns() const {
  int width = GetClientSize().x - 2 * FromDIP(48);
  int step  = FromDIP(196 + 22);
  return std::max(1, (width + FromDIP(22)) / step);
}

// Where tile number `slot` sits (slot 0 is the New set tile), in page coordinates before scrolling.
wxRect WelcomeWindow::tileRect(size_t slot) const {
  int cols = columns();
  int w = FromDIP(196), h = FromDIP(276), gap = FromDIP(22);
  int x = FromDIP(48) + (int)(slot % cols) * (w + gap);
  int y = FromDIP(84 + 128) + (int)(slot / cols) * (h + gap);
  return wxRect(x, y, w, h);
}

int WelcomeWindow::hitAt(const wxPoint& pos) const {
  for (auto it = hits.rbegin(); it != hits.rend(); ++it) {
    if (it->rect.Contains(pos)) return it->id;
  }
  return INT_MIN;
}

// ----------------------------------------------------------------------------- : Drawing

void WelcomeWindow::onPaint(wxPaintEvent&) {
  wxBufferedPaintDC dc(this);
  std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
  if (gc) draw(*gc);
}

void WelcomeWindow::draw(wxGraphicsContext& gc) {
  hits.clear();
  wxSize ws = GetClientSize();
  double W = ws.x, H = ws.y;
  int header = FromDIP(84), margin = FromDIP(48);

  // Backdrop: deep blue-black with a warm glow at the top and a cool one at the left
  gc.SetPen(*wxTRANSPARENT_PEN);
  gc.SetBrush(gc.CreateLinearGradientBrush(0, 0, 0, H, wxColour(0x0b, 0x0d, 0x14), wxColour(0x16, 0x1b, 0x27)));
  gc.DrawRectangle(0, 0, W, H);
  gc.SetBrush(gc.CreateRadialGradientBrush(W * 0.55, -H * 0.1, W * 0.55, -H * 0.1, W * 0.6, with_alpha(gold, 0.16), with_alpha(gold, 0)));
  gc.DrawRectangle(0, 0, W, H);
  gc.SetBrush(gc.CreateRadialGradientBrush(0, H * 0.7, 0, H * 0.7, W * 0.45, wxColour(0x3c, 0x5a, 0xa0, 40), wxColour(0x3c, 0x5a, 0xa0, 0)));
  gc.DrawRectangle(0, 0, W, H);

  // Scrolling content: heading, game filters, tiles
  gc.Clip(0, header, W, H - header);
  double top = -scroll;
  set_font(gc, 20, true, ink);
  gc.DrawText(_("Your sets"), margin, top + header + FromDIP(28));
  double tw, th;
  gc.GetTextExtent(_("Your sets"), &tw, &th);
  set_font(gc, 11, false, ink_dim);
  gc.DrawText(wxString::Format(_("%d"), (int)shown.size()), margin + tw + FromDIP(12), top + header + FromDIP(36));

  // Game filter pills, only when there is more than one game
  if (games.size() > 1) {
    double x = margin, y = top + header + FromDIP(76);
    for (int i = -1; i < (int)games.size(); ++i) {
      String label = i < 0 ? String(_("All")) : game_label(games[i]);
      bool active = i < 0 ? game_filter.empty() : game_filter == games[i];
      int id = HIT_FILTER - (i + 1);
      double g = glow[id];
      set_font(gc, 9, true, active ? wxColour(0x17, 0x15, 0x0e) : with_alpha(ink, 0.65 + 0.35 * g));
      double lw, lh;
      gc.GetTextExtent(label, &lw, &lh);
      wxRect r((int)x, (int)y, (int)lw + FromDIP(28), FromDIP(28));
      gc.SetPen(active ? *wxTRANSPARENT_PEN : wxPen(with_alpha(*wxWHITE, 0.15 + 0.2 * g)));
      gc.SetBrush(active ? wxBrush(gold) : wxBrush(with_alpha(*wxWHITE, 0.04 + 0.06 * g)));
      gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, r.height / 2.0);
      gc.DrawText(label, r.x + FromDIP(14), r.y + (r.height - lh) / 2);
      hits.push_back({id, r});
      x += r.width + FromDIP(8);
    }
  }

  // Tiles: New set first, then the sets
  wxRect view(0, header, ws.x, ws.y - header);
  for (size_t slot = 0; slot <= shown.size(); ++slot) {
    wxRect r = tileRect(slot);
    r.y -= (int)scroll;
    int id = (int)slot; // 0 is New set, else shown index + 1
    double h = glow[id];
    wxRect lifted = r;
    lifted.y -= (int)std::round(FromDIP(6) * h);
    if (r.Intersects(view)) {
      if (slot == 0) drawNewTile(gc, lifted, h);
      else           drawTile(gc, sets[shown[slot - 1]], lifted, h);
      hits.push_back({id, r.Intersect(view)});
    }
  }
  wxRect last = tileRect(shown.size());
  content_height = last.GetBottom() + FromDIP(120);
  if (sets.empty()) {
    set_font(gc, 11, false, ink_dim);
    wxRect r = tileRect(0);
    gc.DrawText(_("No sets yet. Make one, or use Folders to watch a folder that has some."), r.GetRight() + FromDIP(28), r.y - scroll + r.height / 2 - FromDIP(8));
  }
  gc.ResetClip();

  // Scrollbar, only when the page is taller than the window
  double view_h = H - header;
  double total = content_height - header;
  if (total > view_h) {
    double bar_h = std::max<double>(FromDIP(40), view_h * view_h / total);
    double bar_y = header + (view_h - bar_h) * (scroll / (total - view_h));
    gc.SetPen(*wxTRANSPARENT_PEN);
    gc.SetBrush(wxBrush(with_alpha(*wxWHITE, 0.18)));
    gc.DrawRoundedRectangle(W - FromDIP(10), bar_y + FromDIP(4), FromDIP(5), bar_h - FromDIP(8), FromDIP(5) / 2.0);
  }

  // Header band, drawn over the content so tiles slide under it
  gc.SetPen(*wxTRANSPARENT_PEN);
  gc.SetBrush(gc.CreateLinearGradientBrush(0, 0, 0, header, wxColour(0x08, 0x0a, 0x10, 245), wxColour(0x0d, 0x10, 0x18, 235)));
  gc.DrawRectangle(0, 0, W, header);
  wxGraphicsGradientStops line(with_alpha(gold, 0), with_alpha(gold, 0));
  line.Add(with_alpha(gold, 0.55), 0.5);
  gc.SetBrush(gc.CreateLinearGradientBrush(0, 0, W, 0, line));
  gc.DrawRectangle(0, header - 1, W, 1);
  if (logo.Ok()) {
    double lh = FromDIP(38), lw = logo.GetWidth() * lh / logo.GetHeight();
    gc.DrawBitmap(logo, margin - FromDIP(6), (header - lh) / 2, lw, lh);
  }
  int bw = FromDIP(112), bh = FromDIP(36);
  wxRect open_r(ws.x - margin - bw, (header - bh) / 2, bw, bh);
  wxRect fold_r(open_r.x - bw - FromDIP(10), open_r.y, bw, bh);
  drawButton(gc, HIT_FOLDERS, fold_r, _("Folders"), false);
  drawButton(gc, HIT_OPEN, open_r, _("Open..."), false);

  // Continue: the last opened set, bottom right, like a big play button
  if (!settings.recent_sets.empty() && (wxFileName::FileExists(settings.recent_sets.front()) || wxFileName::DirExists(settings.recent_sets.front()))) {
    String name = wxFileName(settings.recent_sets.front()).GetName();
    wxFileName front(settings.recent_sets.front());
    front.Normalize(wxPATH_NORM_ABSOLUTE | wxPATH_NORM_DOTS);
    for (const SetSummary& s : sets) if (s.path.CmpNoCase(front.GetFullPath()) == 0) name = s.title;
    wxRect r(ws.x - margin - FromDIP(250), ws.y - FromDIP(36) - FromDIP(70), FromDIP(250), FromDIP(70));
    drawButton(gc, HIT_CONTINUE, r, _("CONTINUE"), true, name);
  }
}

// A set tile: its picture, a dark fade at the bottom, the title, game, card count and date.
void WelcomeWindow::drawTile(wxGraphicsContext& gc, const SetSummary& s, const wxRect& r, double h) {
  double radius = FromDIP(10);
  // Soft drop shadow, deeper when lifted
  gc.SetPen(*wxTRANSPARENT_PEN);
  for (int i = 1; i <= 6; ++i) {
    gc.SetBrush(wxBrush(wxColour(0, 0, 0, (unsigned char)((26 - i * 4) * (1 + h)))));
    gc.DrawRoundedRectangle(r.x - i, r.y - i + FromDIP(4) + 4 * h, r.width + 2 * i, r.height + 2 * i, radius + i);
  }
  // Gold glow on hover
  if (h > 0.01) {
    for (int i = 1; i <= 8; ++i) {
      gc.SetBrush(wxBrush(with_alpha(gold, 0.07 * h * (1 - i / 9.0))));
      gc.DrawRoundedRectangle(r.x - i * 1.5, r.y - i * 1.5, r.width + 3 * i, r.height + 3 * i, radius + i * 1.5);
    }
  }
  // Placeholder: a gradient in the game's color with the set code, shown until (or instead of) the picture
  wxColour gc_col = game_colour(s.game);
  gc.SetBrush(gc.CreateLinearGradientBrush(r.x, r.y, r.x, r.GetBottom(), wxColour(gc_col.Red() / 4, gc_col.Green() / 4, gc_col.Blue() / 4), wxColour(0x10, 0x12, 0x1a)));
  gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
  auto fade = fade_in.find(s.path);
  wxBitmap bmp = s.image.IsOk() ? tileBitmap(s, r.GetSize()) : wxBitmap();
  if (!bmp.IsOk()) {
    String code = s.code.empty() ? s.title.Left(3).Upper() : s.code;
    set_font(gc, 30, true, with_alpha(gc_col, 0.45));
    double cw, ch;
    gc.GetTextExtent(code, &cw, &ch);
    gc.DrawText(code, r.x + (r.width - cw) / 2, r.y + r.height * 0.3 - ch / 2);
  } else {
    double a = fade == fade_in.end() ? 1.0 : fade->second;
    if (a < 0.99) gc.BeginLayer(a);
    gc.DrawBitmap(bmp, r.x, r.y, r.width, r.height);
    if (a < 0.99) gc.EndLayer();
  }
  // Dark fade under the text
  gc.SetBrush(gc.CreateLinearGradientBrush(r.x, r.y + r.height * 0.40, r.x, r.GetBottom(), wxColour(8, 10, 16, 0), wxColour(8, 10, 16, 240)));
  gc.DrawRoundedRectangle(r.x, r.y + r.height * 0.40, r.width, r.height * 0.60, radius);
  // Brighten on hover
  if (h > 0.01) {
    gc.SetBrush(wxBrush(with_alpha(*wxWHITE, 0.05 * h)));
    gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
  }
  // Rim: faint white, turning gold on hover
  gc.SetBrush(*wxTRANSPARENT_BRUSH);
  gc.SetPen(gc.CreatePen(wxGraphicsPenInfo(h > 0.01 ? with_alpha(gold_light, 0.25 + 0.75 * h) : with_alpha(*wxWHITE, 0.12)).Width(1 + 1.5 * h)));
  gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);

  // Set code badge, top left
  double pad = FromDIP(12);
  if (!s.code.empty()) {
    set_font(gc, 8, true, gold_light);
    double cw, ch;
    gc.GetTextExtent(s.code, &cw, &ch);
    gc.SetPen(*wxTRANSPARENT_PEN);
    gc.SetBrush(wxBrush(wxColour(0, 0, 0, 160)));
    gc.DrawRoundedRectangle(r.x + pad, r.y + pad, cw + FromDIP(14), ch + FromDIP(6), (ch + FromDIP(6)) / 2);
    gc.DrawText(s.code, r.x + pad + FromDIP(7), r.y + pad + FromDIP(3));
  }
  // Title, game and count, date
  double y = r.GetBottom() - pad;
  set_font(gc, 8, false, ink_dim);
  double dw, dh;
  gc.GetTextExtent(_("Ag"), &dw, &dh);
  y -= dh;
  gc.DrawText(when(s.modified), r.x + pad, y);
  set_font(gc, 8, true, game_colour(s.game));
  String tag = game_label(s.game);
  double gw, gh;
  gc.GetTextExtent(tag, &gw, &gh);
  y -= gh + FromDIP(4);
  gc.DrawText(tag, r.x + pad, y);
  set_font(gc, 8, false, ink_dim);
  gc.DrawText(wxString::Format(_("  %d cards"), (int)s.card_names.size()), r.x + pad + gw, y);
  set_font(gc, 12, true, ink);
  String title = fit(gc, s.title, r.width - 2 * pad);
  double tw, th;
  gc.GetTextExtent(title, &tw, &th);
  y -= th + FromDIP(2);
  gc.DrawText(title, r.x + pad, y);
}

// The New set tile: a dashed gold outline with a plus.
void WelcomeWindow::drawNewTile(wxGraphicsContext& gc, const wxRect& r, double h) {
  double radius = FromDIP(10);
  gc.SetBrush(wxBrush(with_alpha(*wxWHITE, 0.03 + 0.05 * h)));
  gc.SetPen(gc.CreatePen(wxGraphicsPenInfo(with_alpha(gold, 0.45 + 0.5 * h)).Width(1.5 + h).Style(wxPENSTYLE_SHORT_DASH)));
  gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
  double cx = r.x + r.width / 2.0, cy = r.y + r.height * 0.42, arm = FromDIP(18) + FromDIP(3) * h;
  gc.SetPen(gc.CreatePen(wxGraphicsPenInfo(with_alpha(gold_light, 0.7 + 0.3 * h)).Width(FromDIP(3)).Cap(wxCAP_ROUND)));
  gc.StrokeLine(cx - arm, cy, cx + arm, cy);
  gc.StrokeLine(cx, cy - arm, cx, cy + arm);
  set_font(gc, 12, true, ink);
  double w, hh;
  gc.GetTextExtent(_("New set"), &w, &hh);
  gc.DrawText(_("New set"), cx - w / 2, cy + FromDIP(40));
  set_font(gc, 8, false, ink_dim);
  gc.GetTextExtent(_("Ctrl+N"), &w, &hh);
  gc.DrawText(_("Ctrl+N"), cx - w / 2, cy + FromDIP(64));
}

// A header button (ghost) or the gold Continue button (primary, with a second line).
void WelcomeWindow::drawButton(wxGraphicsContext& gc, int id, const wxRect& r, const String& label, bool primary, const String& sub) {
  double h = glow[id];
  double radius = primary ? FromDIP(8) : r.height / 2.0;
  gc.SetPen(*wxTRANSPARENT_PEN);
  if (primary) {
    for (int i = 1; i <= 10; ++i) {
      gc.SetBrush(wxBrush(with_alpha(gold, (0.05 + 0.06 * h) * (1 - i / 11.0))));
      gc.DrawRoundedRectangle(r.x - i * 2, r.y - i * 2, r.width + 4 * i, r.height + 4 * i, radius + i * 2);
    }
    wxColour top = gold_light, bottom = gold_dark;
    if (h > 0.01) {
      top    = wxColour(std::min(255, top.Red() + (int)(9 * h)), std::min(255, top.Green() + (int)(20 * h)), std::min(255, top.Blue() + (int)(40 * h)));
      bottom = wxColour(std::min(255, bottom.Red() + (int)(30 * h)), std::min(255, bottom.Green() + (int)(30 * h)), bottom.Blue());
    }
    gc.SetBrush(gc.CreateLinearGradientBrush(r.x, r.y, r.x, r.GetBottom(), top, bottom));
    gc.SetPen(wxPen(with_alpha(wxColour(0xff, 0xf0, 0xc0), 0.6)));
    gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
    set_font(gc, 15, true, wxColour(0x1c, 0x14, 0x05));
    double w, hh;
    gc.GetTextExtent(label, &w, &hh);
    double y = r.y + FromDIP(12);
    gc.DrawText(label, r.x + (r.width - w) / 2, y);
    set_font(gc, 8, false, wxColour(0x3a, 0x2a, 0x0c));
    String s = fit(gc, sub, r.width - FromDIP(28));
    double sw, sh;
    gc.GetTextExtent(s, &sw, &sh);
    gc.DrawText(s, r.x + (r.width - sw) / 2, y + hh + FromDIP(2));
  } else {
    gc.SetBrush(wxBrush(with_alpha(*wxWHITE, 0.05 + 0.08 * h)));
    gc.SetPen(wxPen(with_alpha(h > 0.01 ? gold_light : *wxWHITE, 0.18 + 0.5 * h)));
    gc.DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
    set_font(gc, 10, true, ink);
    double w, hh;
    gc.GetTextExtent(label, &w, &hh);
    gc.DrawText(label, r.x + (r.width - w) / 2, r.y + (r.height - hh) / 2);
  }
  hits.push_back({id, r});
}

// The set's picture cropped to fill the tile, with rounded corners cut into its alpha channel. Cached per set.
wxBitmap WelcomeWindow::tileBitmap(const SetSummary& s, const wxSize& size) {
  auto it = tile_bitmaps.find(s.path);
  if (it != tile_bitmaps.end() && it->second.GetSize() == size) return it->second;
  const wxImage& src = s.image;
  double scale = std::max((double)size.x / src.GetWidth(), (double)size.y / src.GetHeight());
  int cw = std::min(src.GetWidth(),  (int)std::ceil(size.x / scale));
  int ch = std::min(src.GetHeight(), (int)std::ceil(size.y / scale));
  wxImage img = src.GetSubImage(wxRect((src.GetWidth() - cw) / 2, (src.GetHeight() - ch) / 3, cw, ch))
                   .Scale(size.x, size.y, wxIMAGE_QUALITY_HIGH);
  if (!img.HasAlpha()) img.InitAlpha();
  double rad = FromDIP(10);
  for (int y = 0; y < size.y; ++y) {
    for (int x = 0; x < size.x; ++x) {
      double dx = x + 0.5 < rad ? rad - (x + 0.5) : (x + 0.5 > size.x - rad ? (x + 0.5) - (size.x - rad) : 0);
      double dy = y + 0.5 < rad ? rad - (y + 0.5) : (y + 0.5 > size.y - rad ? (y + 0.5) - (size.y - rad) : 0);
      if (dx == 0 || dy == 0) continue;
      double cover = std::max(0.0, std::min(1.0, rad + 0.5 - std::sqrt(dx * dx + dy * dy)));
      img.SetAlpha(x, y, (unsigned char)(img.GetAlpha(x, y) * cover));
    }
  }
  wxBitmap bmp(img);
  tile_bitmaps[s.path] = bmp;
  return bmp;
}

// ----------------------------------------------------------------------------- : Animation

bool WelcomeWindow::step() {
  bool moving = false;
  for (auto& g : glow) {
    if (approach(g.second, g.first == hover ? 1.0 : 0.0, 0.28)) moving = true;
  }
  if (approach(scroll, scroll_target, 0.3)) moving = true;
  for (auto& f : fade_in) {
    if (f.second < 1) { f.second = std::min(1.0, f.second + 0.1); moving = true; }
  }
  return moving;
}

void WelcomeWindow::startAnimation() {
  if (!timer.IsRunning()) timer.Start(15);
}

void WelcomeWindow::onTimer(wxTimerEvent&) {
  if (!step()) timer.Stop();
  Refresh(false);
}

// ----------------------------------------------------------------------------- : Events

void WelcomeWindow::onMouseMove(wxMouseEvent& ev) {
  int id = hitAt(ev.GetPosition());
  if (id == hover) return;
  hover = id;
  if (id != INT_MIN) glow[id]; // make sure it animates in
  SetCursor(id == INT_MIN ? wxNullCursor : wxCursor(wxCURSOR_HAND));
  startAnimation();
}

void WelcomeWindow::onMouseLeave(wxMouseEvent&) {
  hover = INT_MIN;
  SetCursor(wxNullCursor);
  startAnimation();
}

void WelcomeWindow::onWheel(wxMouseEvent& ev) {
  double view_h = GetClientSize().y - FromDIP(84);
  double max_scroll = std::max(0.0, content_height - FromDIP(84) - view_h);
  scroll_target -= ev.GetWheelRotation() / (double)ev.GetWheelDelta() * FromDIP(120);
  scroll_target = std::max(0.0, std::min(max_scroll, scroll_target));
  startAnimation();
}

void WelcomeWindow::onSize(wxSizeEvent& ev) {
  double view_h = GetClientSize().y - FromDIP(84);
  double max_scroll = std::max(0.0, content_height - FromDIP(84) - view_h);
  scroll_target = std::min(scroll_target, max_scroll);
  scroll = std::min(scroll, max_scroll);
  Refresh(false);
  ev.Skip();
}

void WelcomeWindow::onLeftUp(wxMouseEvent& ev) {
  int id = hitAt(ev.GetPosition());
  if (id == INT_MIN) return;
  if (id == HIT_OPEN)          onOpenSet();
  else if (id == HIT_FOLDERS)  { for (const Hit& h : hits) if (h.id == HIT_FOLDERS) showFoldersMenu(h.rect); }
  else if (id == HIT_CONTINUE) openSet(settings.recent_sets.front());
  else if (id <= HIT_FILTER) {
    int i = HIT_FILTER - id - 1;
    game_filter = i < 0 ? String() : games[i];
    scroll = scroll_target = 0;
    refilter();
  }
  else if (id == 0) onNewSet();
  else if (id - 1 < (int)shown.size()) openSet(sets[shown[id - 1]].path);
}

void WelcomeWindow::onRightUp(wxMouseEvent& ev) {
  int id = hitAt(ev.GetPosition());
  if (id >= 1 && id - 1 < (int)shown.size()) showTileMenu(shown[id - 1]);
}

void WelcomeWindow::onKey(wxCommandEvent& ev) {
  if (ev.GetId() == ID_FILE_OPEN) onOpenSet();
  else onNewSet();
}

void WelcomeWindow::onActivate(wxActivateEvent& ev) {
  // Pick up sets that were added, changed or removed while the window was in the background.
  if (ev.GetActive()) rescan();
  ev.Skip();
}

void WelcomeWindow::onIdle(wxIdleEvent& ev) {
  // Setsmith: show errors such as "Error loading set" (there is no Console tab to collect them)
  show_queued_messages(this);
  // Decode one tile picture per idle pass, in display order, so the window stays responsive.
  for (size_t i : shown) {
    if (sets[i].image_loaded) continue;
    load_tile_image(sets[i]);
    if (sets[i].image.IsOk()) { fade_in[sets[i].path] = 0; startAnimation(); }
    ev.RequestMore();
    return;
  }
}

// ----------------------------------------------------------------------------- : Actions

void WelcomeWindow::openSet(const String& path) {
  wxBusyCursor wait;
  try {
    close(import_set(path));
  } catch (Error& e) {
    handle_error(_("Error loading set: ") + e.what());
    // a recent set that cannot be opened is dropped, so the error does not come back
    settings.recent_sets.erase(std::remove(settings.recent_sets.begin(), settings.recent_sets.end(), path), settings.recent_sets.end());
    rescan();
  }
}

void WelcomeWindow::onOpenSet() {
  wxFileDialog dlg(this, _TITLE_("open set"), settings.default_set_dir, wxEmptyString, import_formats(), wxFD_OPEN);
  if (dlg.ShowModal() != wxID_OK) return;
  settings.default_set_dir = dlg.GetDirectory();
  openSet(dlg.GetPath());
}

void WelcomeWindow::onNewSet() {
  close(new_set_window(this));
}

// Folders: add a watched folder, or stop watching one. Documents\Setsmith Cards is always watched.
void WelcomeWindow::showFoldersMenu(const wxRect& below) {
  enum { ADD = 1, REMOVE = 100 };
  wxMenu menu;
  menu.Append(ADD, _("Watch a folder..."));
  menu.AppendSeparator();
  menu.Append(wxID_ANY, _("Always watched: ") + default_watched_folder())->Enable(false);
  for (size_t i = 0; i < settings.watched_folders.size(); ++i) {
    menu.Append(REMOVE + (int)i, _("Stop watching ") + settings.watched_folders[i]);
  }
  int choice = GetPopupMenuSelectionFromUser(menu, wxPoint(below.x, below.GetBottom() + FromDIP(4)));
  if (choice == ADD) {
    wxDirDialog dlg(this, _("Choose a folder with sets to show on the dashboard"), settings.default_set_dir);
    if (dlg.ShowModal() != wxID_OK) return;
    String folder = dlg.GetPath();
    bool known = false;
    for (const String& f : settings.watched_folders) if (f.CmpNoCase(folder) == 0) known = true;
    if (!known) settings.watched_folders.push_back(folder);
    rescan();
  } else if (choice >= REMOVE && choice - REMOVE < (int)settings.watched_folders.size()) {
    settings.watched_folders.erase(settings.watched_folders.begin() + (choice - REMOVE));
    rescan();
  }
}

// Right-click on a tile: open, pick the tile picture, show the file, forget a recent set.
void WelcomeWindow::showTileMenu(size_t set_index) {
  enum { OPEN = 1, PICTURE, SHOW, FORGET };
  const SetSummary s = sets[set_index];
  bool recent = false;
  for (const String& r : settings.recent_sets) if (r.CmpNoCase(s.path) == 0) recent = true;
  wxMenu menu;
  menu.Append(OPEN, _("Open"));
  menu.Append(PICTURE, _("Choose tile picture..."))->Enable(!s.card_names.empty());
  menu.Append(SHOW, _("Show in folder"));
  if (recent) { menu.AppendSeparator(); menu.Append(FORGET, _("Remove from recent sets")); }
  int choice = GetPopupMenuSelectionFromUser(menu);
  if (choice == OPEN) openSet(s.path);
  else if (choice == PICTURE) {
    wxArrayString names;
    for (const String& n : s.card_names) names.Add(n.empty() ? String(_("(unnamed card)")) : n);
    wxSingleChoiceDialog dlg(this, _("Whose art should the tile show?"), _("Tile picture for ") + s.title, names);
    theme::apply(&dlg);
    for (size_t i = 0; i < s.card_names.size(); ++i) {
      if (s.card_names[i] == s.tile_card) dlg.SetSelection((int)i);
    }
    if (dlg.ShowModal() != wxID_OK) return;
    wxBusyCursor wait;
    try {
      save_tile_card(s.path, s.card_names[dlg.GetSelection()]);
    } catch (Error& e) {
      handle_error(_("Could not save the tile picture: ") + e.what());
    }
    rescan();
  } else if (choice == SHOW) {
    wxExecute(_("explorer /select,\"") + s.path + _("\""));
  } else if (choice == FORGET) {
    auto& rs = settings.recent_sets;
    rs.erase(std::remove_if(rs.begin(), rs.end(), [&](const String& r) { return r.CmpNoCase(s.path) == 0; }), rs.end());
    rescan();
  }
}

void WelcomeWindow::close(const SetP& set) {
  if (!set) return;
  (new SetWindow(nullptr, set))->Show();
  Close();
}

BEGIN_EVENT_TABLE(WelcomeWindow, wxFrame)
  EVT_PAINT        (WelcomeWindow::onPaint)
  EVT_MOTION       (WelcomeWindow::onMouseMove)
  EVT_LEAVE_WINDOW (WelcomeWindow::onMouseLeave)
  EVT_LEFT_UP      (WelcomeWindow::onLeftUp)
  EVT_RIGHT_UP     (WelcomeWindow::onRightUp)
  EVT_MOUSEWHEEL   (WelcomeWindow::onWheel)
  EVT_SIZE         (WelcomeWindow::onSize)
  EVT_TIMER        (wxID_ANY, WelcomeWindow::onTimer)
  EVT_ACTIVATE     (WelcomeWindow::onActivate)
  EVT_IDLE         (WelcomeWindow::onIdle)
  EVT_MENU         (ID_FILE_OPEN, WelcomeWindow::onKey)
  EVT_MENU         (ID_FILE_NEW,  WelcomeWindow::onKey)
END_EVENT_TABLE  ()

// ----------------------------------------------------------------------------- : Hover button with label

HoverButtonExt::HoverButtonExt(Window* parent, int id, const wxImage& icon, const String& label, const String& sub_label)
  : HoverButton(parent, id, _("btn"))
  , icon(icon)
  , label(label), sub_label(sub_label)
  , font_large(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _("Arial"))
  , font_small(8,  wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _("Arial"))
{}

void HoverButtonExt::draw(DC& dc) {
  // draw button
  HoverButton::draw(dc);
  int d = drawDelta();
  // icon
  if (icon.Ok()) dc.DrawBitmap(icon, d+7, d+7);
  // text
  dc.SetTextForeground(*wxBLACK);
  dc.SetFont(font_large);
  dc.DrawText(label, d+44, d+7);
  dc.SetFont(font_small);
  dc.DrawText(sub_label, d+45, d+28);
}
