# HUMPIRE ORBIT — bundle de 10 plugins d'effets vocaux (VST3)
by **DJBILBOX BEATS** · JUCE 8 · C++17 · CMake · thème cosmique

## État d'avancement
| Livrable | Statut |
|---|---|
| 1. Arborescence + CMakeLists (10 plugins + Core + Hub) | ✅ |
| 2. HumpireCore (LookAndFeel, Knob, Meter, Waveform, OrbitVisualizer, PresetManager, TopBar, XP, A/B, Undo) | ✅ |
| 3. **HUMPIRE SOLAR** complet (DSP + UI + 22 presets + tests) | ✅ compilé VST3, tests DSP OK |
| 4. Prompts image + assets → [`Docs/ASSETS.md`](Docs/ASSETS.md) | ✅ |
| 5-7. Build/pluginval/FL, roadmap, `products.json` | à venir |
| Plugins 2-10, Hub, licence Ed25519, installateur Inno Setup | à venir |

## Arborescence
```
HUMPIRE_ORBIT/
├─ CMakeLists.txt              # JUCE 8.0.4 (FetchContent) · HumpireCore · 10 plugins · Hub · SolarTest
├─ Core/                       # HumpireCore (lib statique partagée)
│  ├─ HumpireTheme.h           # palette #050508 / #F5B544→#FF7A3D / #7C5CFF, polices (repli système)
│  ├─ HumpireLookAndFeel.*     # knobs orbite, boutons pilules, combos
│  ├─ OrbitKnob.h              # survol=valeur · dbl-clic=reset · Shift=fin · clic droit=saisie
│  ├─ LevelMeter.h  WaveformDisplay.h  XpBar.h
│  ├─ OrbitVisualizer.h        # base des scènes (étoiles, nébuleuses, 60 fps)
│  ├─ PresetManager.*          # usine + utilisateur (.hpreset), A/B, badge « Orbit Complete »
│  ├─ TopBar.*                 # logo · presets < > · A/B · Undo/Redo · Bypass · In/Out
│  └─ HumpireEditorBase.h      # éditeur redimensionnable 50-200 % à ratio verrouillé
├─ Plugins/
│  ├─ Solar/                   # ✅ Source/ + Tools/SolarTest.cpp
│  └─ Saturn Twin Nebula SolarFlare Vacuum Gravity Signal Supernova Void/   # Source/ vide → ignorés par CMake
├─ Hub/Source/                 # HUMPIRE Hub (standalone) — à venir
├─ Installer/                  # Inno Setup — à venir
└─ Docs/ASSETS.md
```
Un plugin sans `.cpp` dans `Plugins/<Nom>/Source/` est ignoré : le bundle se construit au fur et à mesure.

## Build rapide (Windows, VS 2022)
```bat
cmake -S HUMPIRE_ORBIT -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target HumpireSolar_VST3
build\HumpireSolar_artefacts\Release\VST3\HUMPIRE SOLAR.vst3
```
Test DSP console : `cmake --build build --config Release --target SolarTest` puis `SolarTest.exe` (`--snapshot <dossier>` = captures PNG de l'éditeur).

## HUMPIRE SOLAR — technique
- **Détection** : YIN (CMNDF + interpolation parabolique), entrée décimée ~16-22 kHz, trame toutes les 10 ms, 70-1000 Hz, médiane 3 trames anti-octave.
- **Correction** (`PitchCorrector`) : note cible (gamme, quarts de ton inclus : Rast, Bayati, Hijaz…) avec hystérésis 15 ct → Portamento → zone morte Flex-Tune → Amount → lissage Retune (ralenti sur notes tenues par Humanize). Boucle ouverte (analyse l'entrée).
- **Mode Graph** : on dessine la fonction de transfert hauteur d'entrée → sortie (1 octave, 8 pts/demi-ton) ; « SNAP GAMME » génère l'escalier ; clic droit = effacer. Sauvée dans l'état du plugin.
- **Pitch shift** (`PitchShifter`) : 2 prises sur ligne à retard, fondu sin², fenêtre = multiple entier de la période (16-22 ms), Hermite cubique. Sec/humide alignés. Latence annoncée ≈ 9,5 ms (458 éch. à 48 kHz).
- **Vibrato** par échantillon avec délai d'attaque ; **Mix**, gains lissés (anti-zipper) ; aucune allocation dans `processBlock`.
- **Mono/Stéréo**, bypass natif VST3 (`getBypassParameter`).

### Résultats de test (Linux, SolarTest)
224 Hz → **219,99 Hz** (A3, retune 0) · gamme Mineur La OK · Amount 0 transparent (224,00 Hz) · mode Graph OK · 22 presets sans NaN · silence → silence.

### Limites connues (honnêtes)
- Timbre : pitch shifter temporel (type « Doppler fenêtré ») → bon pour voix monophoniques, artefacts possibles sur grands écarts (> 3 demi-tons) ; amélioration PSOLA/phase-vocoder prévue en v1.1.
- **Non testé** : pluginval et FL Studio (à faire sur ta machine Windows, voir livrable 5). Compilation vérifiée sous Linux uniquement (g++, JUCE 8.0.4).
- Polices Space Grotesk / Inter / Press Start 2P non embarquées (repli système).
