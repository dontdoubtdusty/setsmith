//+----------------------------------------------------------------------------+
//| Description:  Setsmith - flat line icons drawn in code                     |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/icons.hpp>
#include <wx/graphics.h>
#include <functional>
#include <cmath>

// ----------------------------------------------------------------------------- : Pen

// Everything is drawn on a 20 x 20 grid, scaled to the requested size.
namespace {
  struct Pen {
    wxGraphicsContext& gc;
    wxColour colour;
    int size;

    void line(double x1, double y1, double x2, double y2) { gc.StrokeLine(x1, y1, x2, y2); }
    void rrect(double x, double y, double w, double h, double r) { gc.DrawRoundedRectangle(x, y, w, h, r); }
    void circle(double x, double y, double r) { gc.DrawEllipse(x - r, y - r, 2 * r, 2 * r); }
    void dot(double x, double y, double r) {
      gc.SetBrush(wxBrush(colour));
      circle(x, y, r);
      gc.SetBrush(*wxTRANSPARENT_BRUSH);
    }
    // A polyline through points given as x0,y0,x1,y1,...
    void poly(std::initializer_list<double> pts, bool closed = false) {
      wxGraphicsPath p = gc.CreatePath();
      auto it = pts.begin();
      p.MoveToPoint(it[0], it[1]);
      for (size_t i = 2; i + 1 < pts.size(); i += 2) p.AddLineToPoint(it[i], it[i + 1]);
      if (closed) p.CloseSubpath();
      gc.StrokePath(p);
    }
    // A cubic curve from (x0,y0) to (x3,y3)
    void curve(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3) {
      wxGraphicsPath p = gc.CreatePath();
      p.MoveToPoint(x0, y0);
      p.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
      gc.StrokePath(p);
    }
    // A letter centred in the icon, in pixel units (the grid scale is undone for text)
    void letter(const String& s, bool bold, bool italic, double dy = 0) {
      gc.PushState();
      gc.SetTransform(gc.CreateMatrix());
      wxFontInfo info(wxSize(0, (int)std::round(size * 0.78)));
      info.FaceName(_("Figtree"));
      if (bold) info.Bold();
      if (italic) info.Italic();
      gc.SetFont(gc.CreateFont(wxFont(info), colour));
      double w, h, descent, leading;
      gc.GetTextExtent(s, &w, &h, &descent, &leading);
      gc.DrawText(s, (size - w) / 2, (size - h) / 2 + dy * size / 20.0);
      gc.PopState();
    }
    void card(double x, double y, double w = 9.5, double h = 13) { rrect(x, y, w, h, 1.6); }
    void plus(double x, double y)  { line(x - 3, y, x + 3, y); line(x, y - 3, x, y + 3); }
    void minus(double x, double y) { line(x - 3, y, x + 3, y); }
  };

  void undo_arrow(Pen& p, bool mirror) {
    auto X = [mirror](double x) { return mirror ? 20 - x : x; };
    p.poly({X(7.5), 4.5, X(4), 8, X(7.5), 11.5});
    p.curve(X(4), 8, X(13), 8, X(17), 8, X(17), 12);
    p.curve(X(17), 12, X(17), 15.5, X(14), 16, X(11), 16);
  }

