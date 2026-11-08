<#
.SYNOPSIS
  Build MSE Slim, then check that nothing broke.

.DESCRIPTION
  1. Builds with MSYS2 (UCRT64) and Ninja.
  2. Renders tools/slim-test.mse-set and compares each image with tools/slim-test.reference.txt
     (hashes of the images the installed MSE 2.5.6 + full M15 pack produces on this PC).
  3. Launches the GUI for a few seconds outside MSYS2 and fails if it prints any error.
  Exits 0 only if all three pass.

  The reference hashes depend on the fonts installed on this PC, so they're only valid here.
#>
param(
  [string]$Msys = "D:\msys64",
  [int]$GuiSeconds = 6
)

$ErrorActionPreference = "Stop"
$repo  = Resolve-Path (Join-Path $PSScriptRoot "..")
$build = Join-Path $repo "build"
$fail  = $false

# 1. Build
"== build"
$env:MSYSTEM = "UCRT64"; $env:CHERE_INVOKING = "1"
Push-Location $repo
# Reconfigure first: sources are found with file(GLOB), which only rescans on configure,
# so added or deleted .cpp files are missed otherwise.
$buildLog = & "$Msys\usr\bin\bash.exe" -lc "cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1 && cmake --build build -j 8 2>&1; echo EXIT=`$?"
Pop-Location
$buildLog | Where-Object { $_ -match 'error|FAILED|undefined reference' } | Select-Object -First 30
if (($buildLog | Select-Object -Last 1) -ne "EXIT=0") { "BUILD FAILED"; exit 1 }
"build ok"

# Run the next steps the way a user would: no MSYS2 folders on PATH.
$env:PATH = "C:\Windows\system32;C:\Windows"

# 2. Render test
"== render test"
$out = Join-Path ([IO.Path]::GetTempPath()) ("mse-slim-check-" + [guid]::NewGuid().ToString("N").Substring(0,8))
New-Item -ItemType Directory $out | Out-Null
Push-Location $build
$msgs = & .\magicseteditor.com --export-images (Join-Path $PSScriptRoot "slim-test.mse-set") (Join-Path $out "{card.name}.png") 2>&1
Pop-Location
if ($msgs) { "render printed messages:"; $msgs | Select-Object -First 10; $fail = $true }
foreach ($line in Get-Content (Join-Path $PSScriptRoot "slim-test.reference.txt")) {
  $hash, $name = $line -split '  ', 2
  $file = Join-Path $out $name
  if (-not (Test-Path $file)) { "MISSING  $name"; $fail = $true; continue }
  if ((Get-FileHash $file).Hash -eq $hash) { "same     $name" } else { "CHANGED  $name"; $fail = $true }
}
"rendered images: $out"

# 3. GUI start, with a copy of the test set open so every tab of the set window gets built
"== gui start (test set open)"
$log = Join-Path $out "gui.txt"
$setCopy = Join-Path $out "gui-test.mse-set"
Copy-Item (Join-Path $PSScriptRoot "slim-test.mse-set") $setCopy
$p = Start-Process (Join-Path $build "magicseteditor.com") -ArgumentList "`"$setCopy`"" -WorkingDirectory $build `
  -RedirectStandardOutput $log -RedirectStandardError "$log.err" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds $GuiSeconds
$gui = Get-Process magicseteditor -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq (Join-Path $build "magicseteditor.exe") }
if ($gui) { "gui running: '$($gui.MainWindowTitle)'"; $gui | Stop-Process -Force } else { "GUI NOT RUNNING"; $fail = $true }
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }
$guiOut = @(Get-Content $log -ErrorAction SilentlyContinue) + @(Get-Content "$log.err" -ErrorAction SilentlyContinue) | Where-Object { $_ }
if ($guiOut) { "gui printed:"; $guiOut | Select-Object -First 15; $fail = $true } else { "gui: no errors" }

if ($fail) { "== CHECK FAILED"; exit 1 } else { "== ALL CHECKS PASSED"; exit 0 }
