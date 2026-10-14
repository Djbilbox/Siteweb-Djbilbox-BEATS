# FUNKYKOPS — synthé funk / disco / G-Funk by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg`.

## Moteurs (menu WAVEFORM)
Clavinet (Karplus-Strong + morsure) · Rhodes (tine + cloche, « bark » à la vélocité) · Synth Bass (saw + sub + carré, filtre résonnant à enveloppe) · Brass (3 saws) · Strings (5 saws, ensemble) · Lead (saw + carré) · Guitar (pincée courte) · Talkbox (saws + formants / voyelles).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · VOLUME · ADSR · sliders de droite **ENV** (profondeur de l'enveloppe de filtre), **DEC** (decay du filtre), **RES** (résonance), **VEL** (sensibilité vélocité) · petits boutons **VOWEL, BODY, BRIGHT, WIDTH, TEXTURE** (caractère du moteur) · REVERB, CHORUS, DISTORTION, DELAY (TIME, FEEDBACK) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.
La maquette ne montrait pas les petits boutons de timbre, et ses 5 menus portaient des étiquettes dupliquées (LFO SOURCE ×3, DELAY) : je les ai rétablis en WAVEFORM / LFO SOURCE / DESTINATION / FILTER TYPE / MONO-POLY. Le groupe « ADSR+VEL » (4 curseurs) est interprété comme enveloppe de filtre + vélocité.

## Presets (70)
7 banques de 10 : Clavinet & Funk Keys, Rhodes & EP, Synth Bass, Brass & Stabs, Strings & Disco, Leads & Talkbox, Guitar & FX. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.fkpreset` dans `Documents\\DJBILBOX BEATS\\FUNKYKOPS\\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`FkTest`, skill dsp-quality-validation)
`cmake --build build --target FkTest && ./FkTest` : 70 presets (finis, pic ≤ 1, DC), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes (vol de voix), MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec**.

## Limites
- Son **jamais écouté** : timbres (clav, rhodes, brass, talkbox) = approximations de synthèse à régler à l'oreille ; les presets sont un point de départ.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
