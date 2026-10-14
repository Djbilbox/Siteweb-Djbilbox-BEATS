# OCCITANIE — synthé d'instruments occitans by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg`.

## Moteurs (menu WAVEFORM)
Cabrette (anche nasale + souffle) · Vielle (cordes frottées, « chien » buzz) · Galoubet (flûte à bec) · Mandoline (Karplus-Strong + trémolo) · Guitare (Karplus-Strong, attaque douce) · Accordéon (3 anches, désaccord musette) · Chœur (5 saws + voyelles) · Carillon (modes inharmoniques).
Le **DRONE** (bourdon à l'octave inférieure) s'ajoute à n'importe quel moteur : cornemuse, vielle à roue.

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · GLIDE · FILTER (+ FILTER TYPE LP/HP/BP) · VOLUME · ADSR · sliders **REED** (anche/voyelle), **BODY** (corps/résonance), **BRIGHT**, **DRONE**, **VEL** (sensibilité à la vélocité) · petits boutons **WIDTH** (largeur/désaccord) et **TEXTURE** (souffle/buzz/trémolo) · REVERB, CHORUS, DISTORTION, DELAY (TIME, FEEDBACK) · MONO/POLY · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.
La maquette ne montrait ni FILTER (cutoff) ni WIDTH/TEXTURE, et ses 5 curseurs de droite étaient étiquetés « AS / D / R / VELOCITY » ; je les ai remplacés par REED / BODY / BRIGHT / DRONE / VEL et j'ai ajouté les boutons manquants sous le titre.

## Presets (70)
7 banques de 10 : Cabrette & Cornemuse, Vielle & Cordes frottées, Flûtes & Galoubet, Cordes pincées, Accordéon & Musette, Chœurs & Pads, Carillons & Textures (noms occitans : Capitole Cabrette, Canal du Midi, Montségur…). Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.occpreset` dans `Documents\\DJBILBOX BEATS\\OCCITANIE\\Presets`.
Le bouton cassette EXPANSION est un placeholder (message + lien boutique).

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`OccTest`, skill dsp-quality-validation)
`cmake --build build --target OccTest && ./OccTest` : 70 presets (finis, pic ≤ 1, DC), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes (vol de voix), MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec**.

## Limites
- Son **jamais écouté** : les timbres (anche, vielle, etc.) sont des approximations de synthèse à régler à l'oreille.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes.
- Pas de signature de code.
