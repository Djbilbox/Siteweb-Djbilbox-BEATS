"""Render every product page p/<slug>.html with the shared product layout:
buy box with promo code, playable presets (play-now), YouTube promo video, related products.

Usage: python tools/build_product_pages.py <site_dir> [audio_dir]
  audio_dir: folder that holds assets/audio/play/*.json (defaults to site_dir)
Safe to run again: each page keeps its source fields in <script id="pp-src">.
Videos and preset sets come from tools/product_media.json.
"""
import html, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
TAG = lambda s, rx: (re.search(rx, s, re.S) or [None, ''])[1]

def parse(src):
    stored = re.search(r'<script type="application/json" id="pp-src">(.*?)</script>', src, re.S)
    if stored:
        return json.loads(stored.group(1))
    head = TAG(src, r'<head>(.*?)</head>')
    head = re.sub(r'<style>.*?</style>\s*', '', head, flags=re.S)
    head = re.sub(r'<link rel="icon"[^>]*>\s*', '', head)
    main = TAG(src, r'<main>(.*?)</main>')
    ld = TAG(head, r'<script type="application/ld\+json">(.*?)</script>')
    try:
        price = float(json.loads(ld).get('offers', {}).get('price', 0))
    except ValueError:
        price = 0.0
    ctas = re.findall(r'<a class="cta" href="([^"]+)">(.*?)</a>', main)
    rel = re.findall(r'<li><a href="([^"]+)"><strong>(.*?)</strong><br>(.*?)</a></li>', main)
    return {
        'head': head.strip(),
        'tag': TAG(main, r'<span class="tag">(.*?)</span>'),
        'name': TAG(main, r'<h1>(.*?)</h1>'),
        'sub': TAG(main, r'<p class="sub">(.*?)</p>'),
        'img': TAG(main, r'<img class="hero" src="([^"]+)"'),
        'alt': TAG(main, r'<img class="hero"[^>]*alt="([^"]*)"'),
        'price_text': TAG(main, r'<p class="price">(.*?)</p>'),
        'price': price,
        'buy': ctas[0][0] if ctas else '',
        'cta': ctas[0][1] if ctas else 'Get it on Gumroad',
        'share': TAG(main, r'<p class="share">(.*?)</p>'),
        'about': TAG(main, r'<div class="card">(.*?)</div>'),
        'rel_title': TAG(main, r'<h2>(.*?)</h2>'),
        'rel': [list(r) for r in rel],
    }

