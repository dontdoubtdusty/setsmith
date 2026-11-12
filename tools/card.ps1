<#
.SYNOPSIS
  Adds one card to a running Setsmith set and renders it to PNG. Used by the /card Claude skill.

.DESCRIPTION
  -CardFile holds the card's fields as plain "key: value" lines, with no "card:" line and no indent.
  A multi-line value goes on the following lines, each indented with one tab:
      name: Ember Scout
      rule text:
      	Haste
      	When this enters, draw a card.

  The card goes into "<Documents>\Setsmith Cards\Claude Magic.mse-set" or "Claude UVS.mse-set"
  (created on first use). A card with the same name is replaced, so a redo updates the card
  instead of adding a copy. The old set is kept as .bak.

  -Art crops the image to the art box's shape (center crop) and stores it in the set.
  -Art2 does the same for the second image (the back face of a double-faced card).
  Without them, a replaced card keeps its old art and a new card has none.

  Prints "png: <path>" and "set: <path>". Exit 1 on errors or if Setsmith printed messages.
#>
param(
  [Parameter(Mandatory)][ValidateSet("magic", "uvs")][string]$Game,
  [Parameter(Mandatory)][string]$CardFile,
  [string]$Art,
  [string]$Art2,
  [string]$Set
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -AssemblyName System.Drawing

$repo   = Resolve-Path (Join-Path $PSScriptRoot "..")
$exe    = Join-Path $repo "build\setsmith.com"
$label  = if ($Game -eq "magic") { "Claude Magic" } else { "Claude UVS" }
$folder = Join-Path ([Environment]::GetFolderPath("MyDocuments")) "Setsmith Cards"
if (-not $Set) { $Set = Join-Path $folder "$label.mse-set" }
$pngDir = Join-Path (Split-Path $Set) ([IO.Path]::GetFileNameWithoutExtension($Set))
$utf8   = New-Object Text.UTF8Encoding $true
if (-not (Test-Path $exe)) { throw "Setsmith is not built: $exe is missing. Run tools\check.ps1 first." }

# Header for a brand new set, per game.
$newHeader = @{
  magic = "mse version: 2.0.0`ngame: magic`nstylesheet: m15`nset info:`n`ttitle: Claude Magic`n`tset code: CLD`n`tset language: EN`n`tcard language: EN`n"
  uvs   = "mse version: 2.0.0`ngame: uvs`nstylesheet: 2026-exact`nset info:`n`ttitle: Claude UVS`n`tset code: CL01`n"
}

# Splits a set file into header lines, card blocks and trailing lines (top-level keys after the cards).
function Split-SetText([string]$text) {
  $head = New-Object Collections.Generic.List[string]
  $cards = New-Object Collections.Generic.List[object]
  $tail = New-Object Collections.Generic.List[string]
  $cur = $null
  foreach ($line in ($text -replace "`r", "" -replace "^\uFEFF", "") -split "`n") {
    if ($line -eq "card:") { $cur = New-Object Collections.Generic.List[string]; $cur.Add($line); $cards.Add($cur); continue }
    if ($cur -and $line.StartsWith("`t")) { $cur.Add($line); continue }
    if ($line -eq "") { continue }
    $cur = $null
    if ($cards.Count -eq 0) { $head.Add($line) } else { $tail.Add($line) }
  }
  return @{ head = $head; cards = $cards; tail = $tail }
}

# Reads a top-level field ("<tab>key: value") from a card block, or "" if missing.
function Get-Field($block, [string]$key) {
  foreach ($l in $block) { if ($l -match "^`t$([regex]::Escape($key)):\s*(.*)$") { return $Matches[1].Trim() } }
  return ""
}

# Shape (width / height) of the art box for this card, from the stylesheet and card type.
function Get-ArtAspect($block, [string]$setStyle) {
  $style = Get-Field $block "stylesheet"; if (-not $style) { $style = $setStyle }
  if ($Game -eq "uvs") {
    if ((Get-Field $block "card type") -ne "character") { return 1.0 }
    switch ($style) { "2026-exact" { return 744 / 1039 } "2026-mpc" { return 822 / 1122 } default { return 819 / 1114 } }
  }
  # Borderless art (frame option of m15-altered) covers the whole card.
  if ($block -match "^`t`tframes:.*(borderless|frameless) art") { return 375 / 523 }
  switch ($style) {
    "m15-saga"                   { return 157 / 379 }
    "m15-mainframe-planeswalker" { return 324 / 427.5 }
    "m15-mainframe-tokens"       { return 317 / 289 }
    default                      { return 316 / 231 }
  }
}

# Center-crops an image to the given aspect, caps it at 1500 px wide, saves it as PNG.
function Save-CroppedArt([string]$src, [double]$aspect, [string]$dest) {
  $img = [Drawing.Image]::FromFile((Resolve-Path $src))
  try {
    $w = $img.Width; $h = [int]($w / $aspect)
    if ($h -gt $img.Height) { $h = $img.Height; $w = [int]($h * $aspect) }
    $x = [int](($img.Width - $w) / 2); $y = [int](($img.Height - $h) / 2)
    $ow = [Math]::Min($w, 1500); $oh = [int]($ow / $aspect)
    $bmp = New-Object Drawing.Bitmap $ow, $oh
    $g = [Drawing.Graphics]::FromImage($bmp)
    $g.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.DrawImage($img, (New-Object Drawing.Rectangle 0, 0, $ow, $oh), (New-Object Drawing.Rectangle $x, $y, $w, $h), [Drawing.GraphicsUnit]::Pixel)
    $g.Dispose(); $bmp.Save($dest, [Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
  } finally { $img.Dispose() }
}

# 1. Unpack the running set (or start a new one) into a work folder.
$work = Join-Path ([IO.Path]::GetTempPath()) ("setsmith-card-" + [guid]::NewGuid().ToString("N").Substring(0, 8))
$setDir = Join-Path $work "set"
New-Item -ItemType Directory $setDir -Force | Out-Null
if (Test-Path $Set) { [IO.Compression.ZipFile]::ExtractToDirectory($Set, $setDir) }
else { New-Item -ItemType Directory (Split-Path $Set) -Force | Out-Null; [IO.File]::WriteAllText((Join-Path $setDir "set"), $newHeader[$Game], $utf8) }
$parts = Split-SetText ([IO.File]::ReadAllText((Join-Path $setDir "set")))
if (-not ($parts.head -match "^game: $Game$")) { throw "$Set is not a $Game set." }
$setStyle = (($parts.head | Where-Object { $_ -match "^stylesheet:" }) -replace "^stylesheet:\s*", "")

# 2. Turn the card file into a card block.
$block = New-Object Collections.Generic.List[string]
$block.Add("card:")
foreach ($l in ([IO.File]::ReadAllText((Resolve-Path $CardFile)) -replace "`r", "" -replace "^\uFEFF", "") -split "`n") {
  if ($l.Trim() -ne "") { $block.Add("`t" + $l) }
}
$name = Get-Field $block "name"
if (-not $name) { throw "The card file has no 'name:' line." }

# 3. Find a card with the same name to replace.
$index = -1
for ($i = 0; $i -lt $parts.cards.Count; $i++) { if ((Get-Field $parts.cards[$i] "name") -eq $name) { $index = $i } }

# 4. Art: crop a new image into the set, or keep the replaced card's image. "image 2" is the back face.
$block = [Collections.Generic.List[string]]($block | Where-Object { $_ -notmatch "^`timage( 2)?:" })
$aspect = Get-ArtAspect $block $setStyle
foreach ($pair in @(@("image", $Art), @("image 2", $Art2))) {
  $field, $src = $pair
  if ($src) {
    $n = 1; while (Test-Path (Join-Path $setDir "image$n")) { $n++ }
    Save-CroppedArt $src $aspect (Join-Path $setDir "image$n")
    $block.Add("`t${field}: image$n")
  } elseif ($index -ge 0) {
    $old = Get-Field $parts.cards[$index] $field
    if ($old) { $block.Add("`t${field}: $old") }
  }
}
if ($index -ge 0) { $parts.cards[$index] = $block; "replaced: $name" } else { $parts.cards.Add($block); "added: $name" }

# 5. Render this card from a copy of the whole set, so its collector number counts the other cards.
#    The copy marks the card with a note; the filename template names it "card" and every other card "skip".
#    (An empty filename for the others is not an option: the exporter hangs on it.)
$renderDir = Join-Path $work "render"
Copy-Item $setDir $renderDir -Recurse
$marked = New-Object Collections.Generic.List[string]
$inNotes = $false
foreach ($l in $block) {
  if ($l -match "^`tnotes:") { $inNotes = $true; continue }
  if ($inNotes -and $l -match "^`t`t") { continue }
  $inNotes = $false; $marked.Add($l)
}
$marked.Add("`tnotes: setsmith-render-target")
$renderCards = @($parts.cards | ForEach-Object { if ([object]::ReferenceEquals($_, $block)) { ,$marked } else { ,$_ } })
$full = (@($parts.head) + @($renderCards | ForEach-Object { $_ }) + @($parts.tail)) -join "`n"
[IO.File]::WriteAllText((Join-Path $renderDir "set"), $full + "`n", $utf8)
$outDir = Join-Path $work "out"
$zoom = if ($Game -eq "magic") { "2" } else { "1" }
# Windows PowerShell 5.1 does not escape quotes for native programs, hence the \".
$template = Join-Path $outDir '{if card.notes == \"setsmith-render-target\" then \"card\" else \"skip\"}.png'
$oldPath = $env:PATH
$env:PATH = "C:\Windows\system32;C:\Windows"
Push-Location (Split-Path $exe)
$msgs = & $exe --export-images $renderDir $template --zoom $zoom 2>&1
Pop-Location
$env:PATH = $oldPath
$rendered = Join-Path $outDir "card.png"
if (-not (Test-Path $rendered)) { $msgs; throw "Setsmith did not produce an image." }

# 6. Save the set (old copy as .bak) and the PNG.
$all = @($parts.head) + @($parts.cards | ForEach-Object { $_ }) + @($parts.tail)
[IO.File]::WriteAllText((Join-Path $setDir "set"), ($all -join "`n") + "`n", $utf8)
$newZip = Join-Path $work "new.mse-set"
[IO.Compression.ZipFile]::CreateFromDirectory($setDir, $newZip)
if (Test-Path $Set) { Copy-Item $Set "$Set.bak" -Force }
Copy-Item $newZip $Set -Force
New-Item -ItemType Directory $pngDir -Force | Out-Null
$safe = ($name -replace '[\\/:*?"<>|]', "").Trim()
$png = Join-Path $pngDir "$safe.png"
Copy-Item $rendered $png -Force
Remove-Item $work -Recurse -Force

"png: $png"
"set: $Set"
if ($msgs) { "setsmith printed:"; $msgs | Select-Object -First 15; exit 1 }
exit 0
