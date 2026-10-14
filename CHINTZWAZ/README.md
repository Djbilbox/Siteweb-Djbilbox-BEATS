# CHINTZWAZ — synthé floral rétro / bohème psychédélique by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`) : plaque rose métal brossé, néons cyan/rose, papier peint à fleurs.

## Moteurs (menu WAVEFORM)
Sitar Bloom (corde pincée + jawari) · Flower EP (tine + cloche, « bark » à la vélocité) · Mellotron (5 saws, flottement de bande, souffle) · Hippie Flute (sinus + souffle + vibrato) · Kalimba (lames courtes + résonance de caisse) · Daisy Pad (saws détunées + sub, filtre qui respire) · Bloom Bass (triangle/saw + sub rond) · Psyche Organ (tirettes + trémolo, type Vox).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · sliders **ATTACK DECAY SUSTAIN RELEASE** (gauche) = enveloppe d'**amplitude** · sliders **ATTACK DECAY SUSTAIN VELOCITY** (droite, « ADSR+VELOCITY ») = enveloppe du **filtre** (le release du filtre suit le decay) + sensibilité vélocité (profondeur : bouton ENV) · VOLUME · REVERB, CHORUS, DISTORTION · DELAY, TIME, FEEDBACK · GLIDE · FILTER (cutoff) + RES · menus WAVEFORM / LFO SOURCE / DESTINATION / FILTER TYPE / MONO-POLY · petits boutons dans les deux haut-parleurs : **TONE, BODY, BRIGHT, WIDTH** (gauche), **TEXTURE, RES, ENV** (droite) · molettes Pitch Bend (« PITCH ») / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 2e (la 1re étiquetait ses sliders de droite « FILTER / 5 / ADSR / 5 / VELOCITY » et n'avait pas de bouton DELAY). Dans la 2e, les étiquettes des sliders de droite étaient tronquées (« ATACK / DE / SUST / VELOCIT ») : rétablies en ATTACK / DECAY / SUSTAIN / VELOCITY. Les petits boutons de timbre, RES et ENV sont ajoutés dans les haut-parleurs. Typographie du titre (lettres à motif floral) approchée par un texte clair à contour cyan. Le papier peint à fleurs est dessiné en motifs simplifiés. Les 5 menus affichent la valeur courante.

## Presets (70)
7 banques de 10 : Sitar & Strings, Flower EP, Mellotron & Choir, Hippie Flutes, Kalimba & Bells, Daisy Pads, Bloom Bass & Organs. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.czpreset` dans `Documents\DJBILBOX BEATS\CHINTZWAZ\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`CzTest`, skill dsp-quality-validation)
`cmake --build build --target CzTest && ./CzTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.11 (min 0.05, max 0.16), pic max 0.94.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille (sitar, mellotron, kalimba surtout). Les presets de basse en mono et les pizz courts sont plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
