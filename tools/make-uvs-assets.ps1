<#
.SYNOPSIS
  Draws the frame images for the UniVersus 2026 template (text boxes, numbers, block shields, labels).

.DESCRIPTION
  Writes PNGs into templates\uvs-2026-common.mse-include\. Everything is drawn at 2x the
  template's card units (the card is 744 x 1039 units, 300 dpi), so exports stay sharp.
  Rerun after changing a color or shape here; the PNGs are generated, edit this script instead.
  Needs the fonts in templates\fonts (loaded straight from the files, no install needed).
#>
param(
  [string]$Out = (Join-Path $PSScriptRoot "..\templates\uvs-2026-common.mse-include"),
  [string]$Fonts = (Join-Path $PSScriptRoot "..\templates\fonts")
)
$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force $Out | Out-Null
$Out = (Resolve-Path $Out).Path; $Fonts = (Resolve-Path $Fonts).Path

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using System.IO;

public static class UvsAssets {
  const float S = 2f;   // pixels per card unit
  static string outDir;
  static PrivateFontCollection fonts = new PrivateFontCollection();

  static FontFamily Family(string file) {
    foreach (var f in fonts.Families) { }
    fonts.AddFontFile(file);
    return fonts.Families[fonts.Families.Length - 1];
  }
  static FontFamily Find(string name) {
    foreach (var f in fonts.Families) if (f.Name == name) return f;
    throw new Exception("font not loaded: " + name);
  }
  static Color C(string hex) { return ColorTranslator.FromHtml(hex); }
  static Color Darker(Color c, float f) { return Color.FromArgb(c.A, (int)(c.R * f), (int)(c.G * f), (int)(c.B * f)); }
  static Bitmap NewBmp(float w, float h) { return new Bitmap((int)Math.Round(w * S), (int)Math.Round(h * S), PixelFormat.Format32bppArgb); }
  static Graphics G(Bitmap b) {
    var g = Graphics.FromImage(b);
    g.SmoothingMode = SmoothingMode.AntiAlias; g.InterpolationMode = InterpolationMode.HighQualityBicubic;
    g.PixelOffsetMode = PixelOffsetMode.HighQuality; g.CompositingQuality = CompositingQuality.HighQuality;
    g.ScaleTransform(S, S);
    return g;
  }
  static void Save(Bitmap b, string name) { b.Save(Path.Combine(outDir, name), ImageFormat.Png); b.Dispose(); }
  static Pen RoundPen(Color c, float w) { var p = new Pen(c, w); p.LineJoin = LineJoin.Round; p.StartCap = p.EndCap = LineCap.Round; return p; }

  static GraphicsPath RoundRect(float x, float y, float w, float h, float rTop, float rBottom) {
    var p = new GraphicsPath();
    if (rTop > 0) { p.AddArc(x, y, 2*rTop, 2*rTop, 180, 90); p.AddArc(x + w - 2*rTop, y, 2*rTop, 2*rTop, 270, 90); }
    else { p.AddLine(x, y, x + w, y); }
    if (rBottom > 0) { p.AddArc(x + w - 2*rBottom, y + h - 2*rBottom, 2*rBottom, 2*rBottom, 0, 90); p.AddArc(x, y + h - 2*rBottom, 2*rBottom, 2*rBottom, 90, 90); }
    else { p.AddLine(x + w, y + h, x, y + h); }
    p.CloseFigure();
    return p;
  }

  // Subtle print-like grain so flat color areas don't look digital.
  static void Grain(Bitmap b, int amount, int seed) {
    var rnd = new Random(seed);
    var data = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
    int n = data.Stride * b.Height; var px = new byte[n];
    System.Runtime.InteropServices.Marshal.Copy(data.Scan0, px, 0, n);
    for (int i = 0; i < n; i += 4) {
      if (px[i + 3] == 0) continue;
      int d = rnd.Next(-amount, amount + 1);
      for (int k = 0; k < 3; k++) px[i + k] = (byte)Math.Max(0, Math.Min(255, px[i + k] + d));
    }
    System.Runtime.InteropServices.Marshal.Copy(px, 0, data.Scan0, n);
    b.UnlockBits(data);
  }

