# ARENA BASS — synthé de basses de stade (sport / hip-hop) by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Arena Sub (sinus, « knock », saturation) · Slap Bass (corde pincée + caisse + claque, transitoires compressés) · Moog Bass (2 saws + carré + sub) · Stadium Reese (saws détunées + sub + saturation) · FM Growl (2 opérateurs, indice qui ondule) · Acid Rubber (saw/pulse + filtre résonant qui se ferme) · Stadium Horns (saws, filtre qui s'ouvre) · Arena Kick (sinus à chute de hauteur + clic).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · VOLUME, REVERB, CHORUS · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · TIME, FEEDBACK · sliders **ATTACK DECAY SUSTAIN RELEASE** (gauche) = enveloppe d'**amplitude** · sliders **A D S R + VELOCITY** (droite) = enveloppe du **filtre** + sensibilité vélocité (profondeur : bouton ENV) · panneau à lamelles (en haut à droite) : **TONE, BODY, BRIGHT, WIDTH, TEXTURE, RES, ENV, DISTORTION, DELAY** · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 1re (la 2e a deux boutons PITCH et pas de DELAY/TIME clairs). Dans la 1re, l'en-tête « ADSR » coiffe en fait VOLUME / REVERB / CHORUS, et les étiquettes « DISTORTION / DELAY » (en doré) surmontent les boutons « TIME / FEEDBACK » : il manquait donc deux boutons. Je garde TIME / FEEDBACK à leur place et place **DISTORTION et DELAY** dans le panneau à lamelles (le haut-parleur décoratif de droite). Les petits boutons de timbre, RES et ENV sont ajoutés dans ce même panneau. Les 5 menus affichent la valeur courante.

## Presets (70)
7 banques de 10 : Arena Sub, Slap & Pluck Bass, Moog Bass, Reese & Growl, Acid & Rubber, Stadium Horns, Kicks & Stomps. Presets de basses en MONO par défaut (sauf Horns). Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.abpreset` dans `Documents\DJBILBOX BEATS\ARENA_BASS\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`AbTest`, skill dsp-quality-validation)
`cmake --build build --target AbTest && ./AbTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.10 (min 0.03, max 0.25), pic max 0.90.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille (slap, growl, kick surtout). Les basses pincées (Slap & Pluck) ont un rms plus faible (pics de pincé) : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
