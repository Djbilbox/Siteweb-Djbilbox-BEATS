# ROCKPUNK GUITAR — Guide d'utilisation

**DJBILBOX BEATS — version 1.1.0**

Guitare électrique rock'n'roll : deux cordes entièrement modélisées, l'ampli
« Urban Slayer », un LFO, un filtre resonant, un drive a 4 caracteres, un delay
synchronisable, une reverb et un chorus. **76 presets d'usine.**

ROCKPUNK **ne contient aucun enregistrement** : chaque note est calculée. Rien à
télécharger en plus, rien qui pèse, et une couche supplémentaire qui joue **vos
propres fichiers** si vous en déposez.

---

## 1. Installation

### Windows

1. Lancez `ROCKPUNK_1.1.0_Installer.exe`.
2. Acceptez l'élévation Windows : le dossier VST3 est sous *Program Files*, et
   sans les droits administrateur l'installeur ne peut pas y écrire.
3. Le plugin s'installe dans `C:\Program Files\Common Files\VST3\ROCKPUNK.vst3`.

### macOS

1. Ouvrez `ROCKPUNK_1.1.0_macOS.dmg`.
2. Glissez `ROCKPUNK.vst3` dans le raccourci **Installer ici (VST3)**, et
   `ROCKPUNK.component` dans **Installer ici (AU)** si vous utilisez Logic.
3. Au premier lancement, macOS demande confirmation pour un plugin non signé :
   *Réglages système → Confidentialité et sécurité → Ouvrir quand même*.

### Après l'installation

Relancez votre DAW et forcez un nouveau scan de plugins :

- **FL Studio** : *Options → Manage plugins → Find more plugins*
- **Ableton Live** : *Préférences → Plug-Ins → Rescan*
- **Reaper** : *Preferences → VST → Re-scan*

---

## 2. Où sont les fichiers

| Quoi | Où |
|---|---|
| Plugin VST3 (Windows) | `C:\Program Files\Common Files\VST3\ROCKPUNK.vst3` |
| Plugin VST3 (macOS) | `/Library/Audio/Plug-Ins/VST3/ROCKPUNK.vst3` |
| Presets d'usine et vôtres | `Documents\DJBILBOX\ROCKPUNK\Presets` |
| Vos échantillons | `Documents\DJBILBOX\ROCKPUNK\Samples` |

Les presets voyagent **dans** le plugin et sont recopiés dans vos Documents au
premier lancement : rien à installer à côté.

---

## 3. L'interface

### La cassette (en haut à gauche)

Le gestionnaire de presets, présenté comme une cassette.

- **INIT** — remet tous les réglages à zéro
- **LOAD / SAVE** — ouvre ou enregistre une cartouche `.rpk`
- **`<` `>`** — fait défiler les 76 presets d'usine

### SAMPLE (en haut à droite) — vos propres guitares

Cette couche joue **vos** fichiers. Déposez des `.wav`, `.aif` ou `.flac` dans

```
Documents\DJBILBOX\ROCKPUNK\Samples
```

et ils apparaissent dans la liste, jouables sur tout le clavier. Un clic sur la
ligne d'état ouvre directement le dossier ; la liste se relit à chaque ouverture
de la fenêtre du plugin.

ROCKPUNK **mesure tout seul la note** de chaque fichier pour accorder le
clavier, et l'affiche sous la liste. Le réglage ROOT la corrige si la mesure
tombe à côté — accords et riffs sont ambigus par nature.

Un sous-dossier devient une catégorie dans la liste :

```
Samples/Rythmiques/palm-mute-E.wav
Samples/Solos/bend-A.wav
```

Le dossier est vide à l'installation, et c'est normal : les 76 presets d'usine
n'en ont pas besoin, ils ne reposent que sur les cordes modélisées.

### Les potards

| Potard | Ce qu'il règle |
|---|---|
| **RATE** | vitesse du LFO (0,05 à 20 Hz) |
| **AMOUNT** | profondeur du LFO sur la destination choisie |
| **PITCH** | profondeur du vibrato, quelle que soit la destination |
| **GLIDE** | temps de glissando entre deux notes |
| **FILTER** | fréquence de coupure du filtre |
| **VOLUME** | volume général |
| **REVERB** | dosage de la réverbération |
| **CHORUS** | dosage du chorus |
| **DISTORTION** | quantité de drive |
| **TIME** | temps du delay |
| **FEEDBACK** | réinjection du delay |

