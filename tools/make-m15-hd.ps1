<#
.SYNOPSIS
  Builds double-size (744 x 1039) frames, crowns and P/T boxes for the M15 style.

.DESCRIPTION
  The M15 Magic Pack only ships the normal M15 frame at 375 x 523 (cards\375 m15 simple, crowns\375,
  pts\375 m15). It also ships a 744 x 1039 M15 kit in separate layers (trims, pinlines, name, type,
  text and P/T boxes, crowns, borders) that no style uses. This script stacks those layers into
  whole images with the same names the M15 scripts expect, in new folders of magic-modules.mse-include:
    cards\744 m15 hd\   {w,u,b,r,g,a,c,m}card.png, {w,u,b,r,g,c,m}lcard.png, blend masks
    crowns\744 m15 hd\  {w,u,b,r,g,a,c,m}crown.png, blend masks, the 375 variant folders enlarged
    pts\744 m15 hd\     {w,u,b,r,g,a,c,m,v}pt.png, blend masks
  Blend masks are the 375 ones enlarged, because masked_blend needs every image the same size.
  Called by make-slim-data.ps1; the output is not committed.
#>
param(
  [string]$Source = "D:\Program Files\Magic Set Editor\M15-Magic-Pack-main\data",
  [string]$Output = (Join-Path $PSScriptRoot "..\build\data")
)
$ErrorActionPreference = "Stop"
$src = Join-Path $Source "magic-modules.mse-include"
$dst = Join-Path (Resolve-Path $Output).Path "magic-modules.mse-include"
Add-Type -AssemblyName System.Drawing

