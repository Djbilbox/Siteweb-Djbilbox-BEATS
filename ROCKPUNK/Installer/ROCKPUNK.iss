; ROCKPUNK GUITAR - installeur Windows
; Construit avec Inno Setup 6
;
; Edition unique : pas d'essai, pas de code d'activation, pas d'extension
; vendue a part - donc pas de section [Code].

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
; Le dossier VST3 est sous Program Files : l'elevation est obligatoire
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
UninstallDisplayIcon={app}\ROCKPUNK.exe
UninstallDisplayName={#AppName} GUITAR {#AppVersion}
AppCopyright=Copyright (C) 2026 {#AppPublisher}

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
french.VST3Desc=Plugin VST3 (pour FL Studio, Ableton, Reaper...)
french.StandaloneDesc=Application autonome (sans DAW)
french.DocsDesc=Guide d'utilisation et licence
french.LaunchApp=Lancer ROCKPUNK
english.VST3Desc=VST3 plugin (for FL Studio, Ableton, Reaper...)
english.StandaloneDesc=Standalone application (no DAW needed)
english.DocsDesc=User guide and licence
english.LaunchApp=Launch ROCKPUNK

[Types]
Name: "full"; Description: "{cm:VST3Desc} + {cm:StandaloneDesc}"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3";       Description: "{cm:VST3Desc}";       Types: full custom; Flags: fixed
Name: "standalone"; Description: "{cm:StandaloneDesc}"; Types: full custom
Name: "docs";       Description: "{cm:DocsDesc}";       Types: full custom

[InstallDelete]
; Un VST3 Windows existe sous deux formes : un simple DLL nomme X.vst3, ou un
; dossier-bundle portant le meme nom. Si un DLL a ete copie a la main a cet
; emplacement, Inno ne peut pas y creer le dossier et s'arrete sur
; "Erreur 183 : impossible de creer un fichier deja existant" - un message qui
; ne dit nulle part que c'est un fichier qui bloque. On efface donc l'ancienne
; forme avant d'installer la nouvelle.
;
; "files" ne supprime pas un repertoire du meme nom : une mise a jour normale
; d'un bundle deja installe passe par le remplacement habituel des fichiers.
Type: files; Name: "{commoncf}\VST3\ROCKPUNK.vst3"

; Restes de la 1.0.0, qui embarquait une banque de sons. La 1.1.0 n'en livre
; aucune, mais le plugin sait encore lire une banque installee : la laisser en
; place ferait rejouer l'ancien contenu par-dessus le nouveau produit.
Type: filesandordirs; Name: "{app}\SampleBank"
Type: filesandordirs; Name: "{commonappdata}\DJBILBOX\ROCKPUNK\SampleBank"

[Files]
; Le bundle VST3 en entier. Contents\Resources porte deja les presets d'usine
; (copies la par le build), donc rien a ajouter a cote.
Source: "{#BuildDir}\VST3\ROCKPUNK.vst3\*"; \
  DestDir: "{commoncf}\VST3\ROCKPUNK.vst3"; \
  Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3

; Application autonome : meme contenu, mais range a cote de l'executable.
Source: "{#BuildDir}\Standalone\ROCKPUNK.exe"; \
  DestDir: "{app}"; Flags: ignoreversion; Components: standalone
Source: "{#BuildDir}\Standalone\Presets\*"; \
  DestDir: "{app}\Presets"; Flags: ignoreversion; Components: standalone

; Les presets d'usine passent aussi par un emplacement partage : l'installeur
; s'execute en mode eleve, donc les "Documents" qu'il voit sont ceux de
; l'administrateur. C'est le plugin qui les recopie chez l'utilisateur au
; premier lancement, depuis le bundle ou depuis ici.
Source: "..\Presets\*.rpk"; \
  DestDir: "{commonappdata}\DJBILBOX\ROCKPUNK\Presets"; Flags: ignoreversion

; Documentation
Source: "..\GUIDE_UTILISATION.md"; DestDir: "{app}"; Flags: ignoreversion; Components: docs
Source: "..\LICENSE.txt";          DestDir: "{app}"; Flags: ignoreversion; Components: docs

[Icons]
Name: "{group}\{#AppName}";          Filename: "{app}\ROCKPUNK.exe"; Components: standalone
Name: "{group}\Guide d'utilisation"; Filename: "{app}\GUIDE_UTILISATION.md"; Components: docs
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
; Les cartouches de l'utilisateur (Documents\DJBILBOX\ROCKPUNK\Presets) ne sont
; deliberement pas touchees : elles lui appartiennent et une reinstallation doit
; les retrouver.
Type: filesandordirs; Name: "{app}\Presets"
Type: filesandordirs; Name: "{commoncf}\VST3\ROCKPUNK.vst3"
