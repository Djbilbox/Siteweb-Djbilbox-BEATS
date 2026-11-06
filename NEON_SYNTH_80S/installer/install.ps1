# NEON SYNTH 80s 2.0 installer (HUMPIRE by Djbilbox BEATS). Run by NEON_SYNTH_80s_2_Setup.exe.
$ErrorActionPreference = 'Stop'
$test = $env:NEON_TEST_ROOT                       # set only by the build script to test without touching the system

function Test-Admin {
    ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not $test -and -not (Test-Admin)) {
    Start-Process -FilePath powershell.exe -Verb RunAs -Wait -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}

$dir = Split-Path -Parent $PSCommandPath
$tmp = Join-Path $env:TEMP 'neonsynth80s2_setup'
if (Test-Path $tmp) { Remove-Item $tmp -Recurse -Force }
Expand-Archive -Path (Join-Path $dir 'payload.zip') -DestinationPath $tmp -Force

if ($test) {
    $vst3Dir = Join-Path $test 'VST3'
    $appDir = Join-Path $test 'HUMPIRE\NEON SYNTH 80s 2.0'
} else {
    $vst3Dir = Join-Path $env:CommonProgramFiles 'VST3'
    $appDir = Join-Path $env:ProgramFiles 'HUMPIRE\NEON SYNTH 80s 2.0'
}
New-Item -ItemType Directory -Force -Path $vst3Dir, $appDir | Out-Null

# VST3 plug-in
$target = Join-Path $vst3Dir 'NEON SYNTH 80s 2.0.vst3'
if (Test-Path $target) { Remove-Item $target -Recurse -Force }
Copy-Item -Path (Join-Path $tmp 'NEON SYNTH 80s 2.0.vst3') -Destination $vst3Dir -Recurse -Force

# Standalone application, documents, uninstaller
Copy-Item -Path (Join-Path $tmp 'NEON SYNTH 80s 2.0.exe') -Destination $appDir -Force
foreach ($f in 'LICENSE.md', 'USER_GUIDE.md', 'uninstall.ps1') { Copy-Item -Path (Join-Path $tmp $f) -Destination $appDir -Force }
Copy-Item -Path (Join-Path $tmp 'docs') -Destination $appDir -Recurse -Force

if (-not $test) {
    $menu = Join-Path $env:ProgramData 'Microsoft\Windows\Start Menu\Programs'
    $shell = New-Object -ComObject WScript.Shell
    $lnk = $shell.CreateShortcut((Join-Path $menu 'NEON SYNTH 80s 2.0.lnk'))
    $lnk.TargetPath = Join-Path $appDir 'NEON SYNTH 80s 2.0.exe'
    $lnk.WorkingDirectory = $appDir
    $lnk.Save()

    $key = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\NeonSynth80s2'
    New-Item -Path $key -Force | Out-Null
    Set-ItemProperty $key DisplayName 'NEON SYNTH 80s 2.0'
    Set-ItemProperty $key DisplayVersion '2.0.0'
    Set-ItemProperty $key Publisher 'HUMPIRE by Djbilbox BEATS'
    Set-ItemProperty $key InstallLocation $appDir
    Set-ItemProperty $key DisplayIcon (Join-Path $appDir 'NEON SYNTH 80s 2.0.exe')
    Set-ItemProperty $key UninstallString "powershell.exe -NoProfile -ExecutionPolicy Bypass -File `"$(Join-Path $appDir 'uninstall.ps1')`""
    Set-ItemProperty $key NoModify 1 -Type DWord
    Set-ItemProperty $key NoRepair 1 -Type DWord
}

Remove-Item $tmp -Recurse -Force
if (-not $test) {
    Add-Type -AssemblyName System.Windows.Forms
    [void][System.Windows.Forms.MessageBox]::Show("NEON SYNTH 80s 2.0 is installed.`n`nVST3: $vst3Dir`nApp: $appDir`n`nRescan your plug-ins in your DAW.", 'NEON SYNTH 80s 2.0', 'OK', 'Information')
}