  // ---------------------------------------------------------------- frame pieces

  // Art window mask: white where the art shows. Rounded top corners like the real cards.
  public static void ArtMask() {
    var b = NewBmp(700, 700);
    using (var g = G(b)) g.FillPath(Brushes.White, RoundRect(0, 0, 700, 700, 20, 0));
    Save(b, "art-mask.png");
  }

  // Rules text box for one card type: 700 x 297 units, black rule on top, rounded bottom corners.
  public static void TextBox(string type, string top, string bottom) {
    var b = NewBmp(700, 297);
    using (var g = G(b)) {
      var shape = RoundRect(0, 0, 700, 297, 0, 20);
      using (var br = new LinearGradientBrush(new PointF(0, 0), new PointF(0, 297), C(top), C(bottom))) g.FillPath(br, shape);
      // faint diagonal light streaks, as on the clean (TK802) frame
      g.SetClip(shape);
      using (var p = new Pen(Color.FromArgb(14, 255, 255, 255), 2f))
        for (int i = -300; i < 700; i += 37) g.DrawLine(p, i, 297, i + 120, 0);
      g.ResetClip();
      g.FillRectangle(Brushes.Black, 0, 0, 700, 3);
    }
    Grain(b, 5, type.GetHashCode());
    Save(b, "box-" + type + ".png");
  }

  // Darker strip at the top of the text box that holds the keyword/trait line.
  public static void Strip(string type, string color) {
    var b = NewBmp(700, 40);
    using (var g = G(b)) {
      using (var br = new LinearGradientBrush(new PointF(0, 0), new PointF(0, 40), C(color), Darker(C(color), 0.85f))) g.FillRectangle(br, 0, 0, 700, 40);
      g.FillRectangle(Brushes.Black, 0, 0, 700, 3);
    }
    Grain(b, 5, type.GetHashCode() + 1);
    Save(b, "strip-" + type + ".png");
  }

  // Rules text mask: white where text may go. The bottom-right corner is kept free for the check number.
  public static void RulesMask(string name, float w, float h, float cutW, float cutH) {
    var b = NewBmp(w, h);
    using (var g = G(b)) {
      g.Clear(Color.White);
      g.FillPath(Brushes.Black, RoundRect(w - cutW, h - cutH, cutW + 20, cutH + 20, 18, 0));
    }
    Save(b, name);
  }

  // ---------------------------------------------------------------- big numbers

  // One number glyph (difficulty or check) in a 126 x 130 unit box, plus a red "rim" image (recolor_image turns red into a color) that the
  // template tints with the card type color (the outer outline).
  public static void Number(string glyph, string fileId, string kind, string c1, string c2, string c3) {
    var fam = Find("Figtree Black");
    float em = 168f;
    var path = new GraphicsPath();
    path.AddString(glyph, fam, (int)FontStyle.Italic, em, new PointF(0, 0), StringFormat.GenericTypographic);
    var r = path.GetBounds();
    float boxW = 126, boxH = 130;
    // fit inside the box with room for the outlines (black 6 + rim 5 units on each side)
    float fit = Math.Min((boxW - 24) / r.Width, (boxH - 24) / r.Height);
    var m = new Matrix();
    m.Translate(boxW / 2, boxH / 2); m.Scale(fit, fit); m.Translate(-(r.X + r.Width / 2), -(r.Y + r.Height / 2));
    path.Transform(m);
    r = path.GetBounds();

    var b = NewBmp(boxW, boxH);
    using (var g = G(b)) {
      g.DrawPath(RoundPen(Color.Black, 12f), path);
      var blend = new ColorBlend(3);
      blend.Colors = new[] { C(c1), C(c2), C(c3) };
      blend.Positions = new[] { 0f, 0.5f, 1f };
      using (var br = new LinearGradientBrush(new PointF(0, r.Top), new PointF(0, r.Bottom), Color.Black, Color.Black)) {
        br.InterpolationColors = blend;
        g.FillPath(br, path);
      }
      // speed lines from the center, like the printed numbers
      g.SetClip(path);
      var center = new PointF(r.X + r.Width * 0.5f, r.Y + r.Height * 0.45f);
      using (var p = new Pen(Color.FromArgb(22, 255, 255, 255), 1f))
        for (int a = 0; a < 360; a += 9) {
          double rad = a * Math.PI / 180;
          g.DrawLine(p, center.X + (float)Math.Cos(rad) * 22, center.Y + (float)Math.Sin(rad) * 22,
                        center.X + (float)Math.Cos(rad) * 90, center.Y + (float)Math.Sin(rad) * 90);
        }
      g.ResetClip();
    }
    Save(b, "num-" + kind + "-" + fileId + ".png");

    if (kind == "d") { // rim: same for difficulty and check
      var rim = NewBmp(boxW, boxH);
      using (var g = G(rim)) { g.DrawPath(RoundPen(Color.Red, 22f), path); g.FillPath(Brushes.Red, path); }
      Save(rim, "num-rim-" + fileId + ".png");
    }
  }

