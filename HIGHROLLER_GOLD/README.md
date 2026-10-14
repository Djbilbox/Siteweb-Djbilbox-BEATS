# HIGHROLLER GOLD — synthé casino / luxe doré by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Gold Bell (FM cloche, jackpot) · Velvet Keys (tine + cloche, « bark » à la vélocité) · Big Band Brass (3 saws, filtre qui s'ouvre) · Vegas Organ (tirettes, percussion, clic) · Roulette Mallets (lames + trémolo) · Silk Strings (5 saws + flottement) · Gold Sub (sinus, « knock », saturation) · Cigar Smoke (pad de saws détunées + souffle, filtre qui respire).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · sliders **ATTACK DECAY SUSTAIN RELEASE** (gauche) = enveloppe d'**amplitude** · sliders **A D S R + VELOCITY** (droite) = enveloppe du **filtre** + sensibilité vélocité (profondeur : bouton ENV) · REVERB, CHORUS, DISTORTION, DELAY, TIME, FEEDBACK · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE** (rangée au-dessus de la mascotte), **RES, ENV** (à gauche du titre), **VOLUME** (à droite du titre) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 1re (la 2e répétait CHORUS deux fois, avait des étiquettes de menus en doublon et des prises IN/OUT/S-PDIF/MIDI décoratives). La maquette n'avait **pas de bouton VOLUME** : ajouté (petit bouton). Les boutons de timbre, RES et ENV sont ajoutés dans les zones libres. La typographie « marquee » dorée du titre est approchée par une police standard dégradée or. Les 5 menus affichent la valeur courante.

## Presets (70)
7 banques de 10 : Gold Bells & Jackpot, Velvet Keys, Big Band Brass, Vegas Organ, Mallets & Strings, Gold Sub & Bass, Cigar Smoke Pads. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.hgpreset` dans `Documents\DJBILBOX BEATS\HIGHROLLER_GOLD\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`HgTest`, skill dsp-quality-validation)
`cmake --build build --target HgTest && ./HgTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.12 (min 0.03, max 0.28), pic max 0.90.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille. Presets courts (Pizz Casino, Short Gold, Disco Hit) plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
