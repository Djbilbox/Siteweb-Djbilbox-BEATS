#!/usr/bin/env python3
"""Create the Gumroad products from MARKETING/gumroad/<id>/ kits through the Gumroad API v2.

  export GUMROAD_ACCESS_TOKEN=...        # never commit it, never paste it in a chat
  python3 MARKETING/tools/gumroad_publish.py            # DRY RUN: prints what would be sent
  python3 MARKETING/tools/gumroad_publish.py --apply    # creates the products (UNPUBLISHED drafts)
  python3 MARKETING/tools/gumroad_publish.py --apply --only arena-bass

Notes
- Products are created as drafts. Publish them from the Gumroad dashboard after a last look
  (the API call that publishes is POST /v2/products/:id/enable; use --enable to also call it).
- Already existing products (matched on name) are skipped: the script never overwrites or deletes.
- Cover / thumbnail / file upload through the API is not guaranteed by Gumroad: each product that the
  API cannot complete is listed at the end with the files to drag & drop by hand (kit folder).
- The API documentation of "create product" is thin: the first --apply run on ONE product (--only <id>)
  is the real test. Read the printed HTTP status/body if it fails and adjust FIELD_MAP below.
"""
import argparse, json, os, re, sys, urllib.parse, urllib.request, urllib.error

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(HERE)
KITS = os.path.join(ROOT, 'gumroad')
API = 'https://api.gumroad.com/v2'


def read(path, default=''):
    try:
        with open(path, encoding='utf-8') as f: return f.read().strip()
    except OSError:
        return default


def html_body(path):
    t = read(path)
    m = re.search(r'<body[^>]*>(.*)</body>', t, re.S)
    return (m.group(1) if m else t).strip()


def call(method, path, token, fields=None):
    data = urllib.parse.urlencode({**(fields or {}), 'access_token': token}).encode()
    req = urllib.request.Request(API + path, data=data if method != 'GET' else None, method=method)
    if method == 'GET': req.full_url += '?' + data.decode()
    try:
        with urllib.request.urlopen(req, timeout=60) as r: return r.status, json.loads(r.read() or b'{}')
    except urllib.error.HTTPError as e:
        body = e.read().decode('utf-8', 'replace')[:600]
        try: return e.code, json.loads(body)
        except ValueError: return e.code, {'raw': body}


# Gumroad create-product fields (cents for price). Adjust here if the API answers 4xx.
def fields_for(kit):
    return {
        'name': read(os.path.join(kit, '1_title.txt')),
        'price': int(float(read(os.path.join(kit, '2_price_usd.txt'), '0')) * 100),
        'description': html_body(os.path.join(kit, '4_description.html')),
        'summary': read(os.path.join(kit, '3_summary.txt')),
        'tags': read(os.path.join(kit, '5_tags.txt')),
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--apply', action='store_true'); ap.add_argument('--only'); ap.add_argument('--enable', action='store_true')
    a = ap.parse_args()
    token = os.environ.get('GUMROAD_ACCESS_TOKEN', '')
    if a.apply and not token: sys.exit('GUMROAD_ACCESS_TOKEN is not set (add it as an environment secret, not in the chat).')
    kits = sorted(d for d in os.listdir(KITS) if os.path.isdir(os.path.join(KITS, d)) and (not a.only or d == a.only))
    existing = set()
    if token:
        st, js = call('GET', '/products', token)
        if st == 200: existing = {p.get('name', '').strip().lower() for p in js.get('products', [])}
        else: print('! cannot list existing products:', st, js)
    todo_by_hand = []
    for k in kits:
        kit = os.path.join(KITS, k); f = fields_for(kit)
        if not f['name']: continue
        if f['name'].strip().lower() in existing:
            print('skip (already exists):', k); continue
        print(('CREATE ' if a.apply else 'DRY    ') + f"{k:38s} ${f['price'] / 100:<6g} {f['name'][:70]}")
        if not a.apply: continue
        st, js = call('POST', '/products', token, f)
        if st not in (200, 201) or not js.get('success', True):
            print('   !! create failed:', st, json.dumps(js)[:400]); todo_by_hand.append((k, 'create product')); continue
        pid = (js.get('product') or {}).get('id')
        print('   created id', pid, (js.get('product') or {}).get('short_url', ''))
        # files / cover / thumbnail: not guaranteed by the public API -> always listed for manual completion
        todo_by_hand.append((k, 'upload the 2 zips + 7_thumbnail + 8_cover (see 6_file_to_upload.txt)'))
        if a.enable and pid:
            st2, js2 = call('PUT', f'/products/{pid}/enable', token); print('   enable:', st2)
    if todo_by_hand:
        print('\nTo finish by hand in the Gumroad dashboard:')
        for k, what in todo_by_hand: print(f'  - {k}: {what}   (kit folder: MARKETING/gumroad/{k}/)')


if __name__ == '__main__':
    main()
