/* ============================================================
   STREETWEAR — collections by season (streetwear.html)
   Data: assets/js/streetwear-data.js (tools/build_streetwear_data.py)
   ============================================================ */
(function(){
  const ITEMS = window.SW_ITEMS || [];
  const COLLECTIONS = [
    { k:'summer', title:'Summer 26', sub:'Oversized tees, caps & bucket hats', banner:'JO8DL2QH8Qk' },
    { k:'autumn', title:'Autumn 26', sub:'Hoodies & joggers for the street', banner:'t3cTSSakMXY' },
    { k:'winter', title:'Winter 26', sub:'Built for the cold — first pieces out now', banner:'tb2YhcoYA0g', more:'More winter pieces drop soon.' },
    { k:'goodies', title:'Goodies', sub:'Mug & tote for the studio', banner:null }
  ];
  const TYPE_LABEL = { tee:'T-shirts', hoodie:'Hoodies', pull:'Pulls', bottom:'Joggers', cap:'Caps & hats', neck:'Balaclavas', goodies:'Goodies' };
  const $ = id => document.getElementById(id);
  const esc = s => String(s).replace(/[&<>"]/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
  const euro = v => v.toFixed(2).replace('.', ',') + ' €';
  const shortName = n => n.replace(/^HUMPIRE( CORPORATION)?\s*[—-]\s*/i, '').replace(/^HUMPIRE\s+/i, '');

  function card(i){
    return `<article class="sw2-card">
      <a class="sw2-img" href="${i.url}" target="_blank" rel="noopener">
        <img src="${i.img}" alt="${esc(i.name)} — ${i.brand} streetwear" loading="lazy">
        ${i.video ? `<button type="button" class="sw2-play" data-video="${i.slug}" aria-label="Watch the video of ${esc(i.name)}"><i class="fa-solid fa-play"></i> Video</button>` : ''}
      </a>
      <p class="sw2-brand">${esc(i.brand)}</p>
      <h3><a href="${i.url}" target="_blank" rel="noopener">${esc(shortName(i.name))}</a></h3>
      <p class="sw2-price">${euro(i.price)}</p>
    </article>`;
  }

  function collections(){
    $('sw2Main').innerHTML = COLLECTIONS.map(c => {
      const items = ITEMS.filter(i => i.season === c.k);
      if (!items.length) return '';
      const banner = c.banner ? `
        <header class="sw2-banner" style="background-image:url('https://i.ytimg.com/vi/${c.banner}/maxresdefault.jpg')">
          <div class="sw2-banner-txt"><h2>${c.title}</h2><p>${c.sub}</p>
            <div class="sw2-banner-btns"><a class="sw2-pill" href="#${c.k}-grid">Discover</a>
            <button type="button" class="sw2-pill ghost" data-yt="${c.banner}"><i class="fa-solid fa-play"></i> Watch the film</button></div></div>
        </header>` : `<div class="sw2-head plain"><h2>${c.title}</h2><p>${c.sub}</p></div>`;
      return `<section class="sw2-col" id="${c.k}">${banner}
        <div class="sw2-grid" id="${c.k}-grid">${items.map(card).join('')}</div>
        ${c.more ? `<p class="sw2-more">${c.more}</p>` : ''}</section>`;
    }).join('');
  }

  let filter = 'all';
  function all(){
    const types = [...new Set(ITEMS.map(i => i.type))];
    const brands = [...new Set(ITEMS.map(i => i.brand))];
    $('sw2Filters').innerHTML = [['all', 'All'], ...types.map(t => [t, TYPE_LABEL[t] || t]), ...brands.filter(b => b !== 'HUMPIRE').map(b => ['b:' + b, b])]
      .map(([k, l]) => `<button type="button" class="sw2-chip${k === filter ? ' on' : ''}" data-f="${esc(k)}">${esc(l)}</button>`).join('');
    const list = ITEMS.filter(i => filter === 'all' || i.type === filter || 'b:' + i.brand === filter);
    $('sw2All').innerHTML = list.map(card).join('');
  }

  function openVideo(id, name, url){
    $('sw2ModalVideo').innerHTML = `<iframe src="https://www.youtube-nocookie.com/embed/${id}?autoplay=1&rel=0" title="${esc(name)}" allow="autoplay; encrypted-media; picture-in-picture; fullscreen" allowfullscreen></iframe>`;
    $('sw2ModalName').textContent = name;
    $('sw2ModalBuy').href = url; $('sw2ModalBuy').hidden = !url;
    $('sw2Modal').hidden = false;
  }
  function closeVideo(){ $('sw2Modal').hidden = true; $('sw2ModalVideo').innerHTML = ''; }

  document.addEventListener('click', e => {
    const t = e.target.closest('[data-video],[data-yt],[data-f],[data-close]');
    if (!t) { if (e.target.id === 'sw2Modal') closeVideo(); return; }
    if (t.dataset.video) {
      e.preventDefault();
      const i = ITEMS.find(x => x.slug === t.dataset.video);
      openVideo(i.video, i.name, i.url);
    } else if (t.dataset.yt) openVideo(t.dataset.yt, 'HUMPIRE Streetwear', 'https://djbilbox-beats-shop.fourthwall.com/en-eur');
    else if (t.dataset.f) { filter = t.dataset.f; all(); }
    else closeVideo();
  });
  document.addEventListener('keydown', e => { if (e.key === 'Escape') closeVideo(); });

  /* highlight the collection in view */
  const links = [...document.querySelectorAll('.sw2-nav a[href^="#"]')];
  function spy(){
    let cur = '';
    links.forEach(a => { const s = document.querySelector(a.getAttribute('href')); if (s && s.getBoundingClientRect().top < 160) cur = a.getAttribute('href'); });
    links.forEach(a => a.classList.toggle('on', a.getAttribute('href') === cur));
  }

  /* keep the collection menu right under the site header (its height varies) */
  function dock(){
    const head = document.querySelector('.topbar');
    if (head) $('sw2Nav').style.top = Math.round(head.getBoundingClientRect().bottom) + 'px';
  }

  collections(); all();
  window.addEventListener('scroll', spy, { passive: true });
  window.addEventListener('load', dock); window.addEventListener('resize', dock); setTimeout(dock, 300);
})();
