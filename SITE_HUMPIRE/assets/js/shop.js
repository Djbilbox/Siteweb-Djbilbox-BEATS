/* ============================================================
   SHOP — collections page (shop.html)
   Data: assets/js/shop-data.js (tools/build_shop_data.py)
   ============================================================ */
(function(){
  const SECTIONS = window.SHOP_SECTIONS || [];
  const P = window.SHOP_PRODUCTS || {};
  const $ = id => document.getElementById(id);
  const esc = s => String(s).replace(/[&<>"]/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
  const unesc = s => { const t = document.createElement('textarea'); t.innerHTML = s; return t.value; };
  const price = v => v > 0 ? '$' + (v % 1 ? v.toFixed(2) : v) : 'Free';

  function card(p){
    const flags = [p.play ? '<span title="Playable presets"><i class="fa-solid fa-keyboard"></i> Play</span>' : '',
                   p.video ? '<span title="Promo video"><i class="fa-brands fa-youtube"></i> Video</span>' : ''].join('');
    return `<a class="sh-card" href="p/${p.slug}.html" data-q="${esc((unesc(p.name) + ' ' + unesc(p.sub)).toLowerCase())}">
      <span class="sh-img" style="--cover:url('${p.img}')"><img src="${p.img}" alt="${esc(unesc(p.name))} — HUMPIRE" loading="lazy">
        ${p.new ? '<em class="sh-badge new">New</em>' : p.price <= 0 ? '<em class="sh-badge free">Free</em>' : ''}</span>
      <span class="sh-body"><b>${p.name}</b><small>${p.sub}</small>
        <span class="sh-foot"><span class="sh-price${p.price <= 0 ? ' free' : ''}">${price(p.price)}</span><span class="sh-flags">${flags}</span></span></span>
    </a>`;
  }

  function render(){
    $('shNav').innerHTML = SECTIONS.map(s => `<a href="#${s.id}">${s.title.replace(' & Generators', '')}</a>`).join('');
    $('shMain').innerHTML = SECTIONS.map(s => {
      const chips = s.groups.length > 1
        ? `<div class="sh-chips">${s.groups.map(g => `<a href="#${g.id}">${g.title.replace(/ —.*/, '')}</a>`).join('')}</div>` : '';
      const groups = s.groups.map(g => `
        <div class="sh-group" id="${g.id}">
          ${s.groups.length > 1 || g.text ? `<div class="sh-group-head"><h3>${g.title}</h3>${g.text ? `<p>${g.text}</p>` : ''}<span>${g.items.length}</span></div>` : ''}
          <div class="sh-grid">${g.items.map(k => card(P[k])).join('')}</div>
        </div>`).join('');
      return `<section class="sh-sec" id="${s.id}">
        <div class="sh-banner">
          <div class="sh-banner-txt"><p class="sh-kicker">${s.kicker}</p><h2>${s.title}</h2><p>${s.text}</p>${chips}</div>
          <div class="sh-banner-art"><img src="https://i.ytimg.com/vi/${s.banner}/maxresdefault.jpg" alt="" loading="lazy"></div>
        </div>${groups}
      </section>`;
    }).join('');
  }

  /* search: hides cards, then empty groups and sections */
  let t;
  $('shSearch').addEventListener('input', e => {
    clearTimeout(t);
    t = setTimeout(() => {
      const q = e.target.value.trim().toLowerCase();
      let any = false;
      document.querySelectorAll('.sh-card').forEach(c => { c.hidden = !!q && !c.dataset.q.includes(q); });
      document.querySelectorAll('.sh-group').forEach(g => { g.hidden = !g.querySelector('.sh-card:not([hidden])'); });
      document.querySelectorAll('.sh-sec').forEach(s => {
        const vis = !!s.querySelector('.sh-group:not([hidden])'); s.hidden = !vis; any = any || vis;
        s.querySelector('.sh-banner').hidden = !!q;
      });
      $('shEmpty').hidden = any;
    }, 120);
  });

  /* section menu: highlight + stay under the site header */
  function spy(){
    let cur = '';
    document.querySelectorAll('.sh-sec').forEach(s => { if (s.getBoundingClientRect().top < 200) cur = s.id; });
    document.querySelectorAll('.sh-nav a').forEach(a => a.classList.toggle('on', a.getAttribute('href') === '#' + cur));
  }
  function dock(){
    const head = document.querySelector('.topbar');
    if (head) $('shNav').style.top = Math.round(head.getBoundingClientRect().bottom) + 'px';
  }

  render();
  window.addEventListener('scroll', spy, { passive: true });
  window.addEventListener('load', () => { dock(); if (location.hash) { const el = document.querySelector(location.hash); el && el.scrollIntoView(); } });
  window.addEventListener('resize', dock); setTimeout(dock, 300);
})();
