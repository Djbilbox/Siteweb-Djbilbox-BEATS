# ROCKPUNK GUITAR — PARAMETER_MAP.md (carte complète des paramètres)

> Convention : `ID` = identifiant APVTS (utilisé dans le code, l'automation et les presets `.rpk`).
> `skew` : `log` = échelle perceptuelle (`NormalisableRange` skew < 1 pour dB/Hz/ms),
> `lin` = linéaire, `step` = choix discret (int).
> Racine d'état APVTS : `ROCKPUNK_GUITAR_STATE` (l'ancien synthé utilisait `ROCKPUNK_STATE`).
> Moteur : double couche de cordes pincées (Karplus-Strong) A/B + ampli « Urban Slayer » + FX signature ROCKPUNK (Drive, Delay).

## 1. Commun / Master

| ID | Nom affiché | Min | Max | Défaut | Unité | Échelle | Ctrl |
|---|---|---|---|---|---|---|---|
| `layerMix` | LAYER MIX | 0 | 100 | 50 | % | lin | Knob |
| `masterVol` | VOLUME | -60 | 6 | -3 | dB | log | Knob (MACRO) |
| `outWidth` | WIDTH | 0 | 100 | 80 | % | lin | Knob |
| `outLimiter` | LIMITER | 0 | 1 | 1 | — | step (off/on) | Toggle LED |

## 2. Couche A (référence rythmique / clean)

| ID | Nom affiché | Min | Max | Défaut | Unité | Échelle | Ctrl |
|---|---|---|---|---|---|---|---|
| `laOn` | LAYER A ON | 0 | 1 | 1 | — | step | Switch 2 pos. |
| `laBlend` | LAYER A BLEND | 0 | 100 | 80 | % | lin | Knob |
| `laTone` | LAYER A TONE | 0 | 100 | 60 | % | lin | Knob |
| `laBody` | LAYER A BODY | 0 | 100 | 30 | % | lin | Knob (résonateur) |
| `laSustain` | LAYER A SUSTAIN | 0 | 100 | 35 | % | lin | Knob |
| `laPick` | LAYER A PICK | 0 | 100 | 30 | % | lin | Knob (brightness du plectre) |
| `laLevel` | LAYER A LEVEL | 0 | 1 | 1 | — | lin | Slider horiz. |

## 3. Couche B (lead / hot)

| ID | Nom affiché | Min | Max | Défaut | Unité | Échelle | Ctrl |
|---|---|---|---|---|---|---|---|
| `lbOn` | LAYER B ON | 0 | 1 | 1 | — | step | Switch 2 pos. |
| `lbBlend` | LAYER B BLEND | 0 | 100 | 90 | % | lin | Knob |
| `lbTone` | LAYER B TONE | 0 | 100 | 25 | % | lin | Knob |
| `lbBody` | LAYER B BODY | 0 | 100 | 25 | % | lin | Knob (résonateur) |
| `lbSustain` | LAYER B SUSTAIN | 0 | 100 | 55 | % | lin | Knob |
| `lbPick` | LAYER B PICK | 0 | 100 | 70 | % | lin | Knob (brightness du plectre) |
| `lbLevel` | LAYER B LEVEL | 0 | 1 | 1 | — | lin | Slider horiz. |

## 4. Ampli « Urban Slayer »

| ID | Nom affiché | Min | Max | Défaut | Unité | Échelle | Ctrl |
|---|---|---|---|---|---|---|---|
| `ampOn` | AMP ON | 0 | 1 | 1 | — | step | Switch 2 pos. |
| `ampGain` | AMP GAIN | 0 | 24 | 6 | dB | log | Knob |
| `ampDist` | AMP DIST | 0 | 100 | 55 | % | lin | Knob |
| `ampTone` | AMP TONE | 100 | 10000 | 2500 | Hz | log | Knob |
| `ampLevel` | AMP LEVEL | -12 | 12 | 0 | dB | lin | Knob |

## 5. Drive (signature ROCKPUNK)

| ID | Nom affiché | Min | Max | Défaut | Unité | Échelle | Ctrl |
|---|---|---|---|---|---|---|---|
| `driveAmt` | DRIVE | 0 | 100 | 20 | % | lin | Knob (MACRO) |
| `driveType` | DRIVE TYPE | 0 | 3 | 0 | — | step (Soft/Hard/Asym/Fuzz) | Switch 4 positions |
| `driveTone` | DRIVE TONE | 100 | 10000 | 2500 | Hz | log | Knob |

