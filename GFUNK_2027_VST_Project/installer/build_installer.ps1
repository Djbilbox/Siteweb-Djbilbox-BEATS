# Builds GFUNK_2027_Setup.exe with IExpress (built into Windows, nothing to install).
# usage: powershell -File installer\build_installer.ps1 [-Out C:\path\GFUNK_2027_Setup.exe]
param([switch]$Free, [string]$Bin, [string]$Out)
# -Free builds GFUNK_2027_Free_Setup.exe from -Bin <dir holding 'GFUNK 2027 Free.vst3' and 'GFUNK 2027 Free.exe'>.
$repoParent = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not $Out) { $Out = Join-Path $repoParent ($(if ($Free) { 'GFUNK_2027_Free_Setup.exe' } else { 'GFUNK_2027_Setup.exe' })) }
$name = if ($Free) { 'GFUNK 2027 Free' } else { 'GFUNK 2027' }
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$stage = Join-Path $env:TEMP 'gfunk2027_installer_stage'
$pay = Join-Path $stage 'payload'
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $pay | Out-Null

$binDir = if ($Free) { $Bin } else { Join-Path $root 'Builds\Windows' }
Copy-Item (Join-Path $binDir "$name.vst3") $pay -Recurse
Copy-Item (Join-Path $binDir "$name.exe") $pay
Copy-Item (Join-Path $root 'LICENSE.md'), (Join-Path $root 'USER_GUIDE.md') $pay
# the Free edition installs side by side with the full one: own folders, shortcut and uninstall entry
function Convert-Script($src, $dst) {
    $t = Get-Content $src -Raw
    if ($Free) { $t = $t.Replace('GFUNK 2027', 'GFUNK 2027 Free').Replace('GFUNK2027', 'GFUNK2027Free') }
    Set-Content -Path $dst -Value $t -Encoding UTF8
}
Convert-Script (Join-Path $PSScriptRoot 'uninstall.ps1') (Join-Path $pay 'uninstall.ps1')
New-Item -ItemType Directory -Force -Path (Join-Path $pay 'docs') | Out-Null
Copy-Item (Join-Path $root 'docs\screenshot.png') (Join-Path $pay 'docs')

Compress-Archive -Path (Join-Path $pay '*') -DestinationPath (Join-Path $stage 'payload.zip') -CompressionLevel Optimal
Convert-Script (Join-Path $PSScriptRoot 'install.ps1') (Join-Path $stage 'install.ps1')
Set-Content -Path (Join-Path $stage 'install.cmd') -Encoding ASCII -Value "@echo off`r`npowershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0install.ps1`""
Copy-Item (Join-Path $root 'LICENSE.md') (Join-Path $stage 'LICENSE.txt')

$sed = @"
[Version]
Class=IEXPRESS
SEDVersion=3
[Options]
PackagePurpose=InstallApp
ShowInstallProgramWindow=0
HideExtractAnimation=1
UseLongFileName=1
InsideCompressed=0
CAB_FixedSize=0
CAB_ResvCodeSigning=0
RebootMode=N
InstallPrompt=%InstallPrompt%
DisplayLicense=%DisplayLicense%
FinishMessage=%FinishMessage%
TargetName=%TargetName%
FriendlyName=%FriendlyName%
AppLaunched=%AppLaunched%
PostInstallCmd=%PostInstallCmd%
AdminQuietInstCmd=%AdminQuietInstCmd%
UserQuietInstCmd=%UserQuietInstCmd%
SourceFiles=SourceFiles
[Strings]
InstallPrompt=Installer $name (VST3 + application autonome) de HUMPIRE by Djbilbox BEATS ?
DisplayLicense=$stage\LICENSE.txt
FinishMessage=
TargetName=$Out
FriendlyName=$name Setup
AppLaunched=cmd.exe /c install.cmd
PostInstallCmd=<None>
AdminQuietInstCmd=
UserQuietInstCmd=
FILE0="install.cmd"
FILE1="install.ps1"
FILE2="payload.zip"
[SourceFiles]
SourceFiles0=$stage\
[SourceFiles0]
%FILE0%=
%FILE1%=
%FILE2%=
"@
Set-Content -Path (Join-Path $stage 'setup.sed') -Value $sed -Encoding ASCII
if (Test-Path $Out) { Remove-Item $Out -Force }
Start-Process -FilePath "$env:WINDIR\System32\iexpress.exe" -ArgumentList '/N', '/Q', (Join-Path $stage 'setup.sed') -Wait -NoNewWindow
if (-not (Test-Path $Out)) { throw 'IExpress did not produce the installer' }
"{0}  {1} MB" -f $Out, [math]::Round((Get-Item $Out).Length / 1MB, 1)
