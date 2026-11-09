"""Build assets/js/shop-data.js for shop.html: every product page p/<slug>.html with its kind (plugin / kit / bundle),
buy slug (for the Gumroad release order) and face image. The shop shows ONE flat grid, newest release first.
SECTIONS below only classifies (synths by style, drum machines, effects, drum kits, bundles, free).

Usage: python tools/build_shop_data.py <site_dir> [audio_dir]
A new product: add its slug to the right list in SECTIONS (first = shown first).
Unlisted pages are reported so nothing is forgotten.
"""
import json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
NEW = {'fennec', 'neon-synth-80s-2'}          # "New" badge
# plugin face (interface shot, front view) shown in the shop instead of the promo art
FACE = {'bigbass': 'img/vst/ui/bigbass-ui.png', 'matrix-modular': 'img/vst/ui/matrix-ui.png',
        'mini-mpc-humpire': 'img/vst/ui/mini-mpc-humpire-ui.jpg', 'mpc-2026': 'img/vst/ui/mpc-2026-alt.png',
        'neon-synth-80s': 'img/vst/ui/neon-synth-80s-ui.jpg', 'neon-synth-80s-2': 'img/vst/ui/neon-synth-80s-ui.jpg',
        'oriental-instrument': 'img/vst/ui/oriental-ui.png', 'thuglife': 'img/vst/ui/thuglife-ui.jpg',
        'vice-city': 'img/vst/ui/vice-city-ui.png', 'mastering': 'img/vst/ui/mastering-ui.png'}
SECTIONS = [
    {'id': 'synths', 'title': 'Synths', 'kicker': 'Instruments by style', 'banner': 'pWfqqZt0py0',
     'text': 'Real presets you can play right here, sorted by the sound you are chasing.',
     'groups': [
        {'id': 'oriental', 'title': 'Oriental & World', 'text': 'Oud, saz, qanun, ney, raï, chaabi, afro and folk.',
         'items': ['fennec', 'oriental-instrument', 'dzpower', 'marabout', 'voodoo', 'shaman', 'occitanie']},
        {'id': 'gfunk', 'title': 'G-Funk & West Coast', 'text': 'Whistle leads, talkbox pads and lowrider basses.',
         'items': ['gfunk-2027', 'thuglife', 'san-andreas-lead', 'bigbass', 'thuglife-v1-0-0']},
        {'id': 'trap', 'title': 'Trap, Drill & 808', 'text': '808s, reese, dark bells and luxury leads.',
         'items': ['warzone808', 'streetlight-trap', 'arena-bass', 'highroller-gold']},
        {'id': 'synthwave', 'title': '80s Synthwave', 'text': 'Supersaws, neon pads and Miami nights.',
         'items': ['neon-synth-80s-2', 'neon-synth-80s', 'vice-city', 'k2000', 'cyberpunk-lead', 'station-synth']},
        {'id': 'cinematic', 'title': 'Cinematic & Dark', 'text': 'Film noir, ghost westerns, abyss and horror keys.',
         'items': ['goldhawk', 'midnight-reels', 'phantom-rider', 'octopus', 'blood-keys']},
        {'id': 'funk', 'title': 'Funk, Disco & Rock', 'text': 'Clavinet, Rhodes, psychedelic keys and real guitars.',
         'items': ['funkykops', 'chintzwaz', 'rockpunk-guitar']},
     ]},
    {'id': 'drum-machines', 'title': 'Drum Machines & Generators', 'kicker': 'Beats in one click', 'banner': 'Tu2XFMJ-bWI',
     'text': 'Pads, real drummer grooves, 808 patterns, basslines and maqam chords.',
     'groups': [
        {'id': 'machines', 'title': 'Drum machines', 'text': 'MPC-style pads and full beat generators.',
         'items': ['mpc-2026', 'mini-mpc-humpire', 'block-gorilla', 'taxi-alger']},
        {'id': 'generators', 'title': 'Bass, 808 & chord generators', 'text': 'Patterns that write themselves.',
         'items': ['alien-808', 'camel-pilot', 'casbah-chords']},
     ]},
    {'id': 'effects', 'title': 'Effects', 'kicker': 'Mix, master, mangle', 'banner': 'eMCRZFnGLb8',
     'text': 'From the first EQ move to the final limiter, plus a whole pedalboard.',
     'groups': [
        {'id': 'mixing', 'title': 'Mixing & Mastering', 'text': 'Compressors, EQs, enhancer and limiters.',
         'items': ['mastering', 'humpire-hardcap', 'humpire-clutch', 'chocolatine', 'la-baguette', 'humpire-skilltree',
                   'eq-pro-spider', 'humpire-overclock']},
        {'id': 'creative', 'title': 'Vocals & Creative FX', 'text': 'Talkbox, reverbs, delays, glitch and stutter.',
         'items': ['robotalk', 'humpire-respawn', 'humpire-lag', 'humpire-glitch', 'humpire-combo', 'matrix-modular']},
        {'id': 'pedals', 'title': 'CAMEL PEDALS — guitar pedals', 'text': 'Ten stompboxes for guitar, keys and beats.',
         'items': ['camel-pedals', 'camel-pedal', 'lava-fuzz', 'storm-rider', 'pharaoh-comp', 'aurora-verb', 'canyon-echo',
                   'coral-chorus', 'neon-phaser', 'surf-tremolo']},
     ]},
    {'id': 'kits', 'title': 'Drum Kits & Loops', 'kicker': 'Sounds from my own records', 'banner': 'SZWUoWc7HYY',
     'text': 'One-shots, drum loops and melody loops — most of them free for profit.',
     'groups': [
        {'id': 'kits-westcoast', 'title': 'G-Funk & West Coast', 'text': 'Lowrider drums, funk kits and melody loops.',
         'items': ['boyz-n-da-loop', 'menace-to-melody', 'lowrider-drum-loops-2026', 'westcoast-vybes-vol-1', 'west-side-drum-kit',
                   'westcoast-chrome', 'cali-g-funk-melody-loops-2026', 'sunset', 'kit-drum-funk', 'breack-ya-neck-vol-2']},
        {'id': 'kits-trap', 'title': 'Hip-Hop, Trap & Drill', 'text': '808s, drill drums, boom bap and vocal chops.',
         'items': ['931-free-rap-beats', 'concrete-vault-vol-2', 'concrete-vault', 'raw-elements-vol-2', 'raw-elements',
                   'vol-3-blue-pack', 'vol-1-drums-loop', 'breack-ya-neck-vol-2-zpcjxk', 'vice-city-vol-4', 'viper', 'supreme',
                   'futur-melody', 'ghost-voice', 'vinyl-breaker', 'tone-vault', 'venom']},
        {'id': 'kits-horror', 'title': 'Horror & Dark Trap', 'text': 'Slasher chops, haunted melodies, industrial hits.',
         'items': ['necropolis-vol-2', 'necropolis', 'biohazard-re', 'haunted-souls', 'scary-movie-slasher-trap-kit',
                   'possessed-doll-chucky-horror-beat-kit']},
        {'id': 'kits-world', 'title': 'Afro, House & Oriental', 'text': 'Afrotrap, afrobeat, house and Middle East loops.',
         'items': ['etnic-ritmik-vol-2', 'etnic-ritmik', 'neon-pulse-vol-2', 'neon-pulse', 'oriental-arabic']},
     ]},
    {'id': 'bundles', 'title': 'Bundles', 'kicker': 'Everything, one payment', 'banner': '58NdjorpGD4',
     'text': 'The best price per plugin — take a whole collection at once.',
     'groups': [
        {'id': 'bundles-all', 'title': 'Bundles', 'text': '',
         'items': ['djbilbox-beats-synth-collection', 'humpire-vst-and-effects', 'humpire-vst-suite', 'humpire-loadout',
                   'djbilbox-pro-bundle', 'djbilbox-beats', 'humpire-free-vst-collection', 'djbilbox-beats-ozwlb']},
     ]},
    {'id': 'free', 'title': 'Free VST', 'kicker': 'Download, no card', 'banner': '_EK5JBNUAoo',
     'text': 'Full free plugins and free demos of the paid ones.',
     'groups': [
        {'id': 'free-all', 'title': 'Free plugins & demos', 'text': '',
         'items': ['neon-synth-80s-2', 'gfunk-2027', 'neon-synth-80s', 'mpc-2026', 'mini-mpc-humpire', 'eq-pro-spider',
                   'humpire-free-vst-collection', 'station-synth-free-demo', 'thuglife-free-demo', 'oriental-instrument-free-demo']},
     ]},
]