def render(slug, f, media, images, audio_dir):
    m = media.get(slug, {})
    play = m.get('play', slug)
    has_play = os.path.exists(os.path.join(audio_dir, 'assets', 'audio', 'play', play + '.json'))
    yt = m.get('yt', '')
    free = f['price'] <= 0
    name = f['name']
    price_html = ('<span class="pp-free">Free</span>' if free else
                  '<span class="pp-now">%s</span>' % html.escape(re.sub(r'\.00$', '', '$%.2f' % f['price'])))
    video = ''
    if yt:
        thumb = ('https://i.ytimg.com/vi/%s/maxresdefault.jpg' % yt) if not yt.startswith('PL') else f['img']
        video = '''<section class="pp-sec" id="video"><h2><small>Hear it in action</small>%s — promo video</h2>
<div class="pp-video"><button type="button" class="pp-yt" data-yt="%s" aria-label="Play the %s video" style="background-image:url('%s')"><span></span><em>Watch on YouTube</em></button></div></section>''' % (
            name, yt, html.escape(name, quote=True), thumb)
    piano = ('<section class="pp-sec" id="play"><div data-play-now="%s"></div></section>' % play) if has_play else ''
    rel = ''.join('<li><a href="%s"><img src="%s" alt="%s" loading="lazy"><b>%s</b><span>%s</span></a></li>' % (
        h, images.get(h[:-5], ''), html.escape(n, quote=True), n, p) for h, n, p in f['rel'])
    src = json.dumps(f, ensure_ascii=False).replace('</', '<\\/')
    return f'''<!DOCTYPE html>
<html lang="en">
<head>
{f['head']}
<link rel="icon" href="../favicon.png">
<link rel="preconnect" href="https://fonts.googleapis.com"><link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700;800&family=Sora:wght@700;800&display=swap">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css">
<link rel="stylesheet" href="../assets/css/play-now.css?v=26110701">
<link rel="stylesheet" href="../assets/css/product-page.css?v=26110701">
</head>
<body class="pp">
<header class="pp-top"><div class="pp-top-in">
<a class="pp-logo" href="../"><img src="../img/humpire-logo.png" alt=""><b>HUM<span>PIRE</span></b></a>
<nav class="pp-nav"><a href="../beats.html">Beats</a><a href="../t/vst-synths.html">Synths</a><a href="../t/guitar-pedal-and-mixing-plugins.html">Effects</a><a href="../t/trap-drill-drum-kits.html">Drum Kits</a><a href="../t/free-vst-plugins.html">Free VST</a><a href="../streetwear.html">Streetwear</a><a href="./">All products</a></nav>
<a class="pp-store" href="https://djbilboxbeats.gumroad.com/" target="_blank" rel="noopener">Store</a>
</div></header>
<main class="pp-main">
<a class="pp-back" href="./">← Back to catalog</a>
<section class="pp-hero">
<div class="pp-media"><img src="{f['img']}" alt="{f['alt']}" loading="eager"></div>
<div>
<span class="pp-tag">{f['tag']}</span>
<h1>{name}</h1>
<p class="pp-sub">{f['sub']}</p>
<div class="pp-buy" data-price="{f['price']:.2f}">
<div class="pp-price">{price_html}</div>
<a class="pp-cta" href="{f['buy']}" target="_blank" rel="noopener">{f['cta']}</a>
<p class="pp-cta-note">Instant download · Secure checkout on Gumroad</p>
<ul class="pp-trust"><li>Instant download</li><li>Windows &amp; macOS</li><li>Royalty-free sounds</li><li>Made by DJBILBOX BEATS</li></ul>
</div>
<p class="pp-share">{f['share']}</p>
</div>
</section>
{piano}
{video}
<section class="pp-sec"><h2><small>Overview</small>About {name}</h2><div class="pp-about">{f['about']}</div></section>
{'<section class="pp-sec"><h2>' + f['rel_title'] + '</h2><ul class="pp-rel">' + rel + '</ul></section>' if rel else ''}
</main>
<footer class="pp-foot">© HUMPIRE by DJBILBOX BEATS · <a href="../">Home</a><a href="./">All products</a><a href="../beats.html">Beats</a><a href="../streetwear.html">Streetwear</a><a href="https://djbilboxbeats.gumroad.com/">Store</a></footer>
<div class="pp-bar"><b>{name}</b><span class="pp-bar-price">{'Free' if free else price_html and re.sub(r'<[^>]+>', '', price_html)}</span><a class="pp-bar-cta" href="{f['buy']}" target="_blank" rel="noopener">{'Download' if free else 'Buy now'}</a></div>
<script type="application/json" id="pp-src">{src}</script>
<script src="../assets/js/promo-code.js?v=26110701"></script>
<script src="../assets/js/product-page.js?v=26110701"></script>
{'<script src="../assets/js/play-now.js?v=26110701"></script>' if has_play else ''}
</body>
</html>
'''

def main(site, audio_dir):
    media = json.load(open(os.path.join(HERE, 'product_media.json'), encoding='utf-8'))
    pdir = os.path.join(site, 'p')
    pages = {}
    for fn in sorted(os.listdir(pdir)):
        if fn.endswith('.html') and fn != 'index.html':
            src = open(os.path.join(pdir, fn), encoding='utf-8').read()
            if '<h1>' in src or 'id="pp-src"' in src:
                pages[fn[:-5]] = parse(src)
    images = {s: f['img'] for s, f in pages.items()}
    stats = {'pages': 0, 'video': 0, 'piano': 0}
    for slug, f in pages.items():
        out = render(slug, f, media, images, audio_dir)
        with open(os.path.join(pdir, slug + '.html'), 'w', encoding='utf-8', newline='\n') as fh:
            fh.write(out)
        stats['pages'] += 1
        stats['video'] += 'class="pp-yt"' in out
        stats['piano'] += 'data-play-now' in out
    print(stats)

main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else sys.argv[1])
