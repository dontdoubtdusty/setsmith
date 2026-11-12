# Setsmith: cuts the 13 official UniVersus resource symbols out of the How to Play guide (page 21, vector art),
# for private use only. Writes sym-<name>.png (circle) and sym-at_<name>.png (attuned, rounded square) into the
# UVS common include, 164 px = 41 card units (4x).
#
# Air and Water attuned are cut from the guide too. The other attuned symbols are built from the circle art:
# the inside of the black ring is mapped onto the square, with the ring's edge colors stretched into the corners.
# That matches the attuned symbols printed on FT01 cards (Earth, Void).
#
#   powershell -ExecutionPolicy Bypass -File tools\extract-uvs-symbols.ps1 [-Pdf How_to_Play-2025.pdf]
param(
  [string]$Pdf = "",
  [string]$Out = (Join-Path $PSScriptRoot "..\templates\uvs-2026-common.mse-include"),
  [string]$Poppler = "D:\msys64\ucrt64\bin"
)
$ErrorActionPreference = "Stop"
$work = Join-Path $env:TEMP "uvs-symbols"
New-Item -ItemType Directory -Force $work | Out-Null
if ($Pdf -eq "") {
  $Pdf = Join-Path $work "How_to_Play-2025.pdf"
  if (-not (Test-Path $Pdf)) {
    Invoke-WebRequest "https://uvsgames.com/wp-content/uploads/2025/09/How_to_Play-2025.pdf" -OutFile $Pdf -UseBasicParsing
  }
}
# The symbol grid at 1200 dpi (the page is 11 x 8.5 in).
$png = Join-Path $work "grid"
& "$Poppler\pdftoppm.exe" -f 21 -l 21 -r 1200 -x 6500 -y 4300 -W 2500 -H 4400 -png -singlefile $Pdf $png
if ($LASTEXITCODE -ne 0) { throw "pdftoppm failed" }

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;

public static class UvsSymbols {
  const int OUT = 164;           // output canvas, px (41 card units at 4x)
  const int SS = 4;              // supersampling before the final downscale
  const double CIRCLE = 40.0 / 41.0;  // circle 40 units (incl. halo), attuned square 41 units, measured on FT01 cards
  static int W, H; static byte[] px; static int stride;

  static double Lum(int x, int y) {
    int i = y * stride + x * 4;
    return 0.3 * px[i + 2] + 0.59 * px[i + 1] + 0.11 * px[i];
  }
  static Color Px(double x, double y) {   // bilinear
    int x0 = (int)Math.Floor(x), y0 = (int)Math.Floor(y); double fx = x - x0, fy = y - y0;
    double r = 0, g = 0, b = 0;
    for (int dy = 0; dy < 2; dy++) for (int dx = 0; dx < 2; dx++) {
      double w = (dx == 0 ? 1 - fx : fx) * (dy == 0 ? 1 - fy : fy);
      int i = (y0 + dy) * stride + (x0 + dx) * 4;
      b += w * px[i]; g += w * px[i + 1]; r += w * px[i + 2];
    }
    return Color.FromArgb(255, (int)Math.Round(r), (int)Math.Round(g), (int)Math.Round(b));
  }

  // Distance from (x,y) stepping (dx,dy) until the first dark pixel, with sub-pixel interpolation.
  static double EdgeFrom(int x, int y, int dx, int dy, double thr) {
    double prev = Lum(x, y);
    for (int k = 1; k < 600; k++) {
      double l = Lum(x + dx * k, y + dy * k);
      if (l < thr) return k - 1 + (prev - thr) / Math.Max(1e-6, prev - l);
      prev = l;
    }
    throw new Exception("edge not found");
  }
  // Ring thickness: from the outer edge inward, how long it stays dark.
  static double DarkRun(double x, double y, int dx, int dy, double thr) {
    int k = 0; while (k < 200 && Lum((int)Math.Round(x + dx * (k + 1)), (int)Math.Round(y + dy * (k + 1))) < thr) k++;
    return k;
  }

  struct Shape { public double cx, cy, rx, ry, ring; }

  // Black outline of a symbol near (ax, ay): the outer edge on all four sides.
  static Shape Find(int ax, int ay) {
    const int R = 270; const double thr = 70;
    double l = EdgeFrom(ax - R, ay, 1, 0, thr), r = EdgeFrom(ax + R, ay, -1, 0, thr);
    double t = EdgeFrom(ax, ay - R, 0, 1, thr), b = EdgeFrom(ax, ay + R, 0, -1, thr);
    double left = ax - R + l, right = ax + R - r, top = ay - R + t, bottom = ay + R - b;
    var s = new Shape { cx = (left + right) / 2, cy = (top + bottom) / 2, rx = (right - left) / 2, ry = (bottom - top) / 2 };
    // ring thickness, smallest of the four sides (a glyph touching the ring makes one side look thicker)
    double cy = s.cy, cx = s.cx;
    s.ring = Math.Min(Math.Min(DarkRun(left, cy, 1, 0, 45), DarkRun(right, cy, -1, 0, 45)),
                      Math.Min(DarkRun(cx, top, 0, 1, 45), DarkRun(cx, bottom, 0, -1, 45)));
    return s;
  }