if (-not ("M15Hd" -as [type])) {
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

// Pixel helpers for stacking frame layers. Pixels are BGRA byte arrays.
public static class M15Hd {
    // Loads an image as BGRA, scaled to w x h, optionally cropped first to (cx, cy, cw, ch).
    public static byte[] Load(string path, int w, int h, int cx = 0, int cy = 0, int cw = 0, int ch = 0) {
        using (var img = Image.FromFile(path))
        using (var bmp = new Bitmap(w, h, PixelFormat.Format32bppArgb)) {
            using (var g = Graphics.FromImage(bmp)) {
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.PixelOffsetMode = PixelOffsetMode.HighQuality;
                var srcRect = cw > 0 ? new Rectangle(cx, cy, cw, ch) : new Rectangle(0, 0, img.Width, img.Height);
                using (var ia = new ImageAttributes()) {
                    ia.SetWrapMode(WrapMode.TileFlipXY);   // no dark fringe at the image edge when scaling
                    g.DrawImage(img, new Rectangle(0, 0, w, h), srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, GraphicsUnit.Pixel, ia);
                }
            }
            var d = bmp.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            var b = new byte[w * h * 4]; Marshal.Copy(d.Scan0, b, 0, b.Length); bmp.UnlockBits(d); return b;
        }
    }

    // Draws src over dst. With a mask, src only shows where the mask is white (MSE mask convention).
    public static void Over(byte[] dst, byte[] src, byte[] mask) {
        for (int i = 0; i < dst.Length; i += 4) {
            double a = src[i + 3] / 255.0;
            if (mask != null) a *= mask[i + 2] / 255.0 * (mask[i + 3] / 255.0);
            if (a <= 0) continue;
            double da = dst[i + 3] / 255.0, oa = a + da * (1 - a);
            for (int c = 0; c < 3; c++) dst[i + c] = (byte)Math.Round((src[i + c] * a + dst[i + c] * da * (1 - a)) / oa);
            dst[i + 3] = (byte)Math.Round(oa * 255);
        }
    }

    // A solid opaque color, used with a mask for the black border and pinline shadows.
    public static byte[] Solid(int w, int h, byte r, byte g, byte b) {
        var o = new byte[w * h * 4];
        for (int i = 0; i < o.Length; i += 4) { o[i] = b; o[i + 1] = g; o[i + 2] = r; o[i + 3] = 255; }
        return o;
    }

    // Copies row `from` over rows y0..y1, outside the middle columns xa..xb (the side lines only).
    public static void RepeatRow(byte[] px, int w, int from, int y0, int y1, int xa, int xb) {
        for (int y = y0; y <= y1; y++)
            for (int x = 0; x < w; x++) {
                if (x > xa && x < xb) continue;
                Array.Copy(px, (from * w + x) * 4, px, (y * w + x) * 4, 4);
            }
    }

    // Coverage (0..1) of a layer: its alpha, or for a mask image its brightness times alpha.
    public static double[] Cover(byte[] px, bool isMask) {
        var o = new double[px.Length / 4];
        for (int i = 0; i < o.Length; i++)
            o[i] = (isMask ? px[i * 4 + 2] / 255.0 : 1.0) * (px[i * 4 + 3] / 255.0);
        return o;
    }

    // Makes px transparent where the hole coverage is (used to open the name box in the crown).
    public static void Cut(byte[] px, double[] hole) {
        for (int i = 0; i < hole.Length; i++)
            px[i * 4 + 3] = (byte)Math.Round(px[i * 4 + 3] * (1 - hole[i]));
    }

    // Writes a coverage map as an opaque grey image (white = 1), the form MSE blend masks take.
    public static byte[] Grey(double[] v) {
        var o = new byte[v.Length * 4];
        for (int i = 0; i < v.Length; i++) {
            byte g = (byte)Math.Round(Math.Max(0, Math.Min(1, v[i])) * 255);
            o[i * 4] = g; o[i * 4 + 1] = g; o[i * 4 + 2] = g; o[i * 4 + 3] = 255;
        }
        return o;
    }

    // Repaints the crown with the frame trim's crisp texture. The kit's crowns are soft upscales with big
    // light blotches (one smears the right end), while the trim has the printed grain. The trim's side
    // strip (columns sx..sx+sw, rows sy..sy+sh) is laid on its side in overlapping bands across the crown;
    // the crown keeps only its fine shading (brightness against a small blur: the bevel and edge lines).
    public static void Retexture(byte[] crown, byte[] trim, int w, int h, int sx, int sw, int sy, int sh) {
        int step = sw - 4;                                   // 4 px overlap between bands, cross-faded
        var lum = new double[w * h];
        for (int i = 0; i < lum.Length; i++)
            lum[i] = (0.299 * crown[i * 4 + 2] + 0.587 * crown[i * 4 + 1] + 0.114 * crown[i * 4]) * (crown[i * 4 + 3] / 255.0);
        var alpha = new double[w * h];
        for (int i = 0; i < alpha.Length; i++) alpha[i] = crown[i * 4 + 3] / 255.0;
        var bl = BoxBlur(lum, w, h, 5); var ba = BoxBlur(alpha, w, h, 5);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                int i = y * w + x;
                if (crown[i * 4 + 3] == 0) continue;
                double[] col = new double[3]; double wsum = 0;
                int k0 = Math.Max(0, (y - sw) / step);
                for (int k = k0; k <= y / step; k++) {
                    int v = y - k * step; if (v < 0 || v >= sw) continue;
                    double wt = Math.Min(1.0, Math.Min(v + 1, sw - v) / 4.0);
                    int row = sy + ((x + k * 211) % sh);         // shift each band so the grain does not repeat
                    int j = (row * w + sx + v) * 4;
                    for (int c = 0; c < 3; c++) col[c] += trim[j + c] * wt;
                    wsum += wt;
                }
                // Where the trim itself lies under the crown (the pillars beside the art), use it directly.
                double ta = trim[i * 4 + 3] / 255.0;
                for (int c = 0; c < 3; c++) col[c] = col[c] / wsum * (1 - ta) + trim[i * 4 + c] * ta;
                wsum = 1;
                double mean = ba[i] > 0 ? bl[i] / ba[i] : 0;
                double f = mean > 1 ? Math.Max(0.45, Math.Min(1.6, lum[i] / alpha[i] / mean)) : 1;
                for (int c = 0; c < 3; c++) crown[i * 4 + c] = (byte)Math.Max(0, Math.Min(255, Math.Round(col[c] / wsum * f)));
            }
    }

    // Box blur of a value map with radius r (separable, edges clamped).
    static double[] BoxBlur(double[] v, int w, int h, int r) {
        var t = new double[v.Length]; var o = new double[v.Length];
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                double s = 0; for (int d = -r; d <= r; d++) s += v[y * w + Math.Max(0, Math.Min(w - 1, x + d))];
                t[y * w + x] = s / (2 * r + 1);
            }
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                double s = 0; for (int d = -r; d <= r; d++) s += t[Math.Max(0, Math.Min(h - 1, y + d)) * w + x];
                o[y * w + x] = s / (2 * r + 1);
            }
        return o;
    }

    public static void Save(byte[] px, int w, int h, string path) {
        using (var bmp = new Bitmap(w, h, PixelFormat.Format32bppArgb)) {
            var d = bmp.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
            Marshal.Copy(px, 0, d.Scan0, px.Length); bmp.UnlockBits(d);
            bmp.Save(path, ImageFormat.Png);
        }
    }
}
'@
}

