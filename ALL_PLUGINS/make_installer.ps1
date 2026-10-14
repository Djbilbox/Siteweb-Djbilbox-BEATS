# =============================================================================
#  DJBILBOX BEATS - generation de l'installateur unique (Inno Setup 6)
#  Entree : <Stage>\synths|loadout|orbit|chaabi\*.vst3  (+ <Stage>\chaabi_samples\)
#  Sortie : <Out>\DJBILBOX_BEATS_ALL_PLUGINS_Setup.exe + LISEZMOI.txt
# =============================================================================
param(
    [Parameter(Mandatory = $true)][string]$Stage,
    [Parameter(Mandatory = $true)][string]$Out,
    [string]$Version = "1.0.0",
    [string]$Iscc = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
)
$ErrorActionPreference = "Stop"
$Stage = (Resolve-Path $Stage).Path
New-Item -ItemType Directory -Force -Path $Out | Out-Null
$Out = (Resolve-Path $Out).Path

# Plugins attendus (pour signaler les absents dans le LISEZMOI)
$expected = [ordered]@{
    synths  = @("ARENA_BASS","CHINTZWAZ","CYBERPUNK_LEAD","DZPOWER","FUNKYKOPS","GOLDHAWK","HIGHROLLER_GOLD","K2000",
                "MARABOUT","MIDNIGHT_REELS","OCCITANIE","OCTOPUS","PHANTOM_RIDER","SAN_ANDREAS_LEAD","SHAMAN",
                "STREETLIGHT_TRAP","VOODOO","WARZONE808")
    loadout = @("HUMPIRE SKILLTREE","HUMPIRE CLUTCH","HUMPIRE OVERCLOCK","HUMPIRE HARDCAP","HUMPIRE RESPAWN",
                "HUMPIRE LAG","HUMPIRE GLITCH","HUMPIRE COMBO")
    orbit   = @("HUMPIRE SOLAR","HUMPIRE SATURN","HUMPIRE TWIN","HUMPIRE NEBULA","HUMPIRE SOLAR FLARE",
                "HUMPIRE VACUUM","HUMPIRE GRAVITY","HUMPIRE SIGNAL","HUMPIRE SUPERNOVA","HUMPIRE VOID")
    chaabi  = @("HUMPIRE CHAABI ORCHESTRA")
}
$groupLabel = @{
    synths  = "Synthetiseurs DJBILBOX BEATS"
    loadout = "HUMPIRE LOADOUT - 8 effets studio"
    orbit   = "HUMPIRE ORBIT - effets vocaux"
    chaabi  = "HUMPIRE CHAABI ORCHESTRA - sampler"
}

$components = New-Object System.Collections.Generic.List[string]
$files      = New-Object System.Collections.Generic.List[string]
$included   = New-Object System.Collections.Generic.List[string]
$missing    = New-Object System.Collections.Generic.List[string]

foreach ($g in $expected.Keys) {
    $dir = Join-Path $Stage $g
    $bundles = @()
    if (Test-Path $dir) { $bundles = @(Get-ChildItem -Path $dir -Directory -Filter "*.vst3" | Sort-Object Name) }
    $names = @($bundles | ForEach-Object { $_.BaseName })
    foreach ($e in $expected[$g]) { if ($names -notcontains $e) { $missing.Add("$($groupLabel[$g]) : $e") } }
    if ($bundles.Count -eq 0) { continue }

    $components.Add("Name: ""$g""; Description: ""$($groupLabel[$g])""; Types: full custom")
    foreach ($b in $bundles) {
        $id = ($b.BaseName.ToLower() -replace '[^a-z0-9]', '_')
        $components.Add("Name: ""$g\$id""; Description: ""$($b.BaseName)""; Types: full custom")
        $files.Add("Source: ""$($b.FullName)\*""; DestDir: ""{commoncf64}\VST3\$($b.Name)""; Components: $g\$id; Flags: recursesubdirs createallsubdirs ignoreversion")
        $included.Add("$($groupLabel[$g]) : $($b.BaseName)")
    }
    if ($g -eq "chaabi" -and (Test-Path (Join-Path $Stage "chaabi_samples"))) {
        $samples = Join-Path $Stage "chaabi_samples"
        # onlyifdoesntexist : ne remplace jamais tes vrais samples si tu les as deja poses
        $files.Add("Source: ""$samples\*""; DestDir: ""{commonappdata}\HUMPIRE\Chaabi Orchestra\Samples""; Components: chaabi; Flags: recursesubdirs createallsubdirs onlyifdoesntexist uninsneveruninstall")
    }
}

if ($included.Count -eq 0) { throw "Aucun plugin VST3 trouve dans $Stage" }

$iss = @"
; Genere automatiquement par ALL_PLUGINS/make_installer.ps1 - ne pas editer a la main
[Setup]
AppId={{6B1F2C8E-4D7A-4E3B-9C15-DJB0ALLPLUGINS}
AppName=DJBILBOX BEATS - All Plugins
AppVersion=$Version
AppPublisher=DJBILBOX BEATS / HUMPIRE
DefaultDirName={commonpf64}\DJBILBOX BEATS
DisableDirPage=yes
DefaultGroupName=DJBILBOX BEATS
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=$Out
OutputBaseFilename=DJBILBOX_BEATS_ALL_PLUGINS_Setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=DJBILBOX BEATS - All Plugins

[Languages]
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"

[Types]
Name: "full"; Description: "Installation complete (tous les plugins)"
Name: "custom"; Description: "Personnalisee"; Flags: iscustom

[Components]
$($components -join "`r`n")

[Files]
$($files -join "`r`n")
"@

$issPath = Join-Path $Out "djbilbox_all.iss"
[System.IO.File]::WriteAllText($issPath, $iss, (New-Object System.Text.UTF8Encoding($true)))
& $Iscc $issPath
if ($LASTEXITCODE -ne 0) { throw "ISCC a echoue ($LASTEXITCODE)" }

$readme = @"
DJBILBOX BEATS - ALL PLUGINS v$Version
=====================================

1. Lance DJBILBOX_BEATS_ALL_PLUGINS_Setup.exe (accepte la demande administrateur).
2. Tous les plugins sont coches par defaut ; decoche ceux que tu ne veux pas.
3. Les VST3 sont installes dans C:\Program Files\Common Files\VST3
4. FL Studio : Options > Manage plugins > Find installed plugins (ou "Start scan").

HUMPIRE CHAABI ORCHESTRA : des samples de TEST (synthetises) sont installes dans
C:\ProgramData\HUMPIRE\Chaabi Orchestra\Samples. Remplace-les par tes vrais WAV (memes noms) ;
l'installateur ne les ecrasera jamais.

PLUGINS INCLUS ($($included.Count)) :
$(($included | ForEach-Object { "  + $_" }) -join "`r`n")

NON INCLUS ($($missing.Count)) :
$(if ($missing.Count -eq 0) { "  (aucun)" } else { ($missing | ForEach-Object { "  - $_" }) -join "`r`n" })
"@
[System.IO.File]::WriteAllText((Join-Path $Out "LISEZMOI.txt"), $readme, (New-Object System.Text.UTF8Encoding($true)))
Write-Host $readme
