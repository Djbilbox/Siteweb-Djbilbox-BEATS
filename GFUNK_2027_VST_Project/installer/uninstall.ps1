# GFUNK 2027 uninstaller
$ErrorActionPreference = 'Stop'
$test = $env:GFUNK_TEST_ROOT
function Test-Admin {
    ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}
if (-not $test -and -not (Test-Admin)) {
    Start-Process -FilePath powershell.exe -Verb RunAs -Wait -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}
if ($test) {
    $vst3 = Join-Path $test 'VST3\GFUNK 2027.vst3'
    $appDir = Join-Path $test 'HUMPIRE\GFUNK 2027'
} else {
    $vst3 = Join-Path $env:CommonProgramFiles 'VST3\GFUNK 2027.vst3'
    $appDir = Join-Path $env:ProgramFiles 'HUMPIRE\GFUNK 2027'
    $lnk = Join-Path $env:ProgramData 'Microsoft\Windows\Start Menu\Programs\GFUNK 2027.lnk'
    if (Test-Path $lnk) { Remove-Item $lnk -Force }
    Remove-Item 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\GFUNK2027' -Recurse -Force -ErrorAction SilentlyContinue
}
if (Test-Path $vst3) { Remove-Item $vst3 -Recurse -Force }
# the folder holds this very script, so delete it after we exit
if (Test-Path $appDir) { Start-Process cmd.exe -WindowStyle Hidden -ArgumentList "/c ping 127.0.0.1 -n 3 >nul & rd /s /q `"$appDir`"" }
if (-not $test) {
    Add-Type -AssemblyName System.Windows.Forms
    [void][System.Windows.Forms.MessageBox]::Show('GFUNK 2027 a ete desinstalle.', 'GFUNK 2027', 'OK', 'Information')
}
