#!/usr/bin/env python3
"""Build Gumroad kits (title, price, summary, description, tags, thumbnail, cover) from MARKETING/fiches/*.json.

Usage: python3 tools/build_kits.py [--only id] [--no-images]
Outputs: MARKETING/gumroad/<id>/..., MARKETING/gumroad/ALL_PRODUCTS.csv, MARKETING/out/<id>-thumb.png / -cover.png
"""
import argparse, csv, html, json, os, shutil, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(HERE)
FICHES = os.path.join(ROOT, 'fiches'); OUT = os.path.join(ROOT, 'out'); KITS = os.path.join(ROOT, 'gumroad')
SYSREQ = ["Windows 10 or 11, 64-bit", "VST3 host (designed for FL Studio; any VST3-compatible host should load it)", "4 GB RAM minimum, 8 GB recommended", "macOS is not supported by this release"]
LICENSE_SHORT = ("Royalty-free: everything you create with this plugin is yours and can be released and sold commercially (streaming, sales, sync, beat leasing) "
                 "with no royalties and no credit required. One license covers installation on two computers you own. "
                 "Redistributing or reselling the plugin, its presets or its sounds is not allowed. Full terms are included in the download.")

def load(path):
    with open(path, encoding='utf-8') as f: return json.load(f)

def esc(s): return html.escape(str(s), quote=False)

def price_of(f, pricing):
    pid = f['id']
    if pid in pricing.get('overrides', {}): return pricing['overrides'][pid]
    return pricing['defaults'].get('synth' if f['category'] == 'instrument' else 'effect')

def description_html(f, price, extra_members=None):
    h = [f"<h2>{esc(f['tagline'])}</h2>"]
    for p in f.get('about', []): h.append(f"<p>{esc(p)}</p>")
    if extra_members:
        h.append("<h3>What's included</h3><ul>" + "".join(f"<li>{esc(m)}</li>" for m in extra_members) + "</ul>")
    if f.get('features'):
        h.append("<h3>Key features</h3><ul>" + "".join(f"<li><strong>{esc(x['t'])}</strong> — {esc(x['d'])}</li>" for x in f['features']) + "</ul>")
    if f.get('genres'):
        h.append(f"<h3>Perfect for</h3><p>{esc(', '.join(f['genres']))}</p>")
    if f.get('includes'):
        h.append("<h3>In the download</h3><ul>" + "".join(f"<li>{esc(i)}</li>" for i in f['includes']) + "</ul>")
    specs = list(f.get('specs', [])) + [{"k": "Validation", "v": "Checked with the pluginval plug-in validator (strictness level 5)"}]
    h.append("<h3>Technical specs</h3><ul>" + "".join(f"<li><strong>{esc(s['k'])}:</strong> {esc(s['v'])}</li>" for s in specs) + "</ul>")
    h.append("<h3>System requirements</h3><ul>" + "".join(f"<li>{esc(s)}</li>" for s in SYSREQ) + "</ul>")
    h.append("<h3>License</h3><p>" + esc(LICENSE_SHORT) + "</p>")
    if f.get('faq'):
        h.append("<h3>FAQ</h3>" + "".join(f"<p><strong>{esc(q['q'])}</strong><br>{esc(q['a'])}</p>" for q in f['faq']))
    return "\n".join(h)