$W = 744; $H = 1039
$kit = "$src\{0}\744x1039\m15\normal"   # layer folder pattern
function L([string]$part) { $kit -f $part }
function Need([string]$path) { if (-not (Test-Path $path)) { throw "Missing layer: $path" }; $path }

# Stacks a list of layers into one image. A layer is a path, "fill:<mask>" (black where the mask is
# white) or "tex:<texture>|<mask>" (the texture where the mask is white).
function Stack([string]$out, [string[]]$layers) {
  $canvas = New-Object byte[] ($W * $H * 4)
  foreach ($l in $layers) {
    if ($l -eq "black") {
      [M15Hd]::Over($canvas, [M15Hd]::Solid($W, $H, 0, 0, 0), $null)
    } elseif ($l.StartsWith("fill:")) {
      [M15Hd]::Over($canvas, [M15Hd]::Solid($W, $H, 0, 0, 0), [M15Hd]::Load((Need $l.Substring(5)), $W, $H))
    } elseif ($l.StartsWith("tex:")) {
      $tex, $mask = $l.Substring(4).Split("|")
      [M15Hd]::Over($canvas, [M15Hd]::Load((Need $tex), $W, $H), [M15Hd]::Load((Need $mask), $W, $H))
    } else {
      [M15Hd]::Over($canvas, [M15Hd]::Load((Need $l), $W, $H), $null)
    }
  }
  [M15Hd]::Save($canvas, $W, $H, $out)
}

# Enlarges a 375 image (a blend mask or a variant crown) to w x h.
function Enlarge([string]$from, [string]$to, [int]$w = $W, [int]$h = $H) {
  [M15Hd]::Save([M15Hd]::Load($from, $w, $h), $w, $h, $to)
}

# The kit leaves the text box's side lines open at rows 843-933, where notched cards put their notch.
# Normal cards need them closed, so copy the side lines' last row (842) down through the gap.
$tmp = Join-Path ([IO.Path]::GetTempPath()) "setsmith-m15-hd"
foreach ($side in "typeline_left", "typeline_right") {
  New-Item -ItemType Directory -Force "$tmp\$side\trim" | Out-Null
  foreach ($f in "base.png", "base_shadow.png") {
    $px = [M15Hd]::Load((Need "$(L 'pinlines')\$side\trim\$f"), $W, $H)
    [M15Hd]::RepeatRow($px, $W, 842, 843, 933, 80, 660)
    [M15Hd]::Save($px, $W, $H, "$tmp\$side\trim\$f")
  }
}

# The thin colored lines around the name, type and text boxes, each over its drop shadow.
function Pinlines([string]$color, [string[]]$parts) {
  $tex = Need "$src\pinlines\744x1039\m15\colors\base\bright\${color}color.png"
  foreach ($p in $parts) {
    $dir = if (Test-Path "$tmp\$p") { "$tmp\$p" } else { "$(L 'pinlines')\$p" }
    "fill:$dir\base_shadow.png"; "tex:$tex|$dir\base.png"
  }
}
$nameLines = "nameline_left\trim", "nameline_right\trim"
$typeLines = "typeline_left\trim", "typeline_right\trim"

