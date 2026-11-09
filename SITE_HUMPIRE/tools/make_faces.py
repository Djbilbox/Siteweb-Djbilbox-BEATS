"""Web-size the plugin faces for the shop tiles (4:3, nothing cropped).

Source, first match wins: tools/face_sources/<slug>.png (finished interface screenshot),
then <PLUGIN>/Assets/faceplate.jpg.
Usage: python tools/make_faces.py <repo_root> <site_dir>
Writes <site_dir>/img/vst/faces/<product-slug>.jpg (1000x750).
UNPUBLISHED plugins are skipped: add a folder there until the product is released.
"""
import os, sys
from PIL import Image, ImageChops

FOLDERS = {  # plugin folder -> product page slug
    'ARENA_BASS': 'arena-bass', 'CHINTZWAZ': 'chintzwaz', 'CYBERPUNK_LEAD': 'cyberpunk-lead', 'DZPOWER': 'dzpower',
    'FUNKYKOPS': 'funkykops', 'GOLDHAWK': 'goldhawk', 'HIGHROLLER_GOLD': 'highroller-gold', 'K2000': 'k2000',
    'MARABOUT': 'marabout', 'MIDNIGHT_REELS': 'midnight-reels', 'OCCITANIE': 'occitanie', 'OCTOPUS': 'octopus',
    'PHANTOM_RIDER': 'phantom-rider', 'ROCKPUNK': 'rockpunk-guitar', 'SAN_ANDREAS_LEAD': 'san-andreas-lead',
    'SHAMAN': 'shaman', 'STREETLIGHT_TRAP': 'streetlight-trap', 'VOODOO': 'voodoo', 'WAZONE808': 'warzone808',
}
EXTRA = ['fennec', 'neon-synth-80s-2']   # products without a plugin folder, screenshot only
EXTRA_FILES = {  # slug -> interface shot already in the site (path relative to the site folder)
    'bigbass': 'img/vst/ui/bigbass-card.jpg', 'matrix-modular': 'img/vst/ui/matrix-modular-card.jpg',
    'mini-mpc-humpire': 'img/vst/ui/mini-mpc-humpire-ui.jpg', 'mpc-2026': 'img/vst/ui/mpc-2026-card.jpg',
    'neon-synth-80s': 'img/vst/ui/neon-synth-80s-card.jpg', 'oriental-instrument': 'img/vst/ui/oriental-instrument-card.jpg',
    'thuglife': 'img/vst/ui/thuglife-ui.jpg', 'thuglife-free-demo': 'img/vst/ui/thuglife-ui.jpg',
    'vice-city': 'img/vst/ui/vice-city-card.jpg', 'mastering': 'img/vst/ui/mastering-card.jpg',
    'station-synth': 'img/vst/ui/station-synth-card.jpg', 'djbilbox-pro-bundle': 'img/vst/ui/pro-bundle-card.jpg',
}
UNPUBLISHED = {'HBX84_TALKBOX', 'QUEEN_OF_XMAS'}   # not released yet: never shown on the site

def trim(im, margin=0.03, thr=22):
    """Crop the empty frame around a plugin card (keeps the whole plugin plus a small margin)."""
    px = im.load(); w, h = im.size
    bg = tuple(sum(px[x, y][c] for x, y in ((2, 2), (w - 3, 2), (2, h - 3), (w - 3, h - 3))) // 4 for c in range(3))
    box = ImageChops.difference(im, Image.new('RGB', im.size, bg)).convert('L').point(lambda v: 255 if v > thr else 0).getbbox()
    if not box:
        return im
    mx, my = round((box[2] - box[0]) * margin), round((box[3] - box[1]) * margin)
    return im.crop((max(0, box[0] - mx), max(0, box[1] - my), min(w, box[2] + mx), min(h, box[3] + my)))

def fit43(im, w=1000, h=750, bg=(12, 17, 20)):
    """Scale to fit inside w x h (never crop) on a dark canvas."""
    k = min(w / im.width, h / im.height)
    im = im.resize((round(im.width * k), round(im.height * k)), Image.LANCZOS)
    canvas = Image.new('RGB', (w, h), bg)
    canvas.paste(im, ((w - im.width) // 2, (h - im.height) // 2))
    return canvas

def main(repo, site):
    here = os.path.dirname(os.path.abspath(__file__))
    out = os.path.join(site, 'img', 'vst', 'faces')
    os.makedirs(out, exist_ok=True)
    jobs = {slug: os.path.join(repo, folder, 'Assets', 'faceplate.jpg') for folder, slug in FOLDERS.items() if folder not in UNPUBLISHED}
    for slug in EXTRA:
        jobs[slug] = None
    for slug, fallback in jobs.items():
        shot = os.path.join(here, 'face_sources', slug + '.png')
        src = shot if os.path.exists(shot) else fallback
        if not src or not os.path.exists(src):
            continue
        fit43(Image.open(src).convert('RGB')).save(os.path.join(out, slug + '.jpg'), quality=86, optimize=True)
    for slug, rel in EXTRA_FILES.items():
        src = os.path.join(site, rel)
        if os.path.exists(src) and not os.path.exists(os.path.join(here, 'face_sources', slug + '.png')):
            fit43(trim(Image.open(src).convert('RGB'))).save(os.path.join(out, slug + '.jpg'), quality=86, optimize=True)
    print(len(os.listdir(out)), 'faces in', out)

main(sys.argv[1], sys.argv[2])
