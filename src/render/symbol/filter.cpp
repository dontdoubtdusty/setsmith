//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/symbol/filter.hpp>
#include <render/symbol/viewer.hpp>
#include <gfx/gfx.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Symbol filtering

void filter_symbol(Image& symbol, const SymbolFilter& filter) {
  Byte* data  = symbol.GetData();
  Byte* alpha = symbol.GetAlpha();
  UInt width = symbol.GetWidth(), height = symbol.GetHeight();
  // HACK: wxGTK seems to fail sometimes if you ask it to allocate the alpha channel.
  //       This manually allocates the memory and gives it to the image to handle.
  if (!alpha) {
    alpha = (Byte*) malloc(width * height);
    symbol.SetAlpha(alpha);
  }
  for (UInt y = 0 ; y < height ; ++y) {
    for (UInt x = 0 ; x < width ; ++x) {
      // Determine set
      //  green           -> border or outside
      //  green+red=white -> border
      if (data[0] != data[2]) {
        // yellow/blue = editing hint, leave alone
      } else {
        SymbolSet point = data[1] ? (data[0] ? SYMBOL_BORDER : SYMBOL_OUTSIDE) : SYMBOL_INSIDE;
        // Call filter
        Color result = filter.color((double)x / width, (double)y / height, point);
        // Store color
        data[0]  = result.Red();
        data[1]  = result.Green();
        data[2]  = result.Blue();
        alpha[0] = result.Alpha();
      }
      // next
      data  += 3;
      alpha += 1;
    }
  }
}

Image render_symbol(const SymbolP& symbol, const SymbolFilter& filter, double border_radius, int width, int height, bool edit_hints, bool allow_smaller) {
  Image i = render_symbol(symbol, border_radius, width, height, edit_hints, allow_smaller);
  filter_symbol(i, filter);
  return i;
}

// ----------------------------------------------------------------------------- : Image symbols (MSE Slim)

bool is_image_symbol_file(const String& filename) {
  return filename.Lower().EndsWith(_(".png"));
}

namespace {
  // Which pixels of the source belong to the shape: alpha if there is any, otherwise dark pixels
  vector<bool> image_symbol_shape(const Image& source) {
    int w = source.GetWidth(), h = source.GetHeight();
    vector<bool> shape(w * h, false);
    const Byte* data  = source.GetData();
    const Byte* alpha = source.HasAlpha() ? source.GetAlpha() : nullptr;
    bool mask = !alpha && source.HasMask();
    Byte mr = mask ? source.GetMaskRed() : 0, mg = mask ? source.GetMaskGreen() : 0, mb = mask ? source.GetMaskBlue() : 0;
    for (int i = 0 ; i < w * h ; ++i) {
      const Byte* p = data + 3 * i;
      if (alpha) {
        shape[i] = alpha[i] >= 128;
      } else if (mask) {
        shape[i] = !(p[0] == mr && p[1] == mg && p[2] == mb);
      } else {
        shape[i] = (p[0] * 299 + p[1] * 587 + p[2] * 114) / 1000 < 128;
      }
    }
    return shape;
  }

  // Bounding box of the shape, or the whole image if the shape is empty
  wxRect image_symbol_bounds(const Image& source) {
    int w = source.GetWidth(), h = source.GetHeight();
    vector<bool> shape = image_symbol_shape(source);
    int x0 = w, y0 = h, x1 = -1, y1 = -1;
    for (int y = 0 ; y < h ; ++y) {
      for (int x = 0 ; x < w ; ++x) {
        if (shape[y * w + x]) {
          x0 = min(x0, x); x1 = max(x1, x);
          y0 = min(y0, y); y1 = max(y1, y);
        }
      }
    }
    if (x1 < 0) return wxRect(0, 0, w, h);
    return wxRect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
  }
}

wxSize image_symbol_shape_size(const Image& source) {
  if (!source.Ok()) return wxSize(0, 0);
  vector<bool> shape = image_symbol_shape(source);
  if (find(shape.begin(), shape.end(), true) == shape.end()) return wxSize(0, 0);
  return image_symbol_bounds(source).GetSize();
}

double image_symbol_aspect_ratio(const Image& source) {
  if (!source.Ok()) return 1.0;
  wxRect r = image_symbol_bounds(source);
  return (double)r.width / r.height;
}

