# Builds Setsmith's icons into resource\icon: app.ico (the exe and window icon) and set.ico (set files).
#   app.ico: one upright card, charcoal with a gold border and a gold diamond.
#   set.ico: the same card with a second card behind it, for "a set of cards".
# Each is drawn once at 512 px and scaled down, so the small sizes stay clean.
# Small sizes are stored as 32-bit DIBs (readable by wx's ICO loader), 256 as PNG.
param(
  [string]$OutDir = (Join-Path $PSScriptRoot "..\resource\icon"),
  [string]$Preview
)
Add-Type -AssemblyName System.Drawing

$charcoal = [Drawing.Color]::FromArgb(38, 40, 45)
$ink      = [Drawing.Color]::FromArgb(22, 23, 26)
$gold     = [Drawing.Color]::FromArgb(224, 176, 58)
$goldDark = [Drawing.Color]::FromArgb(150, 108, 24)
$cream    = [Drawing.Color]::FromArgb(250, 246, 235)

function Rounded-Path([double]$x, [double]$y, [double]$w, [double]$h, [double]$r) {
  $p = New-Object Drawing.Drawing2D.GraphicsPath
  $d = $r * 2
  $p.AddArc($x, $y, $d, $d, 180, 90); $p.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
  $p.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90); $p.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
  $p.CloseFigure(); return $p
}

function Draw-Card($g, [double]$x, [double]$y, [double]$w, [double]$h, [bool]$full) {
  $border = $w * 0.075
  $path = Rounded-Path $x $y $w $h ($w * 0.12)
  $g.FillPath((New-Object Drawing.SolidBrush $charcoal), $path)
  $g.DrawPath((New-Object Drawing.Pen $gold, $border), $path)
  if (-not $full) { return }
  # art window with a gold diamond
  $cx = $x + $w / 2; $cy = $y + $h * 0.40; $r = $w * 0.30
  $art = Rounded-Path ($x + $w * 0.17) ($y + $h * 0.10) ($w * 0.66) ($h * 0.60) ($w * 0.05)
  $g.FillPath((New-Object Drawing.SolidBrush $ink), $art)
  $outer = [Drawing.PointF[]]@((New-Object Drawing.PointF $cx, ($cy - $r * 1.25)), (New-Object Drawing.PointF ($cx + $r), $cy), (New-Object Drawing.PointF $cx, ($cy + $r * 1.25)), (New-Object Drawing.PointF ($cx - $r), $cy))
  $g.FillPolygon((New-Object Drawing.SolidBrush $gold), $outer)
  $ri = $r * 0.42
  $inner = [Drawing.PointF[]]@((New-Object Drawing.PointF $cx, ($cy - $ri * 1.25)), (New-Object Drawing.PointF ($cx + $ri), $cy), (New-Object Drawing.PointF $cx, ($cy + $ri * 1.25)), (New-Object Drawing.PointF ($cx - $ri), $cy))
  $g.FillPolygon((New-Object Drawing.SolidBrush $cream), $inner)
  # two text lines
  $lh = $h * 0.045
  foreach ($row in @(@(0.78, 0.60), @(0.87, 0.42))) {
    $g.FillPath((New-Object Drawing.SolidBrush $goldDark), (Rounded-Path ($x + $w * 0.17) ($y + $h * $row[0]) ($w * $row[1]) $lh ($lh / 2)))
  }
}

function Draw-Master([bool]$stack) {
  $s = 512
  $bmp = New-Object Drawing.Bitmap $s, $s, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
  $g = [Drawing.Graphics]::FromImage($bmp)
  $g.SmoothingMode = 'AntiAlias'; $g.PixelOffsetMode = 'HighQuality'; $g.Clear([Drawing.Color]::Transparent)
  $h = $s * 0.92; $w = $h * 5 / 7
  if ($stack) {
    $w = $s * 0.56; $h = $w * 7 / 5
    # back card, shifted up and right
    Draw-Card $g ($s * 0.30) ($s * 0.04) $w $h $false
    Draw-Card $g ($s * 0.12) ($s * 0.12) $w $h $true
  } else {
    Draw-Card $g (($s - $w) / 2) (($s - $h) / 2) $w $h $true
  }
  $g.Dispose(); return $bmp
}

