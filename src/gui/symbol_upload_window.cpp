//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/symbol_upload_window.hpp>
#include <gui/util.hpp>
#include <render/symbol/filter.hpp>
#include <data/settings.hpp>
#include <wx/dcbuffer.h>
#include <wx/filename.h>

// MSE Slim: the upload dialog that replaces the symbol editor.

namespace {
  const int  ID_CHOOSE    = wxID_HIGHEST + 1;
  const int  PREVIEW_SIZE = 72;  // height of each rarity preview, in pixels
  const int  MAX_STORED   = 1024; // longest side of the image stored in the set
  const Color ACCENT(176, 120, 30); // warm gold, for the heading and notes
  // Rarities to preview, in order. Variations with other names are skipped if these exist.
  const Char* PREVIEW_ORDER[] = { _("common"), _("uncommon"), _("rare"), _("mythic rare") };
}

// ----------------------------------------------------------------------------- : SymbolUploadPreview

/// Shows the chosen image as it will look in each rarity, on a checkerboard
class SymbolUploadPreview : public wxPanel {
public:
  SymbolUploadPreview(Window* parent, const SymbolStyle& style)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(4 * (PREVIEW_SIZE + 24), PREVIEW_SIZE + 34))
  {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // pick the variations to show: the four main rarities if present, otherwise the first four
    for (const Char* name : PREVIEW_ORDER) {
      FOR_EACH_CONST(v, style.variations) if (v->name == name) variations.push_back(v);
    }
    if (variations.empty()) {
      for (size_t i = 0 ; i < style.variations.size() && i < 4 ; ++i) variations.push_back(style.variations[i]);
    }
    Bind(wxEVT_PAINT, &SymbolUploadPreview::onPaint, this);
  }

  /// Render the image in every previewed rarity
  void setImage(const Image& img) {
    bitmaps.clear();
    if (img.Ok()) {
      double ar = min(2.0, max(0.5, image_symbol_aspect_ratio(img)));
      int w = (int)(PREVIEW_SIZE * ar), h = PREVIEW_SIZE;
      FOR_EACH(v, variations) {
        Image big = render_image_symbol(img, *v->filter, v->border_radius, 3 * w, 3 * h);
        bitmaps.push_back(Bitmap(big.Scale(w, h, wxIMAGE_QUALITY_HIGH)));
      }
    }
    Refresh();
  }

private:
  vector<SymbolVariationP> variations;
  vector<Bitmap> bitmaps;

  void onPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetClientSize();
    dc.SetBackground(wxBrush(Color(250, 248, 243)));
    dc.Clear();
    dc.SetPen(wxPen(Color(214, 206, 190)));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRoundedRectangle(0, 0, size.x, size.y, 6);
    if (bitmaps.empty()) {
      dc.SetTextForeground(Color(140, 130, 115));
      String text = _("Choose an image to see it in each rarity");
      wxSize ts = dc.GetTextExtent(text);
      dc.DrawText(text, (size.x - ts.x) / 2, (size.y - ts.y) / 2);
      return;
    }
    // one cell per rarity: the symbol centered over its name
    int cell = size.x / (int)bitmaps.size();
    dc.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(Color(110, 100, 85));
    for (size_t i = 0 ; i < bitmaps.size() ; ++i) {
      int cx = (int)i * cell + cell / 2;
      dc.DrawBitmap(bitmaps[i], cx - bitmaps[i].GetWidth() / 2, 8, true);
      String label = capitalize(variations[i]->name);
      wxSize ts = dc.GetTextExtent(label);
      dc.DrawText(label, cx - ts.x / 2, size.y - ts.y - 6);
    }
  }
};

// ----------------------------------------------------------------------------- : SymbolUploadWindow

