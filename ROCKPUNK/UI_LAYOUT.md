> **MAJ 0.2.2** : la disposition est reconstruite sur les **mesures reelles** du mockup (rockpunk_ui_dimensions, design = mesure x 0.8) : cassette en haut-gauche, LFO, ADSR vertical, wheels, GLIDE/FILTER, 6 potards droits, 5 faders, 5 combos, clavier en bas. Geometrie et recadres dans layout.json. Mapping parametres : PROJECT_STATUS 1quater. Les sections ci-dessous restent le cahier d'animation visuel.

# ROCKPUNK — UI_LAYOUT.md (disposition + cahier d'animation)

> **Mise à jour 2026-08-16 (ROCKPUNK GUITAR)** : la disposition est désormais **pilotée par le code**
> (`Source/PluginEditor.h/.cpp`, design 1120×836) et surchargée par **`layout.json`** à la racine du
> projet (rectangles de composants ; l'image de fond est le premier `*.jpeg` du dossier projet).
> Le présent document conserve le **cahier d'animation / langage visuel** (knobs chrome + pointeur,
> dark mode, néon violet/rose) qui reste d'actualité. Les sections sur le canvas 900×560 et le
> positionnement précis sont obsolètes → se référer à `layout.json`.

> **Avertissement honnête** : le modèle d'IA de cette session ne lit pas les images. Le cahier
> d'animation ci-dessous est fondé sur (1) les conventions documentées de la gamme HUMPIRE /
> des Skills (`vst-design-3d-ui`), (2) la promesse visuelle rock/punk, (3) des hypothèses marquées
> **« ⚠ À VÉRIFIER »**. Chaque contrôle doit être confronté à l'image maître
> `Synthesizer_plugin_interface_mockup_2K_202608121123.jpeg` (voir `TEST_PLAN.md` § UI).