function Scale-To([Drawing.Bitmap]$master, [int]$size) {
  $bmp = New-Object Drawing.Bitmap $size, $size, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
  $g = [Drawing.Graphics]::FromImage($bmp)
  $g.InterpolationMode = 'HighQualityBicubic'; $g.PixelOffsetMode = 'HighQuality'; $g.CompositingQuality = 'HighQuality'
  $g.Clear([Drawing.Color]::Transparent)
  $g.DrawImage($master, 0, 0, $size, $size); $g.Dispose(); return $bmp
}

function Dib-Bytes([Drawing.Bitmap]$bmp) {
  $s = $bmp.Width
  $ms = New-Object IO.MemoryStream; $w = New-Object IO.BinaryWriter $ms
  # BITMAPINFOHEADER; height is doubled (color + AND mask)
  $w.Write([int]40); $w.Write([int]$s); $w.Write([int]($s*2)); $w.Write([int16]1); $w.Write([int16]32)
  $w.Write([int]0); $w.Write([int]0); $w.Write([int]0); $w.Write([int]0); $w.Write([int]0); $w.Write([int]0)
  for ($y = $s - 1; $y -ge 0; $y--) { for ($x = 0; $x -lt $s; $x++) {
    $p = $bmp.GetPixel($x, $y); $w.Write([byte]$p.B); $w.Write([byte]$p.G); $w.Write([byte]$p.R); $w.Write([byte]$p.A) } }
  # AND mask: 1 bit per pixel, rows padded to 4 bytes; set where fully transparent
  $rowBytes = [int]([Math]::Ceiling($s / 32.0) * 4)
  for ($y = $s - 1; $y -ge 0; $y--) {
    $row = New-Object byte[] $rowBytes
    for ($x = 0; $x -lt $s; $x++) { if ($bmp.GetPixel($x, $y).A -eq 0) { $row[[int][Math]::Floor($x/8)] = $row[[int][Math]::Floor($x/8)] -bor (0x80 -shr ($x % 8)) } }
    $w.Write($row)
  }
  $w.Flush(); return ,$ms.ToArray()
}

function Write-Ico([Drawing.Bitmap]$master, [string]$path) {
  $sizes = 16, 24, 32, 48, 64, 256
  $images = foreach ($s in $sizes) {
    $bmp = Scale-To $master $s
    if ($s -eq 256) { $ms = New-Object IO.MemoryStream; $bmp.Save($ms, [Drawing.Imaging.ImageFormat]::Png); $data = $ms.ToArray() }
    else { $data = Dib-Bytes $bmp }
    $bmp.Dispose()
    ,@($s, $data)
  }
  $fs = [IO.File]::Create($path); $w = New-Object IO.BinaryWriter $fs
  $w.Write([int16]0); $w.Write([int16]1); $w.Write([int16]$images.Count)
  $offset = 6 + 16 * $images.Count
  foreach ($im in $images) {
    $s = $im[0]; $len = $im[1].Length; $d = if ($s -ge 256) { 0 } else { $s }
    $w.Write([byte]$d); $w.Write([byte]$d); $w.Write([byte]0); $w.Write([byte]0)
    $w.Write([int16]1); $w.Write([int16]32); $w.Write([int]$len); $w.Write([int]$offset); $offset += $len
  }
  foreach ($im in $images) { $w.Write([byte[]]$im[1]) }
  $w.Close()
  "wrote $path ($((Get-Item $path).Length) bytes)"
}

New-Item -ItemType Directory -Force $OutDir | Out-Null
$app = Draw-Master $false
$set = Draw-Master $true
Write-Ico $app (Join-Path $OutDir "app.ico")
Write-Ico $set (Join-Path $OutDir "set.ico")

if ($Preview) {
  # Preview sheet: both icons at 128, 48, 32 and 16 px on a dark and a light background.
  $sheet = New-Object Drawing.Bitmap 700, 340
  $g = [Drawing.Graphics]::FromImage($sheet)
  $g.InterpolationMode = 'HighQualityBicubic'
  foreach ($row in 0, 1) {
    $bg = if ($row -eq 0) { [Drawing.Color]::FromArgb(32, 34, 38) } else { [Drawing.Color]::FromArgb(240, 240, 240) }
    $g.FillRectangle((New-Object Drawing.SolidBrush $bg), 0, $row * 170, 700, 170)
    $x = 15
    foreach ($m in $app, $set) {
      foreach ($px in 128, 48, 32, 16) {
        $g.DrawImage((Scale-To $m $px), $x, $row * 170 + 20, $px, $px); $x += $px + 14
      }
      $x += 30
    }
  }
  $g.Dispose(); $sheet.Save($Preview); $sheet.Dispose()
}
$app.Dispose(); $set.Dispose()