  // Each icon, by name
  const map<String, std::function<void(Pen&)>>& drawings() {
    static const map<String, std::function<void(Pen&)>> m = {
      {_("new"), [](Pen& p) {
        p.poly({5, 2.5, 11.5, 2.5, 15.5, 6.5, 15.5, 17.5, 5, 17.5}, true);
        p.poly({11.5, 2.5, 11.5, 6.5, 15.5, 6.5});
      }},
      {_("open"), [](Pen& p) {
        p.poly({2.5, 15.5, 2.5, 4.5, 7.5, 4.5, 9.5, 6.5, 16, 6.5, 16, 8.5});
        p.poly({2.5, 15.5, 5, 8.5, 18, 8.5, 15.5, 15.5}, true);
      }},
      {_("save"), [](Pen& p) {
        p.rrect(3, 3, 14, 14, 2);
        p.poly({6.5, 3, 6.5, 7, 12.5, 7, 12.5, 3});
        p.rrect(6, 10.5, 8, 6.5, 1);
      }},
      {_("export"), [](Pen& p) {
        p.poly({3, 11.5, 3, 16.5, 17, 16.5, 17, 11.5});
        p.line(10, 12.5, 10, 3);
        p.poly({6.5, 6.5, 10, 3, 13.5, 6.5});
      }},
      {_("export_image"), [](Pen& p) {
        p.rrect(3, 4, 14, 12, 2);
        p.circle(7.5, 8, 1.4);
        p.poly({3.5, 14.5, 8, 10.5, 11, 13, 13.5, 11, 16.5, 14});
      }},
      {_("export_images"), [](Pen& p) {
        p.poly({5, 3, 17.5, 3, 17.5, 13});
        p.rrect(2.5, 5.5, 12.5, 11, 2);
        p.poly({3, 15, 7, 11.5, 9.5, 13.5, 11.5, 12, 14.5, 14.5});
      }},
      {_("cut"), [](Pen& p) {
        p.circle(6, 14.5, 2.4);
        p.circle(14, 14.5, 2.4);
        p.line(7.4, 12.5, 14, 3);
        p.line(12.6, 12.5, 6, 3);
      }},
      {_("copy"), [](Pen& p) {
        p.rrect(7, 7, 10, 10, 1.8);
        p.poly({4.5, 13, 3, 13, 3, 3, 13, 3, 13, 4.5});
      }},
      {_("paste"), [](Pen& p) {
        p.rrect(4, 4, 12, 13.5, 1.8);
        p.rrect(7, 2.5, 6, 3, 1);
        p.line(7, 10, 13, 10);
        p.line(7, 13, 11, 13);
      }},
      {_("undo"), [](Pen& p) { undo_arrow(p, false); }},
      {_("redo"), [](Pen& p) { undo_arrow(p, true); }},
      {_("bold"),      [](Pen& p) { p.letter(_("B"), true, false); }},
      {_("italic"),    [](Pen& p) { p.letter(_("I"), false, true); }},
      {_("underline"), [](Pen& p) { p.letter(_("U"), false, false, -1.5); p.line(5.5, 17, 14.5, 17); }},
      {_("symbol"), [](Pen& p) {
        p.circle(10, 10, 6.5);
        p.dot(10, 10, 2.2);
      }},
      {_("reminder"), [](Pen& p) {
        p.curve(7, 3.5, 3.5, 7, 3.5, 13, 7, 16.5);
        p.curve(13, 3.5, 16.5, 7, 16.5, 13, 13, 16.5);
        p.dot(7.8, 10, 0.9); p.dot(10, 10, 0.9); p.dot(12.2, 10, 0.9);
      }},
      {_("card_add"), [](Pen& p) {
        p.poly({10.5, 15.5, 3, 15.5, 3, 2.5, 12.5, 2.5, 12.5, 10});
        p.plus(15, 15);
      }},
      {_("card_add_multiple"), [](Pen& p) {
        p.poly({6, 2.5, 15, 2.5, 15, 9});
        p.poly({9.5, 17, 3, 17, 3, 5, 12, 5, 12, 11});
        p.plus(15.5, 15);
      }},
      {_("card_del"), [](Pen& p) {
        p.poly({10.5, 15.5, 3, 15.5, 3, 2.5, 12.5, 2.5, 12.5, 11});
        p.minus(15, 15);
      }},
      {_("card_rotate"), [](Pen& p) {
        p.card(3, 5.5, 9, 12);
        p.curve(9.5, 2.5, 13.5, 2, 16.5, 4, 16.5, 9);
        p.poly({14.2, 7.2, 16.5, 9.5, 18.8, 7.2});
      }},
      {_("card_rotate_0"),   [](Pen& p) { p.card(5.5, 3, 9, 14);  p.line(8, 5.8, 12, 5.8); }},
      {_("card_rotate_180"), [](Pen& p) { p.card(5.5, 3, 9, 14);  p.line(8, 14.2, 12, 14.2); }},
      {_("card_rotate_90"),  [](Pen& p) { p.card(3, 5.5, 14, 9);  p.line(14.2, 8, 14.2, 12); }},
      {_("card_rotate_270"), [](Pen& p) { p.card(3, 5.5, 14, 9);  p.line(5.8, 8, 5.8, 12); }},
      {_("print"), [](Pen& p) {
        p.poly({5.5, 7.5, 5.5, 3, 14.5, 3, 14.5, 7.5});
        p.poly({5.5, 14.5, 2.5, 14.5, 2.5, 7.5, 17.5, 7.5, 17.5, 14.5, 14.5, 14.5});
        p.rrect(5.5, 11.5, 9, 6, 0.8);
      }},
      {_("print_preview"), [](Pen& p) {
        p.poly({10, 17.5, 3.5, 17.5, 3.5, 2.5, 14.5, 2.5, 14.5, 7});
        p.circle(12, 11.5, 3.3);
        p.line(14.4, 13.9, 17.5, 17);
      }},
      {_("find"), [](Pen& p) {
        p.circle(8.5, 8.5, 5);
        p.line(12.2, 12.2, 17, 17);
      }},
      {_("window_cards"), [](Pen& p) {
        p.poly({3.5, 13, 3.5, 3, 11, 3});
        p.card(6.5, 5.5, 10, 12.5);
      }},
      {_("window_style"), [](Pen& p) {
        p.line(16.5, 3, 10, 9.5);
        wxGraphicsPath path = p.gc.CreatePath();
        path.MoveToPoint(10.5, 9);
        path.AddCurveToPoint(6.5, 8.5, 6, 12, 5.5, 13.5);
        path.AddCurveToPoint(5, 15.5, 4, 16.5, 3, 17);
        path.AddCurveToPoint(7, 17.5, 11.5, 15.5, 11, 9.5);
        path.CloseSubpath();
        p.gc.StrokePath(path);
      }},
      {_("window_set_info"), [](Pen& p) {
        p.circle(10, 10, 7);
        p.line(10, 9, 10, 14);
        p.dot(10, 6.3, 0.9);
      }},
      {_("fullscreen"), [](Pen& p) {
        p.poly({3, 7.5, 3, 3, 7.5, 3});   p.poly({12.5, 3, 17, 3, 17, 7.5});
        p.poly({17, 12.5, 17, 17, 12.5, 17}); p.poly({7.5, 17, 3, 17, 3, 12.5});
      }},
      {_("fullscreen_exit"), [](Pen& p) {
        p.poly({3, 7.5, 7.5, 7.5, 7.5, 3});   p.poly({12.5, 3, 12.5, 7.5, 17, 7.5});
        p.poly({17, 12.5, 12.5, 12.5, 12.5, 17}); p.poly({7.5, 17, 7.5, 12.5, 3, 12.5});
      }},
      {_("menu"),     [](Pen& p) { p.line(3.5, 5.5, 16.5, 5.5); p.line(3.5, 10, 16.5, 10); p.line(3.5, 14.5, 16.5, 14.5); }},
      {_("minimize"), [](Pen& p) { p.line(4.5, 10, 15.5, 10); }},
      {_("close"),    [](Pen& p) { p.line(5, 5, 15, 15); p.line(15, 5, 5, 15); }},
    };
    return m;
  }
}

// ----------------------------------------------------------------------------- : draw_icon

wxImage draw_icon(const String& name, int size, const wxColour& colour) {
  auto it = drawings().find(name);
  if (it == drawings().end()) return wxImage();
  wxImage img(size, size);
  img.InitAlpha();
  memset(img.GetAlpha(), 0, size * size);
  memset(img.GetData(), 0, size * size * 3);
  {
    unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(img));
    if (!gc) return wxImage();
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    gc->Scale(size / 20.0, size / 20.0);
    wxGraphicsPenInfo pen(colour, 1.6);
    pen.Cap(wxCAP_ROUND).Join(wxJOIN_ROUND);
    gc->SetPen(gc->CreatePen(pen));
    gc->SetBrush(*wxTRANSPARENT_BRUSH);
    Pen p{*gc, colour, size};
    it->second(p);
  } // the image is written when the context goes away
  return img;
}
