# OCTOPUS — synthé abyssal / sous-marin by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Kraken Sub (sinus, « knock », saturation) · Bubble Pluck (sinus dont la hauteur monte vite : bulle) · Coral Bells (FM cloche) · Abyss Pad (5 saws + souffle, filtre qui respire) · Ink Reese (saws détunées + sub + saturation) · Tentacle Lead (2 pulses, largeur modulée) · Siren Choir (5 saws + voyelles a/o/u/i) · Whale Song (sinus à glissandos lents + harmonique + résonance).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · sliders **ATTACK DECAY SUSTAIN RELEASE** (gauche) = enveloppe d'**amplitude** · sliders **A D S R + VELOCITY** (droite) = enveloppe du **filtre** + sensibilité vélocité (profondeur : bouton ENV) · VOLUME · REVERB, CHORUS, DISTORTION, DELAY, TIME, FEEDBACK · rangée de petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE, RES, ENV** sous le titre · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 2e (la 1re répétait REVERB deux fois et affichait « MARK » à la place de GLIDE). Dans la 2e, les sliders de droite étaient étiquetés « A S D R » (ordre faux) → corrigés en A D S R. Le bouton unique « GLIDE / FILTER » (deux étiquettes pour un bouton) est devenu deux boutons. La maquette n'avait pas de bouton DISTORTION (seulement son étiquette) : ajouté. Les petits boutons de timbre, RES et ENV sont ajoutés. Les 5 menus affichent la valeur courante, avec le titre de catégorie au-dessus.

## Presets (70)
7 banques de 10 : Kraken Sub, Bubbles & Drops, Coral Bells, Abyss Pads, Ink Reese & Leads, Siren Choir, Whale Song. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.ocpreset` dans `Documents\DJBILBOX BEATS\OCTOPUS\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`OcTest`, skill dsp-quality-validation)
`cmake --build build --target OcTest && ./OcTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.11 (min 0.03, max 0.26), pic max 0.948.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille (bulles, chant de baleine surtout). Presets courts (Dolphin Click, Pop Pop, Short Wave) plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
