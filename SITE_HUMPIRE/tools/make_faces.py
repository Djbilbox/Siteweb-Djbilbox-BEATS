"""Web-size the plugin faces (<PLUGIN>/Assets/faceplate.jpg) for the shop tiles.

Usage: python tools/make_faces.py <repo_root> <site_dir>
Writes <site_dir>/img/vst/faces/<product-slug>.jpg (1000 px wide).
UNPUBLISHED plugins are skipped: add a folder there until the product is released.
"""
import os, sys
from PIL import Image

FOLDERS = {  # plugin folder -> product page slug
    'ARENA_BASS': 'arena-bass', 'CHINTZWAZ': 'chintzwaz', 'CYBERPUNK_LEAD': 'cyberpunk-lead', 'DZPOWER': 'dzpower',
    'FUNKYKOPS': 'funkykops', 'GOLDHAWK': 'goldhawk', 'HIGHROLLER_GOLD': 'highroller-gold', 'K2000': 'k2000',
    'MARABOUT': 'marabout', 'MIDNIGHT_REELS': 'midnight-reels', 'OCCITANIE': 'occitanie', 'OCTOPUS': 'octopus',
    'PHANTOM_RIDER': 'phantom-rider', 'ROCKPUNK': 'rockpunk-guitar', 'SAN_ANDREAS_LEAD': 'san-andreas-lead',
    'SHAMAN': 'shaman', 'STREETLIGHT_TRAP': 'streetlight-trap', 'VOODOO': 'voodoo', 'WAZONE808': 'warzone808',
}
UNPUBLISHED = {'HBX84_TALKBOX', 'QUEEN_OF_XMAS'}   # not released yet: never shown on the site

def main(repo, site):
    out = os.path.join(site, 'img', 'vst', 'faces')
    os.makedirs(out, exist_ok=True)
    for folder, slug in FOLDERS.items():
        src = os.path.join(repo, folder, 'Assets', 'faceplate.jpg')
        if folder in UNPUBLISHED or not os.path.exists(src):
            continue
        im = Image.open(src).convert('RGB')
        im = im.resize((1000, round(im.height * 1000 / im.width)), Image.LANCZOS)
        im.save(os.path.join(out, slug + '.jpg'), quality=84, optimize=True)
    print(len(os.listdir(out)), 'faces in', out)

main(sys.argv[1], sys.argv[2])
