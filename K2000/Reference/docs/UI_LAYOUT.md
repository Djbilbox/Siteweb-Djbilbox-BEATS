# UI LAYOUT & CONTROL ANIMATION SPECIFICATION — K2000

Ce document définit l'organisation spatiale, la hiérarchie visuelle et le **cahier d'animation interactif obligatoire** pour chaque élément de l'interface du synthétiseur **K2000**, en conformité stricte avec le modèle image maître (`Synthesizer_plugin_user_interfac…_2K_202608121850.jpeg`).

---

## 1. Structure Globale & Grille de Disposition

L'interface se structure autour d'un rack en métal brossé anthracite au ratio 4:3 (résolution de référence : 1200 x 900 px, fluide et redimensionnable).

```
+---------------------------------------------------------------------------------------+
| [Vis Torx]                                                                [Vis Torx]  |
| [Poignée]   [CASSETTE]        [ <  K2000 - Init Patch  > ]        [DJBILBOXBEATS]     |
| [Chrome]    [VOL. 1  ]        ============================        [  BADGE CB   ]     |
|             [EXPANSION]                                           [ADSR+VELOCITY]     |
|                             [Grille]  (MASCOTTE)  [Grille]        [A D S B A]         |
|             [-- LFO --]               | K2000  |                  [| | | | |]         |
|             (RATE)    |A D S R|       By Humpire                  [| | | | |] [Poignée]
|             [PITCH]   || | | ||                                   (VOL)(REV)(CHO)(DIS)|
|             (AMOUNT)  || | | ||       (GLIDE)   (FILTER)          (DIS)(DLY)(TIM)(FB) |
|                                                                                       |
|             [WAVEFORM ▼] [LFO SOURCE ▼] [DESTINATION ▼] [FILTER TYPE ▼] [MONO/POLY ▼] |
|                                                                                       |
|             [PITCH] [MOD]   [ C C# D D# E F F# G G# A A# B ... C ] (25 Touches Clavier)|
| [Vis Torx]                                                                [Vis Torx]  |
+---------------------------------------------------------------------------------------+
```

---

## 2. Spécification Détaillée des Modules

### 2.1. Châssis & Éléments de Structure
- **Boîtier** : Métal brossé anthracite (finition brossage horizontal, micro-rayures réalistes).
- **Poignées de rack** : 2 poignées verticales chromées robustes sur les flancs gauche et droit.
- **Visserie** : 4 vis Torx/Hexagonales chromées encastrées dans les 4 coins.
- **Gouttières lumineuses** : Liserés d'accentuation néon cyan (`#00F0FF`) entourant chaque module actif.

### 2.2. Bandeau Supérieur (Header & Patch Manager)
- **Sélecteur de Patch Central** :
  - Cadre biseauté à rétro-éclairage néon cyan.
  - Bouton Icône Dossier (Browse Preset) à gauche.
  - Bouton Flèche Gauche `<` (Patch précédent).
  - Afficheur numérique central LCD rétro-éclairé affichant le nom du patch (ex. `K2000 - Init Patch`).
  - Bouton Flèche Droite `>` (Patch suivant).
  - Bouton Icône Disquette 3.5" (Save / Export Preset) à droite.
- **Branding Supérieur Droit** :
  - Typographie futuriste argentée `DJBILBOXBEATS`.
  - Badge Cyber Emblème carré `CB` gravé dans le métal avec rétro-éclairage cyan.

### 2.3. Module EXPANSION (Baie de Cassette Rétro)
- Emplacement : En haut à gauche.
- Visuel : Trappe de lecteur de cassette mécanique vintage avec charnière chromée à droite.
- Cassette audio `VOL.1` insérée avec bobines visibles, étiquette beige rétro et logo sérigraphié `A | VOL.1`.
- Intitulé inférieur : Sérigraphie métallique `EXPANSION`.

### 2.4. Médaillon Mascotte & Grilles Acoustiques
- Emplacement : Centre supérieur.
- Médaillon central circulaire chromé abritant la mascotte Humpire : un Chameau Cyborg stylisé avec visière cybernétique cyan lumineuse et armure d'épaules high-tech.
- Flanqué de part et d'autre par 2 grilles d'aération/haut-parleur verticales perforées et rétro-éclairées de cyan.
- Titre sous le médaillon : Logo digital néon `K2000` en police 7 segments stylisée cyan vif, avec sous-titre `By Humpire` en blanc argenté.

