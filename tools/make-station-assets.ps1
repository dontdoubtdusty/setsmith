<#
.SYNOPSIS
  Draws the images for the station frame (templates\setsmith-station.mse-include).

.DESCRIPTION
  Shapes and colours are measured from printed Star Trek Spacecraft (Borg Cube, Romulan Bird-of-Prey and one
  station card per colour), at 744 x 1040. <c> is the frame colour: w u b r g, m for multicolour, c for colourless.
  badge-<c>.png: the round "N+" badge, a rim in the frame colour around a dark centre.
  pt-<c>.png: the 57 x 28 rounded P/T box printed station cards use, same rim and centre.
  band-<c>.png, band-<c>2.png: see-through tints behind the first and second station rows, measured as the
  change from text box to band.
  Edit this script, not the images.
#>
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
$out = Join-Path $PSScriptRoot "..\templates\setsmith-station.mse-include"
# Images are drawn at 8 pixels per frame unit.
$k = 8

# Returns a rounded rectangle path (radius r) inside the given box.
function New-RoundRect([double]$x, [double]$y, [double]$w, [double]$h, [double]$r) {
  $p = New-Object Drawing.Drawing2D.GraphicsPath
  $d = [Math]::Min(2 * $r, [Math]::Min($w, $h))
  $p.AddArc([single]$x, [single]$y, [single]$d, [single]$d, 180, 90)
  $p.AddArc([single]($x + $w - $d), [single]$y, [single]$d, [single]$d, 270, 90)
  $p.AddArc([single]($x + $w - $d), [single]($y + $h - $d), [single]$d, [single]$d, 0, 90)
  $p.AddArc([single]$x, [single]($y + $h - $d), [single]$d, [single]$d, 90, 90)
  $p.CloseFigure()
  return $p
}

# Returns the printed station P/T shape inside the given box: straight top and bottom edges, ends that bulge
# outward by b (a soft arc over the full height) and small rounded corners (c) where they meet.
function New-PtShape([double]$x, [double]$y, [double]$w, [double]$h, [double]$b, [double]$c) {
  $p = New-Object Drawing.Drawing2D.GraphicsPath
  $x0 = $x + $b; $x1 = $x + $w - $b; $ym = $y + $h / 2; $y1 = $y + $h; $xr = $x + $w
  $p.AddLine([single]($x0 + $c), [single]$y, [single]($x1 - $c), [single]$y)
  $p.AddBezier([single]($x1 - $c), [single]$y, [single]($x1 + $c * 0.6), [single]$y, [single]$xr, [single]($y + $h * 0.22), [single]$xr, [single]$ym)
  $p.AddBezier([single]$xr, [single]$ym, [single]$xr, [single]($y1 - $h * 0.22), [single]($x1 + $c * 0.6), [single]$y1, [single]($x1 - $c), [single]$y1)
  $p.AddLine([single]($x1 - $c), [single]$y1, [single]($x0 + $c), [single]$y1)
  $p.AddBezier([single]($x0 + $c), [single]$y1, [single]($x0 - $c * 0.6), [single]$y1, [single]$x, [single]($y1 - $h * 0.22), [single]$x, [single]$ym)
  $p.AddBezier([single]$x, [single]$ym, [single]$x, [single]($y + $h * 0.22), [single]($x0 - $c * 0.6), [single]$y, [single]($x0 + $c), [single]$y)
  $p.CloseFigure()
  return $p
}

# Fills a path with the dark centre gradient printed boxes use: darker at the top and bottom edges.
function Fill-Centre($g, $path, [Drawing.RectangleF]$rect) {
  $brush = New-Object Drawing.Drawing2D.LinearGradientBrush $rect, ([Drawing.Color]::Black), ([Drawing.Color]::Black), ([single]90)
  $blend = New-Object Drawing.Drawing2D.ColorBlend 3
  $blend.Colors = @([Drawing.ColorTranslator]::FromHtml("#242120"), [Drawing.ColorTranslator]::FromHtml("#363233"), [Drawing.ColorTranslator]::FromHtml("#221F1F"))
  $blend.Positions = @([single]0, [single]0.5, [single]1)
  $brush.InterpolationColors = $blend
  $g.FillPath($brush, $path)
  $brush.Dispose()
}

# Creates a canvas of w x h frame units (transparent unless a background colour is given) and its graphics.
function New-Canvas([double]$w, [double]$h, [string]$back) {
  $bmp = New-Object Drawing.Bitmap ([int][Math]::Round($w * $k)), ([int][Math]::Round($h * $k))
  $g = [Drawing.Graphics]::FromImage($bmp)
  $g.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
  if ($back) { $g.Clear([Drawing.ColorTranslator]::FromHtml($back)) } else { $g.Clear([Drawing.Color]::Transparent) }
  return @($bmp, $g)
}
# Saves a canvas as a PNG in the include folder.
function Save-Canvas($c, [string]$name) {
  $c[1].Dispose()
  $c[0].Save((Join-Path $out $name), [Drawing.Imaging.ImageFormat]::Png)
  $c[0].Dispose()
}