  // White halo outside the black outline: 18 px at 1200 dpi on a 227.7 px radius, measured on the guide.
  static double Halo(Shape s) { return 0.079 * s.rx; }

  static void SaveDown(Bitmap big, string path) {
    using (var b = new Bitmap(OUT, OUT, PixelFormat.Format32bppArgb))
    using (var g = Graphics.FromImage(b)) {
      g.InterpolationMode = InterpolationMode.HighQualityBicubic; g.PixelOffsetMode = PixelOffsetMode.HighQuality;
      g.CompositingMode = CompositingMode.SourceCopy;
      g.DrawImage(big, new Rectangle(0, 0, OUT, OUT));
      b.Save(path, ImageFormat.Png);
    }
    big.Dispose();
  }

  static double Clamp01(double v) { return v < 0 ? 0 : v > 1 ? 1 : v; }

  // Signed distance to a rounded square of half-size h and corner radius c (negative inside).
  static double RoundSquareDist(double x, double y, double h, double c) {
    double qx = Math.Abs(x) - (h - c), qy = Math.Abs(y) - (h - c);
    double ox = Math.Max(qx, 0), oy = Math.Max(qy, 0);
    return Math.Sqrt(ox * ox + oy * oy) + Math.Min(Math.Max(qx, qy), 0) - c;
  }

