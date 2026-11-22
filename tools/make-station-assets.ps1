<#
.SYNOPSIS
  Draws the images for the station frame (templates\setsmith-station.mse-include).

.DESCRIPTION
  badge.png: the round threshold badge ("6+") at the left edge of a station row: black disc, white ring,
  light grey rim, like Edge of Eternities and Star Trek station cards.
  band1.png, band2.png: the see-through grey bands behind the first and second station rows (the second
  row is a little darker, like printed cards).
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
"station images -> $(Resolve-Path $out)"
