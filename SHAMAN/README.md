# SHAMAN — DJBILBOX BEATS (VST3 / Standalone, JUCE 8.0.4)

Synthé tribal / rituel, 8 moteurs : Didgeridoo, Tribal Drum, Jaw Harp, Spirit Flute, Chant Choir, Fire Pad, Bone Chimes, Earth Drone.
70 presets (7 banques x 10), presets utilisateur `.shpreset` dans `Documents\DJBILBOX BEATS\SHAMAN\Presets`.

## Build (Windows, VS2022 + CMake)
`build_windows.bat` puis `install_windows.bat` (copie du VST3).

## Écarts avec la maquette
- 3 maquettes fournies : version « b » retenue (la plus propre).
- Chiffres « 5 » parasites ignorés ; jacks IN/U/L/X décoratifs non repris ; menu « MENU » des maquettes a/c non repris.
- TIME / FEEDBACK existaient sans DELAY → bouton DELAY ajouté (haut-parleur droit).
- Boutons ajoutés dans les haut-parleurs : TONE, BODY, BRIGHT, WIDTH, TEXTURE, RES, ENV, DELAY.
- Titre « ASDR + VELOCITY » retiré (chevauchait REVERB). Marque écrite « DJBILBOX BEATS ».

## Validation (Linux, headless)
Test `ShTest` : 70 presets, matrice 44.1/48/88.2/96 kHz x blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues, changement de preset en lecture, rappel d'état : 0 échec. RMS médian 0,136, pic max 0,938.

## Honnêteté
Le son n'a pas été écouté (validation numérique uniquement). Pas de compilation MSVC ni test FL Studio : build Windows non vérifié.
