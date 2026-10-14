---
name: viral-video
description: Génère des vidéos virales 9:16 (TikTok/Reels/Shorts) pour HUMPIRE by DJBILBOX BEATS via Higgsfield. Utiliser pour toute demande de vidéo promo, teaser plugin, sale, clip viral.
---

# Viral Video — HUMPIRE / DJBILBOX BEATS

## Contexte marque
- HUMPIRE = plugins VST3/AU, drum kits, sample packs, beats (G-Funk, West Coast, Trap, Afrotrap, Oriental).
- Liens : djbilboxbeats.gumroad.com · humpireplugins.netlify.app · YouTube @Djbilboxbeats · IG humpireplugins
- Promo en cours : Halloween -30% code `HUMPIRE30` jusqu'au 31/10 (mettre à jour selon la date).

## Workflow (toujours dans cet ordre)
1. Choisir 1 template ci-dessous (ou preset viral : `get_presets source=viral`).
2. `generate_video` avec `get_cost:true` → annoncer le coût, attendre le GO si > 40 crédits.
3. Modèle par défaut : `seedance_2_5`, 9:16, 720p, 6-8 s, `generate_audio:true`. Multi-plans/sync son : `kling3_0`. Look ciné : `cinematic_studio_video_4_0`.
4. Test pas cher d'abord : `draft:true` (480p), finaliser avec `draft_job_id`.
5. `jobs_wait` (≤15 s) puis afficher le résultat. Ne jamais relancer sans accord (crédits).

## Règles virales
- Hook visuel dans les 1,5 premières secondes (impact, glitch, transformation).
- Un seul sujet, un seul gag par vidéo. 6-8 s, boucle parfaite si possible.
- Drop du beat calé sur le moment fort (~1,5 s et ~5 s).
- Texte/logo/code promo : JAMAIS généré par l'IA (illisible) → ajouter dans CapCut. Audio : extrait du vrai beat DJBILBOX par-dessus.
- Fournir une capture réelle de l'UI du plugin comme `start_image`/référence quand dispo.

## Templates de prompts (remplacer [..])

### 1. Crypt Studio (Halloween)
Vertical 9:16 cinematic horror-comedy. Close-up of a hand slamming a glowing orange drum pad in a candlelit crypt studio, fog on the floor. BOOM: ground cracks, coffin lid blasts open. Skeletons in oversized headphones rise and headbang in sync to a heavy G-Funk trap beat, bats burst out of a giant analog synth. Push-in to a jack-o'-lantern skull with gold chains and headphones who turns to camera and smirks. Orange/purple lighting, volumetric fog, snappy camera.

### 2. Plugin qui prend vie
9:16. Dark studio, a laptop DAW screen shows [PLUGIN] interface. A knob turns by itself, red glitch flash, the whole room pulses to the kick, speakers shake, dust and sparks fly. Final shot: producer silhouette lifts head, eyes glow [COULEUR]. Cinematic, shallow depth of field.

### 3. Before / After sound
9:16 split-reveal. Left: dull gray empty studio, flat silent. Hard cut at the drop: the same room explodes in color, [NEON/GOLD] lights, vibrating subwoofers, confetti of audio waveforms. Fast whip transition, high energy.

### 4. Empire du beat (HUMPIRE)
9:16. Low-angle shot of a king-like beatmaker on a throne made of drum machines and speakers, crowd of silhouettes below, gold smoke. At the drop the throne rises, lights flare, camera orbits fast around him. G-Funk West Coast vibe, sunset gold and purple.

### 5. Meme / absurde
9:16. A solemn emperor in a golden hump-shaped throne dances ridiculously to a bouncy trap beat, serious face, handheld camera, comedic timing, bright lighting, looping.

### 6. Unboxing pack (sample pack / drum kit)
9:16. Hands open a glowing black box labelled [NOM], waveforms and drum hits float out in slow motion, camera dives into the box to reveal a studio. Clean product-shot look, [COULEUR] accent light.

## Checklist post-gen
- Hook < 1,5 s ? Pas de texte IA illisible ? Format 9:16 ?
- Ajouter : beat DJBILBOX, texte "HUMPIRE · -30% · CODE", lien en bio.
- Hashtags : #beatmaker #flstudio #vst #producer #gfunk #typebeat #humpire