$colors = "w", "u", "b", "r", "g", "a", "c", "m"
# The thin black line printed cards have around the name and type boxes (a 3 px ring mask per box).
$boxLines = $src; $boxRing = "744x1039\m15\normal\base\border_mask.png"

# ---- Card frames
$cards = Join-Path $dst "cards\744 m15 hd"
New-Item -ItemType Directory -Force $cards | Out-Null
foreach ($land in $false, $true) {
  foreach ($c in $colors) {
    if ($land -and $c -eq "a") { continue }   # land_template uses c for artifacts
    $trim = if ($land) { "l" } else { $c }
    $text = if ($land) { "${c}l" } else { $c }
    # Black first, like the 375 frames: the art window and the border stay solid under the art.
    $layers = @("black", "$(L 'trims')\base\bright\${trim}background.png")
    $layers += Pinlines $c ($nameLines + $typeLines + "stamp")
    $layers += "$(L 'nameboxes')\base\bright\${c}namebox.png",
               "$(L 'typeboxes')\base\bright\${c}typebox.png",
               "$(L 'textboxes')\base\bright\${text}textbox.png",
               "fill:$boxLines\nameboxes\$boxRing", "fill:$boxLines\typeboxes\$boxRing",
               "fill:$(L 'borders')\base.png"
    $name = if ($land) { "${c}lcard.png" } else { "${c}card.png" }
    Stack (Join-Path $cards $name) $layers
  }
}
# Blend masks, drawn from the same layers so they line up with the HD lines (white = the base colors show).
#   multicolor: gold everywhere except the colored lines, the art window and the text box
#   hybrid:     the base colors everywhere except the name and type boxes (grey or gold there)
#   artifact:   artifact frame outside, the base colors inside (boxes and lines included)
function CoverOf([string]$path, [bool]$isMask) { [M15Hd]::Cover([M15Hd]::Load((Need $path), $W, $H), $isMask) }
$trim   = CoverOf "$(L 'trims')\base\bright\rbackground.png" $false
$border = CoverOf "$(L 'borders')\base.png" $true
$name   = CoverOf "$(L 'nameboxes')\base\bright\rnamebox.png" $false
$type   = CoverOf "$(L 'typeboxes')\base\bright\rtypebox.png" $false
$pin    = New-Object double[] ($W * $H)
foreach ($p in $nameLines + $typeLines + "stamp") {
  $dir = if (Test-Path "$tmp\$p") { "$tmp\$p" } else { "$(L 'pinlines')\$p" }
  $c = CoverOf "$dir\base.png" $true
  for ($i = 0; $i -lt $pin.Length; $i++) { if ($c[$i] -gt $pin[$i]) { $pin[$i] = $c[$i] } }
}
$multi = New-Object double[] ($W * $H); $hyb = New-Object double[] ($W * $H); $art = New-Object double[] ($W * $H)
for ($i = 0; $i -lt $pin.Length; $i++) {
  $inside = (1 - $trim[$i]) * (1 - $border[$i])
  $boxes = [Math]::Max($name[$i], $type[$i])
  $multi[$i] = [Math]::Max($inside * (1 - $boxes), $pin[$i])
  $hyb[$i] = 1 - $boxes
  $art[$i] = [Math]::Max($inside, $pin[$i])
}
[M15Hd]::Save([M15Hd]::Grey($multi), $W, $H, (Join-Path $cards "multicolor_blend_card.png"))
[M15Hd]::Save([M15Hd]::Grey($hyb), $W, $H, (Join-Path $cards "hybrid_blend_card.png"))
[M15Hd]::Save([M15Hd]::Grey($art), $W, $H, (Join-Path $cards "artifact_blend_card.png"))

