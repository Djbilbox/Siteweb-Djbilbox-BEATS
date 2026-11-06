; Installateur Windows NEON SYNTH 80s 2.0 (Inno Setup 6) : assistant Suivant / Suivant / Terminer.
; usage : ISCC.exe /DBinDir=<dossier avec 'NEON SYNTH 80s 2.0.vst3' et 'NEON SYNTH 80s 2.0.exe'> /DOutDir=<sortie> NeonSynth80s2.iss
; Les images de l'assistant sont produites par make_art.py (dossier art\).

#define AppName "NEON SYNTH 80s 2.0"
#define AppVersion "2.0.0"
#define Publisher "HUMPIRE by Djbilbox BEATS"
#ifndef BinDir
  #define BinDir "C:\work\neon_bin"
#endif
#ifndef OutDir
  #define OutDir "C:\work\neon_release"
#endif

[Setup]
AppId={{B484C032-8618-4740-976A-2056B2CA0A00}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName}
AppPublisher={#Publisher}
AppPublisherURL=https://djbilboxbeats.gumroad.com
AppSupportURL=https://humpireplugins.netlify.app
DefaultDirName={autopf}\HUMPIRE\{#AppName}
DefaultGroupName=HUMPIRE
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.md
OutputDir={#OutDir}
OutputBaseFilename=NEON_SYNTH_80s_2_Setup
UninstallDisplayIcon={app}\{#AppName}.exe
UninstallDisplayName={#AppName}
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern dark
WizardSizePercent=110
WizardImageFile=art\wizard_164x314.bmp,art\wizard_192x386.bmp,art\wizard_246x459.bmp,art\wizard_273x556.bmp,art\wizard_328x604.bmp,art\wizard_355x700.bmp,art\wizard_410x797.bmp
WizardSmallImageFile=art\small_55x58.bmp,art\small_64x68.bmp,art\small_83x80.bmp,art\small_92x97.bmp,art\small_110x106.bmp,art\small_119x123.bmp,art\small_138x140.bmp
WizardImageStretch=no
WizardImageBackColor=$22100A

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"

[Types]
Name: "full"; Description: "{cm:TypeFull}"
Name: "custom"; Description: "{cm:TypeCustom}"; Flags: iscustom

[Components]
Name: "vst3"; Description: "{cm:CompVst3}"; Types: full custom; Flags: fixed
Name: "app"; Description: "{cm:CompApp}"; Types: full

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Components: app

[Files]
Source: "{#BinDir}\{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BinDir}\{#AppName}.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion
Source: "..\USER_GUIDE.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Components: app
Name: "{group}\{#AppName} - User Guide"; Filename: "{app}\USER_GUIDE.md"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppName}.exe"; Description: "{cm:LaunchProgram,{#AppName}}"; Components: app; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#AppName}.vst3"

[CustomMessages]
en.TypeFull=Full installation (VST3 plug-in + standalone app)
en.TypeCustom=Custom installation
en.CompVst3=VST3 plug-in (for FL Studio, Ableton Live, Cubase, Reaper...)
en.CompApp=Standalone app (play without a DAW)
en.FinishedExtra=NEON SYNTH 80s 2.0 is installed.%n%nRescan your plug-ins in your DAW. NEON SYNTH 80s 1.0 stays installed next to it.
fr.TypeFull=Installation complète (plugin VST3 + application autonome)
fr.TypeCustom=Installation personnalisée
fr.CompVst3=Plugin VST3 (FL Studio, Ableton Live, Cubase, Reaper...)
fr.CompApp=Application autonome (jouer sans DAW)
fr.FinishedExtra=NEON SYNTH 80s 2.0 est installé.%n%nRelancez le scan des plugins dans votre DAW. NEON SYNTH 80s 1.0 reste installé à côté.

[Messages]
en.WelcomeLabel2=This will install [name/ver] on your computer: an 80s synthwave synthesizer with 70 presets, as a VST3 plug-in and a standalone app.%n%nIt is recommended that you close your DAW before continuing.
fr.WelcomeLabel2=Cet assistant va installer [name/ver] sur votre ordinateur : un synthétiseur synthwave années 80 avec 70 presets, en plugin VST3 et en application autonome.%n%nFermez votre DAW avant de continuer.

[Code]
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpFinished then
    WizardForm.FinishedLabel.Caption := ExpandConstant('{cm:FinishedExtra}');
end;
