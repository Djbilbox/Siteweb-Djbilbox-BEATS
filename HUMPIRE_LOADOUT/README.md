# HUMPIRE LOADOUT — bundle de 8 effets VST3 (JUCE 8 / C++20 / CMake)

## Arborescence
```
HUMPIRE_LOADOUT/
├─ CMakeLists.txt                 racine : JUCE, HumpireCore, 8 plugins (auto-détectés), Hub
├─ cmake/HumpirePlugin.cmake      fonction humpire_add_plugin() (VST3 + Standalone)
├─ external/JUCE                  (optionnel) submodule ; sinon FetchContent 8.0.4
├─ assets/fonts/                  PressStart2P-Regular.ttf (scripts/fetch_fonts.sh)
├─ scripts/fetch_fonts.sh
├─ Source/HumpireCore/            module partagé (compilé dans chaque plugin)
│   ├─ HumpireTheme.*             palette, polices, mode Clean/Skeuo, atomicMax
│   ├─ HumpireLookAndFeel.*       knob métal brossé, boutons pilule
│   ├─ FilmstripKnob.*            filmstrip ou vectoriel, survol, Shift fin, clic droit saisie
│   ├─ LedMeter.*                 meter LED segmenté (peak hold)
│   ├─ LcdScreen.*                LCD olive 4 teintes + ScopeBuffer lock-free
│   ├─ XYPad.*                    pad/stick XY (spring-back)
│   ├─ ConsoleButton.*            boutons 3D glossy + croix directionnelle
│   ├─ PresetManager.*            presets usine/utilisateur XML, A/B
│   ├─ TopBar.*                   barre commune (logo, presets, A/B, undo/redo, bypass, In/Out)
│   └─ ScaledEditor.h             éditeur redimensionnable 50–200 %, ratio verrouillé
├─ plugins/
│   ├─ Glitch/                    ✅ livré
│   │   ├─ CMakeLists.txt
│   │   ├─ assets/glitch_body_2x.png   (optionnel, 1520×1120)
│   │   └─ Source/ GlitchParams.h GlitchDsp.h PluginProcessor.* PluginEditor.*
│   └─ Skilltree Clutch Overclock Hardcap Respawn Lag Combo   (à venir, voir roadmap)
├─ Hub/                           (à venir) app standalone : activation, MAJ
└─ docs/IMAGE_PROMPTS.md          prompts visuels + liste d'assets
```

## Build rapide
```
./scripts/fetch_fonts.sh            # optionnel (police LCD)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target HumpireGlitch_VST3
```
Un plugin est compilé dès que `plugins/<Nom>/CMakeLists.txt` existe.
