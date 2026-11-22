<#
.SYNOPSIS
  Draws the images for the station frame (templates\setsmith-station.mse-include).

.DESCRIPTION
  badge.png: the round threshold badge ("6+") at the left edge of a station row: black disc, white ring,
  light grey rim, like Edge of Eternities and Star Trek station cards.
  band1.png, band2.png: the see-through grey bands behind the first and second station rows (the second
  row is a little darker, like printed cards).
  pt_mask.png: the pill-shaped mask that cuts the frame's own P/T box down to the rim of the station P/T pill
  (white shows, black hides). pt_inner.png: the dark centre of the pill, drawn over it.
  Edit this script, not the images.
#>
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
$out = Join-Path $PSScriptRoot "..\templates\setsmith-station.mse-include"

# Draws the badge: three filled circles, largest first.
$size = 128
$bmp = New-Object Drawing.Bitmap $size, $size
$g = [Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.Clear([Drawing.Color]::Transparent)
foreach ($ring in @(@(0, "#C8C8C8"), @(7, "#FFFFFF"), @(13, "#141414"))) {
  $inset = $ring[0]
  $brush = New-Object Drawing.SolidBrush ([Drawing.ColorTranslator]::FromHtml($ring[1]))
  $g.FillEllipse($brush, $inset, $inset, $size - 2 * $inset - 1, $size - 2 * $inset - 1)
  $brush.Dispose()
}
$g.Dispose()
$bmp.Save((Join-Path $out "badge.png"), [Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()

# Draws a band: flat see-through black, with a slightly darker line along the top edge.
function Save-Band([string]$name, [int]$alpha) {
  $w = 64; $h = 64
  $bmp = New-Object Drawing.Bitmap $w, $h
  for ($y = 0; $y -lt $h; $y++) {
    $a = if ($y -lt 1) { [Math]::Min(255, $alpha * 2) } else { $alpha }
    for ($x = 0; $x -lt $w; $x++) { $bmp.SetPixel($x, $y, [Drawing.Color]::FromArgb($a, 0, 0, 0)) }
  }
  $bmp.Save((Join-Path $out $name), [Drawing.Imaging.ImageFormat]::Png)
  $bmp.Dispose()
}
Save-Band "band1.png" 26
Save-Band "band2.png" 46
# Draws a pill (a rounded rectangle with fully round ends) filled with a vertical gradient.
function Save-Pill([string]$name, [int]$w, [int]$h, [string]$top, [string]$bottom, [string]$back, [int]$cw = $w, [int]$ch = $h) {
  $bmp = New-Object Drawing.Bitmap $cw, $ch
  $ox = [int](($cw - $w) / 2); $oy = [int](($ch - $h) / 2)
  $g = [Drawing.Graphics]::FromImage($bmp)
  $g.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
  if ($back) { $g.Clear([Drawing.ColorTranslator]::FromHtml($back)) } else { $g.Clear([Drawing.Color]::Transparent) }
  $path = New-Object Drawing.Drawing2D.GraphicsPath
  $d = $h - 1
  $path.AddArc($ox, $oy, $d, $d, 90, 180)
  $path.AddArc($ox + $w - 1 - $d, $oy, $d, $d, 270, 180)
  $path.CloseFigure()
  $rect = New-Object Drawing.Rectangle $ox, $oy, $w, $h
  $brush = New-Object Drawing.Drawing2D.LinearGradientBrush $rect, ([Drawing.ColorTranslator]::FromHtml($top)), ([Drawing.ColorTranslator]::FromHtml($bottom)), 90.0
  $g.FillPath($brush, $path)
  $brush.Dispose(); $path.Dispose(); $g.Dispose()
  $bmp.Save((Join-Path $out $name), [Drawing.Imaging.ImageFormat]::Png)
  $bmp.Dispose()
}
# Sizes are 4x the frame units. The mask covers the frame's whole 81 x 42 P/T box with a 57 x 28 pill in its
# middle; the dark centre is 53 x 24.
Save-Pill "pt_mask.png" 228 112 "#FFFFFF" "#FFFFFF" "#000000" 324 168
Save-Pill "pt_inner.png" 212 96 "#2B2A28" "#0E0D0C" ""
"station images -> $(Resolve-Path $out)"