## 6. Delay (signature ROCKPUNK)

| ID | Nom affiché | Min | Max | Défaut | Unité | Échelle | Ctrl |
|---|---|---|---|---|---|---|---|
| `delayOn` | DELAY ON | 0 | 1 | 0 | — | step | Toggle LED |
| `delayTime` | DELAY TIME | 10 | 2000 | 350 | ms | log | Knob |
| `delaySync` | DELAY SYNC | 0 | 1 | 0 | — | step | Toggle LED |
| `delayDiv` | DELAY DIV | 0 | 8 | 2 | — | step (1/16…4 bar) | Switch |
| `delayFb` | DELAY FB | 0 | 95 | 40 | % | lin | Knob |
| `delayMix` | DELAY MIX | 0 | 100 | 25 | % | lin | Knob (MACRO) |
| `delayLP` | DELAY LP | 100 | 20000 | 5000 | Hz | log | Knob |

## 7. Récapitulatif

- **Total paramètres** : 35 (dont 5 booléens, 1 int step (`driveType`), 29 floats continus).
- Tous automatisables via APVTS ; tous référencés par l'éditeur (contrôle + animation néon).
- `driveType` 0..3 : 0 Soft (tanh), 1 Hard (clip), 2 Asymmetric, 3 Fuzz.
- `delayDiv` 0..8 : 1/16, 1/8T, 1/8, 1/4T, 1/4, 1/2, 1 bar, 2 bar, 4 bar (avec `delaySync` activé).
- Plages DSP bornées (`clamp`/`jlimit`) → valeurs invalides impossibles.

## 8. Face avant et couche échantillonnée (ajoutés pour la 1.1.0)

La liste complète des **65** paramètres est dans `Source/Params.h`. En plus des
sections ci-dessus :

| ID | Nom affiché | Plage | Défaut | Contrôle |
|---|---|---|---|---|
| `envA` `envD` `envS` `envR` | Attack / Decay / Sustain / Release | ms, ms, %, ms | 3 / 600 / 70 / 220 | 4 curseurs de gauche |
| `glideMs` | Glide | 0–1000 ms | 0 | GLIDE |
| `monoMode` | Mono | off / on | off | [MONO/POLY] |
| `revMix` `revSize` | Reverb / Reverb Size | 0–100 % | 18 / 60 | REVERB (taille : hôte) |
| `chorAmt` | Chorus | 0–100 % | 0 | CHORUS |
| `lfoRate` `lfoAmount` `lfoPitch` | LFO Rate / Amount / Pitch | 0,05–20 Hz, % | 5 / 0 / 0 | RATE / AMOUNT / PITCH |
| `lfoWave` `lfoDest` | LFO Wave / Destination | Sine Triangle Square S&H ; Pitch Filter Volume Drive | 0 / 1 | [LFO SOURCE] / [DESTINATION] |
| `fltType` `fltCutoff` `fltReso` | Filter Type / Filter / Resonance | LP BP HP Off ; 60–18000 Hz ; 0,5–8 | Off / 14000 / 0,9 | [FILTER TYPE] / FILTER (résonance : hôte) |
| `fenvA` `fenvD` `fenvS` `fenvR` | Filter Attack / Decay / Sustain / Release | ms, ms, %, ms | 5 / 400 / 100 / 300 | 4 curseurs de droite (A S D R) |
| `velAmt` | Velocity | 0–100 % | 60 | curseur VELOCITY |
| `pbend` `modDepth` | Pitch Bend / Modulation | ±2 st ; 0–100 % | 0 / 0 | molettes |
| `pickShape` | Pick Shape | Soft Round Bright Sharp | Round | [WAVEFORM] |
| `smpOn` `smpSel` `smpRoot` `smpLevel` `smpLoop` `smpAttack` `smpRelease` | Sample … | couche échantillonnée | off | menu cassette > Your samples (le reste : hôte) |

`laOn` et `lbOn` sont les deux **grilles de haut-parleur** de la façade ; le delay
s'allume en tournant TIME ou FEEDBACK (ou depuis le menu de la cassette).
