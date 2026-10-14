# WARZONE808 — synthé basse 808 by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Moteur existant du ZIP (oscillateurs PolyBLEP, filtre TPT, unison 7 voix, effets) complété avec processeur, interface, 60 presets d'usine + expansion Vol.1 (10), tests et scripts Windows.

## Ce qui a été ajouté au ZIP d'origine
`CMakeLists.txt`, `Source/PluginProcessor.*`, `Source/PluginEditor.*`, `Source/Params/Params.h`, `Source/Params/PresetData.h` (généré par `Tools/gen_presets.py`), `Assets/EXPANSION_WZ_VOL1.wz808exp`, `Tools/RenderTest.cpp`, scripts `.bat`. Le dossier `_backup_rockpunk_approach/` est conservé tel quel (non compilé).

## Corrections apportées au moteur (audit)
- Le **filtre d'enveloppe** (A / D / S de droite) n'était jamais exécuté (`filterEnv.tick()` absent, setters non définis) : branché ; **VELOCITY** applique maintenant la sensibilité.
- **Mono** : pile de notes tenues, legato (glide sans retrigger), retombée sur la note encore tenue ; avant, un relâchement coupait tout.
- **Allocation dans le callback audio** (`std::vector` par bloc) supprimée.
- **Note-off bloqué** quand on changeait Mono/Poly avec des notes tenues (voix coincées) : mode synchronisé avant le MIDI + note-off final qui relâche toutes les voix.
- **Vol de voix** : préfère une voix en release.
- **Delay** : TIME lissé et lu en interpolé (plus de clic), buffer vidé à la préparation.
- **Reverb** remplacée par `juce::Reverb` (l'ancienne, 2 combs fixes, était métallique et dépendait du sample rate).
- Queues d'effets vidées dans `prepareToPlay` ; bloqueur DC (~6 Hz) + genou doux en sortie.
- Niveau master calé (x0.45) : le moteur saturait (tanh) dès 0 dBFS.

## Interface (d'après la maquette)
RATE / AMOUNT / PITCH (±12 demi-tons), GLIDE, FILTER, VOLUME, REVERB, CHORUS, DISTORTION, TIME, FEEDBACK, ADSR amp (gauche), A / D / S du filtre + VELOCITY (droite), 5 menus, molettes, clavier, barre de presets, cassette EXPANSION.
**Ajouts** (non présents sur la maquette, nécessaires pour exposer le moteur) : RESO, ENV (profondeur d'enveloppe de filtre), UNISON, DETUNE, DLY MIX. La maquette indiquait « ASDR » au-dessus du 3e curseur de droite : je l'ai traité comme **S** (sustain du filtre).
Fenêtre redimensionnable. Paramètres cachés mais automatisables : tonalité de distorsion, rate/depth du chorus, taille/damping/width de la reverb, largeur d'unison.
Le bouton Save écrit un `.wz808preset` dans `Documents\DJBILBOX BEATS\WARZONE808\Presets` (menu Presets > User).

## Expansion (cassette)
`Assets/EXPANSION_WZ_VOL1.wz808exp` (10 presets) est embarqué. Cliquer la cassette : charger un autre pack `.wz808exp` (format `warzone808-expansion`, voir le fichier d'exemple) ou ouvrir le dossier `Documents\DJBILBOX BEATS\WARZONE808\Expansions` (chargé au démarrage).

## Presets
6 banques d'usine de 10 : 808 Subs, Dirty & Reese, Leads, Plucks & Keys, Pads & Ambience, FX & Percussion + « Expansion Vol.1 ». Régénérer : `python3 Tools/gen_presets.py Source/Params/PresetData.h Assets/EXPANSION_WZ_VOL1.wz808exp`.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). Dans FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`WzTest`)
`cmake --build build --target WzTest && ./WzTest` : 70 presets (finis, pic ≤ 1, DC), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes (vol de voix), MIDI aberrant, charge CPU, mono legato, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; pire cas 16 voix × 5 unisons ≈ 12 % d'un cœur (machine de test). `./WzTest <n> [précédent]` : profil RMS d'un preset.

## Limites
- Son **jamais écouté** : régler presets/gains au casque. Pas de pitch-drop (enveloppe de pitch) : un 808 « kick » classique demandera cet ajout.
- Compilé et testé sous GCC/Linux seulement ; non testé sous MSVC ni dans FL Studio.
- Avertissements `-Wsign-conversion` / `-Wfloat-equal` restants dans le code d'origine du moteur.
- Pas de signature de code ni d'installateur dédié.
