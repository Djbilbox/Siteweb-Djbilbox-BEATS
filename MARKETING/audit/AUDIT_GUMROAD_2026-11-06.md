# Audit boutique Gumroad HUMPIRE PLUGINS — 6 novembre 2026

Données réelles : Gumroad Analytics du 6/10 au 6/11/2026 (export complet : `gumroad_audit_30j_2026-11-06.csv`), onglets Share (catégorie, tags, statut Discover) de fiches témoins.

## 1. Le diagnostic en 6 chiffres

| | 30 derniers jours |
|---|---|
| Vues | **7 339** |
| Téléchargements / ventes | **2 205** (presque tous gratuits) |
| Revenu | **449,70 $** |
| Produits publiés | 109 (48 gratuits, 61 payants) |
| Vues des 61 produits payants | **1 908** (31 vues par produit et par mois en moyenne) |
| Produits payants sans aucune vente | **~52 sur 61** |

**Ce que ça veut dire :** le problème n'est ni le son ni le prix. Les gratuits attirent du monde (GFUNK 2027 : 1 260 vues, 46 % de conversion). Mais ce public **n'est jamais conduit vers les payants**, et la plupart des payants sont **invisibles sur Discover**.

## 2. Les 5 causes, par ordre d'importance

### Cause 1 — Les payants sans vente n'existent pas sur Discover (impact très fort)
Gumroad n'affiche un produit sur Discover qu'après **au moins une vente réussie**. Constaté :
- WARZONE808 (19 $, 0 vente) : **non listé**.
- GFUNK 2027 et ORIENTAL INSTRUMENT (qui ont des ventes) : **listés**.

C'est un cercle vicieux : pas de vente, donc pas de visibilité, donc pas de vente. Les ~19 synthés à 19 $ et les 10 pédales à 9 $ sont dans ce cas.

### Cause 2 — Aucun pont entre gratuit et payant (impact très fort)
- 2 205 téléchargements gratuits en un mois, donc autant d'e-mails clients, mais aucune séquence automatique (Workflows) ne propose ensuite le produit payant correspondant.
- ORIENTAL INSTRUMENT (29 $) : 398 vues et seulement 5 ventes (1,3 %). Sa démo gratuite a 204 vues et 76 téléchargements. Ces 76 personnes ne reçoivent aucune relance vers la version complète.

### Cause 3 — Trop de produits qui se ressemblent (impact fort)
- 19 synthés « VST3 + AU Plugin (70 Presets) » à 19 $, avec des titres quasi identiques.
- 5 bundles qui se chevauchent : LOADOUT 49 $, PRO BUNDLE 49 $, VST SUITE 59 $, Synth Collection 79 $, VST & Effects 79 $.
- L'acheteur ne sait pas quoi choisir, et l'attention est dispersée sur 61 fiches à 0 vente.

### Cause 4 — Presque pas de trafic Google (impact moyen, long terme)
- Google : **91 vues en 30 jours** (1,2 %). Le SEO ne rapporte rien aujourd'hui.
- Les liens de fiche sont aléatoires (`/l/svdue`, `/l/bsscfh`…) et les titres sont saturés de mots-clés (« | 808, Reese, Trap, Drill »).
- Ne PAS changer les liens des fiches existantes : Gumroad ne redirige pas les anciens liens, et KVR ou tes vidéos pointent dessus. On garde des liens propres pour les nouveaux produits (comme `/l/neon-synth-80s-2`).

### Cause 5 — Preuve sociale faible ou abîmée (impact moyen)
- GFUNK 2027 : 4,0/5 sur 37 notes, dont **19 % à 1 étoile**. Probablement des utilisateurs Mac, qui achetaient une variante Mac vide (corrigé le 6/11 : la vraie version Mac est en ligne).
- Les payants n'ont presque aucun avis.

## 3. Ce qui marche (à amplifier)

| Source | Vues | Conversion | Revenu |
|---|---|---|---|
| Ton profil Gumroad | 1 432 | 41,7 % | 203 $ |
| **YouTube** | 506 | **43,7 %** | 45 $ |
| Gumroad Discover (« Recommended ») | 1 265 | 34,6 % | 29 $ |
| KVR | 324 | 29 % | 28 $ |
| Ton site Netlify | 47 | **68,1 %** | 0 $ |

