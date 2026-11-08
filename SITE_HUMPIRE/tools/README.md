# Site tools

The live site (https://humpireplugins.netlify.app, Netlify team `sirabaprod`) is deployed from
`C:\work\SITE_MERGED`, not from this folder: that folder is the old full site (Shop, Mastering,
Video Studio, Game, 10 000+ beat pages, 200 MB of preset audio) with every file of this folder
copied on top. Copy changed files from here into it before deploying:

    netlify deploy --prod --no-build --dir C:/work/SITE_MERGED --site bcf46186-04a2-407f-afa6-e2df66052a64

| Script | What it refreshes |
|---|---|
| `youtube_playlists.py <out.json>` | Titles and ids of every playlist of the YouTube channel (`youtube_playlists.json`) |
| `build_beats_data.py` | `assets/js/beats-data.js` for `beats.html`, from the "All Beats" playlist |
| `build_streetwear_data.py` | `assets/js/streetwear-data.js` for `streetwear.html`, from the Fourthwall store |
| `build_product_pages.py <site> [audio_dir]` | Every `p/*.html`: layout, promo code, presets, video (`product_media.json`) |
| `build_shop_data.py <site> [audio_dir]` | `assets/js/shop-data.js` for `shop.html`: every product page sorted into collections (add new slugs to `SECTIONS`) |
| `restyle_simple_pages.py <site>` | Theme pages (t/, fr/, es/, pt/, de/), `p/index.html` and `contact.html` in the site look; newest releases first (`NEW_ITEMS`) |
| `set_site_domain.py <site> <host>` | Rewrites absolute URLs if the site ever moves to another domain |

The store-wide promo code lives in `assets/js/promo-code.js`; turn it on only once the code exists in Gumroad.