  // ---------------------------------------------------------------- block shield

  static GraphicsPath ShieldPath() {
    var p = new GraphicsPath();
    p.AddBezier(4, 9, 18, 15, 32, 13, 43, 4);
    p.AddBezier(43, 4, 54, 13, 68, 15, 82, 9);
    p.AddLine(82, 9, 81, 44);
    p.AddBezier(81, 44, 79, 70, 62, 84, 43, 93);
    p.AddBezier(43, 93, 24, 84, 7, 70, 5, 44);
    p.CloseFigure();
    return p;
  }

  // Shield (116 x 100 units): zone-colored shield, three-slot tab with a "+" in the zone's slot.
  public static void Shield(string zone, string light, string dark, int slot) {
    var b = NewBmp(116, 100);
    var rim = NewBmp(116, 100);
    var shield = ShieldPath();
    var m = new Matrix(); m.Translate(6, 3); shield.Transform(m);
    var tab = RoundRect(72, 15, 38, 66, 4, 4);
    using (var g = G(b)) {
      // tab behind the shield
      g.FillPath(new SolidBrush(C("#2B2B2B")), tab);
      for (int i = 0; i < 3; i++) {
        var cell = RoundRect(86, 19 + i * 20.5f, 21, 17.5f, 2, 2);
        g.FillPath(new SolidBrush(i == slot ? C(light) : C("#6E6E6E")), cell);
        if (i == slot) {
          // white "+" with a thin black outline, centered in the cell
          float cx = 96.5f, cy = 27.75f + i * 20.5f, arm = 6f, half = 1.9f;
          var plus = new GraphicsPath();
          plus.AddPolygon(new[] {
            new PointF(cx - half, cy - arm), new PointF(cx + half, cy - arm), new PointF(cx + half, cy - half),
            new PointF(cx + arm, cy - half), new PointF(cx + arm, cy + half), new PointF(cx + half, cy + half),
            new PointF(cx + half, cy + arm), new PointF(cx - half, cy + arm), new PointF(cx - half, cy + half),
            new PointF(cx - arm, cy + half), new PointF(cx - arm, cy - half), new PointF(cx - half, cy - half) });
          var outline = new Pen(Color.Black, 1.6f); outline.LineJoin = LineJoin.Miter;
          g.DrawPath(outline, plus);
          g.FillPath(Brushes.White, plus);
        }
      }
      g.DrawPath(RoundPen(Color.Black, 5f), shield);
      using (var br = new LinearGradientBrush(new PointF(0, 6), new PointF(0, 96), C(light), C(dark))) g.FillPath(br, shield);
    }
    using (var g = G(rim)) {
      var w = RoundPen(Color.Red, 14f);
      g.DrawPath(w, shield); g.FillPath(Brushes.Red, shield);
      g.DrawPath(w, tab); g.FillPath(Brushes.Red, tab);
    }
    Save(b, "shield-" + zone + ".png");
    if (zone == "high") Save(rim, "shield-rim.png"); else rim.Dispose();
  }