- **YouTube a le meilleur taux de conversion.** Une vidéo par produit est le levier le plus rentable.
- Pays qui **paient** : Pays-Bas (108 $), États-Unis (105 $), Espagne (79 $), Allemagne (50 $), Royaume-Uni (49 $). Les textes doivent rester en anglais.
- Les seuls payants qui vendent : ORIENTAL INSTRUMENT, BLOOD KEYS, CASBAH CHORDS, BLOCK GORILLA, THUGLIFE, TAXI ALGER, Synth Collection. Ce sont des produits **avec une idée claire** (oriental, horreur, générateur d'accords, boîte à rythmes), pas des « synthés à 70 presets » génériques.

## 4. Plan d'action classé (impact / facilité)

| # | Action | Impact | Facilité | Qui |
|---|---|---|---|---|
| 1 | Séquence e-mail automatique après chaque gratuit, vers le payant le plus proche (GFUNK → THUGLIFE/K2000 ; démo Oriental → ORIENTAL 29 $ ; MINI MPC → BLOCK GORILLA) | ★★★★★ | ★★★★ | Claude prépare, tu valides |
| 2 | Offre de lancement aux 2 200 clients gratuits : synthés à **-50 % pendant 72 h** (vraies ventes, donc visibilité Discover) | ★★★★★ | ★★★★ | Claude prépare, tu valides l'envoi |
| 3 | Mettre en avant 1 seul bundle phare (Synth Collection 79 $) et dépublier les bundles en double | ★★★★ | ★★★★★ | Claude, avec ton accord |
| 4 | Une vidéo YouTube Short par produit phare (règles HUMPIRE : sans prix ni nom d'artiste) | ★★★★ | ★★★ | Claude prépare scripts et visuels |
| 5 | Démo audio (30-60 s) sur chaque fiche payante | ★★★★ | ★★★ | rendus automatiques depuis les presets |
| 6 | Titres propres : « NOM — Ce que c'est (atout principal) », sans liste de tags dans le titre | ★★★ | ★★★★★ | Claude |
| 7 | Demander des avis aux utilisateurs actuels (e-mail « Mac est prêt » pour GFUNK + note) | ★★★ | ★★★★ | Claude prépare, tu valides |
| 8 | Presse : 1 envoi par site et par semaine (BPB fait le 6/11) | ★★★ | ★★★★ | en cours |
| 9 | Pages produits sur le site Netlify (68 % de conversion) avec lien Gumroad | ★★★ | ★★★ | Claude |
| 10 | Renommer la section de profil « 🎃 Products🩸 » (Halloween fini) | ★★ | ★★★★★ | Claude |

## 5. Prix : tests A/B proposés

- **Synthés à 19 $** : passer 3 d'entre eux (WARZONE808, STREETLIGHT TRAP, CYBERPUNK LEAD) en prix libre avec **minimum 5 $** pendant 2 semaines. Mesurer les ventes et la visibilité Discover, puis comparer avec 3 synthés restés à 19 $.
- **ORIENTAL INSTRUMENT 29 $** : garder le prix, ajouter un code « DEMO20 » (-20 %) envoyé uniquement aux 76 personnes qui ont la démo.
- **Bundle** : Synth Collection 79 $ = 19 synthés (valeur affichée 361 $, soit -78 %). Montrer cette valeur barrée dans la description.
- **Upsell** : sur chaque synthé à 19 $, ajouter la ligne « Get all 19 synths for $79 ».

## 6. Avis authentiques (sans faux avis)
1. E-mail aux utilisateurs de GFUNK : « La version Mac est disponible, re-téléchargez depuis votre bibliothèque Gumroad ». Ajouter une ligne demandant une note après essai.
2. Dans chaque séquence d'e-mails : au 7e jour, « Comment trouvez-vous X ? Une note sur Gumroad aide énormément un développeur indépendant ».
3. Afficher les notes uniquement là où elles sont bonnes (MINI MPC 4,8/5).
4. Jamais de faux avis, d'avis achetés ni de comptes multiples.

## 7. Calendrier 30 jours

**Semaine 1 (6-12/11)**
- ✅ NEON SYNTH 80s 2.0 publié (Win + Mac), GFUNK Mac en ligne, fiches corrigées, BPB contacté
- Séquences e-mail après gratuit (GFUNK, démo Oriental, MINI MPC, NEON)
- Renommer la section de profil et dépublier les bundles en double (avec ton accord)

**Semaine 2 (13-19/11)**
- E-mail « GFUNK Mac est prêt » + demande de note
- Offre -50 % 72 h sur les synthés à tes clients gratuits
- Presse : MINI MPC chez BPB ; YouTube Short NEON SYNTH 2.0

**Semaine 3 (20-26/11)**
- Démos audio sur les 10 payants les plus vus
- Titres propres sur les 19 synthés
- Presse : Plugins4Free (compte à créer par toi)

**Semaine 4 (27/11-3/12) — Black Friday**
- Black Friday : bundle Synth Collection en avant, -40 %
- Shorts YouTube : ORIENTAL INSTRUMENT, BLOCK GORILLA
- Bilan : comparer les chiffres au 6/11 (vues payants, ventes, Discover)

## 8. Checklist par produit
- [ ] Titre : « NOM — ce que c'est (atout) », sans liste de tags
- [ ] Résumé d'une ligne avec le bénéfice principal
- [ ] Image de couverture + miniature qui montre le plugin en entier
- [ ] Démo audio ou vidéo
- [ ] Formats et OS exacts (pas de « Mac » sans fichier Mac)
- [ ] Catégorie Discover : Music & Sound Design > Sound Design > Plugins
- [ ] 10 tags (genre, type, DAW, « free vst » si gratuit)
- [ ] Lien vers le produit payant ou le bundle correspondant
- [ ] Au moins 1 vente réelle (sinon invisible sur Discover)
- [ ] Séquence e-mail après téléchargement
