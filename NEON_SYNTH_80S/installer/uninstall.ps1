# NEON SYNTH 80s 2.0 uninstaller
$ErrorActionPreference = 'Stop'
$test = $env:NEON_TEST_ROOT
function Test-Admin {
    ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}
if (-not $test -and -not (Test-Admin)) {
    Start-Process -FilePath powershell.exe -Verb RunAs -Wait -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}
if ($test) {
    $vst3 = Join-Path $test 'VST3\NEON SYNTH 80s 2.0.vst3'
    $appDir = Join-Path $test 'HUMPIRE\NEON SYNTH 80s 2.0'
} else {
    $vst3 = Join-Path $env:CommonProgramFiles 'VST3\NEON SYNTH 80s 2.0.vst3'
    $appDir = Join-Path $env:ProgramFiles 'HUMPIRE\NEON SYNTH 80s 2.0'
    $lnk = Join-Path $env:ProgramData 'Microsoft\Windows\Start Menu\Programs\NEON SYNTH 80s 2.0.lnk'
    if (Test-Path $lnk) { Remove-Item $lnk -Force }
    Remove-Item 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\NeonSynth80s2' -Recurse -Force -ErrorAction SilentlyContinue
}
if (Test-Path $vst3) { Remove-Item $vst3 -Recurse -Force }
# the folder holds this very script, so delete it after we exit
if (Test-Path $appDir) { Start-Process cmd.exe -WindowStyle Hidden -ArgumentList "/c ping 127.0.0.1 -n 3 >nul & rd /s /q `"$appDir`"" }
if (-not $test) {
    Add-Type -AssemblyName System.Windows.Forms
    [void][System.Windows.Forms.MessageBox]::Show('NEON SYNTH 80s 2.0 has been uninstalled.', 'NEON SYNTH 80s 2.0', 'OK', 'Information')
}
