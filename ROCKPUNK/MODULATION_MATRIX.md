# ROCKPUNK — MODULATION_MATRIX.md (matrice de modulation V1)

## 1. Sources

| ID | Source | Plage utile | Détail |
|---|---|---|---|
| 0 | OFF | — | slot inactif |
| 1 | LFO1 | -1 … +1 | forme `lfo1Shape`, Hz ou sync `lfo1Div` |
| 2 | LFO2 | -1 … +1 | forme `lfo2Shape`, Hz ou sync `lfo2Div` |
| 3 | ENV FILTRE | 0 … 1 | enveloppe filtre (ADSR `filt*`) |
| 4 | MODWHEEL | 0 … 1 | CC1 |
| 5 | VELOCITY | 0 … 1 | vélocité de la note |
| 6 | AFTERTOUCH | 0 … 1 | poly/chan après-touch |
| 7 | RANDOM | -1 … +1 | S&H lent (nouvelle valeur aléatoire chaque ~50 ms par voix) |

## 2. Destinations

| ID | Destination | Transformation amount → effet |
|---|---|---|
| 0 | OFF | — |
| 1 | PITCH (OSC A+B) | amount % → ±cents (échelle = ±1200 cents × amount/100) |
| 2 | FILTRE CUT | amount % → proportion log du range cutoff (× ±100 %) |
| 3 | FILTRE RES | amount % → offset ±(resRange × amount/100) |
| 4 | PW A | amount % → offset ±(pwRange × amount/100) |
| 5 | PW B | idem PW A |
| 6 | LEVEL A | amount % → offset ±(level × amount/100) |
| 7 | LEVEL B | idem LEVEL A |
| 8 | PAN | amount % → offset ±0.5 autour du pan de voix |

## 3. Règles d'application

1. **Un slot = une source × une destination × un amount** (2 slots au total en V1).
2. Amount **bi-polaire** (-100..100) : signe inversé pour le sens.
3. Toute destination est **clampée** à sa plage (`PARAMETER_MAP.md`) — aucune valeur hors bornes.
4. Pitch converti en rapport de fréquence : `2^(cents/1200)` ; appliqué **avant** glide.
5. Modulations de cutoff/résolution appliquées **après** keytrack & env filtre, puis clamp 20 Hz–20 kHz.
6. Sources modwheel/velocity/aftertouch lues une fois par bloc (non lissées à chaque échantillon).

## 4. LFO → cible fixe (hors matrice)

| LFO | Cible fixe | Amount |
|---|---|---|
| LFO1 | FILTRE CUT | `lfo1Amt` (-100..100 %) |
| LFO2 | PITCH (OSC A+B) | `lfo2Amt` (-1200..1200 cents) |

> Ces deux liaisons sont dédiées (fast path, pas de slot consommé). La matrice (2 slots)
> reste entièrement libre par-dessus.

## 5. Exemple de patch de départ (« Punk Riff »)

| Slot | Src | Dst | Amt |
|---|---|---|---|
| 1 | MODWHEEL | FILTRE CUT | +40 |
| 2 | LFO1 | FILTRE RES | +35 |

## 6. Ordre de priorité des modulations

```
pitchOsc = (note + semi) * (1 + detune) * 2^(modPitch/1200)
cut = clamp(baseCut + keytrack + envFilt*amt + lfo1Amt + Σ modCUT, 20, 20000)
res = clamp(baseRes + Σ modRES, 0.5, 10)
```
- Glide appliqué à la fréquence cible avant les mods de pitch.
- Les sommes sont **additives**, jamais multiplicatives, pour préserver la musicalité.