def main(site, audio_dir):
    media = json.load(open(os.path.join(HERE, 'product_media.json'), encoding='utf-8'))
    products = {}
    for fn in os.listdir(os.path.join(site, 'p')):
        if not fn.endswith('.html') or fn == 'index.html':
            continue
        s = open(os.path.join(site, 'p', fn), encoding='utf-8').read()
        m = re.search(r'<script type="application/json" id="pp-src">(.*?)</script>', s, re.S)
        if not m:
            continue
        f = json.loads(m.group(1))
        slug = fn[:-5]
        play = media.get(slug, {}).get('play', slug)
        face = 'img/vst/faces/%s.jpg' % slug
        if not os.path.exists(os.path.join(site, face)):
            face = FACE.get(slug)
        products[slug] = {'slug': slug, 'name': f['name'], 'sub': f['sub'], 'price': f['price'],
                          'buy': f['buy'].rstrip('/').split('/')[-1], 'kind': 'plugin',
                          'face': bool(face), 'new': slug in NEW, 'img': face or f['img'],
                          'video': bool(media.get(slug, {}).get('yt')),
                          'play': os.path.exists(os.path.join(audio_dir, 'assets', 'audio', 'play', play + '.json'))}
    used = set()
    KIND = {'kits': 'kit', 'bundles': 'bundle'}
    for sec in SECTIONS:
        for g in sec['groups']:
            for k in g['items']:
                if k in products and k not in used:
                    products[k]['kind'] = KIND.get(sec['id'], 'plugin')
            missing = [s for s in g['items'] if s not in products]
            if missing:
                print('no page for', missing)
            g['items'] = [s for s in g['items'] if s in products]
            used.update(g['items'])
    left = sorted(set(products) - used)
    if left:
        print('NOT IN ANY COLLECTION:', left)
    out = os.path.join(site, 'assets', 'js', 'shop-data.js')
    with open(out, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('/* Generated by tools/build_shop_data.py from p/*.html — do not edit. */\n')
        fh.write('window.SHOP_SECTIONS = %s;\n' % json.dumps(SECTIONS, ensure_ascii=False))
        fh.write('window.SHOP_PRODUCTS = %s;\n' % json.dumps(products, ensure_ascii=False))
    print(len(products), 'products,', len(used), 'placed')

main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else sys.argv[1])
