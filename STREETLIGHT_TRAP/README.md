# STREETLIGHT TRAP — synthé trap / rap de rue by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
808 Sub (sinus, « knock » de hauteur, saturation ; glide pour les slides) · Trap Bell (FM cloche / boîte à musique) · Dark Pluck (saws + filtre qui se ferme + plongeon) · Street Keys (tine + cloche, « bark » à la vélocité) · Neon Pad (5 saws + sub) · Trap Choir (5 saws + voyelles a/o/u/i) · Brass Stab (saws, filtre qui s'ouvre) · Flute (sinus + souffle + vibrato).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · sliders **A D S R** (gauche, sous le LFO) = enveloppe du **filtre** (profondeur : bouton ENV) · sliders **ATTACK DECAY SUSTAIN RELEASE** (droite) = enveloppe d'**amplitude** · VOLUME · CHORUS, DISTORTION, DELAY, TIME, FEEDBACK · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE** (caractère du moteur), **RES**, **ENV**, **REVERB**, **VELOCITY** · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 2e (la 1re étiquetait « A / B / S » et mélangeait les groupes). La maquette n'avait **pas de REVERB** et son bouton unique portait deux étiquettes « VELOCITY / VOLUME » : VOLUME garde le grand bouton, REVERB et VELOCITY sont de petits boutons ajoutés. Les petits boutons de timbre sont ajoutés dans les zones libres. La police graffiti du titre est approchée (italique orange + contour violet). Les 5 menus affichent la valeur courante.

## Presets (70)
7 banques de 10 : 808 & Sub Bass, Trap Bells, Dark Plucks, Street Keys, Neon Pads & Brass, Trap Choir, Flutes & FX. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.slpreset` dans `Documents\DJBILBOX BEATS\STREETLIGHT_TRAP\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`SlTest`, skill dsp-quality-validation)
`cmake --build build --target SlTest && ./SlTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.11 (min 0.02, max 0.21), pic max 0.92.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille (808, flûte, chœur surtout). Presets courts (Short 808, Chopped Vox, Horn Stab) plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