### 2.5. Section LFO (Modulation & Enveloppe de LFO)
- Emplacement : Zone moyenne gauche.
- Titre : Sérigraphie `LFO`.
- Sous-module rotatif (à gauche) :
  - Knob `RATE` (vitesse du LFO).
  - Switch basculeur `PITCH` (assignation rapide du LFO au pitch).
  - Knob `AMOUNT` (profondeur du LFO).
- Sous-module faders d'enveloppe LFO (à droite) :
  - 4 faders verticaux : `ATTACK`, `DECAY`, `SUSTAIN`, `RELEASE`.
  - Permettent de profiler la montée et l'évolution temporelle de la modulation LFO.

### 2.6. Section ADSR + VELOCITY BANK
- Emplacement : Zone moyenne droite supérieure.
- Titre : Sérigraphie `ADSR+VELOCITY BANK`.
- 5 faders verticaux :
  - `ATTACK` : Temps d'attaque de l'amplitude générale.
  - `DECAY` : Temps de chute vers le sustain.
  - `SUSTAIN` : Niveau de maintien tant que la note est tenue.
  - `BANK` : Macro-dynamique / banque de gain et courbe de réponse.
  - `ADSR` : Profondeur d'action de la vélocité sur l'enveloppe d'amplitude.

### 2.7. Rangée de Potentiomètres Supérieure Droite (FX & Volume)
- 4 potentiomètres rotatifs chromés :
  - `VOLUME` : Volume de sortie général (0% à 100%).
  - `REVERB` : Dosage de réverbération spatiale 80s (0% à 100%).
  - `CHORUS` : Élargissement stéréo et chorus Dimension (0% à 100%).
  - `DISTORTION` : Saturation asymétrique analogique (0% à 100%).

### 2.8. Rangée de Potentiomètres Médiane (Filtre, Glide & Delay)
- 6 potentiomètres rotatifs chromés :
  - `GLIDE` : Temps de portamento entre les notes (0 à 1000 ms).
  - `FILTER` : Fréquence de coupure du filtre résonant (Cutoff 20 Hz - 20 kHz).
  - `DISTORTION` : Drive / pré-saturation d'étage de filtrage.
  - `DELAY` : Mixage du délai stéréo (Dry/Wet 0% à 100%).
  - `TIME` : Temps de retard de l'écho (10 ms à 1000 ms).
  - `FEEDBACK` : Taux de réinjection du délai (0% à 95%).

### 2.9. Barre de Sélecteurs Cyber Dropdowns
- 5 menus déroulants futuristes bordés de cyan néon :
  1. `WAVEFORM ▼` : Saw, Square, Triangle, Sine, SuperSaw, 80s Brass.
  2. `LFO SOURCE ▼` : Sine, Triangle, Square, Saw Up, Saw Down, Random S&H.
  3. `DESTINATION ▼` : Filter Cutoff, Pitch, Pulse Width, Volume, Chorus Depth, Pan.
  4. `FILTER TYPE ▼` : LowPass 24dB, LowPass 12dB, BandPass, HighPass, Acid LowPass.
  5. `MONO/POLY ▼` : Poly 8-Voice, Poly 16-Voice, Mono Retrig, Mono Legato.

### 2.10. Molettes de Performance (Pitch Bend & Modulation)
- Emplacement : Baie inférieure gauche biseautée avec rétro-éclairage cyan.
- **Molette PITCH BEND** : Cylindre rainuré avec index lumineux cyan central.
- **Molette MODULATION** : Cylindre rainuré avec index lumineux cyan central.

### 2.11. Clavier Virtuel Rétro-Éclairé
- Emplacement : Tout le bas de l'interface (de la fin des molettes jusqu'au bord droit).
- 25 touches (2 octaves complètes, C2 à C4).
- Rétro-éclairage néon cyan sous chaque touche pressée.

---

## 3. Cahier des Animations Interactives des Contrôles

Chaque contrôle dispose d'une réponse visuelle temps-réel exacte :

