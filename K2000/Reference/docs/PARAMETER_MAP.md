# PARAMETER MAP — K2000 SYNTHESIZER

Ce document répertorie l'intégralité des paramètres audio du synthétiseur **K2000**, leurs plages de valeurs, leurs comportements par défaut, leurs caractéristiques d'automatisation et leurs liaisons avec les composants animés de l'interface utilisateur.

---

## 1. Table Complète des Paramètres

| ID Paramètre | Nom Affiché | Min | Max | Défaut | Unité | Courbe | Auto | Élément UI & Animation |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `lfo_rate` | RATE | 0.05 | 25.0 | 4.5 | Hz | Log | Oui | Knob rotatif LFO RATE (rotation -135° à +135°, arc cyan) |
| `lfo_amount` | AMOUNT | 0.0 | 1.0 | 0.35 | % | Lin | Oui | Knob rotatif LFO AMOUNT (rotation -135° à +135°, arc cyan) |
| `lfo_pitch` | PITCH | 0 | 1 | 0 | Switch | Discrète| Oui | Commutateur à bascule PITCH (levier 3D + voyant cyan) |
| `lfo_attack` | ATTACK (LFO) | 0.001 | 5.0 | 0.1 | s | Log | Oui | Fader vertical LFO Attack (curseur métal + LED cyan) |
| `lfo_decay` | DECAY (LFO) | 0.005 | 8.0 | 0.5 | s | Log | Oui | Fader vertical LFO Decay (curseur métal + LED cyan) |
| `lfo_sustain` | SUSTAIN (LFO)| 0.0 | 1.0 | 1.0 | % | Lin | Oui | Fader vertical LFO Sustain (curseur métal + LED cyan) |
| `lfo_release` | RELEASE (LFO)| 0.005 | 8.0 | 0.4 | s | Log | Oui | Fader vertical LFO Release (curseur métal + LED cyan) |
| `amp_attack` | ATTACK (AMP) | 0.001 | 5.0 | 0.02 | s | Log | Oui | Fader vertical AMP Attack (curseur métal + LED cyan) |
| `amp_decay` | DECAY (AMP) | 0.005 | 8.0 | 0.8 | s | Log | Oui | Fader vertical AMP Decay (curseur métal + LED cyan) |
| `amp_sustain` | SUSTAIN (AMP)| 0.0 | 1.0 | 0.7 | % | Lin | Oui | Fader vertical AMP Sustain (curseur métal + LED cyan) |
| `velocity_bank`| BANK | 0.0 | 1.0 | 0.75 | % | Lin | Oui | Fader vertical Velocity BANK (curseur métal + LED cyan) |
| `velocity_adsr`| ADSR (Sens) | 0.0 | 1.0 | 0.4 | % | Lin | Oui | Fader vertical Velocity ADSR (curseur métal + LED cyan) |
| `master_volume`| VOLUME | 0.0 | 1.0 | 0.8 | % | Lin | Oui | Knob rotatif Master VOLUME (rotation -135° à +135°, arc cyan) |
| `fx_reverb` | REVERB | 0.0 | 1.0 | 0.35 | % | Lin | Oui | Knob rotatif FX REVERB (rotation -135° à +135°, arc cyan) |
| `fx_chorus` | CHORUS | 0.0 | 1.0 | 0.45 | % | Lin | Oui | Knob rotatif FX CHORUS (rotation -135° à +135°, arc cyan) |
| `fx_distortion`| DISTORTION | 0.0 | 1.0 | 0.25 | % | Lin | Oui | Knob rotatif FX DISTORTION (rotation -135° à +135°, arc cyan) |
| `glide_time` | GLIDE | 0.0 | 1.0 | 0.05 | s | Log | Oui | Knob rotatif GLIDE (rotation -135° à +135°, arc cyan) |
| `filter_cutoff`| FILTER | 20.0 | 20000.0| 3200.0 | Hz | Log | Oui | Knob rotatif FILTER Cutoff (rotation -135° à +135°, arc cyan) |
| `filter_drive` | DISTORTION (Mid)| 0.0 | 1.0 | 0.15 | % | Lin | Oui | Knob rotatif Mid DISTORTION (rotation -135° à +135°, arc cyan) |
| `fx_delay_mix` | DELAY | 0.0 | 1.0 | 0.3 | % | Lin | Oui | Knob rotatif FX DELAY (rotation -135° à +135°, arc cyan) |
| `fx_delay_time`| TIME | 0.01 | 1.0 | 0.375 | s | Lin | Oui | Knob rotatif DELAY TIME (rotation -135° à +135°, arc cyan) |
| `fx_delay_fb` | FEEDBACK | 0.0 | 0.95 | 0.45 | % | Lin | Oui | Knob rotatif DELAY FEEDBACK (rotation -135° à +135°, arc cyan) |
| `osc_waveform` | WAVEFORM | 0 | 5 | 0 (Saw) | Index | Discrète| Oui | Menu déroulant WAVEFORM (popup cyber cyan) |
| `lfo_source` | LFO SOURCE | 0 | 5 | 1 (Tri) | Index | Discrète| Oui | Menu déroulant LFO SOURCE (popup cyber cyan) |
| `lfo_dest` | DESTINATION | 0 | 5 | 0 (Cutoff)| Index | Discrète| Oui | Menu déroulant DESTINATION (popup cyber cyan) |
| `filter_type` | FILTER TYPE | 0 | 4 | 0 (LP24)| Index | Discrète| Oui | Menu déroulant FILTER TYPE (popup cyber cyan) |
| `voice_mode` | MONO/POLY | 0 | 3 | 0 (Poly8)| Index | Discrète| Oui | Menu déroulant MONO/POLY (popup cyber cyan) |
| `pitch_bend` | PITCH BEND | -1.0 | 1.0 | 0.0 | Semi | Lin | Oui | Molette PITCH BEND (rappel ressort au centre 0.0) |
| `mod_wheel` | MODULATION | 0.0 | 1.0 | 0.0 | % | Lin | Oui | Molette MODULATION (maintien de position latching) |

---

## 2. Table des Menus Déroulants (Index ↔ Valeurs)

### 2.1. `osc_waveform`
- `0` : Sawtooth (Dent de scie)
- `1` : Square / Pulse (Carrée / Impulsion)
- `2` : Triangle (Triangulaire)
- `3` : Sine (Sinusoïdale)
- `4` : SuperSaw (Multi-dents de scie désaccordées)
- `5` : 80s Brass (Cuivres analogiques vintage)

### 2.2. `lfo_source`
- `0` : Sine
- `1` : Triangle
- `2` : Square
- `3` : Saw Up
- `4` : Saw Down
- `5` : Sample & Hold (Random)

### 2.3. `lfo_dest`
- `0` : Filter Cutoff (Fréquence de coupure)
- `1` : Pitch (Hauteur des oscillateurs)
- `2` : Pulse Width (Largeur d'impulsion)
- `3` : Master Volume (Trémolo)
- `4` : Chorus Depth (Modulation d'espace)
- `5` : Stereo Pan (Auto-pan)

### 2.4. `filter_type`
- `0` : LowPass 24dB (Moog Ladder 4-pole)
- `1` : LowPass 12dB (State Variable 2-pole)
- `2` : BandPass 12dB
- `3` : HighPass 12dB
- `4` : Acid LowPass (Diode Ladder avec distorsion de résonance)

### 2.5. `voice_mode`
- `0` : Poly 8 Voices
- `1` : Poly 16 Voices
- `2` : Mono Retrigger
- `3` : Mono Legato
