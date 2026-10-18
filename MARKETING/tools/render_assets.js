// Usage: node render_assets.js <manifest.json> <outDir> [onlyId]
// Renders for each product: <id>-thumb.png (600x600) and <id>-cover.png (1280x720).
const fs = require('fs'), path = require('path');
const { chromium } = require('playwright');
const [,, manifestPath, outDir, only] = process.argv;
const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
const tpl = fs.readFileSync(path.join(__dirname, '..', 'templates', 'product.html'), 'utf8');
const esc = s => String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;');
(async () => {
  const browser = await chromium.launch({ executablePath: process.env.CHROMIUM || undefined });
  fs.mkdirSync(outDir, { recursive: true });
  for (const p of manifest.filter(p => !only || p.id === only)) {
    for (const [mode, w, h] of [['thumb', 600, 600], ['cover', 1280, 720]]) {
      const badges = (p.badges || []).map((b, i) => `<span class="badge${i === 0 ? ' hot' : ''}">${esc(b)}</span>`).join('');
      const logo = p.logo ? `<img src="file://${path.resolve(p.logo)}">` : '';
      const shot = 'file://' + path.resolve(p.screenshot);
      const html = tpl.replace('{{MODE}}', mode).replace('{{ACCENT}}', p.accent || '#f5b544')
        .replace('{{LOGO}}', logo).replace('{{BRAND}}', esc(p.brand || 'DJBILBOX BEATS')).replace('{{KIND}}', esc(p.kind || '')).replace('{{FS}}', (mode === 'thumb' ? p.fsThumb : p.fsCover) || (mode === 'thumb' ? '64px' : '92px'))
        .replace('{{NAME}}', esc(p.name)).replace('{{TAGLINE}}', esc(p.tagline || '')).replace('{{SHOT}}', shot)
        .replace('{{BADGES}}', badges).replace('{{FOOT}}', esc(p.foot || 'VST3 · AU · Windows & macOS · Royalty-free license'));
      const page = await browser.newPage({ viewport: { width: w, height: h }, deviceScaleFactor: 1 });
      const tmp = path.join(outDir, `.tmp-${p.id}-${mode}.html`);
      fs.writeFileSync(tmp, html);
      await page.goto('file://' + path.resolve(tmp));
      await page.waitForTimeout(250);
      // fit the title: one line on the thumbnail, at most ~240 px of height on the cover, never wider than its box;
      // then put the tagline under the title and keep it clear of the badges
      await page.evaluate((m) => {
        const h1 = document.querySelector('h1'), tag = document.querySelector('.tag'), badges = document.querySelector('.badges');
        let fs = parseFloat(getComputedStyle(h1).fontSize);
        const tooWide = () => h1.scrollWidth > h1.clientWidth + 1;
        if (m === 'thumb') {
          while (tooWide() && fs > 20) { fs -= 1; h1.style.setProperty('font-size', fs + 'px', 'important'); }
        } else {
          while ((tooWide() || h1.offsetHeight > 240) && fs > 34) { fs -= 2; h1.style.setProperty('font-size', fs + 'px', 'important'); }
          tag.style.top = (h1.offsetTop + h1.offsetHeight + 22) + 'px';
          let tf = parseFloat(getComputedStyle(tag).fontSize);
          while (tag.getBoundingClientRect().bottom > badges.getBoundingClientRect().top - 14 && tf > 15) { tf -= 1; tag.style.fontSize = tf + 'px'; }
        }
      }, mode);
      await page.screenshot({ path: path.join(outDir, `${p.id}-${mode}.png`), clip: { x: 0, y: 0, width: w, height: h } });
      await page.screenshot({ path: path.join(outDir, `${p.id}-${mode}.jpg`), type: 'jpeg', quality: 90, clip: { x: 0, y: 0, width: w, height: h } });
      await page.close(); fs.unlinkSync(tmp);
    }
    console.log('rendered', p.id);
  }
  await browser.close();
})();
