# PHANTOM RIDER — synthé western fantôme / spectral by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Theremin (sinus à vibrato) · Ghost Choir (5 saws + voyelles a/o/u/i) · Twang Guitar (corde pincée + résonance) · Haunted Organ (tirettes + flottement) · Wraith Pad (saws détunées + souffle, filtre qui respire) · Ghost Bells (FM cloche) · Dead Bass (sinus + knock + saturation) · Whisper Wind (bruit formantique mobile).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · VOLUME · ADSR d'amplitude (sliders de gauche) · 4 sliders de droite = enveloppe du **filtre A, D, R** (profondeur : bouton ENV) + **VELOCITY** (de gauche à droite) · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE** (caractère du moteur), **RES**, **ENV** · REVERB, CHORUS, DISTORTION, DELAY (TIME, FEEDBACK) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 1re (la 2e dupliquait RATE/AMOUNT/PITCH/GLIDE/DELAY/FEEDBACK et ajoutait Cutoff/Resonance en double). Les 4 sliders de droite y étaient étiquetés « AS / SDR / VELOCITY » (garbled) → interprétés A / D / R du filtre + vélocité (pas de sustain de filtre). Petits boutons TONE/BODY/BRIGHT/WIDTH/TEXTURE/RES/ENV ajoutés dans les zones libres. Les 5 menus affichent la valeur courante. Police du titre (lettres « fantôme » griffées) approchée par une police standard avec halo.

## Presets (70)
7 banques de 10 : Theremin & Whistles, Ghost Choir, Twang & Strings, Haunted Organs, Wraith Pads, Ghost Bells, Dead Bass & Wind. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.prpreset` dans `Documents\DJBILBOX BEATS\PHANTOM_RIDER\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`PrTest`, skill dsp-quality-validation)
`cmake --build build --target PrTest && ./PrTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.11 (min 0.03, max 0.20), pic max < 0.96.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille. Les presets de guitare/banjo sont plus faibles en rms (pics de pincé) : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
