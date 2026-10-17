# ROCKPUNK — Cahier des charges (Product Brief V1)

## 1. Positionnement

- **Nom** : ROCKPUNK
- **Marque** : HUMPIRE (gamme de synthés VST)
- **Promesse sonore** : synthé **virtual analog rock / punk** — leads saturés, riffs pêchus,
  power chords agressifs, basses « drive », grit analogique. Pensé pour le jeu en direct
  (live / enregistrement de riffs).
- **Public** : producteurs et guitaristes/bassistes qui veulent des sons de synthé rock punky
  sans réglages complexes ; utilisateurs FL Studio / DAW.
- **Position dans la gamme** : déclinaison « énergie brute » du langage visuel premium HUMPIRE.

## 2. Objectifs V1

1. Un son « plug & play » agressif : n'importe quel preset doit sonner rock dès le chargement.
2. Un signal path simple et lisible : OSC → MIXER → FILTRE → DRIVE → DELAY → OUTPUT.
3. 4 macro-contrôles principaux (DRIVE / CUT / DELAY / VOLUME) en tête d'interface.
4. Interface sombre, rugueuse, premium, cohérente avec le modèle image maître HUMPIRE,
   avec animation de chaque contrôle (rotation, états hover/press, LED).
5. Sauvegarde/restauration d'état, presets `.rpk`, automation de tous les paramètres.

## 3. Fonctionnalités retenues (V1)

| Bloc | Détail |
|---|---|
| Voix / polyphonie | 8 voix, modes Poly / Mono / Legato (retenu), glide 0–500 ms |
| Oscillateurs | OSC A + OSC B (saw, square, pulse PWM, triangle) — anti-aliasing PolyBLEP ; semi + detune cents + PW ; sub-osc (sine/square/tri, -24..0 semi) ; noise blanc |
| Mixer | Level OSC A / OSC B / SUB / NOISE + `MIX DRIVE` (saturation douce avant filtre) |
| Filtre | SVF résonant (LP/BP/HP 12 dB/oct), cutoff 20–20 kHz (log), résonance 0.5–10, env amount bi-polaire, keytrack, drive de filtre |
| Enveloppes | Amp ADSR + Filter ADSR (segments exponentiels) |
| LFO | LFO1 → filtre (5 formes), LFO2 → pitch (5 formes) ; sync tempo (16e..4/1) ou Hz |
| Matrice de mod | 2 slots (source → destination → amount bi-polaire) |
| Drive (post-filtre) | 4 types (soft, hard, asym, fuzz), amount 0–100 %, tone 100 Hz–10 kHz |
| Delay | On/off, tempo-sync ou ms, feedback, mix, LP tone, spread stéréo (ping-pong) |
| Sortie | Volume maître dB, stéréo width, limiter doux anti-clip |
| Presets | Banque `.rpk`, dossier `Presets/`, chargement depuis l'UI |
| Automation | Tous les paramètres automatisables (APVTS) |

## 4. Fonctionnalités différées (V2 — pas en V1)

Justification : ne pas surcharger la V1 (« V1 simple, stable, jouable »).

- Unison / detune multi-voix (remplacé par detune OSC A/B en V1)
- Second filtre / pente 24 dB/oct en série
- Effets supplémentaires (reverb, chorus, vocoder) → delay uniquement en V1
- Wavetable / échantillons (hors concept virtual analog)
- Macros librement assignables (la rangée DRIVE/CUT/DELAY/VOLUME joue ce rôle en V1)
- Banque de presets multi-fichiers / navigation clavier avancée

## 5. Contraintes de réalisation

- **Obligatoire** : `processBlock` sans allocation, sans lock, sans I/O ; `ScopedNoDenormals`.
- **Obligatoire** : tous les paramètres audibles lissés (`SmoothedValue`) → zéro zip.
- **Obligatoire** : coefficients dépendant du sample-rate recalculés dans `prepareToPlay`.
- **Anti-aliasing** : PolyBLEP sur saw/square/pulse ; saturation moderate → pas d'oversampling
  en V1 (documenté, coût CPU ~5–10 % visé).
- **Protection clipping** : limiter doux de sortie, gain staging interne (mixer ~-6 dB).
- **Licence** : aucun sample inclus, aucun asset tiers sans licence vérifiée.

## 6. Critères d'acceptation V1

1. Le plugin charge en DAW sans erreur et produit du son dès le preset par défaut.
2. Chaque contrôle de l'UI modifie réellement le paramètre annoncé (`PARAMETER_MAP.md`).
3. Aucun clic audible sur changement de paramètre.
4. Presets sauvegardés/rechargés identiques (ValueTree).
5. Animation de chaque contrôle cohérente avec le cahier d'animation (`UI_LAYOUT.md`)
   et vérifiable face au modèle maître.
6. CPU mesuré raisonnable (< ~15 % mono à 48 kHz sur machine de référence, à confirmer au test).