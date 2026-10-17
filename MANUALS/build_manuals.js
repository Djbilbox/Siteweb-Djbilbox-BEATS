// Builds the PDF manuals from MANUALS/src/*.md (pandoc -> HTML -> Chromium PDF).
//   NODE_PATH=/node-tools/node_modules node MANUALS/build_manuals.js
// Output : dist/products/<slug>.pdf  (one per product, slug = file name with "_" -> "-")
//          dist/DJBILBOX_BEATS_Complete_Manual.pdf  (introduction + every product)
const fs = require('fs'), path = require('path'), { execFileSync } = require('child_process');
const { chromium } = require('playwright');

const root = __dirname;
const src = path.join(root, 'src');
const dist = path.join(root, 'dist');
const tmp = path.join(dist, '.tmp');
fs.mkdirSync(path.join(dist, 'products'), { recursive: true });
fs.mkdirSync(tmp, { recursive: true });

const esc = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;');
const files = fs.readdirSync(src).filter(f => f.endsWith('.md')).sort();
const intro = files.find(f => f.startsWith('00_'));
const products = files.filter(f => f !== intro);

function titleOf(md) { const m = md.match(/^#\s+(.+)$/m); return m ? m[1].trim() : 'Manual'; }
function leadOf(md) {
  const lines = md.split('\n'); let i = lines.findIndex(l => /^#\s/.test(l)) + 1;
  while (i < lines.length && !lines[i].trim()) i++;
  return /^[A-Za-z]/.test(lines[i] || '') ? lines[i].trim().replace(/[*_`]/g, '') : '';
}

function html(md, title, subtitle, meta) {
  const mdFile = path.join(tmp, 'in.md');
  fs.writeFileSync(mdFile, md);
  const out = execFileSync('pandoc', [mdFile, '-f', 'markdown+pipe_tables+smart', '-t', 'html5', '-s', '--toc', '--toc-depth=2',
    '--metadata', 'pagetitle=' + title, '--metadata', 'lang=en', '-c', path.join(root, 'manual.css')], { encoding: 'utf8', maxBuffer: 1 << 28 });
  const cover = `<div class="cover"><div class="brand">DJBILBOX BEATS / HUMPIRE</div><div><div class="title">${esc(title)}</div>` +
    `<div class="subtitle">${esc(subtitle)}</div><div class="bar"></div></div><div class="meta">${esc(meta)}</div></div>`;
  return out.replace(/<body[^>]*>/, m => m + cover);
}

async function pdf(browser, htmlStr, outFile) {
  const f = path.join(tmp, 'doc.html');
  fs.writeFileSync(f, htmlStr);
  const page = await browser.newPage();
  await page.goto('file://' + f);
  await page.pdf({
    path: outFile, format: 'A4', printBackground: true,
    margin: { top: '18mm', bottom: '20mm', left: '16mm', right: '16mm' },
    displayHeaderFooter: true, headerTemplate: '<span></span>',
    footerTemplate: '<div style="width:100%;font-size:8px;color:#8d8b86;padding:0 16mm;display:flex;justify-content:space-between"><span>DJBILBOX BEATS / HUMPIRE - djbilboxbeats.com</span><span><span class="pageNumber"></span> / <span class="totalPages"></span></span></div>',
  });
  await page.close();
  console.log('wrote', path.relative(root, outFile));
}

(async () => {
  const browser = await chromium.launch({ executablePath: process.env.CHROMIUM || undefined });
  const version = process.env.MANUAL_VERSION || '1.0';
  const meta = `Version ${version} - Windows (VST3) and macOS (VST3 / Audio Unit) - Royalty-free license`;
  const bodies = [];
  for (const f of products) {
    const md = fs.readFileSync(path.join(src, f), 'utf8');
    const slug = f.replace(/\.md$/, '').replace(/_/g, '-');
    await pdf(browser, html(md, titleOf(md), leadOf(md), meta), path.join(dist, 'products', slug + '.pdf'));
    bodies.push(md);
  }
  const introMd = intro ? fs.readFileSync(path.join(src, intro), 'utf8') : '';
  const all = [introMd, ...bodies].join('\n\n');
  await pdf(browser, html(all, 'Complete User Manual', 'Installation, license and detailed guide for every plugin', meta),
    path.join(dist, 'DJBILBOX_BEATS_Complete_Manual.pdf'));
  await browser.close();
  fs.rmSync(tmp, { recursive: true, force: true });
})().catch(e => { console.error(e); process.exit(1); });
