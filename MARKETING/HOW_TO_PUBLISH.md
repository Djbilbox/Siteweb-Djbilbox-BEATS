# Publier sur Gumroad et sur le site — mode d'emploi

Tout est prêt dans `MARKETING/gumroad/<produit>/` (un dossier par produit) :

| Fichier | Où le coller dans Gumroad |
|---|---|
| `1_title.txt` | Name |
| `2_price_usd.txt` | Price (USD) |
| `3_summary.txt` | Summary |
| `4_description.html` | Description (ouvre le .html dans un navigateur, copie tout, colle dans l'éditeur) |
| `5_tags.txt` | Tags |
| `6_file_to_upload.txt` | Dit quels fichiers envoyer |
| `7_thumbnail_600x600.png` | Thumbnail |
| `8_cover_1280x720.png` | Cover |

`MARKETING/gumroad/ALL_PRODUCTS.csv` = liste de tous les produits (nom, prix, titre, tags, fichiers).

## Fichiers à vendre (GitHub Releases du dépôt)
- Windows : `<produit>.zip` dans la release `djbilbox-all-plugins-v1.0.N` (Setup.exe + README + licence + manuel PDF).
- macOS : `<produit>-macOS.zip` dans la release `djbilbox-macos-v1.0.N` (.pkg + README + licence + manuel PDF).
- Mettre **les deux zip** dans chaque produit Gumroad (même prix Windows et Mac).
- Packs : `humpire-loadout`, `humpire-orbit`, `djbilbox-beats-synth-collection`, `djbilbox-beats-complete-collection`.

## Publication automatique (optionnelle)
1. Gumroad > Settings > Advanced > Applications > créer une application > Generate access token.
2. Mettre le token dans les réglages de l'environnement cloud (jamais dans le chat) : variable `GUMROAD_ACCESS_TOKEN`, et autoriser l'hôte `api.gumroad.com` dans la politique réseau.
3. Nouvelle session, puis :
   ```
   python3 MARKETING/tools/gumroad_publish.py                 # simulation
   python3 MARKETING/tools/gumroad_publish.py --apply --only arena-bass   # test sur UN produit
   python3 MARKETING/tools/gumroad_publish.py --apply         # tous (brouillons)
   ```
   Les produits sont créés en brouillon ; fichiers, thumbnail et cover sont à glisser à la main si l'API ne les accepte pas (le script liste ce qui reste).

## Site web
`python3 MARKETING/tools/apply_to_site.py <chemin du dépôt djboilboxbeats.github.io>` ajoute les produits à `assets/js/vst-data.js` et `products-data.js` + images. Tant que `MARKETING/slugs.json` n'a pas le lien Gumroad d'un produit, le bouton affiche « Coming soon ».

## Honnêteté commerciale
- Les installeurs macOS ne sont pas signés ni notarisés par Apple : le README de chaque zip explique l'étape « Ouvrir quand même ». Pour supprimer cet écran il faut un compte Apple Developer (99 $/an).
- Les installeurs Windows ne sont pas signés : SmartScreen peut afficher un avertissement (README).
- Aucun test d'écoute humain n'a été fait : écoute chaque plugin dans ton DAW avant de publier.
- Licence (EULA) rédigée sans avocat : à faire relire.
