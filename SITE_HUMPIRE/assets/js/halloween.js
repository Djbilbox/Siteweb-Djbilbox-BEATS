/* ============================================================
   HUMPIRE — HALLOWEEN 2026 theme (decorative layers + promo banner)
   The sale itself is the real Gumroad code HUMPIRE30 (-30 %, until
   2026-10-24 23:59 Paris) handled by promo.js — nothing is invented here.
   The banner reads Promo.UNTIL / Promo.CODE / Promo.PERCENT.
   ============================================================ */
(function () {
  'use strict';
  const PUMPKIN = '<svg viewBox="0 0 100 90" xmlns="http://www.w3.org/2000/svg" aria-hidden="true">' +
    '<defs><radialGradient id="pg" cx="50%" cy="45%" r="60%"><stop offset="0" stop-color="#ffb347"/><stop offset=".6" stop-color="#ff7a00"/><stop offset="1" stop-color="#b53a00"/></radialGradient></defs>' +
    '<path d="M46 10c2-6 8-8 12-6-2 3-3 6-3 10z" fill="#3a8a2a"/>' +
    '<ellipse cx="50" cy="52" rx="44" ry="36" fill="url(#pg)"/>' +
    '<path d="M50 16c-12 6-18 20-18 36s6 30 18 36M50 16c12 6 18 20 18 36S62 82 50 88" fill="none" stroke="#a83200" stroke-width="2" opacity=".6"/>' +
    '<g class="glow" fill="#fff3a0"><path d="M26 44l16 6-18 6z"/><path d="M74 44l-16 6 18 6z"/><path d="M28 66q22 18 44 0l-6-4-6 6-6-6-6 6-6-6-6 6-6-6z"/></g></svg>';

  const ZOMBIE = '<svg viewBox="0 0 80 130" xmlns="http://www.w3.org/2000/svg" aria-hidden="true"><g fill="#0b0618" stroke="#2a1a55" stroke-width="1.5">' +
    '<circle cx="40" cy="20" r="14"/><path d="M26 36c-10 6-22 18-24 30l8 4c4-8 10-14 16-18l-2 38-10 22h14l6-20 6 20h14l-10-22-2-38c8 4 14 10 18 18l8-4c-2-12-14-24-24-30z"/></g>' +
    '<circle cx="35" cy="19" r="2.6" fill="#7dff3a"/><circle cx="46" cy="19" r="2.6" fill="#7dff3a"/></svg>';

  const TRAIN = '<svg viewBox="0 0 1180 150" xmlns="http://www.w3.org/2000/svg" aria-hidden="true">' +
    '<g fill="#0c0820" stroke="#7fffc4" stroke-width="1.4" opacity=".95">' +
    '<path d="M20 120V62h60l22-26h70v26h40v58z"/><rect x="60" y="20" width="26" height="42"/><path d="M52 20h42l-6-14H58z"/>' +
    '<rect x="270" y="52" width="270" height="68" rx="6"/><rect x="560" y="52" width="270" height="68" rx="6"/><rect x="850" y="52" width="270" height="68" rx="6"/></g>' +
    '<g class="win" fill="#9dffd0">' + [290, 350, 410, 470, 580, 640, 700, 760, 870, 930, 990, 1050].map(function (x, i) { return '<rect x="' + x + '" y="66" width="34" height="26" rx="3" opacity="' + (i % 3 === 0 ? .35 : .9) + '"/>'; }).join('') + '</g>' +
    '<g fill="#0c0820" stroke="#7fffc4" stroke-width="1.4"><circle cx="70" cy="128" r="18"/><circle cx="130" cy="128" r="18"/><circle cx="190" cy="128" r="18"/>' +
    [300, 480, 600, 780, 890, 1070].map(function (x) { return '<circle cx="' + x + '" cy="128" r="15"/>'; }).join('') + '</g>' +
    '<rect x="0" y="144" width="1180" height="3" fill="#7fffc4" opacity=".35"/></svg>';

  function el(tag, cls, html) { const e = document.createElement(tag); if (cls) e.className = cls; if (html) e.innerHTML = html; return e; }

  function buildLayers() {
    const b = document.body;
    b.appendChild(el('div', 'hw-layer hw-fog'));
    b.appendChild(el('div', 'hw-layer hw-flash'));
    ['c1', 'c2', 'c3', 'c4', 'c5', 'c6'].forEach(function (c) { b.appendChild(el('div', 'hw-crystal ' + c)); });
    ['p1', 'p2', 'p3'].forEach(function (p) { b.appendChild(el('div', 'hw-pumpkin ' + p, PUMPKIN)); });
    const train = el('div', 'hw-train', TRAIN);
    ['', 's2', 's3'].forEach(function (s) { train.appendChild(el('i', 'hw-smoke ' + s)); });
    b.appendChild(train);
    const kinds = ['', 'o', 'v'];
    for (let i = 0; i < 18; i++) {
      const s = el('i', 'hw-spore ' + kinds[i % 3]);
      s.style.left = (Math.random() * 100) + 'vw';
      s.style.animationDuration = (9 + Math.random() * 12) + 's';
      s.style.animationDelay = (-Math.random() * 18) + 's';
      b.appendChild(s);
    }
    const foot = document.querySelector('footer.footer');
    if (foot) { const z = el('div', 'hw-zombies'); for (let i = 0; i < 9; i++) z.insertAdjacentHTML('beforeend', ZOMBIE); foot.prepend(z); }
  }

  function buildBanner() {
    const P = window.Promo || {};
    const until = P.UNTIL || Date.parse('2026-10-24T23:59:00+02:00');
    const pct = P.PERCENT || 30, code = P.CODE || 'HUMPIRE30';
    const box = el('section', 'hw-banner');
    box.id = 'halloween';
    box.innerHTML =
      '<div class="hw-kicker">Dark Aether · Ghost Train · Halloween 2026</div>' +
      '<div class="hw-title">HALLOWEEN <b>SALE</b></div>' +
      '<p class="hw-sub"><strong>' + pct + '% OFF</strong> on plugins, guitar pedals, bundles and sample packs. ' +
      'Applied automatically at checkout on every eligible product — <strong>offer ends October 24</strong>.</p>' +
      '<div class="hw-count" id="hwCount"><div><span id="hwD">--</span><small>days</small></div><div><span id="hwH">--</span><small>hours</small></div><div><span id="hwM">--</span><small>min</small></div><div><span id="hwS">--</span><small>sec</small></div></div><br>' +
      '<a class="hw-cta" href="#homeVstGrid">🎃 Shop the sale</a><a class="hw-cta alt" href="#homeVstGrid">💀 New plugins</a><br>' +
      '<span class="hw-code">CODE ' + code + ' · AUTO-APPLIED</span>';
    const anchor = document.querySelector('section.hero2') || document.querySelector('main') || document.body.firstChild;
    anchor.parentNode.insertBefore(box, anchor);
    const d = box.querySelector('#hwD'), h = box.querySelector('#hwH'), m = box.querySelector('#hwM'), s = box.querySelector('#hwS');
    function tick() {
      let t = Math.max(0, until - Date.now());
      if (t === 0) { box.querySelector('.hw-count').innerHTML = '<div><span>0</span><small>sale ended</small></div>'; return; }
      const pad = function (n) { return String(n).padStart(2, '0'); };
      d.textContent = Math.floor(t / 864e5); h.textContent = pad(Math.floor(t / 36e5) % 24);
      m.textContent = pad(Math.floor(t / 6e4) % 60); s.textContent = pad(Math.floor(t / 1e3) % 60);
    }
    tick(); setInterval(tick, 1000);
  }

  function init() { document.documentElement.classList.add('halloween'); buildBanner(); buildLayers(); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init); else init();
})();

