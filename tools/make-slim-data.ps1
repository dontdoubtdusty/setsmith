<#
.SYNOPSIS
  Builds the slim template folder for MSE Slim.

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
    "magic-m15-mainframe-tokens.mse-style"
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
  # The package description file has no extension (game, style, include, symbol-font, ...).
  $desc = Get-ChildItem $dir -File | Where-Object { $_.Extension -eq "" } | Select-Object -First 1
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

# Remove packages that are no longer on the list.
$keepNames = @($needed.Keys) + "en.mse-locale" + "dictionaries"
Get-ChildItem $Output -Directory | Where-Object { $keepNames -notcontains $_.Name } | ForEach-Object {
  Remove-Item -Recurse -Force $_.FullName
  "removed old package: $($_.Name)"
}

"{0} packages, {1:N1} MB -> {2}" -f $needed.Count, ($total / 1MB), (Resolve-Path $Output)