Image render_image_symbol(const Image& source, const SymbolFilter& filter, double border_radius, int width, int height) {
  width = max(width, 1); height = max(height, 1);
  // classified output: black = inside, white = border, green = outside (see filter_symbol)
  Image out(width, height, false);
  {
    Byte* d = out.GetData();
    for (int i = 0 ; i < width * height ; ++i) { d[3*i] = 0; d[3*i+1] = 255; d[3*i+2] = 0; }
  }
  if (source.Ok()) {
    // trim empty margins, then fit the shape inside the output, leaving room for the border
    wxRect bounds = image_symbol_bounds(source);
    Image trimmed = source.GetSubImage(bounds);
    double border = max(0.0, border_radius) * min(width, height);
    int margin = (int)ceil(border) + 1;
    double room_w = max(1, width - 2 * margin), room_h = max(1, height - 2 * margin);
    double scale = min(room_w / bounds.width, room_h / bounds.height);
    int sw = max(1, (int)(bounds.width * scale)), sh = max(1, (int)(bounds.height * scale));
    Image scaled = trimmed.Scale(sw, sh, wxIMAGE_QUALITY_HIGH);
    vector<bool> shape = image_symbol_shape(scaled);
    int ox = (width - sw) / 2, oy = (height - sh) / 2;
    // distance from each output pixel to the nearest shape pixel (two-pass chamfer approximation)
    const float far_away = 1e9f, diag = 1.41421356f;
    vector<float> dist(width * height, far_away);
    for (int y = 0 ; y < sh ; ++y) {
      for (int x = 0 ; x < sw ; ++x) {
        if (shape[y * sw + x]) dist[(y + oy) * width + (x + ox)] = 0;
      }
    }
    for (int y = 0 ; y < height ; ++y) {
      for (int x = 0 ; x < width ; ++x) {
        float& v = dist[y * width + x];
        if (x > 0)                  v = min(v, dist[y * width + x - 1] + 1);
        if (y > 0)                  v = min(v, dist[(y - 1) * width + x] + 1);
        if (x > 0 && y > 0)         v = min(v, dist[(y - 1) * width + x - 1] + diag);
        if (x + 1 < width && y > 0) v = min(v, dist[(y - 1) * width + x + 1] + diag);
      }
    }
    for (int y = height - 1 ; y >= 0 ; --y) {
      for (int x = width - 1 ; x >= 0 ; --x) {
        float& v = dist[y * width + x];
        if (x + 1 < width)                   v = min(v, dist[y * width + x + 1] + 1);
        if (y + 1 < height)                  v = min(v, dist[(y + 1) * width + x] + 1);
        if (x + 1 < width && y + 1 < height) v = min(v, dist[(y + 1) * width + x + 1] + diag);
        if (x > 0 && y + 1 < height)         v = min(v, dist[(y + 1) * width + x - 1] + diag);
      }
    }
    // classify
    Byte* d = out.GetData();
    for (int i = 0 ; i < width * height ; ++i) {
      if (dist[i] == 0)                       { d[3*i] = 0;   d[3*i+1] = 0;   d[3*i+2] = 0;   } // inside
      else if (border > 0 && dist[i] <= border) { d[3*i] = 255; d[3*i+1] = 255; d[3*i+2] = 255; } // border
    }
  }
  filter_symbol(out, filter);
  return out;
}

// ----------------------------------------------------------------------------- : SymbolFilter

IMPLEMENT_REFLECTION_NO_SCRIPT(SymbolFilter) {
  REFLECT_IF_NOT_READING {
    String fill_type = fillType();
    REFLECT(fill_type);
  }
}
template <> void GetMember::handle(const intrusive_ptr<SymbolFilter>& f) {
  handle(*f);
}

template <>
intrusive_ptr<SymbolFilter> read_new<SymbolFilter>(Reader& reader) {
  // there must be a fill type specified
  String fill_type;
  reader.handle(_("fill_type"), fill_type);
  if      (fill_type == _("solid"))           return make_intrusive<SolidFillSymbolFilter>();
  else if (fill_type == _("linear gradient")) return make_intrusive<LinearGradientSymbolFilter>();
  else if (fill_type == _("radial gradient")) return make_intrusive<RadialGradientSymbolFilter>();
  else if (fill_type.empty()) {
    reader.warning(_ERROR_1_("expected key", _("fill type")));
    throw ParseError(_ERROR_("aborting parsing"));
  } else {
    reader.warning(_ERROR_1_("unsupported fill type", fill_type));
    throw ParseError(_ERROR_("aborting parsing"));
  }
}

// ----------------------------------------------------------------------------- : SolidFillSymbolFilter

String SolidFillSymbolFilter::fillType() const { return _("solid"); }

