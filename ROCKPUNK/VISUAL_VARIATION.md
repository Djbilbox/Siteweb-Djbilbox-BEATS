# ROCKPUNK — VISUAL_VARIATION.md (variation visuelle rock/punk)

> Déclinaison **ROCKPUNK** du langage visuel maître HUMPIRE. Le modèle image maître reste la
> référence de finition, de grammaire et de marque : on n'adapte que **couleur, ambiance, noms et
> modules** propres à ROCKPUNK (règle du cahier des charges). ⚠ À VÉRIFIER face à l'image maître
> et à l'image ROCKPUNK (non lisibles par ce modèle d'IA).

## 1. Concept visuel

Un **ampli rock détruit mais haut de gamme** : tolex texturé, chrome usé, LED incandescentes,
accents sang et feu. « Premium brut » — pas de cartoon, pas de gore : de la matière agressive.

## 2. Palette (≤ 5 couleurs maîtres)

| Rôle | Couleur | Usage |
|---|---|---|
| Base / fond | `#0B0A0A` (noir charbon) | panneaux, fond de l'UI |
| Panneau | `#161312` (charcoal warm) | sections « rack » |
| Accent principal | `#E8302A` (rouge sang) | LED, pointeurs de knob, titres actifs |
| Accent secondaire | `#FF7A1A` (orange feu) | halos, valeurs, DELAY/DRIVE actifs |
| Highlight | `#FFF2D0` (blanc chaud) | textes, valeurs, pointeurs brillants |
| Acier | `#3A3A38` + dégradés chrome | corps des knobs / faders |

### Bonnes pratiques respectées (Skill vst-design-3d-ui)
- Dark mode par défaut (OLED-friendly).
- Contraste texte ≥ 4.5:1.
- Knob ≥ 30 px.
- Palette limitée à 5 rôles → cohérence de marque.

## 3. Matériaux & finition (alignés sur le modèle maître)

- **Knobs** : chrome brossé foncé, pointeur LED rouge sang, ring lumineux au drag.
- **Faders** : rail sombre + capuchon métal + LED de tête.
- **Switchs** : capuchons « amp » avec LED rouge sur position active.
- **Panneaux** : fond tolex foncé (bruit subtil), liserés 1 px acier.
- **Typo** : sans-serif capitales pour titres de module ; monospace pour les valeurs
  (compatible `vst-design-3d-ui`).
- **Logo** : emblème HUMPIRE en bandeau haut (asset propriétaire référencé, non recréé).

## 4. Ambiance rock/punk

- Slogan subtil en sous-bandeau : **« ROCKPUNK — BRUTE ANALOG »**.
- Liseré « déchiré » : une ligne brisée en bas de l'UI (élément décoratif statique, sans gêner la lecture).
- LED d'état DELAY et LIMITER : orange feu / blanc chaud selon l'état.
- Aucune texture photographique externe non licenciée : tout est procédural (dégradés, bruit
  simple, formes JUCE) — zéro asset tiers à risque (règle licence).

## 5. Cohérence de marque (non négociable)

- Ne pas créer d'interface incompatible avec le modèle maître : si une contradiction apparaît
  (ex. le modèle montre des knobs bleus), c'est le **modèle maître qui prime** (règle de priorité),
  et cette variation est ajustée — à faire à l'étape de vérification visuelle humaine.