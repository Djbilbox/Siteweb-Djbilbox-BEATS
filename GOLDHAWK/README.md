# GOLDHAWK — synthé cinématique doré (rapace / royal) by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, 70 presets. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`).

## Moteurs (menu WAVEFORM)
Hawk Lead (2 pulses à largeur modulée + saw + sub) · Golden Brass (3 saws, filtre qui s'ouvre) · Royal Pad (5 saws + sub, filtre qui respire) · Falcon Pluck (saw + plongeon de hauteur + filtre qui se ferme) · Sky Bells (FM cloche) · Feather Strings (5 saws + vibrato retardé) · Talon Bass (saws détunées + sub + saturation) · Eagle Cry (glissando descendant + formants sur le souffle).

## Commandes
RATE / AMOUNT / PITCH (LFO ; source Sine…S&H, destination Pitch/Filter/Amp/Timbre) · sliders **A D S R** (gauche) = enveloppe d'**amplitude** · sliders **ENVELOPE A D S VEL** (droite) = enveloppe du **filtre** (attack, decay, sustain ; le release du filtre suit le decay) + sensibilité vélocité (profondeur : bouton ENV) · VOLUME · REVERB, CHORUS, DISTORTION · DELAY (niveau), TIME, FEEDBACK · GLIDE · FILTER (cutoff) + RES · FILTER TYPE (petit menu) · menus WAVEFORM / LFO SOURCE / DESTINATION / MONO-POLY · petits boutons **TONE, BODY, BRIGHT, WIDTH, TEXTURE, RES, ENV, DELAY** sous la cassette · molettes Pitch Bend / Modulation (CC1) · clavier · barre de presets.

Écarts avec la maquette : j'ai pris la 1re (chameau-faucon, cohérent avec la série ; la 2e est un aigle seul, avec des étiquettes « AS / S / DR » fausses). Dans la 1re, les 4 sliders de droite « A D S VEL » n'ont pas de release : celui du filtre suit le decay. Les menus y étaient mal étiquetés (le 3e affichait « MONO/POLY » sous « DESTINATION ») : je les rétablis en WAVEFORM / LFO SOURCE / DESTINATION / MONO-POLY, avec FILTER TYPE en petit menu à part. La maquette n'avait pas de bouton DELAY (niveau) ni de RES, ni de boutons de timbre : ajoutés sous la cassette. Les menus affichent la valeur courante. Typographie du titre (italique doré lumineux) approchée.

## Presets (70)
7 banques de 10 : Hawk Leads, Golden Brass, Royal Pads, Falcon Plucks, Sky Bells & Strings, Talon Bass, Eagle Cry & FX. Source : `Tools/gen_presets.py` → `Source/Presets.h`. Save écrit un `.ghpreset` dans `Documents\DJBILBOX BEATS\GOLDHAWK\Presets`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`GhTest`, skill dsp-quality-validation)
`cmake --build build --target GhTest && ./GhTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.10 (min 0.02, max 0.25), pic max 0.955.

## Limites
- Son **jamais écouté** : timbres = approximations de synthèse à régler à l'oreille (cri d'aigle, cuivres surtout). Presets courts (stabs, Pizz Gold) plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Vol de voix : coupure immédiate au-delà de 16 notes. Pas de signature de code.