  // ---------------------------------------------------------------- ability labels (symbol font images)

  public static void Label(string word, string fill) {
    var fam = Find("Barlow Condensed ExtraBold");
    float scale = 4f;                         // labels are drawn large; the symbol font scales them
    float em = 30f * scale;
    var path = new GraphicsPath();
    path.AddString(word, fam, (int)FontStyle.Regular, em, new PointF(0, 0), StringFormat.GenericTypographic);
    var r = path.GetBounds();
    float padX = 6 * scale, h = 30 * scale;
    int w = (int)Math.Ceiling(r.Width + 2 * padX);
    var b = new Bitmap(w, (int)h, PixelFormat.Format32bppArgb);
    using (var g = Graphics.FromImage(b)) {
      g.SmoothingMode = SmoothingMode.AntiAlias; g.PixelOffsetMode = PixelOffsetMode.HighQuality;
      g.FillRectangle(new SolidBrush(C(fill)), 0, 0, w, h);
      var m = new Matrix(); m.Translate(padX - r.X, (h - r.Height) / 2 - r.Y); path.Transform(m);
      g.FillPath(Brushes.Black, path);
    }
    Save(b, "label-" + word.ToLower() + ".png");
  }

  // ---------------------------------------------------------------- entry point

  public static void Run(string outPath, string fontDir) {
    outDir = outPath;
    foreach (var f in Directory.GetFiles(fontDir, "*.ttf")) fonts.AddFontFile(f);

    ArtMask();
    RulesMask("rules-mask.png", 660, 284, 128, 150);   // MSE tests only the top of each line, so the cut starts a line higher
    RulesMask("rules-mask-kw.png", 660, 248, 128, 150);

    // type colors: text box top, bottom, keyword strip
    string[][] types = {
      new[] { "foundation", "#A9AFB3", "#868D92", "#5B646B" },
      new[] { "action",     "#2E8BDB", "#1B5EA8", "#154A85" },
      new[] { "asset",      "#43AE52", "#24803A", "#1C5E2A" },
      new[] { "attack",     "#F39A30", "#DA661C", "#A8410F" },
      new[] { "backup",     "#E5489E", "#B82370", "#8A1752" },
    };
    foreach (var t in types) { TextBox(t[0], t[1], t[2]); Strip(t[0], t[3]); }

    // The num-d/c/rim digit images are no longer drawn here: they are the official FT01 glyphs,
    // cut by tools/uvs-study/make_digits.py (needs the git-ignored study/ scans) and committed as PNGs.

    Shield("high", "#F04A3C", "#B81F22", 0);
    Shield("mid",  "#FFA43A", "#E4701B", 1);
    Shield("low",  "#FFE03A", "#E8B417", 2);

    Label("ENHANCE",  "#FFE500");
    Label("RESPONSE", "#8CC63F");
    Label("FORM",     "#7DD3F5");
    Label("BLITZ",    "#E7A2D8");

    LogoRing();
    CommitIcon();

    // Character, Backup and Attack pieces
    CharacterArt();
    ArtBlank();
    BonusBrush();
    RulesMask("rules-mask-col.png", 660, 284, 128, 284);      // Backup / Attack: right column kept free for the stat stack
    RulesMask("rules-mask-col-kw.png", 660, 248, 128, 248);
    RulesMask("rules-mask-full.png", 544, 376, 0, 0);          // Character: no cut-out
    Heart("health", "#F2414B", "#B0141F");
    Heart("stamina", "#4DBE57", "#1F7A2E");
    HandSize();
    Speed("high", "#F04A3C", "#B81F22");
    Speed("mid",  "#FFA43A", "#E4701B");
    Speed("low",  "#FFE03A", "#E8B417");
    Damage();
    // Footer resource symbols (sym-*.png) come from tools/extract-uvs-symbols.ps1.
  }

