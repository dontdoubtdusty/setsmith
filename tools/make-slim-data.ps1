<#
.SYNOPSIS
  Builds the slim template folder for Setsmith.

.DESCRIPTION
  Starts from the frames we keep, follows each package's declared "depends on" entries,
  and copies only the packages needed from a full template pack into the output folder.
  The UI language file comes from this repo's data/ folder, so it matches the exe.

  The templates are not part of this repo (they come from the M15 Magic Pack), so the
  output folder is not committed.

.EXAMPLE
  .\tools\make-slim-data.ps1
  .\tools\make-slim-data.ps1 -Source "E:\M15-Magic-Pack-main\data" -Output ".\build\data"
#>
param(
  [string]$Source = "D:\Program Files\Magic Set Editor\M15-Magic-Pack-main\data",
  [string]$Output = (Join-Path $PSScriptRoot "..\build\data"),
  # Frames to keep. Their dependencies are added automatically.
  [string[]]$Keep = @(
    "magic.mse-game",
    "magic-m15.mse-style",
    "magic-m15-mainframe-planeswalker.mse-style",
    "magic-m15-mainframe-tokens.mse-style",
    "magic-m15-saga.mse-style",
    "magic-m15-mainframe-dfc.mse-style",
    "magic-m15-adventure.mse-style",
    "magic-m15-altered.mse-style",
    "magic-m15-showcase-etched-foil.mse-style"
  ),
  # Packages that templates load at runtime without declaring them. Add any found while testing.
  [string[]]$Extra = @(
    "magic-blends.mse-include"   # loaded by template scripts (multicolor frame blending)
    # magic-symbols.mse-include is also referenced, but it isn't in the M15 pack and isn't needed.
  )
)

$ErrorActionPreference = "Stop"
$repoData = Join-Path $PSScriptRoot "..\data"

if (-not (Test-Path (Join-Path $Source "magic.mse-game"))) {
  throw "No template pack found at '$Source'. Pass -Source with the pack's data folder."
}

