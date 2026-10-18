; ROCKPUNK GUITAR - Windows installer
; Built with Inno Setup 6
;
; Single edition: no trial, no activation code, no add-on sold separately -
; hence no [Code] section.

#define AppName        "ROCKPUNK"
#define AppVersion     "1.1.0"
#define AppPublisher   "DJBILBOX BEATS"
#define AppURL         "https://djbilboxbeats.com"
#define BuildDir       "..\Build\ROCKPUNK_artefacts\Release"
#define AssetsDir      "..\Assets"

[Setup]
AppId={{9F41D7C2-6E38-4A15-B0D9-ROCKPUNK0001}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} GUITAR {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
DefaultDirName={commonpf}\DJBILBOX\{#AppName}
DefaultGroupName={#AppPublisher}
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
OutputDir=..\Dist
OutputBaseFilename=ROCKPUNK_{#AppVersion}_Installer
SetupIconFile={#AssetsDir}\ROCKPUNK.ico
WizardImageFile={#AssetsDir}\installer_side.bmp
WizardSmallImageFile={#AssetsDir}\installer_small.bmp
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
; The VST3 folder is under Program Files: elevation is mandatory
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
UninstallDisplayIcon={app}\ROCKPUNK.exe
UninstallDisplayName={#AppName} GUITAR {#AppVersion}
AppCopyright=Copyright (C) 2026 {#AppPublisher}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
english.VST3Desc=VST3 plugin (for FL Studio, Ableton, Reaper...)
english.StandaloneDesc=Standalone application (no DAW needed)
english.DocsDesc=User guide and license
english.LaunchApp=Launch ROCKPUNK
english.UserGuide=User Guide

[Types]
Name: "full"; Description: "{cm:VST3Desc} + {cm:StandaloneDesc}"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3";       Description: "{cm:VST3Desc}";       Types: full custom; Flags: fixed
Name: "standalone"; Description: "{cm:StandaloneDesc}"; Types: full custom
Name: "docs";       Description: "{cm:DocsDesc}";       Types: full custom

[InstallDelete]
; A Windows VST3 exists in two forms: a plain DLL named X.vst3, or a bundle
; folder with the same name. If a DLL was copied by hand to that location,
; Inno cannot create the folder and stops on "Error 183: cannot create a file
; that already exists" - a message that never says that a file is in the way.
; The old form is therefore deleted before the new one is installed.
;
; "files" does not delete a directory of the same name: a normal update of an
; already installed bundle goes through the usual file replacement.
Type: files; Name: "{commoncf}\VST3\ROCKPUNK.vst3"

; Leftovers of 1.0.0, which shipped a sound bank. 1.1.0 ships none, but the
; plugin can still read an installed bank: leaving it in place would make it
; play the old content on top of the new product.
Type: filesandordirs; Name: "{app}\SampleBank"
Type: filesandordirs; Name: "{commonappdata}\DJBILBOX\ROCKPUNK\SampleBank"

[Files]
; The whole VST3 bundle. Contents\Resources already carries the factory presets
; (copied there by the build), so nothing has to be added next to it.
Source: "{#BuildDir}\VST3\ROCKPUNK.vst3\*"; \
  DestDir: "{commoncf}\VST3\ROCKPUNK.vst3"; \
  Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3

; Standalone application: same content, but placed next to the executable.
Source: "{#BuildDir}\Standalone\ROCKPUNK.exe"; \
  DestDir: "{app}"; Flags: ignoreversion; Components: standalone
Source: "{#BuildDir}\Standalone\Presets\*"; \
  DestDir: "{app}\Presets"; Flags: ignoreversion; Components: standalone

; The factory presets also go through a shared location: the installer runs
; elevated, so the "Documents" folder it sees is the administrator's. The
; plugin copies them to the user's folder on first launch, from the bundle or
; from here.
Source: "..\Presets\*.rpk"; \
  DestDir: "{commonappdata}\DJBILBOX\ROCKPUNK\Presets"; Flags: ignoreversion

; Documentation
Source: "..\USER_GUIDE.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: docs
Source: "..\LICENSE.txt";    DestDir: "{app}"; Flags: ignoreversion; Components: docs

[Icons]
Name: "{group}\{#AppName}";          Filename: "{app}\ROCKPUNK.exe"; Components: standalone
Name: "{group}\{cm:UserGuide}";      Filename: "{app}\USER_GUIDE.txt"; Components: docs
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}";    Filename: "{app}\ROCKPUNK.exe"; \
  Tasks: desktopicon; Components: standalone

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; \
  GroupDescription: "{cm:AdditionalIcons}"; Components: standalone

[Run]
Filename: "{app}\ROCKPUNK.exe"; Description: "{cm:LaunchApp}"; \
  Flags: nowait postinstall skipifsilent; Components: standalone

[UninstallDelete]
; The user's presets (Documents\DJBILBOX\ROCKPUNK\Presets) are deliberately
; left alone: they belong to the user, and a reinstall must find them again.
Type: filesandordirs; Name: "{app}\Presets"
Type: filesandordirs; Name: "{commoncf}\VST3\ROCKPUNK.vst3"