  // Circle symbol: the guide's pixels inside the black outline, a clean white halo outside it.
  static void Circle(Shape s, string path) {
    int N = OUT * SS; var big = new Bitmap(N, N, PixelFormat.Format32bppArgb);
    double rOut = s.rx + Halo(s);                 // source px
    double scale = rOut / (N * CIRCLE / 2);       // source px per canvas px
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
      double ux = (x + 0.5 - N / 2.0) * scale, uy = (y + 0.5 - N / 2.0) * scale;
      double d = Math.Sqrt(ux * ux + uy * uy);
      double a = Clamp01((rOut - d) / scale + 0.5);
      if (a <= 0) { big.SetPixel(x, y, Color.Transparent); continue; }
      Color c = d <= s.rx + 1 ? Px(s.cx + ux, s.cy + uy) : Color.White;
      big.SetPixel(x, y, Color.FromArgb((int)Math.Round(a * 255), c));
    }
    SaveDown(big, path);
  }

  // Attuned symbol cut from the guide: the square outline, plus a clean white halo.
  static void SquareCut(Shape s, string path) {
    int N = OUT * SS; var big = new Bitmap(N, N, PixelFormat.Format32bppArgb);
    double half = Math.Max(s.rx, s.ry) + Halo(s);
    double scale = half / (N / 2.0);
    double corner = 0.13 * half;
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
      double ux = (x + 0.5 - N / 2.0) * scale, uy = (y + 0.5 - N / 2.0) * scale;
      double dist = RoundSquareDist(ux, uy, half, corner);
      double a = Clamp01(-dist / scale + 0.5);
      if (a <= 0) { big.SetPixel(x, y, Color.Transparent); continue; }
      bool inside = Math.Abs(ux) <= s.rx + 1 && Math.Abs(uy) <= s.ry + 1 && RoundSquareDist(ux, uy, Math.Max(s.rx, s.ry), corner * 0.8) <= 1;
      Color c = inside ? Px(s.cx + ux, s.cy + uy) : Color.White;
      big.SetPixel(x, y, Color.FromArgb((int)Math.Round(a * 255), c));
    }
    SaveDown(big, path);
  }

  // Attuned symbol built from a circle: white rim, black border, and the circle's inside mapped onto the square.
  // Measured on FT01 cards (41 units): white rim 1.5, black border 1.4, outer corner radius 5.
  static void SquareFromCircle(Shape s, string path) {
    int N = OUT * SS; var big = new Bitmap(N, N, PixelFormat.Format32bppArgb);
    double u = N / 41.0;                               // canvas px per card unit
    double hOut = N / 2.0, hBlack = hOut - 1.5 * u, hIn = hBlack - 1.4 * u;
    double cOut = 5 * u, cBlack = cOut - 1.5 * u, cIn = Math.Max(1, cBlack - 1.4 * u);
    double rIn = s.rx - s.ring - 7;                    // inside of the black ring, past its dark inner edge, source px
    double map = rIn / hIn;                            // source px per canvas px
    // Corner colors: per-channel median of the colors just inside the ring over +-25 degrees, so thin glyph
    // tips that touch the ring don't streak into the corners, while wide areas (Earth's black base) carry over.
    var edge = new Color[360];
    for (int d = 0; d < 360; d++) { double t = d * Math.PI / 180; edge[d] = Px(s.cx + Math.Cos(t) * (rIn - 3), s.cy + Math.Sin(t) * (rIn - 3)); }
    var corner = new Color[360];
    for (int d = 0; d < 360; d++) {
      var rs = new int[51]; var gs = new int[51]; var bs = new int[51];
      for (int k = -25; k <= 25; k++) { var e = edge[(d + k + 360) % 360]; rs[k + 25] = e.R; gs[k + 25] = e.G; bs[k + 25] = e.B; }
      Array.Sort(rs); Array.Sort(gs); Array.Sort(bs);
      corner[d] = Color.FromArgb(255, rs[25], gs[25], bs[25]);
    }
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
      double vx = x + 0.5 - N / 2.0, vy = y + 0.5 - N / 2.0;
      double dOut = RoundSquareDist(vx, vy, hOut, cOut);
      double a = Clamp01(0.5 - dOut);
      if (a <= 0) { big.SetPixel(x, y, Color.Transparent); continue; }
      double dB = RoundSquareDist(vx, vy, hBlack, cBlack), dI = RoundSquareDist(vx, vy, hIn, cIn);
      // inside: sample the circle, clamped to just inside its ring so the corners take the edge colors
      double sx = vx * map, sy = vy * map, r = Math.Sqrt(sx * sx + sy * sy);
      Color inner;
      if (r <= rIn) inner = Px(s.cx + sx, s.cy + sy);
      else {
        // outside the circle: fade from the clamped edge pixel to the median edge color within 4% of the radius
        Color e = Px(s.cx + sx * rIn / r, s.cy + sy * rIn / r);
        int deg = ((int)Math.Round(Math.Atan2(sy, sx) * 180 / Math.PI) + 360) % 360;
        Color m = corner[deg];
        double f = Clamp01((r - rIn) / (0.04 * rIn));
        inner = Color.FromArgb(255, (int)(e.R + (m.R - e.R) * f), (int)(e.G + (m.G - e.G) * f), (int)(e.B + (m.B - e.B) * f));
      }
      double tWhite = Clamp01(0.5 + dB), tBlack = Clamp01(0.5 + dI);   // 1 = fully outside that edge
      double R = inner.R * (1 - tBlack), G = inner.G * (1 - tBlack), B = inner.B * (1 - tBlack); // blend to black
      R = R * (1 - tWhite) + 255 * tWhite; G = G * (1 - tWhite) + 255 * tWhite; B = B * (1 - tWhite) + 255 * tWhite;
      big.SetPixel(x, y, Color.FromArgb((int)Math.Round(a * 255), (int)R, (int)G, (int)B));
    }
    SaveDown(big, path);
  }

  public static void Run(string grid, string outDir) {
    using (var src = new Bitmap(grid)) {
      W = src.Width; H = src.Height;
      var data = src.LockBits(new Rectangle(0, 0, W, H), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
      stride = data.Stride; px = new byte[stride * H];
      System.Runtime.InteropServices.Marshal.Copy(data.Scan0, px, 0, px.Length);
      src.UnlockBits(data);
    }
    // approximate centers in the 1200 dpi crop (3 columns x 5 rows)
    int[] cols = { 479, 1192, 1911 }; int[] rows = { 384, 1235, 2092, 2946, 3809 };
    string[,] grid5 = {
      { "air", "all", "chaos" }, { "death", "earth", "evil" }, { "fire", "good", "life" },
      { "water", "void", "order" }, { "infinity", "at_air", "at_water" } };
    var shapes = new System.Collections.Generic.Dictionary<string, Shape>();
    for (int r = 0; r < 5; r++) for (int c = 0; c < 3; c++) {
      var s = Find(cols[c], rows[r]);
      shapes[grid5[r, c]] = s;
      Console.WriteLine(string.Format("{0,-10} center {1:F1},{2:F1}  r {3:F1} x {4:F1}  ring {5:F0}", grid5[r, c], s.cx, s.cy, s.rx, s.ry, s.ring));
    }
    foreach (var kv in shapes) {
      if (kv.Key.StartsWith("at_")) SquareCut(kv.Value, Path.Combine(outDir, "sym-" + kv.Key + ".png"));
      else {
        Circle(kv.Value, Path.Combine(outDir, "sym-" + kv.Key + ".png"));
        if (kv.Key != "air" && kv.Key != "water")
          SquareFromCircle(kv.Value, Path.Combine(outDir, "sym-at_" + kv.Key + ".png"));
      }
    }
  }
}
'@
[UvsSymbols]::Run("$png.png", (Resolve-Path $Out).Path)
"symbols written to $Out"