Color SolidFillSymbolFilter::color(double x, double y, SymbolSet point) const {
  if      (point == SYMBOL_INSIDE) return fill_color;
  else if (point == SYMBOL_BORDER) return border_color;
  else                             return Color(0,0,0,0);
}

bool SolidFillSymbolFilter::operator == (const SymbolFilter& that) const {
  const SolidFillSymbolFilter* that2 = dynamic_cast<const SolidFillSymbolFilter*>(&that);
  return that2 && fill_color   == that2->fill_color
               && border_color == that2->border_color;
}

IMPLEMENT_REFLECTION(SolidFillSymbolFilter) {
  REFLECT_BASE(SymbolFilter);
  REFLECT(fill_color);
  REFLECT(border_color);
}

// ----------------------------------------------------------------------------- : GradientSymbolFilter

template <typename T>
Color GradientSymbolFilter::color(double x, double y, SymbolSet point, const T* t) const {
  if      (point == SYMBOL_INSIDE) return lerp(fill_color_1,   fill_color_2,   t->t(x,y));
  else if (point == SYMBOL_BORDER) return lerp(border_color_1, border_color_2, t->t(x,y));
  else                             return Color(0,0,0,0);
}

bool GradientSymbolFilter::equal(const GradientSymbolFilter& that) const {
  return fill_color_1   == that.fill_color_1
      && fill_color_2   == that.fill_color_2
      && border_color_1 == that.border_color_1
      && border_color_2 == that.border_color_2;
}

IMPLEMENT_REFLECTION(GradientSymbolFilter) {
  REFLECT_BASE(SymbolFilter);
  REFLECT(fill_color_1);
  REFLECT(fill_color_2);
  REFLECT(border_color_1);
  REFLECT(border_color_2);
}

// ----------------------------------------------------------------------------- : LinearGradientSymbolFilter

// TODO: move to some general util header
inline double sqr(double x) { return x * x; }

String LinearGradientSymbolFilter::fillType() const { return _("linear gradient"); }

LinearGradientSymbolFilter::LinearGradientSymbolFilter()
  : center_x(0.5), center_y(0.5)
  , end_x(1), end_y(1)
{}
LinearGradientSymbolFilter::LinearGradientSymbolFilter
    ( const Color& fill_color_1, const Color& border_color_1
    , const Color& fill_color_2, const Color& border_color_2
    , double center_x, double center_y, double end_x, double end_y)
  : GradientSymbolFilter(fill_color_1, border_color_1, fill_color_2, border_color_2)
  , center_x(center_x), center_y(center_y)
  , end_x(end_x), end_y(end_y)
{}

Color LinearGradientSymbolFilter::color(double x, double y, SymbolSet point) const {
  len = sqr(end_x - center_x) + sqr(end_y - center_y);
  if (len == 0) len = 1; // prevent div by 0
  return GradientSymbolFilter::color(x,y,point,this);
}

double LinearGradientSymbolFilter::t(double x, double y) const {
  double t= fabs( (x - center_x) * (end_x - center_x) + (y - center_y) * (end_y - center_y)) / len;
  return min(1.,max(0.,t));
}

bool LinearGradientSymbolFilter::operator == (const SymbolFilter& that) const {
  const LinearGradientSymbolFilter* that2 = dynamic_cast<const LinearGradientSymbolFilter*>(&that);
  return that2 && equal(*that2)
               && center_x == that2->center_x && end_x == that2->end_x
               && center_y == that2->center_y && end_y == that2->end_y;
}

IMPLEMENT_REFLECTION(LinearGradientSymbolFilter) {
  REFLECT_BASE(GradientSymbolFilter);
  REFLECT(center_x); REFLECT(center_y);
  REFLECT(end_x);    REFLECT(end_y);
}

// ----------------------------------------------------------------------------- : RadialGradientSymbolFilter

String RadialGradientSymbolFilter::fillType() const { return _("radial gradient"); }

Color RadialGradientSymbolFilter::color(double x, double y, SymbolSet point) const {
  return GradientSymbolFilter::color(x,y,point,this);
}

double RadialGradientSymbolFilter::t(double x, double y) const {
  return sqrt( (sqr(x - 0.5) + sqr(y - 0.5)) * 2); 
}

bool RadialGradientSymbolFilter::operator == (const SymbolFilter& that) const {
  const RadialGradientSymbolFilter* that2 = dynamic_cast<const RadialGradientSymbolFilter*>(&that);
  return that2 && equal(*that2);
}
