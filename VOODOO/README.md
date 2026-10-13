# VOODOO — Synthé mystique by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8, C++17). 8 moteurs de synthèse physique/modale sans samples, 12 voix, 70 presets.

## Moteurs (menu WAVEFORM)
Flute · Pan Flute · Shakuhachi (souffle + guide d'onde) · Sitar · Oud (Karplus-Strong, jawari + cordes sympathiques) · Kalimba (modes inharmoniques) · Erhu · Voodoo Choir (saws + formants).

## Commandes
RATE/AMOUNT = LFO (source, destination : Pitch/Filter/Amp/Timbre) · PITCH · GLIDE · FILTER (LP/HP/BP) · VOLUME ·
ADSR (ATK/DEC/SUS/REL) · TIMBRE 1-5 = Souffle-Voyelle / Corps-Résonance / Brillance / Largeur-Détune / Texture ·
REVERB · CHORUS · DISTORTION · DELAY (TIME, FEEDBACK, ping-pong) · MONO/POLY · molettes Pitch Bend et Modulation (CC1 aussi) · clavier.

## Presets (70)
7 banques de 10 : Flutes, Raga & Strings, Afrique, Amérique Latine, Pads Mystiques, Bass & Plucks, Textures.
Source : `Tools/gen_presets.py` -> `Source/Presets.h` (relancer : `python3 Tools/gen_presets.py Source/Presets.h`).
Bouton Save : exporte l'état en `.voodoopreset` (XML).

## Compiler (Windows, Visual Studio 2022 + CMake)
```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target VOODOO_VST3
```
Le `.vst3` est dans `build/VOODOO_artefacts/Release/VST3/`. Copier dans `C:\Program Files\Common Files\VST3\`.
macOS : `cmake -S . -B build -G Xcode` puis `--target VOODOO_VST3` (AU possible : ajouter `AU` dans FORMATS).
Linux : installer libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libfreetype-dev libfontconfig1-dev libgl-dev.

## Test automatique
`cmake --build build --target VoodooTest && ./VoodooTest` : rend les 70 presets (accord C3, 4 s) en WAV dans `renders/`, vérifie NaN et niveaux.
Résultat actuel : 70/70 ok, pas de NaN, pics < 1.

## Limites connues
- Qualité sonore non écoutée par l'auteur du code : régler au casque dans FL Studio et ajuster les gains par moteur (`VoodooVoice.h`, `renderEngine`) et les presets.
- Interface dessinée en code d'après la maquette (pas les textures exactes). Camel et logo repris de la maquette (`Assets/`).
- EXPANSION VOL.1 : bouton placeholder (message + lien boutique).
- Les en-têtes de groupes de sliders sont ADSR et TIMBRE (la maquette indiquait LFO / ASDR).
- Pas de signature de code ni d'installateur.
