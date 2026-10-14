# MARABOUT — synthé oriental / afro mystique by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Kora (Karplus-Strong de harpe + octave de sympathie) · Balafon (lames inharmoniques + mirliton de calebasse) · Oud (pincé doux + caisse de résonance) · Ney (flûte de roseau, souffle filtré, vibrato) · Zurna (anche double, résonance nasale) · Choir (5 saws + voyelles a/o/u/i) · Dark Pad (5 saws + sub) · Sub Voodoo (sinus, « knock » de hauteur, saturation).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · VOLUME · ADSR d'amplitude (sliders de gauche) · sliders de droite **A D S R** = enveloppe du **filtre**, **VELOCITY** = sensibilité · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE** (caractère du moteur), **RES** (résonance), **ENV** (profondeur de l'enveloppe de filtre) · REVERB, CHORUS, DISTORTION, DELAY (TIME, FEEDBACK) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : les 5 sliders de droite y portaient « A D S R » en doublon des sliders de gauche (la 2e maquette disait même « REVERB A S D R » / « ASDR ») → je les ai interprétés comme enveloppe de filtre + vélocité ; les petits boutons TONE/BODY/BRIGHT/WIDTH/TEXTURE/RES/ENV, absents de la maquette, sont ajoutés dans les zones libres. Les 5 menus affichent la valeur courante (pas le titre de catégorie).

## Presets (70)
7 banques de 10 : Kora & Harps, Balafon & Mallets, Oud & Plucks, Ney & Zurna, Choir & Voices, Dark Pads & Drones, Sub Voodoo & FX. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.mbpreset` dans `Documents\DJBILBOX BEATS\MARABOUT\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`MbTest`, skill dsp-quality-validation)
`cmake --build build --target MbTest && ./MbTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.10 (min 0.03, max 0.18), pic max 0.958, légèrement au-dessus de la cible 0.95 (transitoires de pincé : Kora/Oud).

## Limites
- Son **jamais écouté** : les timbres (kora, ney, zurna, voyelles du chœur) sont des approximations de synthèse à régler à l'oreille ; les presets sont un point de départ. Niveaux : Kora/Oud/Sub sont plus faibles en rms que Balafon/Ney (pics de pincé), à ajuster au volume.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