def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--only'); ap.add_argument('--no-images', action='store_true'); a = ap.parse_args()
    pricing = load(os.path.join(ROOT, 'pricing.json'))
    slugs = load(os.path.join(ROOT, 'slugs.json')) if os.path.exists(os.path.join(ROOT, 'slugs.json')) else {}
    fiches = [load(os.path.join(FICHES, n)) for n in sorted(os.listdir(FICHES)) if n.endswith('.json') and not n.startswith('_')]
    fiches = [f for f in fiches if f['id'] not in pricing.get('not_for_sale_yet', {}) and f['id'] not in pricing.get('already_in_store_do_not_create', [])]   # e.g. Chaabi: no real samples yet
    manifest = []
    rows = []
    for f in fiches:
        if a.only and f['id'] != a.only: continue
        shot = os.path.join(ROOT, 'screens', f['id'] + '.png')
        if not os.path.exists(shot):
            print('! no screenshot for', f['id'], '(skipped images)'); shot = None
        brand = 'HUMPIRE' if f['group'] in ('loadout', 'orbit') else 'DJBILBOX BEATS'
        logo = os.path.join(ROOT, 'assets', 'humpire-logo.png' if brand == 'HUMPIRE' else 'djbilbox-emblem.png')
        if shot:
            manifest.append({"id": f['id'], "name": f['name'], "kind": f.get('kind', ''), "tagline": f['tagline'], "screenshot": shot, "logo": logo,
                             "brand": brand, "accent": f.get('accent', '#f5b544'), "badges": f.get('badges', []),
                             "fsThumb": f.get('fs_thumb'), "fsCover": f.get('fs_cover')})
        price = price_of(f, pricing)
        kit = os.path.join(KITS, f['id']); os.makedirs(kit, exist_ok=True)
        write = lambda name, text: open(os.path.join(kit, name), 'w', encoding='utf-8').write(text)
        write('1_title.txt', f['gumroad_title'])
        write('2_price_usd.txt', str(price))
        write('3_summary.txt', f.get('summary', f['tagline']))
        d = description_html(f, price)
        write('4_description.html', '<!doctype html><meta charset="utf-8"><title>' + esc(f['name']) + '</title><body style="font-family:Inter,Arial,sans-serif;max-width:760px;margin:24px auto">' + d + '</body>')
        write('5_tags.txt', ", ".join(f.get('tags', [])[:10]))
        write('6_file_to_upload.txt', f"{f['id']}.zip  (from the GitHub Release: Setup.exe + README + license + manual PDF)")
        rows.append({"id": f['id'], "name": f['name'], "type": f['kind'], "price_usd": price, "title": f['gumroad_title'], "tags": ", ".join(f.get('tags', [])[:10]),
                     "file": f"{f['id']}.zip", "gumroad_slug": slugs.get(f['id'], '')})
    # bundles
    by_group = {}
    for f in fiches: by_group.setdefault(f['group'], []).append(f)
    for b in pricing['bundles']:
        members = fiches if b['members_group'] == 'all' else by_group.get(b['members_group'], [])
        if a.only and b['id'] != a.only: continue
        if not members: continue
        total = sum(price_of(m, pricing) for m in members)
        kit = os.path.join(KITS, b['id']); os.makedirs(kit, exist_ok=True)
        write = lambda name, text: open(os.path.join(kit, name), 'w', encoding='utf-8').write(text)
        title = f"{b['name']} — {len(members)} VST3 Plugins, One Payment"
        names = [m['name'] + ' — ' + m['tagline'] for m in members]
        fb = {"tagline": b['tagline'], "about": [b['tagline'], f"Buying the {len(members)} plugins separately would cost ${total}. The bundle is ${b['price']}."],
              "features": [], "genres": [], "includes": [f"{len(members)} VST3 plugins", "One Windows installer (choose what to install)", "PDF user manuals", "Royalty-free license"],
              "specs": [{"k": "Format", "v": "VST3 64-bit (Windows)"}, {"k": "Plugins", "v": str(len(members))}], "faq": []}
        write('1_title.txt', title); write('2_price_usd.txt', str(b['price'])); write('3_summary.txt', b['tagline'])
        write('4_description.html', '<!doctype html><meta charset="utf-8"><body style="font-family:Inter,Arial,sans-serif;max-width:760px;margin:24px auto">' + description_html(fb, b['price'], names) + '</body>')
        write('5_tags.txt', 'vst bundle, vst3, plugins, fl studio, producer tools')
        write('6_file_to_upload.txt', f"{b['id']}.zip (from the GitHub Release)")
        rows.append({"id": b['id'], "name": b['name'], "type": b['kind'], "price_usd": b['price'], "title": title, "tags": 'vst bundle, vst3, plugins',
                     "file": f"{b['id']}.zip", "gumroad_slug": slugs.get(b['id'], '')})
    os.makedirs(KITS, exist_ok=True)
    with open(os.path.join(KITS, 'ALL_PRODUCTS.csv'), 'w', newline='', encoding='utf-8') as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys())) if rows else None
        if w: w.writeheader(); [w.writerow(r) for r in rows]
    if manifest and not a.no_images:
        mf = os.path.join(OUT, '_manifest.json'); os.makedirs(OUT, exist_ok=True); json.dump(manifest, open(mf, 'w'), indent=1)
        env = dict(os.environ); env['NODE_PATH'] = (env.get('NODE_PATH', '') + ':/node-tools/node_modules').strip(':')
        subprocess.check_call(['node', os.path.join(HERE, 'render_assets.js'), mf, OUT], env=env)
        for m in manifest:
            kit = os.path.join(KITS, m['id'])
            for kind, name in (('thumb', '7_thumbnail_600x600.png'), ('cover', '8_cover_1280x720.png')):
                src = os.path.join(OUT, f"{m['id']}-{kind}.png")
                if os.path.exists(src): shutil.copy(src, os.path.join(kit, name))
    print('kits:', len(rows))

if __name__ == '__main__': main()