# Refuse to write through a junction or symlink (for example an old build\data link to the full pack).
if (Test-Path $Output) {
  $item = Get-Item $Output -Force
  if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
    throw "'$Output' is a link. Remove it with: cmd /c rmdir `"$Output`"  (this removes only the link)"
  }
}

# Walk the dependency graph.
$queue = [System.Collections.Generic.Queue[string]]::new()
$needed = [ordered]@{}
foreach ($p in $Keep + $Extra) { $queue.Enqueue($p) }
while ($queue.Count) {
  $p = $queue.Dequeue()
  if ($needed.Contains($p)) { continue }
  $dir = Join-Path $Source $p
  if (-not (Test-Path $dir)) { throw "Package '$p' is not in '$Source'." }
  $needed[$p] = $true
  # The package description file is named after the package type: x.mse-style has "style", x.mse-game has "game".
  $desc = Get-Item (Join-Path $dir ($p -replace '^.*\.mse-', '')) -ErrorAction SilentlyContinue
  if (-not $desc) { continue }
  $inDepends = $false
  foreach ($line in Get-Content $desc.FullName) {
    if ($line -match '^depends on:') { $inDepends = $true; continue }
    if ($inDepends -and $line -match '^\s+package:\s*(\S+)') { $queue.Enqueue($Matches[1]); continue }
    if ($line -notmatch '^\s') { $inDepends = $false }
  }
}

# Copy: replace each package folder so removed files don't linger.
New-Item -ItemType Directory -Force $Output | Out-Null
$total = 0
foreach ($p in $needed.Keys) {
  $dest = Join-Path $Output $p
  if (Test-Path $dest) { Remove-Item -Recurse -Force $dest }
  Copy-Item -Recurse (Join-Path $Source $p) $dest
  $size = (Get-ChildItem $dest -Recurse -File | Measure-Object Length -Sum).Sum
  $total += $size
  "{0,-48} {1,7:N1} MB" -f $p, ($size / 1MB)
}
Copy-Item -Force -Recurse (Join-Path $repoData "en.mse-locale") $Output
"{0,-48} {1,7}" -f "en.mse-locale (from repo)", "copied"

# Spell checker dictionary (a plain folder, not a package). English only.
$dictOut = Join-Path $Output "dictionaries"
New-Item -ItemType Directory -Force $dictOut | Out-Null
Copy-Item -Force (Join-Path $Source "dictionaries\en_US.*") $dictOut
"{0,-48} {1,7}" -f "dictionaries\en_US (spell checker)", "copied"

# Setsmith's own templates (UniVersus), kept in this repo under templates\.
$ownPackages = @()
foreach ($pkg in Get-ChildItem (Join-Path $PSScriptRoot "..\templates") -Directory | Where-Object { $_.Name -like "*.mse-*" }) {
  $dest = Join-Path $Output $pkg.Name
  if (Test-Path $dest) { Remove-Item -Recurse -Force $dest }
  Copy-Item -Recurse $pkg.FullName $dest
  $ownPackages += $pkg.Name
  "{0,-48} {1,7}" -f "$($pkg.Name) (from repo)", "copied"
}

# Setsmith patches to the copied Magic pack. Each edit must match exactly the expected number of times,
# so a changed pack stops the script instead of being half patched.
function Patch-File([string]$Package, [string]$Rel, [array]$Edits) {
  $path = Join-Path (Join-Path $Output $Package) $Rel
  $text = [IO.File]::ReadAllText($path)
  foreach ($e in $Edits) {
    $n = [regex]::Matches($text, $e.Pattern).Count
    if ($n -ne $e.Count) { throw "Patch '$($e.Name)' in $Package\$Rel matched $n times, expected $($e.Count)." }
    $with = if ($e.ContainsKey("With")) { $e.With } else { "" }
    $text = [regex]::Replace($text, $e.Pattern, $with)
  }
  [IO.File]::WriteAllText($path, $text, (New-Object Text.UTF8Encoding($false)))
}
# Masterpiece Series cards are not wanted: no rarity choice, no set code or symbol, no style option.
Patch-File "magic.mse-game" "card_fields" @(
  @{ Name = "rarity choice"; Count = 3; Pattern = '(?m)^\tchoice: masterpiece\r?\n' }
)
Patch-File "magic.mse-game" "set_fields" @(
  @{ Name = "set code field";     Count = 1; Pattern = '(?m)^set field:\r?\n\ttype: text\r?\n\tname: masterpiece set code\r?\n\tdescription:[^\r\n]*\r?\n' }
  @{ Name = "set symbol field";   Count = 1; Pattern = '(?m)^set field:\r?\n\ttype: symbol\r?\n\tname: masterpiece symbol\r?\n\tdescription:[^\r\n]*\r?\n' }
  @{ Name = "set symbol style";   Count = 1; Pattern = '(?ms)^\tmasterpiece symbol:\r?\n.*?(?=^\tinverted common symbol:)' }
)
Patch-File "magic-m15.mse-style" "style" @(
  @{ Name = "style option"; Count = 1; Pattern = '(?m)^styling field:\r?\n\ttype: boolean\r?\n\tname: masterpiece\r?\n\tdescription:[^\r\n]*\r?\n\tinitial: no\r?\n' }
)
Patch-File "magic-modules.mse-include" "rarities/choice_images" @(
  @{ Name = "rarity image"; Count = 1; Pattern = '(?m)^\tmasterpiece:\r?\n\t\tscript:\r?\n(\t\t\t[^\r\n]*\r?\n)+' }
)
Patch-File "magic-modules.mse-include" "rarities/choice_images_old" @(
  @{ Name = "old rarity image"; Count = 1; Pattern = '(?m)^\tmasterpiece:\r?\n\t\tscript:\r?\n(\t\t\t[^\r\n]*\r?\n)+' }
)
foreach ($info in "card_fields", "card_fields_dfc", "card_fields_tfc") {
  Patch-File "magic-modules.mse-include" "information/$info" @(
    @{ Name = "set code"; Count = 2; Pattern = 'if is_masterpiece\(\) then set\.masterpiece_set_code else set\.set_code'; With = "set.set_code" }
  )
}
"{0,-48} {1,7}" -f "Setsmith patches (no Masterpiece)", "applied"

# Keep rules and flavor text out of the P/T box corner. The pack's text boxes run under the P/T box,
# so a long last line (often the flavor text) was drawn beneath it. pt-mask.png is card sized (two faces
# wide, for DFCs) and black (no text) over each P/T corner. Each text box crops its own piece of it from
# its current position and size (card_style.<field>), so frames that move or resize the box with scripts
# (borderless, DFC, adventure, text chops) still line up. The mask is only used when that face has a P/T.
# $Show and $Image are script expressions: when to use the mask, and which image. They default to
# "this face has a P/T" and pt-mask.png.
function PT-Mask([string]$Field, [string]$Pt, [string]$Show = "card.$Pt != `"`"", [string]$Image = "`"pt-mask.png`"") {
  $s = "card_style.$Field"
  # Setsmith's engine runs mask scripts after the box's position and size are final (Style::update).
  "`t`tmask: { if ($Show) and $s.width >= 1 and $s.height >= 1 then crop(input: $Image, " +
  "width: to_int($s.width), height: to_int($s.height), offset_x: to_int($s.left), offset_y: to_int($s.top)) else `"`" }`n"
}
$ptFrames = [ordered]@{
  "magic-m15.mse-style"                      = @(,@("text", "pt"))
  "magic-m15-showcase-etched-foil.mse-style" = @(,@("text", "pt"))
  "magic-m15-altered.mse-style"              = @(,@("text", "pt"))
  "magic-m15-adventure.mse-style"            = @(,@("text", "pt"))
  # The DFC front can show the back face's P/T just above its own P/T box ("pt 2 copy", the hovering P/T).
  # pt-mask-hover.png also blocks that spot, and is used whenever the hovering P/T is drawn.
  "magic-m15-mainframe-dfc.mse-style"        = @(
    @("text", "pt", 'card.pt != "" or (card.pt_2 != "" and extra_card_style.pt_2_copy.visible)',
      'if card.pt_2 != "" and extra_card_style.pt_2_copy.visible then "pt-mask-hover.png" else "pt-mask.png"'),
    @("text 2", "pt_2"))
}
foreach ($p in $ptFrames.Keys) {
  Copy-Item -Force (Join-Path $PSScriptRoot "patches\pt-mask*.png") (Join-Path $Output $p)
  $edits = foreach ($f in $ptFrames[$p]) {
    $rest = @($f[1..($f.Count - 1)])
    @{ Name = "$($f[0]) P/T mask"; Count = 1; Pattern = "(?m)(^\t$($f[0]):\r?\n)"; With = '$1' + (PT-Mask ($f[0] -replace ' ', '_') @rest) }
  }
  Patch-File $p "style" @($edits)
}
"{0,-48} {1,7}" -f "Setsmith patches (text clear of P/T box)", "applied"

# Remove packages that are no longer on the list.
$keepNames = @($needed.Keys) + $ownPackages + "en.mse-locale" + "dictionaries"
Get-ChildItem $Output -Directory | Where-Object { $keepNames -notcontains $_.Name } | ForEach-Object {
  Remove-Item -Recurse -Force $_.FullName
  "removed old package: $($_.Name)"
}

"{0} packages, {1:N1} MB -> {2}" -f $needed.Count, ($total / 1MB), (Resolve-Path $Output)
