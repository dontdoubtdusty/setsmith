<#
.SYNOPSIS
  Installs the fonts the UniVersus templates use, for the current Windows user only (no admin needed).

.DESCRIPTION
  Copies templates\fonts\*.ttf to %LOCALAPPDATA%\Microsoft\Windows\Fonts and registers them under
  HKCU so GDI (and so Setsmith) can find them. Running it again is safe.
  Fonts: Figtree and Barlow (both SIL Open Font License, licenses next to the files).
  Restart Setsmith afterwards so it sees new fonts.
#>
$ErrorActionPreference = "Stop"
$src  = Join-Path $PSScriptRoot "..\templates\fonts"
$dest = Join-Path $env:LOCALAPPDATA "Microsoft\Windows\Fonts"
$reg  = "HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Fonts"
New-Item -ItemType Directory -Force $dest | Out-Null
if (-not (Test-Path $reg)) { New-Item -Force $reg | Out-Null }

Add-Type -AssemblyName System.Drawing
# Load each font into the running session right away (otherwise only after the next sign-in).
Add-Type -Namespace Win32 -Name Fonts -MemberDefinition @'
[DllImport("gdi32.dll", CharSet = CharSet.Unicode)] public static extern int AddFontResource(string file);
[DllImport("user32.dll")] public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint msg, IntPtr w, IntPtr l, uint flags, uint timeout, out IntPtr result);
'@
foreach ($f in Get-ChildItem $src -Filter *.ttf) {
  $target = Join-Path $dest $f.Name
  # Windows locks fonts in use, so skip files that are already installed unchanged.
  if ((Test-Path $target) -and (Get-FileHash $target).Hash -eq (Get-FileHash $f.FullName).Hash) {
    "up to date $($f.Name)"; continue
  }
  Copy-Item -Force $f.FullName $target
  # Registry entry name: the font's family name, like Windows' own installer does.
  $pfc = New-Object System.Drawing.Text.PrivateFontCollection
  $pfc.AddFontFile($f.FullName)
  $name = "$($pfc.Families[0].Name) $($f.BaseName -replace '^.*-', '') (TrueType)"
  $pfc.Dispose()
  Set-ItemProperty -Path $reg -Name $name -Value $target
  [void][Win32.Fonts]::AddFontResource($target)
  "installed  $($f.Name)"
}
# Tell running programs the font list changed (WM_FONTCHANGE to all top-level windows).
$res = [IntPtr]::Zero
[void][Win32.Fonts]::SendMessageTimeout([IntPtr]0xffff, 0x001D, [IntPtr]::Zero, [IntPtr]::Zero, 2, 1000, [ref]$res)
"Done. Restart Setsmith to use the new fonts."
