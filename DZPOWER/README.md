# DZPOWER — synthé algérien (raï / chaâbi / casbah) by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`). Couleurs rouge / blanc / vert sur fond kaki.

## Moteurs (menu WAVEFORM)
Mandole (corde pincée à caisse, mandoline algérienne) · Qanun (cithare : pincé brillant + octave de sympathie) · Gasba (flûte de roseau, souffle + vibrato) · Ghaita (zorna : anche double, résonance nasale) · Rai Organ (tirettes + trémolo, sonorité raï) · Chaabi Strings (5 saws + vibrato retardé) · Derbouka (doum + tak : sinus à chute de hauteur + claque) · Bendir Sub (sub profond + grelots).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · VOLUME, REVERB · GLIDE · **CUTOFF** + **RESONANCE** (filtre ; FILTER TYPE LP/HP/BP) · CHORUS, DISTORTION · TIME, FEEDBACK · sliders **ATTACK DECAY SUSTAIN RELEASE** (gauche) = enveloppe d'**amplitude** · sliders **ATTACK DECAY SUSTAIN RELEASE + VELOCITY** (droite, « ASDR ») = enveloppe du **filtre** + sensibilité vélocité (profondeur : bouton ENV) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets. Petits boutons dans les deux haut-parleurs perforés : **TONE, BODY, BRIGHT, WIDTH** (gauche), **TEXTURE, ENV, DELAY** (droite ; DELAY = niveau du delay, dont TIME/FEEDBACK sont les réglages).

Écarts avec la maquette : j'ai pris la 2e (la 1re portait des « R, W, G » et « UPG » parasites, et pas de CUTOFF/RESONANCE). Le groupe « DELAY » n'avait qu'un en-tête au-dessus de TIME/FEEDBACK : le bouton de niveau DELAY est ajouté dans le haut-parleur droit. Les boutons de timbre et ENV sont ajoutés dans les haut-parleurs. Typographie du titre (éclairs) approchée par un texte blanc à contour sombre. Les 5 menus affichent la valeur courante.

## Presets (70)
7 banques de 10 : Mandole, Qanun & Plucks, Gasba & Ghaita, Rai Organ, Chaabi Strings, Derbouka & Perc, Bendir Sub & Bass. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.dzpreset` dans `Documents\DJBILBOX BEATS\DZPOWER\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`DzTest`, skill dsp-quality-validation)
`cmake --build build --target DzTest && ./DzTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.08 (min 0.03, max 0.27), pic max 0.92.

## Limites
- Son **jamais écouté** : ce sont des approximations de synthèse d'instruments algériens (mandole, qanun, ghaita, gasba, derbouka, bendir) à régler à l'oreille ; aucune échelle ni quart de ton (accord tempéré) — à gérer côté MIDI ou avec PITCH. Presets de pincés/percussions plus faibles en rms (pics de transitoires) : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