$edge = New-Object Drawing.Pen ([Drawing.Color]::FromArgb(220, 20, 14, 6)), ([single](0.6 * $k))
$innerEdge = New-Object Drawing.Pen ([Drawing.Color]::FromArgb(230, 34, 28, 10)), ([single](0.7 * $k))
# Rim colours per frame colour, light (top) and dark (bottom), sampled from the printed badges and P/T boxes.
$rims = [ordered]@{
  w = @("#E0DDDC", "#B8B2B2"); u = @("#2A86B8", "#0E5A80"); b = @("#5A5454", "#3A3535"); r = @("#D4564A", "#A8343A")
  g = @("#1F7A50", "#12523A"); m = @("#E2D18A", "#B09E55"); c = @("#D8D6D4", "#A8A3A2")
}
# Fills a path with a vertical gradient between two colours.
function Fill-Rim($g, $path, [Drawing.RectangleF]$rect, [string]$top, [string]$bottom) {
  $brush = New-Object Drawing.Drawing2D.LinearGradientBrush $rect, ([Drawing.ColorTranslator]::FromHtml($top)), ([Drawing.ColorTranslator]::FromHtml($bottom)), ([single]90)
  $g.FillPath($brush, $path)
  $brush.Dispose()
}
foreach ($key in $rims.Keys) {
  $rim = $rims[$key]
  # Badge: 29.5 units across, rim 2.4 units, measured on Borg Cube.
  $bd = 29.5; $in = 2.4
  $c = New-Canvas $bd $bd ""
  $outer = New-Object Drawing.Drawing2D.GraphicsPath
  $outer.AddEllipse([single](0.3 * $k), [single](0.3 * $k), [single](($bd - 0.6) * $k), [single](($bd - 0.6) * $k))
  Fill-Rim $c[1] $outer (New-Object Drawing.RectangleF 0, 0, ([single]($bd * $k)), ([single]($bd * $k))) $rim[0] $rim[1]
  $c[1].DrawPath($edge, $outer)
  $disc = New-Object Drawing.Drawing2D.GraphicsPath
  $disc.AddEllipse([single]($in * $k), [single]($in * $k), [single](($bd - 2 * $in) * $k), [single](($bd - 2 * $in) * $k))
  Fill-Centre $c[1] $disc (New-Object Drawing.RectangleF ([single]($in * $k)), ([single]($in * $k)), ([single](($bd - 2 * $in) * $k)), ([single](($bd - 2 * $in) * $k)))
  $c[1].DrawPath($innerEdge, $disc)
  Save-Canvas $c "badge-$key.png"
  # P/T box: 57 x 28 with ends bulging 2.2 units, dark centre 50.5 x 21.5 bulging 1.6, measured on the 745 px
  # scan of U.S.S. Enterprise-A, Free Spirit.
  $c = New-Canvas 57 28 ""
  $outer = New-PtShape (0.3 * $k) (0.3 * $k) (56.4 * $k) (27.4 * $k) (2.2 * $k) (1.4 * $k)
  Fill-Rim $c[1] $outer (New-Object Drawing.RectangleF 0, 0, ([single](57 * $k)), ([single](28 * $k))) $rim[0] $rim[1]
  $c[1].DrawPath($edge, $outer)
  $box = New-PtShape (3.25 * $k) (3.25 * $k) (50.5 * $k) (21.5 * $k) (1.6 * $k) (1.1 * $k)
  Fill-Centre $c[1] $box (New-Object Drawing.RectangleF ([single](3.25 * $k)), ([single](3.25 * $k)), ([single](50.5 * $k)), ([single](21.5 * $k)))
  $c[1].DrawPath($innerEdge, $box)
  Save-Canvas $c "pt-$key.png"
}

# Bands: a flat tint per frame colour. Row 1 at alpha 64 (a quarter), row 2 a little stronger.
$tints = [ordered]@{ w = "#765D4E"; u = "#2288BD"; b = "#505B51"; r = "#B4321C"; g = "#175F25"; m = "#A18C47"; c = "#778B99" }
foreach ($key in $tints.Keys) {
  $col = [Drawing.ColorTranslator]::FromHtml($tints[$key])
  foreach ($row in @(@("", 64), @("2", 100))) {
    $bmp = New-Object Drawing.Bitmap 16, 16
    $g = [Drawing.Graphics]::FromImage($bmp)
    $g.Clear([Drawing.Color]::FromArgb($row[1], $col.R, $col.G, $col.B))
    $g.Dispose()
    $bmp.Save((Join-Path $out "band-$key$($row[0]).png"), [Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
  }
}
"station images -> $(Resolve-Path $out)"