### Les curseurs

Les quatre de gauche (**ATTACK DECAY SUSTAIN RELEASE**) dessinent l'enveloppe de
volume : de la rythmique sèche à la nappe qui monte.

Les quatre de droite (**A S D R**) dessinent l'enveloppe du **filtre** : c'est
elle qui produit les balayages. Le cinquième, **VELOCITY**, dose la nuance du
jeu — à zéro toutes les notes attaquent pareil.

### Les molettes

**PITCH BEND** transpose de deux demi-tons vers le haut ou vers le bas.
**MODULATION** ouvre le vibrato, en plus du potard PITCH.

### Les grilles de haut-parleur

Elles servent d'interrupteurs : cliquez pour couper ou rallumer chaque corde.

### Les sélecteurs du bas

| Sélecteur | États |
|---|---|
| **WAVEFORM** | dureté du médiator : DOUX / ROND / CLAIR / TRANCHANT |
| **LFO SOURCE** | forme du LFO : SINUS / TRI / CARRÉ / S&H |
| **DESTINATION** | ce que le LFO module : PITCH / FILTRE / VOLUME / DRIVE |
| **FILTER TYPE** | PASSE-BAS / BANDE / PASSE-HAUT / HORS SERVICE |
| **MONO/POLY** | POLY, ou MONO pour lier les notes au glissando |

### Le clavier

Cliquable à la souris, et il s'allume avec les notes MIDI reçues.

---

## 4. Les 76 presets d'usine

Ils sont numerotes et regroupes par famille. Les fleches `<` `>` du bandeau du
haut les font defiler ; un clic sur le nom ouvre la liste complete.

| Famille | Nombre | Caractere |
|---|---|---|
| Rythmiques crunch | 14 | attaque seche, note courte, le coeur du produit |
| Solos | 12 | ca tient, ca chante, delay et vibrato |
| Gros gain | 10 | fuzz et saturation asymetrique |
| Clairs et mi-clairs | 12 | l'ampli en gain propre, chorus et delay |
| Cordes nues | 8 | aucune distorsion, la caisse parle |
| Nocturnes | 8 | sombre, filtre ferme, espace |
| Textures | 8 | le LFO et le filtre sont le sujet |
| Graves | 4 | power chords, corps profond |

Tous sont cales au meme niveau : passer de l'un a l'autre ne fait pas de saut
de volume. Aucun n'a besoin du dossier `Samples`.

## 5. Questions fréquentes

**Le plugin n'apparaît pas dans mon DAW.**
Vérifiez que le scan pointe bien vers `C:\Program Files\Common Files\VST3` (sur
ce poste, `%LOCALAPPDATA%\Programs\Common\VST3` est déclaré comme dossier VST2 :
un VST3 posé là reste invisible). Puis relancez un scan complet.

**Je n'entends rien.**
Vérifiez qu'au moins une des deux grilles de haut-parleur est allumée : ce sont
les interrupteurs des deux cordes, et si les deux sont coupées il ne reste rien
à sortir.

**La liste SAMPLE est vide.**
C'est l'état normal d'une installation neuve : ROCKPUNK ne livre aucun
enregistrement. Déposez vos fichiers dans `Documents\DJBILBOX\ROCKPUNK\Samples`
(cliquez sur la ligne sous la liste pour ouvrir le dossier), puis rouvrez la
fenêtre du plugin.

**Comment retrouver mes presets ?**
Ils sont dans `Documents\DJBILBOX\ROCKPUNK\Presets`. Sauvegardez ce dossier :
la désinstallation ne le touche pas.

**Puis-je vendre la musique faite avec ROCKPUNK ?**
Oui, sans redevance ni mention. Voir `LICENSE.txt`.

---

## 6. Support

**djbilboxbeats.com** — support@djbilboxbeats.com

© 2026 DJBILBOX BEATS
