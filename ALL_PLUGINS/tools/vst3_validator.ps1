# Validateur VST3 officiel de Steinberg sur des plug-ins Windows (CI GitHub, MSVC x64 + Ninja deja prets).
# Le validateur est un vrai hote VST3 compile SANS manifeste Common Controls v6, comme le scanner d'Ableton Live :
# il charge la DLL, cree le plug-in, active l'audio, envoie du MIDI et fait tourner le traitement.
# usage : vst3_validator.ps1 -Report rapport.txt [-Strict] <chemin\Plugin.vst3> [...]
param([string]$Report = 'vst3_validator_report.txt', [switch]$Strict, [Parameter(ValueFromRemainingArguments)][string[]]$Plugins)
$ErrorActionPreference = 'Stop'
$sdk = Join-Path $env:RUNNER_TEMP 'vst3sdk'
if (-not (Test-Path "$sdk\CMakeLists.txt")) {
    git clone --quiet --depth 1 --branch v3.7.12_build_20 --recursive --shallow-submodules https://github.com/steinbergmedia/vst3sdk.git $sdk
    if ($LASTEXITCODE -ne 0) { throw 'clone vst3sdk' }
}
$bld = Join-Path $env:RUNNER_TEMP 'vst3sdk_build'
cmake -S $sdk -B $bld -G Ninja -DCMAKE_BUILD_TYPE=Release -DSMTG_ENABLE_VSTGUI_SUPPORT=OFF -DSMTG_ENABLE_VST3_PLUGIN_EXAMPLES=OFF -DSMTG_ENABLE_VST3_HOSTING_EXAMPLES=ON -DSMTG_RUN_VST_VALIDATOR=OFF | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'configure vst3sdk' }
cmake --build $bld --target validator | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'build validator' }
$val = (Get-ChildItem $bld -Recurse -Filter validator.exe | Select-Object -First 1).FullName
if (-not $val) { throw 'validator.exe introuvable' }
$lines = @(); $bad = @()
foreach ($p in $Plugins) {
    $name = Split-Path $p -Leaf
    $out = & $val $p 2>&1 | Out-String
    $code = $LASTEXITCODE
    $failed = ([regex]::Matches($out, '\[Failed\]')).Count
    $passed = ([regex]::Matches($out, '\[Succeeded\]')).Count
    $state = if ($code -eq 0) { 'OK' } else { 'ECHEC' }
    if ($code -ne 0) { $bad += $name }
    $lines += "$state  $name  (tests reussis: $passed, echoues: $failed, code $code)"
    Write-Host "::group::validator $name"; Write-Host $out; Write-Host '::endgroup::'
}
$lines | Tee-Object -FilePath $Report | Write-Host
if ($Strict -and $bad.Count) { throw "Validateur VST3 en echec : $($bad -join ', ')" }