## 1. Hiérarchie d'écran (canvas 900 × 560, résolution indépendante, scalable)

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ BANDEAU TOP : LOGO HUMPIRE · ROCKPUNK · MODE POLY/MONO · GLIDE · VU-LED OUT  │
├──────────────────────────────────────────────────────────────────────────────┤
│ MACROS :  DRIVE ▸▸▸ | CUT ▸▸▸ | DELAY ▸▸▸ | VOLUME ▸▸▸            (4 gros knobs) │
├──────────────────────────────────────────────────────────────────────────────┤
│ OSC A ▸▸▸ | OSC B ▸▸▸ | SUB ▸▸▸ | NOISE ▸▸▸ | MIXER ▸▸▸                       │
│  SHAPE SEMI DET PW LVL │ SHAPE SEMI DET PW LVL │ SHAPE SEMI LVL │ LVL │ DRIVE │
├──────────────────────────────────────────────────────────────────────────────┤
│ FILTRE ▸▸▸ | ENV FILTRE ▸▸▸ | ENV AMP ▸▸▸ | LFO 1 ▸▸▸ | LFO 2 ▸▸▸          │
│ TYPE CUT RES ENVAMT KEY DRIVE │ A D S R │ A D S R │ SHAPE RATE SYNC AMT │ ... │
├──────────────────────────────────────────────────────────────────────────────┤
│ DRIVE ▸▸▸ | DELAY ▸▸▸ | MOD MATRIX ▸▸▸ | OUTPUT ▸▸▸ | PRESETS ▸▸▸          │
│ AMT TYPE TONE │ ON TIME SYNC FB MIX LP │ S1 S2 │ VOL WIDTH LIMITER │ BROWSER │
└──────────────────────────────────────────────────────────────────────────────┘
```

- Taille minimale des contrôles : **30 px** (touch-friendly), knobs macros : 56 px.
- Sections délimitées par des panneaux « rack » avec texte de module (monospace capital).
- Contraste texte ≥ 4.5:1 (checklist UI). Valeurs numériques en monospace.

## 2. Cahier d'animation — règles communes dérivées du modèle maître

Principes généraux documentés de la marque (⚠ à vérifier image à image) :
- **Finition** : métal/chrome brossé, reflets lisses, émission légère sur les accents.
- **Pointer de valeur** : LED rouge/ambre alignée sur la valeur (rotation exacte = proportion).
- **Curseur souris** : pointeur main sur tout contrôle interactif.
- **Hover** : léger grossissement (×1.04) + éclaircissement du ring + halo.
- **Pressed/drag** : ring accentué, ombre portée réduite, pointeur lumineux.
- **Transition** : toute rotation/fade smooth (~100 ms), jamais de saut brusque.
- **Feedback de valeur** : tooltip + affichage numérique au-dessus du knob pendant le drag.

### 2.1 Knob rotatif (le plus répandu)
- Plage de rotation : **-135° … +135°** (270°), pointeur vers le haut au défaut de paramètre
  (position 0 / mid pour les bi-polaires).
- **Sens** : horaire = valeur croissante (standard audio).
- **États** : repos (pointeur fixe) → hover (halo) → drag (rotation + valeur affichée +
  pointeur lumineux).
- **Rendu** : `AnimatedKnob` (JUCE) : rotation = `valueToProportionOfLength()`, redraw throttlé
  par `Timer` (30 fps), zéro animation bloquante sur le thread audio.

### 2.2 Switch (bascule / bouton)
- **États distincts** : OFF = mat sombre, ON = illuminé (LED) + texte lumineux.
- **Feedback** : clic → LED s'allume instantanément, pulse bref (2 frames), rebond jamais > 1 clic.
- 3 positions (mode voice, type filtre, forme, type drive) : bascule pas-à-pas, LED sur la position active.

### 2.3 Slider / fader
- **Course** : verticale pour faders (de bas en haut = valeur croissante) ou horizontale pour
  les mixes (gauche → droite = croissant).
- **Indicateur** : capuchon métal + rail lumineux proportionnel + LED de tête.

### 2.4 LED indicatrices
- LED de section : verte à l'actif, grise sinon.
- **VU-LED** (sortie) : gradient vert → ambre → rouge, mise à jour ~30 ms (peak-hold 1 s).
- LED d'état DELAY ON / LIMITER : verte quand actif.

### 2.5 Pad / écran (si ajouté en V1.1)
- Non retenu en V1 (documenté pour cohérence future : pas de pad, pas d'écran tactiles).

## 3. Mapping contrôle → paramètre (résumé ; détail dans `PARAMETER_MAP.md`)

| Contrôle UI | Type | Paramètre | Animation |
|---|---|---|---|
| MACRO DRIVE | Knob 56px | `driveAmt` | rotation 270° + pointeur LED |
| MACRO CUT | Knob 56px | `filterCut` | idem (log) |
| MACRO DELAY | Knob 56px | `delayMix` | idem |
| MACRO VOLUME | Knob 56px | `masterVol` | idem |
| OSC A SHAPE | Switch 3 positions | `oscAShape` | LED position active |
| OSC A SEMI | Knob | `oscASemi` | rotation, tick intégrés |
| OSC A DET | Knob | `oscACent` | rotation bi-polaire |
| OSC A PW | Knob | `oscAPW` | rotation |
| OSC A LVL | Slider horizontal | `oscALevel` | rail proportionnel |
| OSC B * | idem OSC A | `oscB*` | idem |
| SUB SHAPE / SEMI / LVL | Switch + Knob + Slider | `sub*` | idem |
| NOISE LVL | Slider horizontal | `noiseLevel` | idem |
| MIX DRIVE | Knob | `mixDrive` | rotation |
| FILTER TYPE | Switch 3 positions | `filterType` | LED |
| CUT / RES | Knobs | `filterCut` / `filterRes` | rotation |
| ENV AMT / KEY / DRV | Knobs | `filterEnvAmt` / `filterKeyTrk` / `filterDrive` | rotation |
| ENV A · D · S · R (×2) | Knobs | `amp*` / `filt*` | rotation |
| LFO1/2 SHAPE | Switch | `lfo1Shape` / `lfo2Shape` | LED |
| LFO1/2 RATE | Knob | `lfo*Rate` | rotation |
| LFO1/2 SYNC | Toggle LED | `lfo*Sync` | LED ON/OFF |
| LFO1/2 DIV | Switch | `lfo*Div` | LED |
| LFO1 AMT (filtre) | Knob bi-polaire | `lfo1Amt` | rotation |
| LFO2 AMT (pitch) | Knob bi-polaire | `lfo2Amt` | rotation |
| MOD1/2 SRC/DST | Switchs | `mod*Src/Dst` | LED |
| MOD1/2 AMT | Knob bi-polaire | `mod*Amt` | rotation |
| DRIVE AMT / TONE | Knobs | `driveAmt` / `driveTone` | rotation |
| DRIVE TYPE | Switch 4 positions | `driveType` | LED |
| DELAY ON | Toggle LED | `delayOn` | LED |
| DELAY TIME / FB / MIX / LP | Knobs | `delayTime/Fb/Mix/LP` | rotation |
| DELAY SYNC / DIV | Toggle + Switch | `delaySync` / `delayDiv` | LED |
| VOICE MODE | Switch 3 positions | `voiceMode` | LED |
| GLIDE | Knob | `glideTime` | rotation |
| OUT VOL / WIDTH | Knobs | `masterVol` / `outWidth` | rotation |
| LIMITER | Toggle LED | `outLimiter` | LED |
| VU OUT | Meter | — (lecture) | gradient LED 30 ms |
| PRESET BROWSER | Bouton + list | — (charge `.rpk`) | feedback clic + label |

## 4. Contrôles du modèle maître sans équivalent ROCKPUNK

Décision documentée plutôt qu'invention (règle §3 du cahier des charges) :
- **Pad XY, écran LCD, séquencer intégré** : absents du concept V1 → si le modèle maître les
  montre, documenter « non implémenté en V1 » plutôt que de les simuler.
- Tout contrôle supplémentaire visible sur le modèle maître devra être ajouté à ce tableau
  (étape humaine, voir `KNOWN_LIMITATIONS.md`).