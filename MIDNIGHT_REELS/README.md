# MIDNIGHT REELS — synthé film noir / jazz de nuit by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Vibes (lames + moteur de trémolo) · Reel EP (tine + cloche, « bark » à la vélocité) · Muted Trumpet (saws + sourdine résonante) · Tenor Sax (anche, formants, vibrato retardé) · Upright Bass (corde pincée + caisse) · Organ (tirettes) · Noir Strings (5 saws + flottement de bande) · Celesta (cloches inharmoniques).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · ADSR d'amplitude (sliders de gauche) · sliders de droite **A, D, R** = enveloppe du **filtre** (profondeur : bouton ENV), **VEL** = sensibilité vélocité, **VOLUME** · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE** (caractère du moteur : mallet/sourdine, drawbars graves-médiums-aigus, percussion, clic, motor/flottement…), **RES**, **ENV** · REVERB, CHORUS, DISTORTION, DELAY (TIME, FEEDBACK) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : la 1re maquette portait « LVRINFACE » et trois « ASDR » → j'ai pris la 2e. Ses sliders de droite « A / ASDR / R / VELOCITY / VOLUME » sont interprétés A / D / R du filtre + vélocité + volume ; « VELOCITY » est abrégé « VEL » (place). Les petits boutons TONE/BODY/BRIGHT/WIDTH/TEXTURE/RES/ENV, absents de la maquette, sont ajoutés dans les zones libres. Le nom affiché du plugin est « MIDNIGHT REELS » (nom de fichier : MIDNIGHT_REELS.vst3).

## Presets (70)
7 banques de 10 : Vibes & Mallets, Reel EP, Trumpet & Sax, Upright Bass, Organ & Lounge, Noir Strings, Celesta & Reels. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.mrpreset` dans `Documents\DJBILBOX BEATS\MIDNIGHT_REELS\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`MrTest`, skill dsp-quality-validation)
`cmake --build build --target MrTest && ./MrTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.11 (min 0.01, max 0.22), pic max 0.952 (transitoires de pincé).

## Limites
- Son **jamais écouté** : timbres (trompette, sax, contrebasse, orgue) = approximations de synthèse à régler à l'oreille. La contrebasse et les presets courts (Pizzicato, Projector Click) sont plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