### 3.1. Potentiomètres Rotatifs (Knobs)
- **Plage de rotation** : -135° (valeur min) à +135° (valeur max), soit un arc total de 270°.
- **Animation de rotation** : L'index/encoche blanche et le corps brossé du potentiomètre tournent avec un lissage parfait lors du glissement vertical de la souris (drag haut = augmentation, drag bas = diminution) ou de l'utilisation de la molette de la souris.
- **Feedback lumineux (Cyan Glow)** : Un arc néon cyan entourant l'embase du potentiomètre s'allume et progresse proportionnellement à la valeur actuelle (arc vide à 0%, arc complet à 100%).
- **Survol (Hover)** : Éclairage subtil du liseré cyan (+20% de luminosité) et curseur `ns-resize`.
- **Double-clic** : Réinitialisation instantanée à la valeur par défaut du paramètre.

### 3.2. Faders Verticaux
- **Course verticale** : Déplacement fluide du capuchon métallique chromé de bas (0%) en haut (100%).
- **Piste à LED pointillée** : La réglette verticale comporte des graduations pointillés cyan néon qui s'illuminent sous le passage du curseur pour indiquer le niveau.
- **Capuchon lumineux** : La ligne horizontale centrale du capuchon s'illumine en cyan vif lors de l'interaction.
- **Interaction** : Clic direct sur la piste pour positionner le fader ou glisser-déplacer continu.

### 3.3. Commutateur à Bascule (Switch PITCH)
- **États** : Binaire ON / OFF (Haut / Bas).
- **Animation** : Basculement visuel mécanique 3D du levier métallique vers le haut (actif) ou vers le bas (inactif).
- **Feedback LED** : La pastille lumineuse au-dessus du switch s'allume en cyan éclatant lorsque le routage vers le pitch est activé.

### 3.4. Molettes PITCH BEND & MODULATION
- **Pitch Bend** :
  - Déplacement angulaire vers le haut (+100%) ou vers le bas (-100%).
  - **Animation Spring-Back** : Dès le relâchement du bouton de la souris, la molette retourne instantanément à sa position centrale (0) avec un amortissement élastique ultra-réaliste.
- **Modulation** :
  - Déplacement linéaire continu de 0% (bas) à 100% (haut).
  - **Animation Latching** : La molette reste exactement à la position où l'utilisateur la relâche.
- **Luminosité dynamique** : L'arête cyan de la molette augmente d'intensité avec le déplacement.

### 3.5. Sélecteurs Déroulants (Dropdowns)
- **État repos** : Boîte biseautée sombre avec texte cyan et flèche `▼` cyan.
- **Clic d'ouverture** : Déploiement fluide d'un menu rétro-futuriste sombre avec bordure cyan néon.
- **Survol des options** : Fond bleu néon semi-transparent avec texte blanc éclatant.
- **Sélection** : Micro-impulsion lumineuse cyan sur le texte sélectionné, mise à jour immédiate du DSP, et fermeture automatique.

### 3.6. Module EXPANSION (Cassette VOL.1)
- **Survol** : Léger soulèvement mécanique et augmentation du halo cyan du boîtier.
- **Clic** : Déclenche une micro-animation d'éjection/insertion de la cartouche cassette et ouvre le gestionnaire de banques de sons / extensions.

### 3.7. Sélecteur de Patch (Preset Header)
- **Flèches `<` et `>`** : Micro-flash cyan à l'activation et passage immédiat au preset adjacent.
- **Bouton Dossier (Browse)** : Ouvre la liste hiérarchisée des presets de la banque courante.
- **Bouton Disquette (Save)** : Ouvre la modale de sauvegarde personnalisée du patch courant (exportation JSON).

### 3.8. Clavier Virtuel 25 Touches
- **Enfoncement physique** : Déplacement vertical de 3 px de la touche blanche ou noire lors du clic souris, de la frappe clavier AZERTY/QWERTY ou de la réception d'un événement MIDI `NoteOn`.
- **Halo lumineux** : Projection d'une nappe de lumière néon cyan sous la touche jouée, dont l'intensité est rigoureusement indexée sur la vélocité de la note (40% de glow pour vélocité faible, 100% pour vélocité maximale).
- **Relâchement** : Extinction progressive douce du halo (fondu de 60 ms) lors de l'événement `NoteOff`.

### 3.9. Mascotte Chameau Cyborg (Ambiance)
- La visière cyan du chameau cyborg émet une lueur néon continue avec une subtile respiration lumineuse pulsée synchronisée sur le LFO ou l'enveloppe générale du synthé.
