# SYNTH ARCHITECTURE — K2000

Ce document détaille l'architecture audio, le pipeline de traitement numérique du signal (DSP), la gestion des voix polyphoniques et la chaîne d'effets du synthétiseur **K2000**.

---

## 1. Schéma Global du Signal Audio (Signal Flow)

```
[ MIDI / Clavier ] ---> [ Voice Allocator & Glide ] ---> [ Polyphonic Voices (x16) ]
                                                                 |
                                       +-------------------------+-------------------------+
                                       |                                                   |
                            [ PolyBLEP Oscillators ]                              [ LFO & Envelopes ]
                           (Saw/Sq/Tri/Sin/SSaw/Brass)                           (LFO ADSR + Amp ADSR)
                                       |                                                   |
                                       v                                                   v
                            [ Filter Drive / Saturation ]                       [ Modulation Matrix ]
                                       |                                                   |
                                       v                                                   |
                            [ ZDF Multimode Filter ] <-------------------------------------+
                           (LP24 / LP12 / BP / HP / Acid)
                                       |
                                       v
                            [ Voice Amp & DCA (Amp ADSR) ]
                                       |
                   ====================+====================
                                       | (Voice Summing Bus)
                                       v
                           [ Master Distortion Stage ]
                                       |
                                       v
                           [ 80s Dimension Chorus ]
                                       |
                                       v
                           [ Tape / Analog Delay ]
                                       |
                                       v
                           [ Algorithmic Ambient Reverb ]
                                       |
                                       v
                           [ Master Volume & Soft Limiter ]
                                       |
                                       v
                           [ Stereo Audio Output (L/R) ]
```

---

## 2. Modules DSP Détaillés

### 2.1. Voice Allocator & Glide (Portamento)
- **Gestion des Voix** : Allocation dynamique jusqu'à 16 voix simultanées avec algorithme de « voice stealing » basé sur le niveau d'enveloppe le plus bas et la note la plus ancienne.
- **Modes de Voix** :
  - `Poly 8` / `Poly 16` : Polyphonie standard avec réassignation naturelle.
  - `Mono Retrigger` : Monophonique avec redéclenchement systématique des enveloppes d'amplitude et de LFO à chaque note.
  - `Mono Legato` : Monophonique sans redéclenchement de l'enveloppe si la note précédente est maintenue, avec transition continue de pitch.
- **Portamento (Glide)** : Interpolation logarithmique de fréquence :
  $$f(t) = f_{target} + (f_{start} - f_{target}) \cdot e^{-t / \tau}$$
  où $\tau$ est déterminé par le potentiomètre `GLIDE` (0 à 1000 ms).

### 2.2. Générateur d'Oscillateurs PolyBLEP (Anti-aliasing)
Pour éliminer tout repliement de spectre dans les aigus tout en conservant le mordant harmonique des synthétiseurs vintage des années 80, le moteur utilise la technique **PolyBLEP** (Polynomial Band-Limited Step) :
- Pour une discontinuité à la phase $t \in [0, 1)$ et un incrément de phase $dt$ :
  - Si $t < dt$ : $BLEP(t) = \frac{t}{dt} + \frac{t^2}{2 dt^2} - \frac{1}{2}$
  - Si $t > 1 - dt$ : $t' = t - 1$, $BLEP(t') = \frac{t'}{dt} - \frac{t'^2}{2 dt^2} + \frac{1}{2}$
- **Formes d'ondes supportées** :
  1. `Sawtooth` : Onde en dent de scie avec correction PolyBLEP aux sauts de phase.
  2. `Square / Pulse` : Onde carrée avec correction PolyBLEP sur les fronts montant et descendant.
  3. `Triangle` : Intégration de l'onde carrée anti-repliée pour un timbre chaleureux et pur.
  4. `Sine` : Onde sinusoïdale pure haute précision issue de tables trigonométriques interpolées.
  5. `SuperSaw` : Empilement de 7 oscillateurs désaccordés (spread stéréo et détuning exponentiel) pour le son signature lead/pad des années 80.
  6. `80s Brass` : Combinaison dynamique d'impulsion asymétrique et de dent de scie avec modulation d'harmoniques paires.

