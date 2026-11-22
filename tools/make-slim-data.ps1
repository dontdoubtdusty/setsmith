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
  # Double-size frames, crowns and P/T boxes built by make-m15-hd.ps1 (below) instead of the 375 ones.
  @{ Name = "hd card folder"; Count = 1; Pattern = 'cards/375 m15 simple/'; With = 'cards/744 m15 hd/' }
  @{ Name = "hd card suffix"; Count = 1; Pattern = 'card: "card\.jpg"'; With = 'card: "card.png"' }
  @{ Name = "hd pt folder";   Count = 1; Pattern = 'pts/375 m15/'; With = 'pts/744 m15 hd/' }
  @{ Name = "hd crowns";      Count = 1; Pattern = '(?m)^(\ttemplate_suffix := \[)'; With = "`tcrowns_folder := {`"/magic-modules.mse-include/crowns/744 m15 hd/`"}`r`n`$1" }
)
# The same double-size images for M15 Altered (pinline and frame options on top of M15). Its 375 trims
# and masks still blend with them: the engine scales mismatched blend images (generated_image.cpp).
Patch-File "magic-m15-altered.mse-style" "style" @(
  @{ Name = "hd card folder"; Count = 1; Pattern = 'card: "/magic-modules\.mse-include/cards/375 m15 simple/"'; With = 'card: "/magic-modules.mse-include/cards/744 m15 hd/"' }
  @{ Name = "hd card suffix"; Count = 1; Pattern = 'card: "card\.jpg"'; With = 'card: "card.png"' }
  @{ Name = "hd pt folder";   Count = 1; Pattern = 'pt: "/magic-modules\.mse-include/pts/375 m15/"'; With = 'pt: "/magic-modules.mse-include/pts/744 m15 hd/"' }
  @{ Name = "hd crowns";      Count = 1; Pattern = 'crown: "/magic-modules\.mse-include/crowns/375/"'; With = 'crown: "/magic-modules.mse-include/crowns/744 m15 hd/"' }
  # mainframe_card_base (applied last, via harder_script) names the plain frame directly, bypassing template_prefix.
  @{ Name = "hd standard frame"; Count = 1; Pattern = 'standard: "/magic-modules\.mse-include/cards/375 m15 simple/" \+ (\(if land and input == "a" then "cl" else if land then input \+ "l" else input\)) \+ "card\.jpg"'; With = 'standard: "/magic-modules.mse-include/cards/744 m15 hd/" + $1 + "card.png"' }
  @{ Name = "hd crown folder"; Count = 1; Pattern = '"/magic-modules\.mse-include/crowns/375/" \+ \(if is_planeshifted'; With = '"/magic-modules.mse-include/crowns/744 m15 hd/" + (if is_planeshifted' }
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

# Closer to printed cards: the type line stops before the set symbol by default (new sets), the card name
# keeps a small gap before the mana cost, and the holo stamp uses the sharp 744 x 1039 image instead of
# the 375 one scaled up.
Patch-File "magic.mse-game" "set_fields" @(
  @{ Name = "shorten types default"; Count = 1; Pattern = '(?m)(^\tname: shorten types for rarity\r?\n)'; With = "`$1`tinitial: yes`r`n" }
)
Patch-File "magic.mse-game" "script" @(
  @{ Name = "sharp stamp"; Count = 1; Pattern = '(card_stamp_possible_dimensions :=\r?\n\[\r?\n)\t"375x523":\t"375x523"\r?\n\t"523x375":\t"523x375"'; With = "`$1`t`"375x523`":`t`"744x1039`"`r`n`t`"523x375`":`t`"1039x744`"" }
)
foreach ($p in "magic-m15.mse-style", "magic-m15-altered.mse-style", "magic-m15-adventure.mse-style", "magic-m15-mainframe-dfc.mse-style") {
  Patch-File $p "style" @(
    @{ Name = "name gap"; Count = 1; Pattern = '(?m)(^\t\tright:\s+\{[^\r\n]*?)- card_style\.casting_cost\.content_width \}'; With = '$1- card_style.casting_cost.content_width - 4 }' }
  )
}
# The type line also keeps a small gap before the set symbol (the pack frees exactly the symbol's width).
Patch-File "magic.mse-game" "script" @(
  @{ Name = "type gap"; Count = 1; Pattern = '(?m)^(\tif card_style\.rarity\.width == 0 then 0 else cw)\r?$'; With = '$1 + 4' }
)
Patch-File "magic-m15-altered.mse-style" "style" @(
  @{ Name = "type gap"; Count = 1; Pattern = '(?m)^(\t\tif card_style\.rarity\.width == 0 then 0 else cw)\r?$'; With = '$1 + 4' }
)
Patch-File "magic-m15.mse-style" "style" @(
  @{ Name = "type gap"; Count = 1; Pattern = '- max\(22,card_style\.rarity\.content_width\) \}'; With = '- max(22,card_style.rarity.content_width) - 4 }' }
)
"{0,-48} {1,7}" -f "Setsmith patches (type line, name gap, stamp)", "applied"

# Offer the borderless legend crown (smooth, no bevel) on framed cards. The game script and the choice icon
# already support it; the pack just leaves it out of the list.
Patch-File "magic-modules.mse-include" "crowns/styling_fields" @(
  @{ Name = "borderless crown choice"; Count = 1; Pattern = '(?m)^(\tchoice: standard\r?\n)'; With = "`$1`tchoice: borderless`r`n" }
)
"{0,-48} {1,7}" -f "Setsmith patches (borderless crown choice)", "applied"

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

# Keep rules and flavor text above the holo stamp. The pack's text box runs down to y 481 but the stamp
# starts at y 471, so a full text box drew its last line over the stamp. Printed cards keep a bottom
# margin there instead, so the box gets bottom padding (text that does not fit shrinks, as on real cards).
foreach ($p in "magic-m15.mse-style", "magic-m15-showcase-etched-foil.mse-style", "magic-m15-altered.mse-style") {
  Patch-File $p "style" @(
    @{ Name = "text stamp margin"; Count = 1; Pattern = '(?m)(^\ttext:\r?\n)'; With = "`$1`t`tpadding bottom: 12`n" }
  )
}
"{0,-48} {1,7}" -f "Setsmith patches (text clear of holo stamp)", "applied"

# Rules text set like printed M15 cards (measured on Star Trek scans: Borg Cube, Romulan Bird-of-Prey, Deep
# Space 9, Rogue Artificial Intelligence, Command Subadjunct). Printed text starts 4 units in from the box,
# not 6, and its lines sit closer (soft line height 0.82, not 0.9). The largest size stays the pack's 14: short
# printed text (Command Subadjunct) is set at about that size.
$textBlock = '(?m)(^\ttext:\r?\n(?:\t\t[^\r\n]*\r?\n)*?'
foreach ($p in "magic-m15.mse-style", "magic-m15-altered.mse-style") {
  Patch-File $p "style" @(
    @{ Name = "text padding left"; Count = 1; Pattern = $textBlock + '\t\tpadding left:\s*)6'; With = '${1}4' }
    @{ Name = "text line height";  Count = 1; Pattern = $textBlock + '\t\tline height soft:\s*)0\.9'; With = '${1}0.82' }
  )
}
"{0,-48} {1,7}" -f "Setsmith patches (printed text setting)", "applied"

# Alias bar: a reskinned or Godzilla series card prints its real (Oracle) name in a bar hanging under the title
# bar, like printed Godzilla series cards. templates\setsmith-alias.mse-include draws the bar (the pack's own
# images, a see-through grey plate trimmed in the frame colour); here every Setsmith Magic frame gets the bar,
# its top (alias_bar_top) and the alias text in white italic centred in it. The frames' own "godzilla style
# alias" option stays off; its box is replaced so the alias always uses this bar.
function Alias-Style([double]$Top) {
  "`talias:`n`t`tleft: 45`n`t`ttop: $($Top + 1)`n`t`twidth: 284`n`t`theight: 19`n`t`tz index: 950`n" +
  "`t`tvisible: { card.alias != `"`" }`n`t`talignment: middle center shrink-overflow`n" +
  "`t`tfont:`n`t`t`tname: MPlantin-Italic`n`t`t`tsize: 9.5`n`t`t`tscale down to: 6`n`t`t`tcolor: white`n"
}
# Bar top per frame: under the title bar (planeswalker and token title bars sit a little higher or lower).
$aliasFrames = [ordered]@{
  "magic-m15.mse-style"                        = 57
  "magic-m15-altered.mse-style"                = 57
  "magic-m15-adventure.mse-style"              = 57
  "magic-m15-showcase-etched-foil.mse-style"   = 57
  "magic-m15-mainframe-dfc.mse-style"          = 57
  "magic-m15-mainframe-planeswalker.mse-style" = 49
  "magic-m15-mainframe-tokens.mse-style"       = 60
  "magic-m15-saga.mse-style"                   = 57
}
$aliasDepends = @{ Name = "alias dependency"; Count = 1; Pattern = '(?m)(^version:[^\r\n]*\r?\n)'; With = "`$1depends on:`n`tpackage: setsmith-alias.mse-include`n`tversion: 2026-11-22`n" }
# Several frames already define an "alias" box that only shows with their "godzilla style alias" or "pokemon
# evobar" option. The engine keeps the first box with a name and ignores later ones, so that box is replaced in
# place; frames without one get the box at the start of card style.
foreach ($p in $aliasFrames.Keys) {
  $top = $aliasFrames[$p]
  $block = Alias-Style $top
  $styleText = [IO.File]::ReadAllText((Join-Path (Join-Path $Output $p) "style"))
  $edit = if ($styleText -match '(?m)^\talias: ?\r?$') {
    @{ Name = "alias line (replace)"; Count = 1; Pattern = '(?ms)^\talias: ?\r?\n.*?(?=^\t(?!\t))'; With = $block }
  } else {
    @{ Name = "alias line"; Count = 1; Pattern = '(?m)(^card style:\r?\n)'; With = '$1' + $block }
  }
  Patch-File $p "style" @(
    $edit
    $aliasDepends
    @{ Name = "alias bar script"; Count = 1; Pattern = '(?m)(^init script:\r?\n)'; With = "`$1`talias_bar_top := { $top }`n`tinclude file: /setsmith-alias.mse-include/script`n" }
    @{ Name = "alias bar layout"; Count = 1; Pattern = '(?m)(^card style:\r?\n)'; With = "include file: /setsmith-alias.mse-include/card_fields`n`$1" }
  )
}
# A saga's reminder text starts right under the title bar, so with an alias it starts under the bar instead.
Patch-File "magic-m15-saga.mse-style" "style" @(
  @{ Name = "saga text under alias bar"; Count = 1; Pattern = '(?m)(^\tchapter text:\r?\n\t\tleft:[^\r\n]*\r?\n\t\ttop: )60'; With = '${1}{ if card.alias != "" then 78 else 60 }' }
)
"{0,-48} {1,7}" -f "Setsmith patches (alias line)", "applied"

# Station rows (Edge of Eternities and Star Trek Spacecraft): templates\setsmith-station.mse-include adds the
# fields, bands and badges. Here the game gets the fields, and M15 and M15 Altered (gold cards) get the layout
# and a text box shortened by the rows' height. station_bottom is the bottom of each frame's text box.
$stationDepends = @{ Name = "station dependency"; Count = 1; Pattern = '(?m)(^version:[^\r\n]*\r?\n)'; With = "`$1depends on:`n`tpackage: setsmith-station.mse-include`n`tversion: 2026-11-22`n" }
Patch-File "magic.mse-game" "game" @(
  $stationDepends
  @{ Name = "station fields"; Count = 1; Pattern = '(?m)(^include file: card_fields\r?\n)'; With = "`$1include file: /setsmith-station.mse-include/game_fields`r`n" }
)
$stationFrames = [ordered]@{
  "magic-m15.mse-style" = @{
    Bottom = "481"
    Size   = "14"
    Text   = @{ Name = "station text box"; Count = 1; Pattern = '(?m)(^\ttext:\r?\n(?:\t\t[^\r\n]*\r?\n)*?\t\theight:\s*)154'; With = '${1}{ 154 - station_height() }' }
  }
  "magic-m15-altered.mse-style" = @{
    Bottom = "bottom_of_textbox()"
    Size   = "body_font_size()"
    Text   = @{ Name = "station text box"; Count = 1; Pattern = '(?m)(^\ttext:\r?\n(?:\t\t[^\r\n]*\r?\n)*?\t\tbottom:\s*)\{ bottom_of_textbox\(\) \}'; With = '${1}{ bottom_of_textbox() - station_height() }' }
  }
}
# On a station card the P/T sits in the last station row, in a smaller dark rounded box (57 x 28) with a rim
# in the card's colour, like printed Spacecraft. Each listed property of the frame's "pt" and "pt box" styles
# becomes "if station_pt() then <station value> else <original>"; z index is not scriptable, so it is raised
# outright to draw above the station bands (z 850-851). The box image comes from the station include.
function Patch-StationPt([string]$Package) {
  $path = Join-Path (Join-Path $Output $Package) "style"
  $text = [IO.File]::ReadAllText($path)
  $plan = [ordered]@{
    "pt"     = [ordered]@{ "z index" = "=960"; "left" = "298"; "top" = "station_pt_mid() - 14"; "width" = "57"; "`tsize" = "12.2"; "`tcolor" = '"white"' }
    "pt box" = [ordered]@{ "z index" = "=952"; "left" = "298"; "top" = "station_pt_mid() - 14"; "width" = "57"; "height" = "28"; "image" = "station_pt_image()" }
  }
  foreach ($block in $plan.Keys) {
    $bm = [regex]::Matches($text, "(?ms)^\t$([regex]::Escape($block)):\r?\n.*?(?=^\S|^\t\S)")
    if ($bm.Count -ne 1) { throw "Station P/T patch: block '$block' in $Package matched $($bm.Count) times, expected 1." }
    $body = $bm[0].Value
    foreach ($key in $plan[$block].Keys) {
      $val = $plan[$block][$key]
      $km = [regex]::Matches($body, "(?m)^(\t\t$([regex]::Escape($key)):[ \t]*)([^\r\n]+)")
      if ($km.Count -ne 1) { throw "Station P/T patch: '$key' in '$block' of $Package matched $($km.Count) times, expected 1." }
      $old = $km[0].Groups[2].Value.Trim()
      if ($old -match '^\{(.*)\}$') { $old = $Matches[1].Trim() }
      elseif ($old -notmatch '^-?[\d.]+$') { $old = '"' + $old + '"' }  # a plain value such as black becomes a string
      $new = if ($val.StartsWith("=")) { $val.Substring(1) } else { "{ if station_pt() then $val else ($old) }" }
      $body = $body.Remove($km[0].Groups[2].Index, $km[0].Groups[2].Length).Insert($km[0].Groups[2].Index, $new)
    }
    $text = $text.Remove($bm[0].Index, $bm[0].Length).Insert($bm[0].Index, $body)
  }
  [IO.File]::WriteAllText($path, $text, (New-Object Text.UTF8Encoding($false)))
}
foreach ($p in $stationFrames.Keys) {
  $f = $stationFrames[$p]
  Patch-File $p "style" @(
    $stationDepends
    @{ Name = "station script"; Count = 1; Pattern = '(?m)(^init script:\r?\n)'; With = "`$1`tstation_bottom := { $($f.Bottom) }`n`tstation_rules_size := { $($f.Size) }`n`tinclude file: /setsmith-station.mse-include/script`n" }
    @{ Name = "station layout"; Count = 1; Pattern = '(?m)(^card style:\r?\n)'; With = "include file: /setsmith-station.mse-include/card_fields`n`$1" }
    $f.Text
    # The holo stamp sits under the station band, so the rules text only keeps a small margin above the band.
    @{ Name = "station text margin"; Count = 1; Pattern = '(?m)(^\ttext:\r?\n\t\tpadding bottom: )12'; With = '${1}{ if station_height() > 0 then 4 else 12 }' }
  )
  Patch-StationPt $p
}
"{0,-48} {1,7}" -f "Setsmith patches (station rows)", "applied"

# Double-size M15 frames, crowns and P/T boxes from the pack's 744 x 1039 layer kit.
& (Join-Path $PSScriptRoot "make-m15-hd.ps1") -Source $Source -Output $Output

# Remove packages that are no longer on the list.
$keepNames = @($needed.Keys) + $ownPackages + "en.mse-locale" + "dictionaries"
Get-ChildItem $Output -Directory | Where-Object { $keepNames -notcontains $_.Name } | ForEach-Object {
  Remove-Item -Recurse -Force $_.FullName
  "removed old package: $($_.Name)"
}

"{0} packages, {1:N1} MB -> {2}" -f $needed.Count, ($total / 1MB), (Resolve-Path $Output)
