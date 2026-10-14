# SAN ANDREAS LEAD — DJBILBOX BEATS (VST3 / Standalone, JUCE 8.0.4)

Synthé West Coast / G-Funk, 8 moteurs : G-Funk (whistle), Moog, Square, Talkbox, Bass, Rhodes, Strings, Clav.
70 presets (7 banques x 10), presets utilisateur `.sapreset` dans `Documents\DJBILBOX BEATS\SAN_ANDREAS_LEAD\Presets`.

## Build (Windows, VS2022 + CMake)
`build_windows.bat` puis `install_windows.bat`.

## Écarts avec la maquette
- 3 maquettes fournies (1 doublon exact) : la version sombre (…1840) retenue, la plus complète et lisible.
- la maquette cuivre a des listes vides et des libellés fautifs ("ASDR", "V") ; la maquette 3 (OSC/DRIVE/KYBD) ajoute des contrôles non cohérents : non reprises.
- "A D S R VELOCITY" (droite) = enveloppe de filtre + vélocité ; ADSR de gauche = enveloppe d'amplitude.
- "MIX" = niveau du delay (le titre "DELAY" est au-dessus de TIME / FEEDBACK / MIX).
- Haut-parleurs vides dans la maquette : 7 petits boutons ajoutés (TONE, BODY, BRIGHT, WIDTH, TEXTURE, RES, ENV).
- Noms de moteurs et de sources LFO raccourcis pour tenir dans les menus (Sine/Tri/Saw/Square/S&H).
- Marque écrite "DJBILBOX BEATS".

## Validation (Linux, headless)
Test `SaTest` : 70 presets, matrice 44.1/48/88.2/96 kHz x blocs 32/480/2048, silence, notes rapides, 40 notes, MIDI aberrant, panic, sustain, queues, changement de preset en lecture, rappel d'état : 0 échec. RMS médian 0,124, pic max 0,945.

## Honnêteté
Le son n'a pas été écouté (validation numérique uniquement). Pas de compilation MSVC ni test FL Studio.
