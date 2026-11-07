"""Point every absolute site URL (canonical, sitemap, Open Graph, share links) at a new domain.

Usage: python tools/set_site_domain.py <site_dir> <new_host>
       python tools/set_site_domain.py C:/work/SITE_MERGED humpire.pages.dev
"""
import os, sys
from urllib.parse import quote

OLD = 'humpireplugins.netlify.app'
EXT = ('.html', '.xml', '.txt', '.js', '.json', '.webmanifest')

def main(root, new):
    pairs = [(OLD, new), (quote('https://' + OLD, safe=''), quote('https://' + new, safe=''))]
    changed = 0
    for d, _, files in os.walk(root):
        for f in files:
            if not f.endswith(EXT):
                continue
            p = os.path.join(d, f)
            with open(p, encoding='utf-8', errors='surrogateescape') as fh:
                s = fh.read()
            o = s
            for a, b in pairs:
                s = s.replace(a, b)
            if s != o:
                with open(p, 'w', encoding='utf-8', errors='surrogateescape', newline='') as fh:
                    fh.write(s)
                changed += 1
    print(changed, 'files now point at', new)

main(sys.argv[1], sys.argv[2])
