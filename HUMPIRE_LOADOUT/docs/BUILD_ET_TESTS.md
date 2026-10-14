# Build, tests, validation
## Build Windows
`cmake -S . -B build -G "Visual Studio 17 2022" -A x64` puis `cmake --build build --config Release`.
Installateur : `ISCC installer\humpire.iss` -> `dist\HUMPIRE_LOADOUT_Setup.exe`. Ou laisser le workflow GitHub Actions `humpire-windows.yml` le faire (artefact `HUMPIRE_LOADOUT_Installer`).
## pluginval
`pluginval --strictness-level 10 --validate "HUMPIRE GLITCH.vst3"` (viser niveau 10 ; 5 minimum).
## FL Studio
1. Options > Manage plugins > Find installed plugins (dossier `C:\Program Files\Common Files\VST3`).
2. Verifier : automation de chaque knob, bypass hote, sauvegarde/rechargement projet, redimensionnement, changement de buffer/sample-rate (44.1/48/96k), mono et stereo.
## Checklist DSP GLITCH
bits=16 + mix 100 % ~ transparent ; bits=1 : 3 niveaux ; rate 48k = pas d'effet ; pas de clic en bougeant les knobs.