  // Commit icon for rules text (typed as {C}): an upright green card turning into a sideways white card.
  public static void CommitIcon() {
    float sc = 4f;
    var b = new Bitmap((int)(34 * sc), (int)(32 * sc), PixelFormat.Format32bppArgb);
    using (var g = Graphics.FromImage(b)) {
      g.SmoothingMode = SmoothingMode.AntiAlias; g.PixelOffsetMode = PixelOffsetMode.HighQuality;
      g.ScaleTransform(sc, sc);
      var green = RoundRect(3.5f, 3f, 14f, 18.5f, 2.6f, 2.6f);
      var white = RoundRect(12.5f, 14f, 18.5f, 14.5f, 2.6f, 2.6f);
      var arrow = new GraphicsPath();
      arrow.AddBezier(9f, 9.5f, 15f, 8f, 23.5f, 9.5f, 24f, 18.5f);
      var head = new GraphicsPath();
      head.AddPolygon(new[] { new PointF(20f, 18f), new PointF(28f, 18f), new PointF(24f, 24.5f) });
      foreach (var p in new[] { green, white }) g.DrawPath(RoundPen(Color.White, 5.2f), p);
      foreach (var p in new[] { green, white }) g.DrawPath(RoundPen(Color.Black, 2.6f), p);
      using (var gb = new LinearGradientBrush(new PointF(0, 3), new PointF(0, 21), C("#2BC296"), C("#13886A"))) g.FillPath(gb, green);
      g.DrawPath(RoundPen(Color.Black, 2.6f), white);
      using (var wb = new LinearGradientBrush(new PointF(0, 14), new PointF(0, 28), Color.White, C("#CFCFCF"))) g.FillPath(wb, white);
      g.DrawPath(RoundPen(Color.Black, 3.8f), arrow);
      g.DrawPath(RoundPen(Color.White, 1.7f), arrow);
      g.FillPath(Brushes.Black, head);
    }
    Save(b, "icon-commit.png");
  }

  // White ring around the set's IP logo plus the divider bar after it (60 x 42 units, placed at card x 47, y 967).
  // Measured on FT01 2x scans: ring 41.5 across, 1.7 thick, centered at 68,988; bar 2.3 wide at x 102, y 974-1002.5.
  public static void LogoRing() {
    var b = NewBmp(60, 42);
    using (var g = G(b)) {
      g.FillEllipse(Brushes.Black, 0.85f, 0.85f, 39.8f, 39.8f);   // black inside, as on full-art cards
      g.DrawEllipse(new Pen(Color.White, 1.7f), 0.85f, 0.85f, 39.8f, 39.8f);
      g.FillRectangle(Brushes.White, 102 - 47 - 1.15f, 974 - 967, 2.3f, 28.5f);
    }
    Save(b, "logo-ring.png");
    // Mask for the set's IP logo (28 x 28 unit box at the ring's center): white inside the ring's inner edge (radius 19).
    var m = NewBmp(28, 28);
    using (var g = G(m)) { g.Clear(Color.Black); g.FillEllipse(Brushes.White, 14 - 19f, 14 - 19f, 38f, 38f); }
    Save(m, "logo-mask.png");
  }

  // ---------------------------------------------------------------- Character, Backup and Attack pieces

  // Characters are full art, to the card edge and into the bleed. Like the official cards there is no text
  // box: the text has a heavy outline and drop shadow, and this scrim only darkens the art under it a little
  // (bottom half, strongest on the left where the text is). Half the card's size; the style stretches it.
  public static void CharacterArt() {
    var m = NewBmp(40, 40);
    using (var g = G(m)) g.Clear(Color.White);
    Save(m, "mask-white.png");
    var b = NewBmp(372, 520);
    for (int y = 0; y < b.Height; y++) {
      double v = (double)y / (b.Height - 1);
      double t = Math.Max(0, Math.Min(1, (v - 0.46) / 0.34));
      double va = t * t * (3 - 2 * t) * 150 + Math.Max(0, v - 0.8) / 0.2 * 30;
      for (int x = 0; x < b.Width; x++) {
        double u = (double)x / (b.Width - 1);
        double s = Math.Max(0, Math.Min(1, (u - 0.5) / 0.5));
        double ha = 1 - 0.55 * s * s * (3 - 2 * s);
        b.SetPixel(x, y, Color.FromArgb((int)Math.Round(va * ha), 0, 0, 0));
      }
    }
    Save(b, "shade-character.png");
  }

