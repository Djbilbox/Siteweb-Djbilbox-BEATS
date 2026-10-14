# VISUAL VARIATION & DESIGN SYSTEM — K2000

Ce document formalise la charte graphique, les matériaux, les couleurs et les composants visuels du synthétiseur **K2000**, assurant une fidélité absolue au modèle image maître (`Synthesizer_plugin_user_interfac…_2K_202608121850.jpeg`).

---

## 1. Palette Chromatique & Tokens de Couleur

```
+-------------------+-------------------+-------------------+-------------------+
|  ANTHRACITE BASE  |   BRUSHED CHROME  |     NEON CYAN     |   NEON CYAN GLOW  |
|      #16191D      |      #C5D1DC      |      #00F0FF      | rgba(0,240,255,.4)|
+-------------------+-------------------+-------------------+-------------------+
|    DEEP SHADOW    |  CHASSIS ACCENT   |   SILVER TYPO     |  CASSETTE BEIGE   |
|      #0A0C0E      |      #262B32      |      #E2EBF2      |      #D8CDBC      |
+-------------------+-------------------+-------------------+-------------------+
```

### Valeurs Exactes des Couleurs
- `--k2000-bg-chassis`: `#16191D` (Métal sombre brossé avec texture anisotrope)
- `--k2000-bg-recessed`: `#0E1013` (Cavités et baies en retrait)
- `--k2000-bezel-highlight`: `#3E4752` (Arêtes supérieures éclairées)
- `--k2000-bezel-shadow`: `#070809` (Ombres portées des biseaux)
- `--k2000-chrome-light`: `#EAF0F6` (Éclat spéculaire chrome)
- `--k2000-chrome-mid`: `#98A6B5` (Corps métallique tourné des potentiomètres)
- `--k2000-chrome-dark`: `#485461` (Ombre métallique usinée)
- `--k2000-neon-cyan`: `#00F0FF` (Accent principal électrique 80s)
- `--k2000-neon-cyan-dim`: `#0090A8` (LED au repos / graduation inactive)
- `--k2000-neon-glow`: `0 0 12px rgba(0, 240, 255, 0.65), 0 0 24px rgba(0, 240, 255, 0.35)`
- `--k2000-text-primary`: `#E8EFF5` (Sérigraphie blanche chromée)
- `--k2000-text-secondary`: `#7A8999` (Labels secondaires)
- `--k2000-cassette-cream`: `#D8CDBC` (Étiquette papier vintage)
- `--k2000-key-white`: `#EFECE6` (Touches de piano blanches ivoire avec reflet néon)
- `--k2000-key-black`: `#17191C` (Touches de piano noires polies)

---

## 2. Matériaux & Textures

1. **Châssis Métallique Usiné** :
   - Texture de brossage linéaire horizontal à grain fin.
   - Micro-rayures et patine subtile sur les rebords externes et les poignées.
   - Reflets métalliques anisotropes générés par des dégradés coniques et linéaires précis.

2. **Poignées et Visserie Industrielle** :
   - Poignées tubulaires verticales en chrome poli avec brossage longitudinal et ombres portées douces sur le boîtier.
   - Vis Torx à empreinte 6 pans creux en acier trempé brossé avec reflets radiaux.

3. **Optiques Néon Cyan** :
   - Gouttières fraisées diffusant une lumière cyan continue.
   - Liserés et voyants LED avec double diffusion gaussienne (cœur blanc/cyan intense + halo externe bleu électrique).

4. **Baie de Cassette Rétro EXPANSION** :
   - Plastique polycarbonate transparent fumé sur la fenêtre des bobines.
   - Texture papier couché légèrement vieilli pour l'étiquette sérigraphiée « VOL.1 ».
   - Tête de lecture et galet presseur en métal foncé visibles en transparence.

---

## 3. Typographie

- **Logo Principal « K2000 »** : Police digitale futuriste rétro néon à segments stylisés, trait lumineux cyan avec halo haute intensité (`font-family: 'Orbitron', 'Chakra Petch', sans-serif; font-weight: 900; letter-spacing: 2px`).
- **Sous-titre « By Humpire »** : Sans-serif moderne épurée en blanc argenté (`font-family: 'Montserrat', sans-serif; font-weight: 500; font-size: 11px`).
- **Afficheur de Patch (Header)** : Afficheur numérique rétro à matrice LCD cyan (`font-family: 'Share Tech Mono', monospace; font-size: 13px`).
- **Labels des Contrôles (Knobs, Faders, Dropdowns)** : Sérigraphie industrielle gravée en majuscules (`font-family: 'Inter', sans-serif; font-weight: 700; font-size: 9.5px; letter-spacing: 0.8px`).
- **Logo Mascotte & Écussons** : `DJBILBOXBEATS` avec badge cybernétique `CB` aux arêtes cyan lumineuses.

---

## 4. Composants Graphiques Clés

### 4.1. Potentiomètres Rotatifs (K2000 Knobs)
- Diamètre : 44 px à 48 px.
- Face avant : Cercle de chrome usiné concentrique avec encoche gravée blanche.
- Embase : Gorge circulaire avec rétro-éclairage néon cyan dynamique traçant l'arc de valeur.
- Ombre : Ombre portée portée à 120° pour un ancrage tridimensionnel parfait dans la façade.

### 4.2. Réglettes et Faders Métalliques
- Hauteur de piste : 60 px à 72 px.
- Piste centrale : Double rail sombre avec tirets gradués cyan néon des deux côtés.
- Curseur (Capuchon) : Rectangle biseauté métallique chromé (largeur 24 px, hauteur 8 px) avec une ligne horizontale néon cyan au centre.

### 4.3. Molettes de Pitch et Modulation (Wheels)
- Texture : Cylindres en gomme sombre striée de rainures horizontales profondes.
- Éclairage : Deux gorges néon cyan latérales et index néon cyan central suivant la rotation angulaire.

### 4.4. Clavier de Synthétiseur 25 Touches
- 15 touches blanches de 18 px de largeur, avec chanfreins réalistes et légère lueur cyan sous le bandeau supérieur.
- 10 touches noires biseautées de 12 px de largeur avec arêtes adoucies et reflets spéculaires.
- Éclairage dynamique : Dès qu'une note est jouée, un halo néon cyan se diffuse sous la touche correspondante.
