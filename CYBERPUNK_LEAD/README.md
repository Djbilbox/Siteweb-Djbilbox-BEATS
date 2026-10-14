# CYBERPUNK LEAD — synthé lead néon / synthwave by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Neon Saw (supersaw 5 voix + sub + octave) · Chrome PWM (2 pulses, largeur modulée) · Glitch FM (2 opérateurs, indice à enveloppe, feedback) · Reese Bass (saws détunées + sub + saturation) · Laser Pluck (plongeon de hauteur + filtre qui se ferme) · Neon Brass (saws, filtre qui s'ouvre) · Data Crush (ré-échantillonnage + réduction de bits) · Hollow Glass (FM cloche).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · VOLUME · ADSR d'amplitude (sliders de gauche) · 5 sliders de droite « ASDR+VELOCITY » = **A, D, S, R du filtre + vélocité** (profondeur : bouton ENV) · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE** (caractère du moteur), **RES**, **ENV** · REVERB, CHORUS, DISTORTION, DELAY (TIME, FEEDBACK) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 2e (la 1re portait « ASDR / D / VELOCITY / V » en étiquettes fausses). La maquette n'avait pas de bouton DELAY (alors que TIME et FEEDBACK y sont) : je l'ai ajouté sous RES. Les petits boutons de timbre sont ajoutés dans les zones libres. Les 5 sliders de droite n'ont pas d'étiquette individuelle (comme la maquette) : de gauche à droite A, D, S, R, VELOCITY. Les 5 menus affichent la valeur courante. La police du titre (glitch cyberpunk) est approchée (italique jaune + décalage cyan/rose).

## Presets (70)
7 banques de 10 : Neon Leads, Chrome & Pulse, Glitch FM, Reese & Bass, Laser Plucks, Neon Brass & Pads, Data Crush & FX. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.cppreset` dans `Documents\DJBILBOX BEATS\CYBERPUNK_LEAD\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`CpTest`, skill dsp-quality-validation)
`cmake --build build --target CpTest && ./CpTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.115 (min 0.02, max 0.24), pic max 0.936.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille (le FM et Data Crush surtout). Presets courts (stabs, Bass Stab) plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