  // Backdrop for a card with no art yet: plain charcoal, a little lighter at the top. Without it an empty
  // art box exports as the editor's checkerboard. Small; the style stretches it.
  public static void ArtBlank() {
    var b = NewBmp(50, 70);
    using (var g = G(b))
    using (var br = new LinearGradientBrush(new RectangleF(0, 0, 50, 70), C("#3c3c41"), C("#1e1e21"), 90f))
      g.FillRectangle(br, 0, 0, 50, 70);
    Save(b, "art-blank.png");
    // Fully transparent: the image of a field that is only a click target (the Character card type).
    Save(NewBmp(2, 2), "blank.png");
  }

  // ---------------------------------------------------------------- Bonus Stage character theme
  // After the Bonus Stage webtoon art (salmon two-tone, thick black ink): a torn salmon brush swatch behind
  // the name, like the ink swatches behind the names on FT01 / SF6 Characters.
  // 600 x 120 units, stretched to the name's width and line count by the style.
  public static void BonusBrush() {
    var r = new Random(26);
    var b = NewBmp(600, 120);
    Func<float, float, PointF[]> shape = (dx, dy) => {
      var pts = new System.Collections.Generic.List<PointF>();
      for (float x = 18; x <= 560; x += 12) pts.Add(new PointF(x + dx, 14 + (float)r.NextDouble() * 9 + dy));      // top edge
      for (float y = 14; y <= 106; y += 8) pts.Add(new PointF(560 + (float)r.NextDouble() * 36 + dx, y + dy));     // ragged right end
      for (float x = 560; x >= 18; x -= 12) pts.Add(new PointF(x + dx, 98 + (float)r.NextDouble() * 12 + dy));     // bottom edge
      for (float y = 106; y >= 14; y -= 8) pts.Add(new PointF(4 + (float)r.NextDouble() * 18 + dx, y + dy));       // left end
      return pts.ToArray();
    };
    using (var g = G(b)) {
      g.FillPolygon(new SolidBrush(Color.FromArgb(24, 18, 22)), shape(5, 6));           // ink drop behind
      var path = new GraphicsPath(); path.AddPolygon(shape(0, 0));
      using (var br = new LinearGradientBrush(new PointF(0, 9), new PointF(0, 113), Color.FromArgb(250, 132, 126), Color.FromArgb(222, 84, 88))) g.FillPath(br, path);
      g.SetClip(path);
      // dry-brush streaks along the stroke
      for (int i = 0; i < 22; i++) {
        float y = 14 + (float)r.NextDouble() * 92, x = (float)r.NextDouble() * 300, w = 120 + (float)r.NextDouble() * 300;
        var c = r.Next(2) == 0 ? Color.FromArgb(90, 255, 190, 180) : Color.FromArgb(70, 170, 40, 50);
        using (var p = new Pen(c, 1.5f + (float)r.NextDouble() * 3)) g.DrawLine(p, x, y, x + w, y + (float)r.NextDouble() * 4 - 2);
      }
      // white cel-shading slashes, as in the art
      using (var w = new SolidBrush(Color.FromArgb(225, 255, 250, 245))) {
        g.FillPolygon(w, new[] { new PointF(470, 0), new PointF(500, 0), new PointF(456, 120), new PointF(426, 120) });
        g.FillPolygon(w, new[] { new PointF(512, 0), new PointF(522, 0), new PointF(478, 120), new PointF(468, 120) });
      }
      g.ResetClip();
      // splatter off the right end
      using (var ink = new SolidBrush(Color.FromArgb(232, 96, 98)))
        for (int i = 0; i < 9; i++) { float d = 3 + (float)r.NextDouble() * 9; g.FillEllipse(ink, 570 + (float)r.NextDouble() * 26, 10 + (float)r.NextDouble() * 96, d, d); }
    }
    Save(b, "bonus-brush.png");
  }

