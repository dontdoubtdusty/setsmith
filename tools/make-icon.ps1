# Builds Setsmith's app.ico: the gold diamond from the logo, at 16-256 px.
# Small sizes are stored as 32-bit DIBs (readable by wx's ICO loader), 256 as PNG.
param([string]$Out, [string]$Preview)
Add-Type -AssemblyName System.Drawing
$gold  = [Drawing.Color]::FromArgb(176,120,30)
$edge  = [Drawing.Color]::FromArgb(110,72,14)
$cream = [Drawing.Color]::FromArgb(250,248,243)

function Draw-Icon([int]$s) {
  $bmp = New-Object Drawing.Bitmap $s, $s, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
  $g = [Drawing.Graphics]::FromImage($bmp)
  $g.SmoothingMode = 'AntiAlias'; $g.PixelOffsetMode = 'HighQuality'; $g.Clear([Drawing.Color]::Transparent)
  $c = $s / 2.0; $r = $s * 0.48; $ri = $s * 0.20
  $outer = [Drawing.PointF[]]@((New-Object Drawing.PointF $c,($c-$r)),(New-Object Drawing.PointF ($c+$r),$c),(New-Object Drawing.PointF $c,($c+$r)),(New-Object Drawing.PointF ($c-$r),$c))
  $inner = [Drawing.PointF[]]@((New-Object Drawing.PointF $c,($c-$ri)),(New-Object Drawing.PointF ($c+$ri),$c),(New-Object Drawing.PointF $c,($c+$ri)),(New-Object Drawing.PointF ($c-$ri),$c))
  $g.FillPolygon((New-Object Drawing.SolidBrush $gold), $outer)
  $g.DrawPolygon((New-Object Drawing.Pen $edge, ([Math]::Max(1.0, $s / 32.0))), $outer)
  $g.FillPolygon((New-Object Drawing.SolidBrush $cream), $inner)
  $g.Dispose()
  return $bmp
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

$sizes = 16, 24, 32, 48, 64, 256
$images = foreach ($s in $sizes) {
  $bmp = Draw-Icon $s
  if ($s -eq 256) { $ms = New-Object IO.MemoryStream; $bmp.Save($ms, [Drawing.Imaging.ImageFormat]::Png); $data = $ms.ToArray(); if ($Preview) { $bmp.Save($Preview) } }
  else { $data = Dib-Bytes $bmp }
  $bmp.Dispose()
  ,@($s, $data)
}
$fs = [IO.File]::Create($Out); $w = New-Object IO.BinaryWriter $fs
$w.Write([int16]0); $w.Write([int16]1); $w.Write([int16]$images.Count)
$offset = 6 + 16 * $images.Count
foreach ($im in $images) {
  $s = $im[0]; $len = $im[1].Length; $d = if ($s -ge 256) { 0 } else { $s }
  $w.Write([byte]$d); $w.Write([byte]$d); $w.Write([byte]0); $w.Write([byte]0)
  $w.Write([int16]1); $w.Write([int16]32); $w.Write([int]$len); $w.Write([int]$offset); $offset += $len
}
foreach ($im in $images) { $w.Write([byte[]]$im[1]) }
$w.Close()
"wrote $Out ($((Get-Item $Out).Length) bytes, sizes $($sizes -join ','))"
