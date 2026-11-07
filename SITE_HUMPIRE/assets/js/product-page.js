/* ============================================================
   PRODUCT PAGE (p/*.html) — promo price block, code copy,
   YouTube click-to-play, mobile buy bar.
   ============================================================ */
(function(){
  const P = window.HUMPIRE_PROMO || {};
  const live = P.active && P.code && P.percent > 0 && (!P.ends || Date.now() < Date.parse(P.ends));
  const money = v => '$' + (Math.round(v * 100) / 100).toFixed(v % 1 ? 2 : 0);

  function withCode(href){
    const m = href.match(/^(https:\/\/[^/]*gumroad\.com\/l\/[^/?#]+)/);
    return m ? m[1] + '/' + encodeURIComponent(P.code) : href;
  }

  function countdown(el){
    if (!P.ends) return;
    const tick = () => {
      const s = Math.max(0, Math.floor((Date.parse(P.ends) - Date.now()) / 1000));
      const d = Math.floor(s / 86400), h = Math.floor(s % 86400 / 3600), m = Math.floor(s % 3600 / 60);
      el.textContent = 'Ends in ' + (d ? d + 'd ' : '') + h + 'h ' + String(m).padStart(2, '0') + 'm';
    };
    tick(); setInterval(tick, 30000);
  }

  function promo(){
    const box = document.querySelector('.pp-buy');
    if (!box || !live) return;
    const price = parseFloat(box.dataset.price || '0');
    if (!(price > 0)) return;
    const now = price * (1 - P.percent / 100), save = price - now;

    box.querySelector('.pp-price').innerHTML =
      '<span class="pp-now">' + money(now) + '</span><s class="pp-was">' + money(price) + '</s>' +
      '<span class="pp-save">Save ' + money(save) + ' · ' + P.percent + '% off</span>';

    const code = document.createElement('div');
    code.className = 'pp-code';
    code.innerHTML =
      '<div class="pp-code-l"><span class="pp-code-k">' + (P.title || 'Sale') + ' — use this code</span>' +
      '<b class="pp-code-v">' + P.code + '</b><span class="pp-code-t"></span></div>' +
      '<button type="button" class="pp-copy">Copy code</button>';
    box.querySelector('.pp-price').after(code);
    countdown(code.querySelector('.pp-code-t'));
    code.querySelector('.pp-copy').addEventListener('click', e => {
      const b = e.currentTarget;
      (navigator.clipboard ? navigator.clipboard.writeText(P.code) : Promise.reject()).then(
        () => { b.textContent = 'Copied ✓'; setTimeout(() => b.textContent = 'Copy code', 1800); },
        () => { b.textContent = P.code; });
    });

    document.querySelectorAll('a.pp-cta, a.pp-bar-cta').forEach(a => { a.href = withCode(a.href); });
    document.querySelectorAll('.pp-bar-price').forEach(s => { s.innerHTML = money(now) + ' <s>' + money(price) + '</s>'; });
    const note = document.querySelector('.pp-cta-note');
    if (note) note.textContent = 'Code ' + P.code + ' is applied automatically at checkout.';
  }

  function videos(){
    document.querySelectorAll('.pp-yt').forEach(btn => btn.addEventListener('click', () => {
      const id = btn.dataset.yt;
      const src = id.startsWith('PL')
        ? 'https://www.youtube-nocookie.com/embed/videoseries?list=' + id + '&autoplay=1&rel=0'
        : 'https://www.youtube-nocookie.com/embed/' + id + '?autoplay=1&rel=0';
      const f = document.createElement('iframe');
      f.src = src; f.title = btn.getAttribute('aria-label') || 'Video';
      f.allow = 'autoplay; encrypted-media; picture-in-picture; fullscreen'; f.allowFullscreen = true;
      btn.replaceWith(f);
    }));
  }

  /* mobile buy bar: shown once the main button has scrolled away */
  function bar(){
    const b = document.querySelector('.pp-bar'), cta = document.querySelector('.pp-buy .pp-cta');
    if (!b || !cta || !('IntersectionObserver' in window)) return;
    new IntersectionObserver(([e]) => b.classList.toggle('show', !e.isIntersecting && e.boundingClientRect.top < 0))
      .observe(cta);
  }

  document.addEventListener('DOMContentLoaded', () => { promo(); videos(); bar(); });
})();
