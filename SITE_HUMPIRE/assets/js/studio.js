/* HUMPIRE RECORDS — studio page motion
   hero clip rotation, intro, scroll reveals, counters, tilt + glow, floating CTA */
(function () {
  const reduce = matchMedia('(prefers-reduced-motion: reduce)').matches;
  const $ = (s, r = document) => r.querySelector(s), $$ = (s, r = document) => [...r.querySelectorAll(s)];

  /* intro: neon power-on, letters drop in */
  document.documentElement.classList.add('hr-js');
  requestAnimationFrame(() => requestAnimationFrame(() => document.documentElement.classList.add('hr-ready')));

  /* hero clips: crossfade to the next clip when one ends */
  const clips = $$('.hr-hero-bg video');
  if (clips.length > 1 && !reduce) {
    let i = 0;
    clips.forEach((v, k) => { v.loop = false; v.classList.toggle('on', k === 0); if (k) v.pause(); });
    const next = () => {
      const cur = clips[i]; i = (i + 1) % clips.length; const nv = clips[i];
      nv.currentTime = 0; nv.play().catch(() => {});
      nv.classList.add('on'); cur.classList.remove('on');
      setTimeout(() => cur.pause(), 1400);
    };
    clips.forEach(v => v.addEventListener('ended', next));
    clips[0].play().catch(() => {});
  }

  /* hero spotlight follows the pointer */
  const hero = $('.hr-hero');
  if (hero && !reduce) hero.addEventListener('pointermove', e => {
    const r = hero.getBoundingClientRect();
    hero.style.setProperty('--mx', ((e.clientX - r.left) / r.width * 100) + '%');
    hero.style.setProperty('--my', ((e.clientY - r.top) / r.height * 100) + '%');
  });

  /* scroll reveals (staggered inside each group) */
  const groups = ['.hr-pillars > div', '.hr-stats > div', '.hr-gallery figure', '.hr-pack', '.hr-row', '.hr-steps li', '.hr-faq details', '.hr-sec > h2', '.hr-sec > .hr-eyebrow', '.hr-sec > .hr-sub', '.hr-book'];
  groups.forEach(sel => $$(sel).forEach((el, k) => { el.classList.add('rv'); el.style.setProperty('--d', (k % 6) * 90 + 'ms'); }));
  const io = new IntersectionObserver(es => es.forEach(e => {
    if (e.isIntersecting) { e.target.classList.add('in'); io.unobserve(e.target); }
  }), { threshold: .15, rootMargin: '0px 0px -40px 0px' });
  $$('.rv').forEach(el => reduce ? el.classList.add('in') : io.observe(el));

  /* counters */
  const fmt = (n, d) => d ? n.toFixed(d) : Math.round(n).toLocaleString(document.documentElement.lang || 'en');
  const cio = new IntersectionObserver(es => es.forEach(e => {
    if (!e.isIntersecting) return; cio.unobserve(e.target);
    const el = e.target, to = +el.dataset.to, d = +(el.dataset.dec || 0), t0 = performance.now(), dur = 1600;
    const step = t => { const p = Math.min(1, (t - t0) / dur), q = 1 - Math.pow(1 - p, 3);
      el.textContent = fmt(to * q, d); if (p < 1) requestAnimationFrame(step); };
    reduce ? (el.textContent = fmt(to, d)) : requestAnimationFrame(step);
  }), { threshold: .6 });
  $$('[data-to]').forEach(el => cio.observe(el));

  /* tilt + neon glow under the pointer on cards */
  if (!reduce && matchMedia('(hover: hover)').matches) {
    $$('.hr-pack, .hr-gallery figure, .hr-pillars > div').forEach(c => {
      c.classList.add('tilt');
      c.addEventListener('pointermove', e => {
        const r = c.getBoundingClientRect(), x = (e.clientX - r.left) / r.width, y = (e.clientY - r.top) / r.height;
        c.style.setProperty('--gx', x * 100 + '%'); c.style.setProperty('--gy', y * 100 + '%');
        c.style.transform = `perspective(900px) rotateX(${(.5 - y) * 6}deg) rotateY(${(x - .5) * 8}deg) translateY(-4px)`;
      });
      c.addEventListener('pointerleave', () => { c.style.transform = ''; });
    });
  }

  /* floating book button: shows after the hero, hides on the booking block */
  const fl = $('.hr-float'), book = $('#book');
  if (fl) {
    const upd = () => {
      const past = scrollY > innerHeight * .6;
      const r = book && book.getBoundingClientRect(), onBook = r && r.top < innerHeight && r.bottom > 0;
      fl.classList.toggle('show', past && !onBook);
    };
    addEventListener('scroll', upd, { passive: true }); upd();
  }

  /* currency: prices are set in euros, shown converted (approximate, payment stays in EUR) */
  const RATES = { EUR: 1, USD: 1.08, GBP: 0.85, CHF: 0.94, CAD: 1.50, BRL: 6.0, MXN: 19.5, MAD: 10.8, DZD: 145, TND: 3.4,
                  XOF: 655.96, SAR: 4.05, AED: 3.97, EGP: 52, TRY: 37, RUB: 100, JPY: 165 };
  const REGION = { US: 'USD', GB: 'GBP', CH: 'CHF', CA: 'CAD', BR: 'BRL', MX: 'MXN', MA: 'MAD', DZ: 'DZD', TN: 'TND', SN: 'XOF',
                   CI: 'XOF', ML: 'XOF', SA: 'SAR', AE: 'AED', EG: 'EGP', TR: 'TRY', RU: 'RUB', JP: 'JPY' };
  const BYLANG = { en: 'USD', fr: 'EUR', es: 'EUR', pt: 'BRL', de: 'EUR', ar: 'SAR', ru: 'RUB', ja: 'JPY' };
  const sel = $('#hrCur'), note = $('#hrCurNote');
  if (sel) {
    const lang = (document.documentElement.lang || 'en').slice(0, 2);
    /* the country counts only when the browser language matches the page (fr-CA → CAD, ar-DZ → DZD) */
    const region = (navigator.languages || [navigator.language]).filter(l => l.slice(0, 2).toLowerCase() === lang)
      .map(l => (l.split('-')[1] || '').toUpperCase()).find(r => REGION[r]);
    let cur; try { cur = localStorage.getItem('hp_cur'); } catch (e) {}
    if (!RATES[cur]) cur = region ? REGION[region] : (BYLANG[lang] || 'EUR');
    sel.innerHTML = Object.keys(RATES).map(c => `<option value="${c}"${c === cur ? ' selected' : ''}>${c}</option>`).join('');
    const fmt = (v, c) => { try { return new Intl.NumberFormat(document.documentElement.lang || 'en', { style: 'currency', currency: c, maximumFractionDigits: 0 }).format(v); } catch (e) { return v + ' ' + c; } };
    const nice = (v, c) => { if (c === 'EUR') return v; const s = v >= 10000 ? 500 : v >= 1000 ? 50 : v >= 100 ? 5 : 1; return Math.round(v / s) * s; };
    const apply = c => {
      $$('.hr-price[data-eur]').forEach(el => {
        const t = fmt(nice(+el.dataset.eur * RATES[c], c), c);
        el.querySelector('b').textContent = t.replace(/[^\d\s.,  ]/g, '').trim();
        el.querySelector('span').textContent = t.replace(/[\d\s.,  ]/g, '').trim() || c;
      });
      $$('.eur[data-eur]').forEach(el => { el.textContent = fmt(nice(+el.dataset.eur * RATES[c], c), c); });
      if (note) note.hidden = c === 'EUR';
      try { localStorage.setItem('hp_cur', c); } catch (e) {}
    };
    sel.addEventListener('change', () => apply(sel.value));
    apply(cur);
  }
})();
