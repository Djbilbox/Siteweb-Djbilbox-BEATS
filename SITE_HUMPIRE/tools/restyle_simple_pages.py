"""Give the theme pages (t/, fr/, es/, pt/, de/), p/index.html and contact.html the site look:
same header, fonts and palette as the product pages. Puts the newest releases (NEW_ITEMS) at the top of their lists.

Usage: python tools/restyle_simple_pages.py <site_dir>   (safe to run again)
"""
import os, re, sys

OLD_STYLE = 'background:#0d0710'
CARD = ('<li><a href="{p}%s.html"><img src="%s" alt="%s" loading="lazy">'
        '<strong>%s</strong><span class="s">%s</span><span class="pr">%s</span></a></li>')
FENNEC = CARD % ('fennec', '/img/vst/fennec-thumb.jpg', 'FENNEC', 'FENNEC',
                 'Oriental Instruments VST3 Plugin (10 real instruments, 330 presets)', '$79')
NEON2 = CARD % ('neon-synth-80s-2', 'https://public-files.gumroad.com/uzamf8xvbh46810eppwcr0it8b55', 'NEON SYNTH 80s 2.0',
                'NEON SYNTH 80s 2.0', '80s Synthwave Synth VST3 / AU (70 Presets)', 'Free')
NEW_ITEMS = {  # newest releases first in these lists (every language)
    'vst-synths.html': [('fennec', FENNEC), ('neon-synth-80s-2', NEON2)],
    'oriental-arabic-vst.html': [('fennec', FENNEC)],
    'free-vst-plugins.html': [('neon-synth-80s-2', NEON2)],
}
HEAD = ('<link rel="preconnect" href="https://fonts.googleapis.com"><link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>\n'
        '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700;800&family=Sora:wght@700;800&display=swap">\n'
        '<link rel="stylesheet" href="/assets/css/product-page.css?v=26110702">\n'
        '<link rel="stylesheet" href="/assets/css/simple-page.css?v=26110702">')
HEADER = ('<header class="pp-top"><div class="pp-top-in">'
          '<a class="pp-logo" href="/"><img src="/img/humpire-logo.png" alt=""><b>HUM<span>PIRE</span></b></a>'
          '<nav class="pp-nav"><a href="/beats.html">Beats</a><a href="/shop.html">Plugins</a><a href="/drum-kits.html">Drum Kits</a>'
          '<a href="/streetwear.html">Streetwear</a><a href="/ai-mastering.html">Mastering</a><a href="/video-studio.html">Video Studio</a></nav>'
          '<a class="pp-store" href="https://djbilboxbeats.gumroad.com/" target="_blank" rel="noopener">Store</a></div></header>')
FOOTER = ('<footer class="pp-foot">© HUMPIRE by DJBILBOX BEATS · <a href="/">Home</a><a href="/p/">All products</a>'
          '<a href="/beats.html">Beats</a><a href="/streetwear.html">Streetwear</a><a href="/contact.html">Contact</a>'
          '<a href="https://djbilboxbeats.gumroad.com/">Store</a></footer>')

def restyle(path, rel_p):
    s = open(path, encoding='utf-8').read()
    o = s
    if OLD_STYLE in s:
        s = re.sub(r'<style>.*?</style>', HEAD, s, count=1, flags=re.S)
        s = re.sub(r'<body(?![^>]*class=)([^>]*)>', r'<body class="pp sp"\1>', s, count=1)
        s = re.sub(r'<header>.*?</header>', HEADER, s, count=1, flags=re.S)
        s = re.sub(r'<footer>.*?</footer>', FOOTER, s, count=1, flags=re.S)
        s = s.replace('<main>', '<main class="sp-main">', 1)
    if 'class="pp-top"' in s:                       # pages already restyled: refresh header and footer
        s = re.sub(r'<header class="pp-top">.*?</header>', HEADER, s, count=1, flags=re.S)
        s = re.sub(r'<footer class="pp-foot">.*?</footer>', FOOTER, s, count=1, flags=re.S)
    # product cards: name / subtitle / price as separate lines, no inline image style
    s = re.sub(r' style="width:100%;border-radius:8px;margin-bottom:6px"', '', s)
    s = re.sub(r'</strong><br>([^<]*)<br>([^<]*)</a></li>', r'</strong><span class="s">\1</span><span class="pr">\2</span></a></li>', s)
    s = re.sub(r'(<p>)(<a href="https://humpireplugins\.netlify\.app/(?:fr|es|pt|de|t)/)', r'<p class="sp-langs">\2', s)
    if 'pp-top' in s and 'class="pp sp"' not in s:   # pages restyled before the <body id> fix
        s = re.sub(r'<body(?![^>]*class=)([^>]*)>', r'<body class="pp sp"\1>', s, count=1)
    for slug, card in reversed(NEW_ITEMS.get(os.path.basename(path), [])):
        if slug + '.html"' not in s:
            s = s.replace('<ul class="rel">', '<ul class="rel">' + card.replace('{p}', rel_p), 1)
    if s != o:
        open(path, 'w', encoding='utf-8', newline='').write(s)
        return True
    return False

def main(site):
    n = 0
    targets = [(os.path.join(site, 'contact.html'), 'p/'), (os.path.join(site, 'p', 'index.html'), '')]
    for d in ('t', 'fr', 'es', 'pt', 'de'):
        folder = os.path.join(site, d)
        if os.path.isdir(folder):
            targets += [(os.path.join(folder, f), '../p/') for f in sorted(os.listdir(folder)) if f.endswith('.html')]
    for path, rel_p in targets:
        if os.path.exists(path):
            n += restyle(path, rel_p)
    print(n, 'pages updated')

main(sys.argv[1])