### 2.3. Filtre Multimode ZDF (Zero-Delay Feedback)
Le filtre K2000 est une implémentation numérique à topologie préservée (TPT / ZDF) émulant les filtres analogiques 4 pôles (24 dB/octave) et 2 pôles (12 dB/octave).
- **Structure Moog Ladder 4 pôles** :
  - 4 étages intégrateurs à rétroaction unitaire.
  - Saturation non-linéaire modélisée à chaque pôle par une fonction tangente hyperbolique douce $\tanh(x)$.
  - Auto-oscillation musicale sans instabilité numérique même aux fréquences proches de Nyquist.
- **Modes sélectionnables** :
  - `LP24` : Coupe-bas 24dB avec résonance grasse et compression naturelle en bas du spectre.
  - `LP12` : Coupe-bas 12dB plus aéré et brillant.
  - `BandPass` : Filtre passe-bande 12dB centré sur la fréquence de coupure.
  - `HighPass` : Filtre passe-haut 12dB pour nettoyer les graves.
  - `Acid LP` : Filtre agressif avec surmultiplication de la résonance et clipping asymétrique.

### 2.4. Double Générateur d'Enveloppes (ADSR)
- **Amp ADSR + Velocity Bank** :
  - `ATTACK` (1 ms à 5.0 s) : Profil exponentiel concave.
  - `DECAY` (5 ms à 8.0 s) : Profil exponentiel convexe naturel.
  - `SUSTAIN` (0.0 à 1.0) : Niveau de maintien stabilisé.
  - `BANK` : Facteur d'amplification / dynamique globale de banque.
  - `ADSR (Velocity Sens)` : Modifie la profondeur de l'enveloppe selon la force d'impact sur la touche.
- **LFO Envelope (LFO ADSR)** :
  - Enveloppe 4 étages (`ATTACK`, `DECAY`, `SUSTAIN`, `RELEASE`) multipliée directement au signal du LFO avant son injection dans la matrice de modulation.

### 2.5. LFO Multi-Formes d'Onde & Routage Rapide
- **Générateur LFO** : Formes d'ondes Sine, Triangle, Square, Saw Up, Saw Down, Random Sample & Hold.
- **Fréquence** : 0.05 Hz à 25.0 Hz.
- **Switch `PITCH`** : Aiguille immédiatement le LFO vers le pitch des oscillateurs (vibrato 80s).
- **Destination Assignable** : Filtre Cutoff, Pitch, Pulse Width, Volume, Chorus Depth, Pan.

---

## 3. Chaîne d'Effets Studio (Master FX Rack)

### 3.1. Distortion / Overdrive
- Modélisation de pré-ampli à lampes et diodes avec clipper asymétrique :
  $$y(x) = \frac{x + 0.15 x^2}{1 + |x| + 0.15 x^2}$$
- Ajoute de la présence, du mordant et de la chaleur vintage sans agressivité numérique.

### 3.2. Chorus Stéréo 80s (Dimension Style)
- Deux lignes de retard couplées avec modulation en quadrature de phase (LFO déphasé de 90° entre canal gauche et canal droit).
- Largeur stéréo panoramique immersive caractéristique des Roland Juno et Dimension D.

### 3.3. Tape / Analog Delay
- Ligne de retard stéréo avec interpolation cubique (Hermite).
- Temps réglable de 10 ms à 1000 ms.
- Filtrage passe-bas à 3.5 kHz dans la boucle de feedback pour émuler l'atténuation progressive des hautes fréquences sur bande magnétique.
- Saturation douce dans la boucle pour éviter tout emballement du feedback.

### 3.4. Reverb Algorithmique Spatiale
- Réseau de 4 filtres passe-tout (Allpass) et 4 lignes de retard à peigne (Comb filters) en parallèle avec amortissement des aigus (Damping).
- Produit des queues de réverbération amples, diffuses et denses rappelant les processeurs Lexicon et EMT des années 80.

### 3.5. Limiteur de Sortie & Protection Audio
- Étage de soft-clipping transparent en sortie master ($tanh$ soft-knee).
- Élimine tout risque de saturation numérique à 0 dBFS.
- Filtre passe-haut DC Blocker à 10 Hz pour supprimer toute composante continue.
