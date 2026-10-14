; HUMPIRE LOADOUT - installateur Windows (Inno Setup 6). Les plugins absents du build sont ignores.
#define Ver "1.0.0"
#define Art "..\build\plugins"
[Setup]
AppName=HUMPIRE LOADOUT
AppVersion={#Ver}
AppPublisher=HUMPIRE by Djbilbox BEATS
DefaultDirName={commonpf64}\HUMPIRE
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=HUMPIRE_LOADOUT_Setup
Compression=lzma2
SolidCompression=yes
[Types]
Name: "full"; Description: "Complet (8 plugins)"
Name: "custom"; Description: "Personnalise"; Flags: iscustom
[Components]
Name: "skilltree";  Description: "HUMPIRE SKILLTREE (EQ)";         Types: full custom
Name: "clutch";     Description: "HUMPIRE CLUTCH (Compresseur)";   Types: full custom
Name: "overclock";  Description: "HUMPIRE OVERCLOCK (Enhancer)";   Types: full custom
Name: "hardcap";    Description: "HUMPIRE HARDCAP (Limiteur)";     Types: full custom
Name: "respawn";    Description: "HUMPIRE RESPAWN (Reverb)";       Types: full custom
Name: "lag";        Description: "HUMPIRE LAG (Delay)";            Types: full custom
Name: "glitch";     Description: "HUMPIRE GLITCH (Lo-Fi)";         Types: full custom
Name: "combo";      Description: "HUMPIRE COMBO (Performance)";    Types: full custom
[Files]
Source: "{#Art}\Skilltree\HumpireSkilltree_artefacts\Release\VST3\HUMPIRE SKILLTREE.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE SKILLTREE.vst3"; Components: skilltree; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Clutch\HumpireClutch_artefacts\Release\VST3\HUMPIRE CLUTCH.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE CLUTCH.vst3"; Components: clutch; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Overclock\HumpireOverclock_artefacts\Release\VST3\HUMPIRE OVERCLOCK.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE OVERCLOCK.vst3"; Components: overclock; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Hardcap\HumpireHardcap_artefacts\Release\VST3\HUMPIRE HARDCAP.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE HARDCAP.vst3"; Components: hardcap; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Respawn\HumpireRespawn_artefacts\Release\VST3\HUMPIRE RESPAWN.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE RESPAWN.vst3"; Components: respawn; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Lag\HumpireLag_artefacts\Release\VST3\HUMPIRE LAG.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE LAG.vst3"; Components: lag; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Glitch\HumpireGlitch_artefacts\Release\VST3\HUMPIRE GLITCH.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE GLITCH.vst3"; Components: glitch; Flags: recursesubdirs skipifsourcedoesntexist
Source: "{#Art}\Combo\HumpireCombo_artefacts\Release\VST3\HUMPIRE COMBO.vst3\*"; DestDir: "{commoncf64}\VST3\HUMPIRE COMBO.vst3"; Components: combo; Flags: recursesubdirs skipifsourcedoesntexist
