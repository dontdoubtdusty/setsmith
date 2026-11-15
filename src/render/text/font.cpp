//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/text/element.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Kerning

// Setsmith: the MPlantin fonts have no kerning table, so a period or comma before a closing quote
// leaves a wide gap (very visible in italic flavor text). Returns how far to pull the quote left,
// as a share of the period's width in this font, or 0 when the pair needs no kerning.
static double kern_share(Char prev, Char c, bool italic) {
  if ((prev == _('.') || prev == _(',')) && (c == 0x201D || c == 0x2019)) {
    return italic ? 0.7 : 0.27;
  }
  return 0;
}

// ----------------------------------------------------------------------------- : FontTextElement

void FontTextElement::draw(RotatedDC& dc, double scale, const RealRect& rect, const double* xs, DrawWhat what, size_t start, size_t end) const {
  if ((what & draw_as) != draw_as) return; // don't draw
  // text
  String text = content.substr(start - this->start, end - start);
  if (!text.empty() && text.GetChar(text.size() - 1) == _('\n')) {
    text = text.substr(0, text.size() - 1); // don't draw last \n
  }
  // draw, split into runs where a kerned pair starts so each run lands on its own x position
  dc.SetFont(*font, scale);
  bool italic = font->flags & FONT_ITALIC;
  size_t run = 0;
  for (size_t k = 1 ; k <= text.size() ; ++k) {
    if (k == text.size() || kern_share(text.GetChar(k - 1), text.GetChar(k), italic) != 0) {
      dc.DrawTextWithShadow(text.substr(run, k - run), *font, RealPoint(rect.x + xs[run] - xs[0], rect.y));
      run = k;
    }
  }
}

void FontTextElement::getCharInfo(RotatedDC& dc, double scale, vector<CharInfo>& out) const {
  // font
  dc.SetFont(*font, scale);
  // find sizes & breaks
  double prev_width = 0;
  size_t line_start = start; // start of the current line
  bool italic = font->flags & FONT_ITALIC;
  double period_width = dc.GetTextExtent(_(".")).width;
  for (size_t i = start ; i < end ; ++i) {
    Char c = content.GetChar(i - this->start);
    if (c == _('\n')) {
      out.push_back(CharInfo(RealSize(0, dc.GetCharHeight()), break_style, draw_as == DRAW_ACTIVE));
      line_start = i + 1;
      prev_width = 0;
    } else {
      RealSize s = dc.GetTextExtent(content.substr(line_start - this->start, i - line_start + 1));
      // Setsmith: a kerned pair takes the pull off the previous character, so the quote starts further left
      double kern = 0;
      if (i > line_start && !out.empty()) {
        kern = period_width * kern_share(content.GetChar(i - 1 - this->start), c, italic);
        out.back().size.width -= kern;
      }
      out.push_back(CharInfo(
                       RealSize(s.width - prev_width, s.height),
                       c == _(' ') ? LineBreak::SPACE : LineBreak::MAYBE,
                       draw_as == DRAW_ACTIVE // from <soft> tag
                   ));
      prev_width = s.width;
    }
  }
}

double FontTextElement::minScale() const {
  return min(font->size(), font->scale_down_to) / max(0.01, font->size());
}
double FontTextElement::scaleStep() const {
  return 1. / max(font->size() * 4, 1.);
}