# ---- Legend crowns: shadow, crown and name lines, with the name box left open (like the 375 crowns)
# so the frame's own name box, already blended for multicolor and hybrid cards, shows through.
$crowns = Join-Path $dst "crowns\744 m15 hd"
New-Item -ItemType Directory -Force $crowns | Out-Null
foreach ($c in $colors) {
  # Only the name lines' dark shadow, no colored line: on printed legends the crown meets the name box
  # with a thin dark edge.
  $px = [M15Hd]::Load((Need "$(L 'crowns')\base\bright\${c}crown.png"), $W, $H)
  $tx = [M15Hd]::Load((Need "$(L 'trims')\base\bright\${c}background.png"), $W, $H)
  [M15Hd]::Retexture($px, $tx, $W, $H, 31, 14, 120, 720)   # left trim strip: x 31-44, y 120-839
  New-Item -ItemType Directory -Force "$tmp\crowns" | Out-Null
  [M15Hd]::Save($px, $W, $H, "$tmp\crowns\${c}crown.png")
  $layers = @("$(L 'crowns')\base\bright\shadow.png", "$tmp\crowns\${c}crown.png")
  $layers += foreach ($p in $nameLines) { "fill:$(L 'pinlines')\$p\base_shadow.png" }
  $out = Join-Path $crowns "${c}crown.png"
  Stack $out $layers
  $px = [M15Hd]::Load($out, $W, $H); [M15Hd]::Cut($px, $name)
  # The crown covers the frame's name box outline, so draw it again on top.
  [M15Hd]::Over($px, [M15Hd]::Solid($W, $H, 0, 0, 0), [M15Hd]::Load("$boxLines\nameboxes\$boxRing", $W, $H))
  [M15Hd]::Save($px, $W, $H, $out)
}
foreach ($m in "artifact", "hybrid", "multicolor") {
  Enlarge "$src\crowns\375\${m}_blend_crown.png" (Join-Path $crowns "${m}_blend_crown.png")
}
# Crown variants (borderless, nyx, companion, brawl) have no 744 layers; enlarge them so blends still match in size.
foreach ($dir in Get-ChildItem "$src\crowns\375" -Directory) {
  $to = Join-Path $crowns $dir.Name
  New-Item -ItemType Directory -Force $to | Out-Null
  foreach ($f in Get-ChildItem $dir.FullName -Filter *.png) { Enlarge $f.FullName (Join-Path $to $f.Name) }
}

# ---- P/T boxes: the kit draws each box at card scale in the middle of a 2078 px square. Crop the
# part that covers the M15 pt box field (81 x 42 card units at 273, 466), with the box centered where
# the 375 box sits, and keep 2x card units (162 x 84).
$pts = Join-Path $dst "pts\744 m15 hd"
New-Item -ItemType Directory -Force $pts | Out-Null
$PW = 162; $PH = 84
foreach ($c in $colors + "v") {
  $px = [M15Hd]::Load((Need "$(L 'ptboxes')\base\bright\${c}ptbox.png"), $PW, $PH, 951, 1008, 161, 83)
  [M15Hd]::Save($px, $PW, $PH, (Join-Path $pts "${c}pt.png"))
}
foreach ($m in "artifact", "hybrid", "multicolor") {
  Enlarge "$src\pts\375 m15\${m}_blend_pt.png" (Join-Path $pts "${m}_blend_pt.png") $PW $PH
}

# Anything else in the 375 folders (the clear "x" frame and crown, for example) has no 744 layers:
# enlarge it so every name the scripts ask for still exists.
foreach ($pair in @(@("cards\375 m15 simple", $cards, $W, $H), @("crowns\375", $crowns, $W, $H), @("pts\375 m15", $pts, $PW, $PH))) {
  foreach ($f in Get-ChildItem (Join-Path $src $pair[0]) -File | Where-Object { $_.Extension -in ".png", ".jpg" }) {
    $to = Join-Path $pair[1] ([IO.Path]::ChangeExtension($f.Name, ".png"))
    if (-not (Test-Path $to)) { Enlarge $f.FullName $to $pair[2] $pair[3] }
  }
}
# The 375 P/T variant folders (paradox, inverted, shifted) are left where they are; the engine scales them.

$n = (Get-ChildItem $cards, $crowns, $pts -Recurse -File).Count
"{0,-48} {1,7}" -f "M15 HD frames, crowns, P/T boxes", "$n files"
