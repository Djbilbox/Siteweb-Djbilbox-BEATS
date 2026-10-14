# PRODUCT BRIEF — K2000 SYNTHESIZER

**Nom du produit** : K2000  
**Sous-titre / Marque** : By Humpire & DJBILBOXBEATS  
**Version** : 1.0.0  
**Date** : 2026-08-16  
**Type** : Synthétiseur Virtuel Polyphonique (VST3 / AU / Standalone + Web Audio)

---

## 1. Vision & Identité Produit

Le **K2000** est un synthétiseur virtuel hybride inspiré de l'âge d'or des années 80, de l'esthétique Synthwave, Darksynth et Retrowave, combinant la chaleur de la synthèse soustractive analogique vintage et la précision d'un rack futuriste cyberpunk.

### ADN Sonore
- **Basses puissantes et définies** : Basslines 80s percutantes, arpeggiated basses analogiques, subs riches et sans aliasing.
- **Pads cinématographiques & soyeux** : Nappes larges et enveloppantes propulsées par un double oscillateur riche et un chorus spatial type Dimension 80s.
- **Leads incisifs & solos expressifs** : Leads brillants avec portamento/glide fluide, pitch bend réactif et saturation analogique chaleureuse.
- **Plucks & Claviers rétro** : Sons courts et percutants inspirés des bandes originales cultes des années 80 (Miami Vice, Blade Runner, Knight Rider, Terminator).

---

## 2. Public Cible

- **Producteurs de musique électronique** : Synthwave, Retrowave, Outrun, Synthpop, Cyberpunk, Darksynth, Vaporwave.
- **Compositeurs de musique à l'image & jeux vidéo** : Musiques de films rétro-futuristes, ambiances 80s, bandes-son neon-arcade.
- **Sound designers & Claviéristes** : Recherche d'un instrument au look rackable vintage iconique, facile à jouer et hautement expressif.

---

## 3. Spécifications Fonctionnelles V1

### 3.1. Générateur Polyphonique (Oscillateurs)
- **Polyphonie** : Jusqu'à 16 voix en mode polyphonique, gestion de voix ultra-optimisée avec stealing intelligent.
- **Mode Voix** : Mono Retrigger, Mono Legato (avec Glide/Portamento continu), Poly 8-voix, Poly 16-voix.
- **Formes d'onde de base** :
  1. *Sawtooth (Dents de scie)* : Riche en harmoniques pour leads et pads.
  2. *Square / Pulse (Carrée / Impulsion)* : Idéal pour basses et sons creux 80s.
  3. *Triangle* : Douceur et rondeur pour flûtes et nappes subtiles.
  4. *Sine (Sinus)* : Sub-bass pure et fondamentale percutante.
  5. *SuperSaw (Detuned Multi-Saw)* : Épaisseur maximale pour leads puissants.
  6. *Retro Brass* : Forme d'onde cuivrée hybride classique.

### 3.2. Filtre Multimode Résonant
- **Types de filtres** :
  - `LowPass 24dB` : Émulation échelle à transistors (Moog/Oberheim style) 4 pôles avec résonance auto-oscillante contrôlée.
  - `LowPass 12dB` : Filtre 2 pôles plus doux (Oberheim SEM style).
  - `BandPass` : Isolation fréquentielle médium pour textures rétro et formants.
  - `HighPass` : Élimination des basses pour plucks cristallins et nappes aériennes.
  - `Acid LowPass` : Filtre non-linéaire avec distorsion de résonance accentuée.
- **Contrôles** : Knob `FILTER` (Cutoff de 20 Hz à 20 kHz), modulation assignable par LFO et Enveloppe.

### 3.3. Enveloppes & Dynamique
- **ADSR Principale + Velocity Bank** :
  - `ATTACK` (1 ms à 5.0 s)
  - `DECAY` (5 ms à 8.0 s)
  - `SUSTAIN` (0.0 à 1.0)
  - `BANK` (Étage de macro-dynamique / gain de banque)
  - `ADSR` (Sensibilité dynamique de la vélocité sur l'enveloppe)
- **Enveloppe LFO (LFO ADSR)** :
  - 4 faders dédiés (`ATTACK`, `DECAY`, `SUSTAIN`, `RELEASE`) permettant de modeler l'apparition temporelle de la modulation LFO (ex. vibrato progressif après attaque).

### 3.4. Module LFO & Matrice de Modulation
- **Knobs** : `RATE` (0.05 Hz à 25 Hz / ou tempo-sync), `AMOUNT` (profondeur globale 0 à 100%).
- **Switch `PITCH`** : Permet de router instantanément le LFO sur la hauteur globale des oscillateurs pour créer un vibrato vintage instantané.
- **Sélecteur `LFO SOURCE`** : Sine, Triangle, Square, Saw Up, Saw Down, Sample & Hold (Random).
- **Sélecteur `DESTINATION`** : Filter Cutoff, Master Pitch, Distortion Drive, Delay Time, Chorus Depth, Pan.

### 3.5. Chaîne d'Effets Intégrée (Studio Rack FX)
1. **Distortion / Drive** : Saturation analogique asymétrique chaleureuse avec modélisation d'écrêtage doux à lampes/diodes.
2. **Chorus 80s** : Double ligne de retard modulée en quadrature de phase pour un élargissement stéréo tridimensionnel sans perte de phase centrale.
3. **Delay Stéréo** : Knob `DELAY` (Mix Wet/Dry), `TIME` (10 ms à 1000 ms), `FEEDBACK` (0 à 95% avec saturation analogique pour éviter tout larsen destructif).
4. **Reverb Ambiante** : Modélisation à plaques et diffusion algorithmique 80s (Wet/Dry de 0 à 100%).
5. **Glide / Portamento** : Knob `GLIDE` (0 à 1000 ms) avec courbe logarithmique musicale.
6. **Volume Master** : Étage de sortie avec limiteur transparent à soft-knee.

### 3.6. Module Expansion (Cassette VOL.1) & Gestion de Presets
- Baie de cassette physique animée représentant la cartouche sonore `VOL.1`.
- Au clic : sélection rapide de packs d'extension ou de banques complètes.
- Sélecteur de patch supérieur avec boutons fléchés `<` et `>`, affichage numérique rétro, icône de navigation de dossiers et icône de sauvegarde de preset.

### 3.7. Contrôles de Performance
- **Molette Pitch Bend** : +/- 2 à +/- 12 demi-tons, avec ressort physique qui revient automatiquement à zéro au relâchement.
- **Molette Modulation** : 0 à 100%, restant en position (latching).
- **Clavier virtuel 25 touches (2 octaves)** : Rétro-éclairé par LED cyan sous chaque touche avec intensité proportionnelle à la vélocité.

---

## 4. Contraintes de Design & Ergonomie

1. **Fidélité absolue au modèle maître** : Respect strict du châssis en métal brossé anthracite, des poignées chromées, des vis Torx, du médaillon central cyborg chameau Humpire et des lueurs néon cyan `#00F0FF`.
2. **Animation complète des contrôles** : Tous les potentiomètres, faders, commutateurs, molettes et touches de clavier réagissent visuellement de manière fluide aux interactions utilisateur.
3. **Pas de clipping destructif** : Limiteur de sécurité transparent en sortie de bus audio.
4. **Consommation CPU maîtrisée** : Interpolation de table d'ondes anti-repliement PolyBLEP et calculs vectorisés pour garantir une latence quasi-nulle.