  static GraphicsPath HeartPath(float w, float h) {
    // symmetric heart in a w x h box
    var p = new GraphicsPath();
    float cx = w / 2;
    p.AddBezier(cx, h * 0.27f, cx - w * 0.06f, h * 0.02f, 0, -h * 0.02f, w * 0.02f, h * 0.36f);
    p.AddBezier(w * 0.02f, h * 0.36f, w * 0.05f, h * 0.62f, cx - w * 0.2f, h * 0.78f, cx, h);
    p.AddBezier(cx, h, cx + w * 0.2f, h * 0.78f, w * 0.95f, h * 0.62f, w * 0.98f, h * 0.36f);
    p.AddBezier(w * 0.98f, h * 0.36f, w, -h * 0.02f, cx + w * 0.06f, h * 0.02f, cx, h * 0.27f);
    p.CloseFigure();
    return p;
  }

  // Heart badge (116 x 100 units): health (red, Character) and stamina (green, Backup). Plus a red rim image
  // that the template tints with the card type color.
  public static void Heart(string id, string light, string dark) {
    var b = NewBmp(116, 100); var rim = NewBmp(116, 100);
    var heart = HeartPath(92, 80); var m = new Matrix(); m.Translate(12, 9); heart.Transform(m);
    using (var g = G(b)) {
      g.DrawPath(RoundPen(Color.Black, 6f), heart);
      using (var br = new LinearGradientBrush(new PointF(0, 9), new PointF(0, 89), C(light), C(dark))) g.FillPath(br, heart);
      // soft highlight on the upper left lobe
      using (var hl = new SolidBrush(Color.FromArgb(55, 255, 255, 255))) g.FillEllipse(hl, 24, 17, 26, 16);
    }
    using (var g = G(rim)) { var w = RoundPen(Color.Red, 16f); g.DrawPath(w, heart); g.FillPath(Brushes.Red, heart); }
    Save(b, "heart-" + id + ".png");
    if (id == "health") Save(rim, "heart-rim.png"); else rim.Dispose();
  }

  // Hand size badge (116 x 112 units): two green cards fanned out, like the official card-stack icon.
  public static void HandSize() {
    var b = NewBmp(116, 112); var rim = NewBmp(116, 112);
    var back = RoundRect(0, 0, 52, 72, 7, 7); var mb = new Matrix(); mb.Translate(26, 20); mb.RotateAt(-14, new PointF(26, 72)); back.Transform(mb);
    var front = RoundRect(0, 0, 56, 76, 7, 7); var mf = new Matrix(); mf.Translate(36, 24); mf.RotateAt(8, new PointF(28, 76)); front.Transform(mf);
    using (var g = G(b)) {
      foreach (var p in new[] { back, front }) {
        g.DrawPath(RoundPen(Color.Black, 6f), p);
        using (var br = new LinearGradientBrush(new PointF(0, 16), new PointF(0, 104), C("#3CC79E"), C("#16805F"))) g.FillPath(br, p);
        using (var pen = new Pen(Color.FromArgb(120, 210, 255, 235), 2f)) {
          var inner = (GraphicsPath)p.Clone(); var mi = new Matrix();
          var r = p.GetBounds(); mi.Translate(r.X + r.Width / 2, r.Y + r.Height / 2); mi.Scale(0.8f, 0.85f); mi.Translate(-(r.X + r.Width / 2), -(r.Y + r.Height / 2));
          inner.Transform(mi); g.DrawPath(pen, inner);
        }
      }
    }
    using (var g = G(rim)) { var w = RoundPen(Color.Red, 16f); foreach (var p in new[] { back, front }) { g.DrawPath(w, p); g.FillPath(Brushes.Red, p); } }
    Save(b, "hand-size.png"); Save(rim, "hand-size-rim.png");
  }

