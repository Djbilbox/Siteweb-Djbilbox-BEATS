# MODULATION MATRIX — K2000 SYNTHESIZER

Ce document explicite les routes de modulation, les équations de calcul et les interactions dynamiques entre les contrôleurs physiques, les générateurs de modulation et les processeurs audio du **K2000**.

---

## 1. Vue d'Ensemble de la Matrice

```
+-------------------+-------------------------------------------------------------+
| SOURCE            | DESTINATIONS PRINCIPALES & EFFETS                           |
+-------------------+-------------------------------------------------------------+
| LFO (Source * Env)| -> [Cutoff, Pitch, PulseWidth, Volume, Chorus, Pan]         |
| Switch PITCH      | -> Forçage immédiat du LFO vers le Pitch global (Vibrato)   |
| Velocity          | -> Amplitude globale (BANK) + Sensibilité Enveloppe (ADSR)  |
| Pitch Bend Wheel  | -> Hauteur de l'oscillateur (+/- 2 à +/- 12 demi-tons)      |
| Modulation Wheel  | -> Profondeur LFO + Ouverture Filtre + Intensité Chorus     |
| Glide Knob        | -> Vitesse de transition de note (Portamento logarithmique) |
| LFO ADSR Envelope | -> Mise à l'échelle temporelle de la modulation LFO         |
+-------------------+-------------------------------------------------------------+
```

---

## 2. Équations de Modulation Détaillées

### 2.1. Signal du LFO Modulé (LFO Composite)
Le signal de sortie brut du LFO $LFO_{raw}(t)$ dépend de la forme d'onde sélectionnée dans `LFO SOURCE` :
- Sine : $\sin(2\pi \cdot f_{lfo} \cdot t)$
- Triangle : $4 |(f_{lfo} \cdot t \bmod 1) - 0.5| - 1$
- Square : $\text{sgn}(\sin(2\pi \cdot f_{lfo} \cdot t))$
- Saw Up : $2 (f_{lfo} \cdot t \bmod 1) - 1$
- Saw Down : $1 - 2 (f_{lfo} \cdot t \bmod 1)$
- Random (S&H) : Valeur aléatoire maintenue à chaque période $1 / f_{lfo}$.

Le signal final $LFO_{out}(t)$ appliqué aux destinations est profilé par l'enveloppe LFO (LFO ADSR) et modulé par le potentiomètre `AMOUNT` et la molette de modulation :
$$LFO_{eff} = LFO_{raw}(t) \cdot Env_{LFO}(t) \cdot \left( Amount + ModWheel \cdot (1 - Amount) \right)$$

### 2.2. Routage vers les Destinations

#### A. Filter Cutoff (Destination 0)
$$f_{cutoff}(t) = Cutoff_{base} \cdot 2^{\left( LFO_{eff} \cdot 4.0 \right)}$$
(Permet un balayage dynamique jusqu'à +/- 4 octaves).

#### B. Master Pitch (Destination 1 ou Switch PITCH = ON)
$$f_{osc}(t) = f_{note} \cdot 2^{\left( \frac{PitchBend \cdot 2.0 + LFO_{eff} \cdot 1.0 + SwitchPitch \cdot LFO_{eff} \cdot 0.5}{12} \right)}$$

#### C. Pulse Width (Destination 2)
$$PW(t) = 0.5 + 0.45 \cdot LFO_{eff}$$

#### D. Master Volume (Tremolo - Destination 3)
$$Gain_{tremolo}(t) = 1.0 - 0.5 \cdot \max(0, LFO_{eff})$$

#### E. Chorus Depth (Destination 4)
$$Chorus_{depth}(t) = \text{clamp}\left( Chorus_{base} + 0.5 \cdot LFO_{eff}, 0, 1 \right)$$

#### F. Stereo Pan (Destination 5)
$$Pan_L(t) = \cos\left( \frac{\pi}{4} (1 + LFO_{eff}) \right), \quad Pan_R(t) = \sin\left( \frac{\pi}{4} (1 + LFO_{eff}) \right)$$

---

## 3. Dynamique de Vélocité (Section ADSR + VELOCITY BANK)

La vélocité d'une touche $v \in [0, 127]$ est normalisée $v_{norm} = v / 127$.

1. **Impact sur le Gain Global (Fader `BANK`)** :
   $$Gain_{voice} = (1.0 - Bank_{depth}) + Bank_{depth} \cdot v_{norm}$$
2. **Impact sur le Contour d'Enveloppe (Fader `ADSR`)** :
   $$Envelope_{active}(t) = Env_{AMP}(t) \cdot \left[ (1.0 - ADSR_{sens}) + ADSR_{sens} \cdot (v_{norm})^{1.5} \right]$$
   Cette courbe à exposant $1.5$ offre une réponse dynamique particulièrement naturelle au jeu au clavier.
