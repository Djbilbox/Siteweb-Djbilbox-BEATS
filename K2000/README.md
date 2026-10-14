# K2000 — synthé 80s synthwave / darksynth by Humpire / DJBILBOX BEATS

VST3 + Standalone (JUCE 8.0.4, C++17). Synthèse sans samples, 16 voix, **70 presets**. Interface d'après la maquette `Assets/faceplate_mockup.jpg` (aperçu : `screenshot.png`). Projet construit à partir du dossier fourni (cahier des charges, PARAMETER_MAP, MODULATION_MATRIX, presets, code).

## État du code fourni → ce qui a été fait
Le C++ fourni (`Reference/original_cpp/`) **ne compilait pas** : `Voice.h` utilisait `PolyBlepOscillator`, `MultimodeFilter`, `AdsrEnvelope`, `LowFrequencyOscillator`, alors que les fichiers définissaient `Oscillator`, `StateVariableFilter`, `ADSREnvelope`, `LFO` ; le `CMakeLists.txt` référençait des fichiers inexistants (`Source/Main/…`, `SynthEngine.cpp`, `LicenseManager.cpp`…) et un chemin JUCE local ; pas de bend/mod wheel, pas de modes de voix, pas de release d'amplitude, pas de résonance.
- **Conservé tel que fourni** : tous les ids et plages de `PARAMETER_MAP.md`, l'architecture (oscillateurs → filtre → ADSR → LFO avec enveloppe → distorsion → chorus → delay → reverb → limiteur), les équations de `MODULATION_MATRIX.md`, `Effects.cpp` (corrigé : reset d'état, bloqueur DC à ~10 Hz), les 16 presets d'usine (`presets.js` → `Tools/factory.json`).
- **Réécrit** (`Source/KVoice.h`, `PluginProcessor.*`, `PluginEditor.*`) : oscillateurs PolyBLEP (6 formes), filtre Moog ladder 4 pôles TPT/ZDF + SVF 12 dB (LP/BP/HP) + Acid, enveloppes ADSR, LFO 6 formes × enveloppe LFO × (amount + molette), 6 destinations (Cutoff, Pitch, Pulse Width, Volume, Chorus, Pan), glide, vélocité BANK/ADSR, modes Poly 8 / Poly 16 / Mono Retrig / Mono Legato, molettes, UI dessinée en code d'après la maquette.
- **Ajouts** (absents de la maquette et du brief, indispensables) : **RES** (résonance), **ENV** (profondeur d'enveloppe de filtre, qui suit le decay d'amplitude), **REL** (release d'amplitude), **DETUNE** (écart du SuperSaw). Ils sont dans les deux grilles de ventilation.
- **Nouveau** : 53 presets (banques Leads, Bass, Pads, Plucks & Arps, Keys & Brass, FX & Atmos) + CMake/scripts Windows + suite de tests.
- Non repris : le simulateur Web Audio (`Reference/web/`) est gardé tel quel, non modifié ni testé ; `LicenseManager` (inexistant, non demandé) ; AU (hors périmètre : VST3 + Standalone).

## Commandes
RATE / AMOUNT (LFO) · commutateur **PITCH** (envoie le LFO sur la hauteur, vibrato) · sliders LFO ATTACK/DECAY/SUSTAIN/RELEASE · sliders **ATTACK DECAY SUSTAIN BANK ADSR** (ADSR d'amplitude ; BANK = profondeur de vélocité sur le volume, ADSR = sensibilité de vélocité sur l'enveloppe) · VOLUME, REVERB, CHORUS, DISTORTION (master) · GLIDE · FILTER (cutoff) · DISTORTION (drive d'entrée du filtre) · DELAY (mix), TIME, FEEDBACK · menus WAVEFORM (Sawtooth, Square, Triangle, Sine, SuperSaw, 80s Brass) / LFO SOURCE / DESTINATION / FILTER TYPE (LP 24, LP 12, BP, HP, Acid) / MONO-POLY · molettes Pitch Bend (±2 demi-tons, retour au centre) et Modulation (CC1) · clavier · barre de presets (Save → `.k2preset`).

Écarts avec la maquette : la maquette n'a qu'un seul bouton « DISTORTION » dans la rangée du milieu et un autre en haut : les deux sont conservés (haut = distorsion master, milieu = drive du filtre). Pas de release d'amplitude sur la maquette : REL ajouté. Le clavier est celui de la maquette (~18 touches blanches), non les 25 touches du brief. Le titre « K2000 » (afficheur à LED) est approché par une police standard avec halo cyan.

## Presets (70)
`VOL.1 Factory` (17, ceux du projet fourni, valeurs inchangées ; les contrôles ajoutés y ont leur valeur par défaut) + 6 banques (53). Source : `Tools/gen_presets.py` → `Source/Presets.h`. Le bouton cassette EXPANSION est un placeholder.

## Compiler (Windows)
Visual Studio 2022 (C++), CMake, Git : `build_windows.bat` puis `install_windows.bat` (administrateur). FL Studio : Options > Manage plugins > Find installed plugins.

## Tests (`K2kTest`, skill dsp-quality-validation)
`cmake --build build --target K2kTest && ./K2kTest` : 70 presets (finis, DC < 0.02), matrice 44.1/48/88.2/96 kHz × blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, **4 modes de voix avec notes qui se chevauchent**, panic, sustain, queues terminées, changement de preset en lecture, rappel d'état. Résultat : **TOUT OK, 0 échec** ; rms médian ≈ 0.105 (min 0.007, max 0.21), pic max 0.57.

## Limites
- Son **jamais écouté** : filtre ladder, SuperSaw, 80s Brass et presets à régler à l'oreille (les 17 presets d'usine ont été écrits pour le moteur d'origine, qui n'existait pas sous forme compilable : leur rendu avec ce moteur peut différer de l'intention). Presets courts (plucks, arps) plus faibles en rms : monter VOLUME.
- Compilé/testé sous GCC/Linux uniquement ; non testé sous MSVC ni dans FL Studio.
- Poly 8 limite à 8 voix ; en mono, la pédale de sustain est ignorée. Pas de signature de code.