  // Speed badge (116 x 100 units): a circle in the attack zone's color with two arrow tips on its right side.
  public static void Speed(string zone, string light, string dark) {
    var b = NewBmp(116, 100); var rim = NewBmp(116, 100);
    var circle = new GraphicsPath(); circle.AddEllipse(14, 9, 82, 82);
    var arrows = new GraphicsPath();
    arrows.AddPolygon(new[] { new PointF(84, 22), new PointF(108, 12), new PointF(100, 38) });
    arrows.AddPolygon(new[] { new PointF(84, 78), new PointF(108, 88), new PointF(100, 62) });
    using (var g = G(b)) {
      g.FillPath(Brushes.Black, arrows);
      g.DrawPath(RoundPen(Color.Black, 6f), circle);
      using (var br = new LinearGradientBrush(new PointF(0, 9), new PointF(0, 91), C(light), C(dark))) g.FillPath(br, circle);
      using (var hl = new SolidBrush(Color.FromArgb(50, 255, 255, 255))) g.FillEllipse(hl, 30, 16, 44, 22);
    }
    using (var g = G(rim)) {
      var w = RoundPen(Color.Red, 14f);
      g.DrawPath(w, circle); g.FillPath(Brushes.Red, circle); g.DrawPath(w, arrows); g.FillPath(Brushes.Red, arrows);
    }
    Save(b, "speed-" + zone + ".png");
    if (zone == "high") Save(rim, "speed-rim.png"); else rim.Dispose();
  }

  // Damage badge (116 x 100 units): a yellow starburst.
  public static void Damage() {
    var b = NewBmp(116, 100); var rim = NewBmp(116, 100);
    var star = new GraphicsPath(); int n = 12; var pts = new PointF[n * 2];
    for (int i = 0; i < n * 2; i++) {
      double a = Math.PI * i / n - Math.PI / 2; float r = (i % 2 == 0) ? 46f : 31f;
      pts[i] = new PointF(58 + (float)Math.Cos(a) * r * 1.05f, 50 + (float)Math.Sin(a) * r);
    }
    star.AddPolygon(pts);
    using (var g = G(b)) {
      var pen = new Pen(Color.Black, 6f); pen.LineJoin = LineJoin.Miter; g.DrawPath(pen, star);
      using (var br = new LinearGradientBrush(new PointF(0, 4), new PointF(0, 96), C("#FFF07A"), C("#F5A81C"))) g.FillPath(br, star);
    }
    using (var g = G(rim)) { var w = new Pen(Color.Red, 14f); w.LineJoin = LineJoin.Round; g.DrawPath(w, star); g.FillPath(Brushes.Red, star); }
    Save(b, "damage.png"); Save(rim, "damage-rim.png");
  }

  // Package icon (the check "5"), copied into the game and style packages.
  public static void Icon(string path) {
    using (var src = new Bitmap(Path.Combine(outDir, "num-c-5.png")))
    using (var b = new Bitmap(64, 64, PixelFormat.Format32bppArgb))
    using (var g = Graphics.FromImage(b)) {
      g.InterpolationMode = InterpolationMode.HighQualityBicubic;
      float k = 64f / Math.Max(src.Width, src.Height); // keep the digit's proportions
      float w = src.Width * k, h = src.Height * k;
      g.DrawImage(src, (64 - w) / 2, (64 - h) / 2, w, h);
      b.Save(path, ImageFormat.Png);
    }
  }
}
'@

[UvsAssets]::Run($Out, $Fonts)
$templates = Split-Path $Out
foreach ($pkg in "uvs.mse-game", "uvs-2026.mse-style", "uvs-2026-exact.mse-style", "uvs-2026-mpc.mse-style") {
  [UvsAssets]::Icon((Join-Path $templates "$pkg\icon.png"))
}
"assets written to $Out"
Get-ChildItem $Out -Filter *.png | Measure-Object | ForEach-Object { "$($_.Count) png files" }