SymbolUploadWindow::SymbolUploadWindow(Window* parent, const SymbolStyle& style)
  : wxDialog(parent, wxID_ANY, _("Set symbol"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
  // heading
  wxStaticText* heading = new wxStaticText(this, wxID_ANY, _("Upload a set symbol"));
  heading->SetFont(wxFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  heading->SetForegroundColour(ACCENT);
  wxStaticText* subheading = new wxStaticText(this, wxID_ANY,
    _("Like real Magic sets, one shape is colored for each rarity."));
  // preview and file picker
  preview    = new SymbolUploadPreview(this, style);
  wxButton* choose = new wxButton(this, ID_CHOOSE, _("Choose image..."));
  file_label = new wxStaticText(this, wxID_ANY, _("No image chosen"));
  file_label->SetForegroundColour(Color(110, 100, 85));
  notes      = new wxStaticText(this, wxID_ANY, _(""));
  notes->SetForegroundColour(ACCENT);
  // best practices
  const Char* tips[] = {
    _("Use a PNG with a transparent background. Every pixel that isn't transparent is part of the shape."),
    _("Make it one solid silhouette. Colors and shading inside it are ignored, since the rarity colors replace them."),
    _("Keep it bold. The symbol prints about 3 mm tall, so thin lines and small gaps disappear."),
    _("Aim for a roughly square shape, no wider than 2:1. Empty space around it is trimmed automatically."),
    _("Use at least 256 px on the short side. Larger is fine; it is scaled down."),
    _("No transparency? Dark pixels become the shape and light pixels the background."),
  };
  wxSizer* tip_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Best practices"));
  for (const Char* tip : tips) {
    wxStaticText* t = new wxStaticText(this, wxID_ANY, String(_("•  ")) + tip);
    t->Wrap(420);
    tip_box->Add(t, 0, wxALL, 4);
  }
  // layout
  wxSizer* s = new wxBoxSizer(wxVERTICAL);
  s->Add(heading,    0, wxLEFT | wxRIGHT | wxTOP, 12);
  s->Add(subheading, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
  s->Add(preview,    0, wxEXPAND | wxLEFT | wxRIGHT, 12);
    wxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(choose,     0, wxALIGN_CENTER_VERTICAL);
    row->Add(file_label, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
  s->Add(row,   0, wxEXPAND | wxALL, 12);
  s->Add(notes, 0, wxEXPAND | wxLEFT | wxRIGHT, 12);
  s->Add(tip_box, 0, wxEXPAND | wxALL, 12);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
    wxButton* ok = new wxButton(this, wxID_OK, _("Use this symbol"));
    ok->SetDefault();
    ok->Disable(); // enabled once a usable image is chosen (onChoose)
    buttons->AddButton(ok);
    buttons->AddButton(new wxButton(this, wxID_CANCEL));
    buttons->Realize();
  s->Add(buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
  SetSizerAndFit(s);
  CentreOnParent();
}

void SymbolUploadWindow::onChoose(wxCommandEvent&) {
  String filename = wxFileSelector(_("Choose a set symbol image"), settings.default_image_dir, _(""), _(""),
    _("Images|*.png;*.bmp;*.jpg;*.jpeg;*.gif;*.tif;*.tiff|PNG images (*.png)|*.png"),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
  if (filename.empty()) return;
  settings.default_image_dir = wxPathOnly(filename);
  Image img;
  {
    wxLogNull no_log; // a bad file is reported below, not by wx
    img = wxImage(filename);
  }
  if (!checkImage(img, filename)) return;
  image = img;
  file_label->SetLabel(wxFileName(filename).GetFullName() + String::Format(_("   (%d x %d px)"), img.GetWidth(), img.GetHeight()));
  preview->setImage(image);
  FindWindow(wxID_OK)->Enable();
  Layout();
  Fit();
}

bool SymbolUploadWindow::checkImage(const Image& img, const String& filename) {
  if (!img.Ok()) {
    wxMessageBox(_("That file couldn't be read as an image."), _("Set symbol"), wxOK | wxICON_ERROR, this);
    return false;
  }
  wxSize shape = image_symbol_shape_size(img);
  if (shape.x == 0) {
    wxMessageBox(_("No shape found. The image is fully transparent, or (without transparency) has no dark pixels."),
                 _("Set symbol"), wxOK | wxICON_ERROR, this);
    return false;
  }
  // things that work but may not look good
  String text;
  if (!img.HasAlpha() && !img.HasMask()) {
    text += _("No transparency found, so the dark pixels are used as the shape.\n");
  }
  if (min(shape.x, shape.y) < 64) {
    text += String::Format(_("The shape is small (%d x %d px) and may look soft. 256 px or more is best.\n"), shape.x, shape.y);
  }
  double ar = (double)shape.x / shape.y;
  if (ar > 2.0 || ar < 0.5) {
    text += _("The shape is very wide or tall, so it will be squeezed to fit. Aim for no wider than 2:1.\n");
  }
  notes->SetLabel(text.Trim());
  notes->Wrap(440);
  return true;
}

Image SymbolUploadWindow::getImage() const {
  int longest = max(image.GetWidth(), image.GetHeight());
  if (longest <= MAX_STORED) return image;
  double scale = (double)MAX_STORED / longest;
  return image.Scale(max(1, (int)(image.GetWidth() * scale)), max(1, (int)(image.GetHeight() * scale)), wxIMAGE_QUALITY_HIGH);
}

BEGIN_EVENT_TABLE(SymbolUploadWindow, wxDialog)
  EVT_BUTTON    (ID_CHOOSE, SymbolUploadWindow::onChoose)
END_EVENT_TABLE()
