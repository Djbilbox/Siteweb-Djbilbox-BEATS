/* ============================================================
   SHOP — one flat grid, newest release first (shop.html)
   Data: assets/js/shop-data.js (tools/build_shop_data.py)
   Order: Gumroad release order (assets/js/release-order.js), newest first;
          a product missing from that list goes last.
   ============================================================ */
(function(){
  const MODE = window.SHOP_MODE || 'plugins';          // 'plugins' or 'kits'
  const P = Object.values(window.SHOP_PRODUCTS || {}).filter(p => MODE === 'kits' ? p.kind === 'kit' : p.kind !== 'kit');
  const $ = id => document.getElementById(id);
  const esc = s => String(s).replace(/[&<>"]/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
  const unesc = s => { const t = document.createElement('textarea'); t.innerHTML = s; return t.value; };
  const price = v => v > 0 ? '$' + (v % 1 ? v.toFixed(2) : v) : 'Free';

  const rankOf = (() => {
    const map = {};
    (window.RELEASE_ORDER || []).forEach(([permalink, slug], i) => { map[permalink] = map[slug] = i; });
    return p => map[p.buy] ?? 1e6;
  })();
  P.sort((a, b) => rankOf(a) - rankOf(b) || a.name.localeCompare(b.name));

  const FILTERS = MODE === 'kits'
    ? [['all', 'All'], ['kits-westcoast', 'G-Funk & West Coast'], ['kits-trap', 'Hip-Hop, Trap & Drill'], ['kits-horror', 'Horror'], ['kits-world', 'Afro, House & Oriental'], ['free', 'Free']]
    : [['all', 'All'], ['synth', 'Synths'], ['machine', 'Drum machines'], ['effect', 'Effects'], ['bundle', 'Bundles'], ['free', 'Free']];
  const matches = (p, f) => f === 'all' || (f === 'free' ? p.price <= 0 : (MODE === 'kits' ? p.group === f : p.cat === f));
  let filter = 'all', query = '';

  function card(p){
    const flags = [p.play ? '<span title="Playable presets"><i class="fa-solid fa-keyboard"></i> Play</span>' : '',
                   p.video ? '<span title="Promo video"><i class="fa-brands fa-youtube"></i> Video</span>' : ''].join('');
    return `<a class="sh-card" href="p/${p.slug}.html">
      <span class="sh-img${p.face ? ' face' : ''}" style="--cover:url('${p.img}')"><img src="${p.img}" alt="${esc(unesc(p.name))} — HUMPIRE" loading="lazy">
        ${p.new ? '<em class="sh-badge new">New</em>' : p.price <= 0 ? '<em class="sh-badge free">Free</em>' : ''}</span>
      <span class="sh-body"><b>${p.name}</b><small>${p.sub}</small>
        <span class="sh-foot"><span class="sh-price${p.price <= 0 ? ' free' : ''}">${price(p.price)}</span><span class="sh-flags">${flags}</span></span></span>
    </a>`;
  }

  function render(){
    const q = query.toLowerCase();
    const list = P.filter(p => matches(p, filter) &&
      (!q || (unesc(p.name) + ' ' + unesc(p.sub)).toLowerCase().includes(q)));
    $('shGrid').innerHTML = list.map(card).join('');
    $('shEmpty').hidden = list.length > 0;
    $('shCount').textContent = list.length + (list.length > 1 ? ' products' : ' product') + ' · newest first';
    $('shFilters').innerHTML = FILTERS.map(([k, l]) => `<button type="button" class="sh-chip${k === filter ? ' on' : ''}" data-f="${k}">${l}</button>`).join('');
  }

  const fromHash = location.hash.slice(1);
  if (FILTERS.some(f => f[0] === fromHash)) filter = fromHash;
  document.addEventListener('click', e => {
    const b = e.target.closest('[data-f]');
    if (b) { filter = b.dataset.f; render(); }
  });
  let t;
  $('shSearch').addEventListener('input', e => { clearTimeout(t); t = setTimeout(() => { query = e.target.value.trim(); render(); }, 120); });

  render();
})();
