"""USER_GUIDE.md -> USER_GUIDE.html autonome (images integrees), pour le zip Windows.
usage : python installer/make_guide_html.py <sortie.html>     (pip install markdown)
"""
import base64, os, re, sys
import markdown

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
md = open(os.path.join(root, "USER_GUIDE.md"), encoding="utf-8").read()
css = open(os.path.join(root, "installer", "guide_style.css"), encoding="utf-8").read()
body = markdown.markdown(md, extensions=["tables", "fenced_code"])

def inline(m):
    src = m.group(1)
    path = os.path.join(root, src)
    if not os.path.exists(path):
        return m.group(0)
    data = base64.b64encode(open(path, "rb").read()).decode()
    return 'src="data:image/png;base64,' + data + '"'

body = re.sub(r'src="([^"]+\.png)"', inline, body)
html = ('<!doctype html><html><head><meta charset="utf-8"><title>GFUNK 2027 - User Guide</title>'
        '<style>' + css + '</style></head><body>' + body + '</body></html>')
open(sys.argv[1], "w", encoding="utf-8").write(html)
print("ok", len(html))
