/* ============================================================
   BEATS — catalogue + YouTube player bar (beats.html)
   Plays the videos of the "All Beats" playlist through the YouTube
   IFrame API, with the video visible, so every listen is a real
   YouTube view. Data: assets/js/beats-data.js.
   ============================================================ */
(function(){
  /* Checkout links: paste each Gumroad product URL here when it exists.
     Empty = the button opens an email with the beat and licence filled in. */
  const CHECKOUT = { mp3: '', wav: '', unlimited: '', exclusive: '' };
  const EMAIL = 'djbilboxbeats@gmail.com';
  const FREE_PACK = 'https://djbilboxbeats.gumroad.com/l/djbilbox-beats-big-pack-931-beats';
  const LICENSES = [
    { k:'mp3', name:'MP3 Lease', price:25, files:'MP3 320 kbps',
      rights:['Up to 100,000 streams','1 music video','Live shows','Credit: Prod. by DJBILBOX BEATS'] },
    { k:'wav', name:'WAV Lease', price:45, files:'WAV + MP3', best:true,
      rights:['Up to 500,000 streams','Radio broadcasting','EP / mixtape release','Credit: Prod. by DJBILBOX BEATS'] },
    { k:'unlimited', name:'Unlimited', price:99, files:'WAV + MP3 + stems',
      rights:['Unlimited streams & sales','Full monetisation','Unlimited videos','Credit: Prod. by DJBILBOX BEATS'] },
    { k:'exclusive', name:'Exclusive', price:null, files:'All files + ownership transfer',
      rights:['The beat is removed from sale','100% yours to exploit','Contract negotiated together','Make an offer'] }
  ];
  const PAGE = 48;
  const BEATS = (window.BEATS || []).map((b, i) => ({ ...b, n: i }));
  const $ = id => document.getElementById(id);
  const img = (id, q) => 'https://i.ytimg.com/vi/' + id + '/' + (q || 'hqdefault') + '.jpg';
  const esc = s => String(s).replace(/[&<>"]/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
  const fmt = s => { s = Math.max(0, Math.floor(s || 0)); return Math.floor(s / 60) + ':' + String(s % 60).padStart(2, '0'); };

  let genre = 'All', query = '', sort = 'new', shown = PAGE, list = BEATS.slice();
  let queue = [], qi = -1, current = null, yt = null, ytReady = false, pending = null;
  let shuffle = false, repeat = false, seeking = false;

  /* ---------------- catalogue ---------------- */
  function genres(){
    const counts = {};
    BEATS.forEach(b => counts[b.genre] = (counts[b.genre] || 0) + 1);
    const names = ['All', ...Object.keys(counts).sort((a, b) => counts[b] - counts[a])];
    $('btGenres').innerHTML = names.map(g =>
      `<button type="button" class="bt-chip${g === genre ? ' on' : ''}" data-g="${esc(g)}">${esc(g)}${g === 'All' ? '' : ' <small>' + counts[g] + '</small>'}</button>`).join('');
  }

  function filter(){
    const q = query.toLowerCase();
    list = BEATS.filter(b => (genre === 'All' || b.genre === genre) &&
      (!q || (b.name + ' ' + b.type + ' ' + b.genre).toLowerCase().includes(q)));
    if (sort === 'az') list.sort((a, b) => a.name.localeCompare(b.name));
    else if (sort === 'shuffle') list.sort(() => Math.random() - .5);
    shown = PAGE; render();
  }

  function card(b){
    const meta = [b.genre, b.bpm ? b.bpm + ' BPM' : '', b.key].filter(Boolean).map(t => `<span>${esc(t)}</span>`).join('');
    return `<article class="bt-card${current && current.id === b.id ? ' playing' : ''}" data-id="${b.id}">
      <button type="button" class="bt-cover" data-play="${b.id}" aria-label="Play ${esc(b.name)}">
        <img src="${img(b.id, 'mqdefault')}" alt="${esc(b.name)} — ${esc(b.type || b.genre)} type beat by DJBILBOX BEATS" loading="lazy">
        <span class="bt-cover-ic"><i class="fa-solid fa-play"></i><i class="fa-solid fa-pause"></i></span>
        ${b.free ? '<span class="bt-free">FREE</span>' : ''}
      </button>
      <h3>${esc(b.name)}</h3>
      <p class="bt-type">${esc(b.type ? b.type + ' type beat' : b.genre + ' instrumental')}</p>
      <div class="bt-meta">${meta}</div>
      ${b.free
        ? `<a class="bt-price" href="${FREE_PACK}" target="_blank" rel="noopener">Free download</a>`
        : `<button type="button" class="bt-price" data-buy="${b.id}">From $${LICENSES[0].price}</button>`}
    </article>`;
  }

  function render(){
    $('btGrid').innerHTML = list.slice(0, shown).map(card).join('') ||
      '<p class="bt-empty">No beat matches — try another word or genre.</p>';
    $('btCount').textContent = list.length + ' beat' + (list.length > 1 ? 's' : '');
    $('btMore').hidden = shown >= list.length;
  }

  /* ---------------- licences ---------------- */
  function licHtml(b){
    return LICENSES.map(l => {
      const url = CHECKOUT[l.k] || ('mailto:' + EMAIL + '?subject=' + encodeURIComponent(
        (l.k === 'exclusive' ? 'Exclusive offer' : l.name) + (b ? ' — ' + b.name : '')) +
        '&body=' + encodeURIComponent('Beat: ' + (b ? b.name + ' (https://youtu.be/' + b.id + ')' : '') + '\nLicense: ' + l.name + '\n'));
      return `<div class="bt-lic${l.best ? ' best' : ''}">
        ${l.best ? '<span class="bt-lic-tag">Most popular</span>' : ''}
        <h4>${l.name}</h4>
        <p class="bt-lic-price">${l.price ? '$' + l.price : 'Offer'}</p>
        <p class="bt-dim">${l.files}</p>
        <ul>${l.rights.map(r => `<li><i class="fa-solid fa-check"></i>${r}</li>`).join('')}</ul>
        <a class="bt-btn${l.best ? '' : ' ghost'}" href="${url}" ${CHECKOUT[l.k] ? 'target="_blank" rel="noopener"' : ''}>${l.price ? 'Buy ' + l.name : 'Make an offer'}</a>
      </div>`;
    }).join('');
  }

  function openLicense(b){
    $('btModalImg').src = img(b.id, 'mqdefault');
    $('btModalTitle').textContent = b.name;
    $('btModalType').textContent = b.type ? b.type + ' type beat' : b.genre + ' instrumental';
    $('btModalLic').innerHTML = licHtml(b);
    $('btModal').hidden = false;
  }

  /* ---------------- YouTube player ---------------- */
  function loadYT(){
    if (window.YT && window.YT.Player) return onApi();
    window.onYouTubeIframeAPIReady = onApi;
    const s = document.createElement('script'); s.src = 'https://www.youtube.com/iframe_api'; document.head.append(s);
  }
  function onApi(){
    yt = new YT.Player('btYT', {
      width: 356, height: 200, host: 'https://www.youtube-nocookie.com',
      playerVars: { autoplay: 1, playsinline: 1, rel: 0, modestbranding: 1 },
      events: {
        onReady: () => { ytReady = true; yt.setVolume(+$('btVol').value); if (pending) { yt.loadVideoById(pending); pending = null; } },
        onStateChange: e => {
          const playing = e.data === YT.PlayerState.PLAYING;
          $('btPlay').innerHTML = `<i class="fa-solid fa-${playing ? 'pause' : 'play'}"></i>`;
          document.body.classList.toggle('bt-is-playing', playing);
          if (e.data === YT.PlayerState.ENDED) repeat ? yt.seekTo(0) : step(1);
        },
        onError: () => step(1)
      }
    });
  }

  function play(id, fromList){
    const b = BEATS.find(x => x.id === id); if (!b) return;
    if (current && current.id === id && ytReady) { toggle(); return; }
    if (fromList !== false) { queue = list.slice(); qi = queue.findIndex(x => x.id === id); }
    current = b;
    $('btPlayer').hidden = false; $('btVideo').hidden = false; document.body.classList.add('bt-has-player');
    $('btNowImg').src = img(b.id, 'default');
    $('btNowName').textContent = b.name;
    $('btNowType').textContent = b.type ? b.type + ' type beat' : b.genre;
    $('btOnYT').href = 'https://www.youtube.com/watch?v=' + b.id;
    $('btBuyNow').hidden = b.free;
    document.querySelectorAll('.bt-card').forEach(c => c.classList.toggle('playing', c.dataset.id === id));
    if (ytReady) yt.loadVideoById(id); else { pending = id; if (!yt) loadYT(); }
  }
  function toggle(){
    if (!ytReady) return;
    yt.getPlayerState() === YT.PlayerState.PLAYING ? yt.pauseVideo() : yt.playVideo();
  }
  function step(d){
    if (!queue.length) return;
    qi = shuffle ? Math.floor(Math.random() * queue.length) : (qi + d + queue.length) % queue.length;
    play(queue[qi].id, false);
  }
  function close(){
    if (ytReady) yt.stopVideo();
    current = null;
    $('btPlayer').hidden = true; $('btVideo').hidden = true;
    document.body.classList.remove('bt-has-player', 'bt-is-playing');
    document.querySelectorAll('.bt-card.playing').forEach(c => c.classList.remove('playing'));
  }
  setInterval(() => {
    if (!ytReady || !current || seeking) return;
    const t = yt.getCurrentTime(), d = yt.getDuration();
    $('btCur').textContent = fmt(t); $('btDur').textContent = fmt(d);
    $('btSeek').value = d ? Math.round(t / d * 1000) : 0;
  }, 500);

  /* ---------------- events ---------------- */
  document.addEventListener('click', e => {
    const t = e.target.closest('[data-g],[data-play],[data-buy],[data-close]');
    if (!t) { if (e.target.id === 'btModal') $('btModal').hidden = true; return; }
    if (t.dataset.g) { genre = t.dataset.g; genres(); filter(); }
    else if (t.dataset.play) play(t.dataset.play);
    else if (t.dataset.buy) openLicense(BEATS.find(x => x.id === t.dataset.buy));
    else if (t.hasAttribute('data-close')) $('btModal').hidden = true;
  });
  document.addEventListener('keydown', e => { if (e.key === 'Escape') $('btModal').hidden = true; });
  let typing;
  $('btSearch').addEventListener('input', e => { clearTimeout(typing); typing = setTimeout(() => { query = e.target.value.trim(); filter(); }, 150); });
  $('btSort').addEventListener('change', e => { sort = e.target.value; filter(); });
  $('btMore').addEventListener('click', () => { shown += PAGE; render(); });
  $('btPlay').addEventListener('click', toggle);
  $('btPrev').addEventListener('click', () => step(-1));
  $('btNext').addEventListener('click', () => step(1));
  $('btClose').addEventListener('click', close);
  $('btShuffle').addEventListener('click', e => { shuffle = !shuffle; e.currentTarget.setAttribute('aria-pressed', shuffle); });
  $('btRepeat').addEventListener('click', e => { repeat = !repeat; e.currentTarget.setAttribute('aria-pressed', repeat); });
  $('btBuyNow').addEventListener('click', () => current && openLicense(current));
  $('btVol').addEventListener('input', e => ytReady && yt.setVolume(+e.target.value));
  $('btSeek').addEventListener('input', () => { seeking = true; });
  $('btSeek').addEventListener('change', e => {
    seeking = false; if (ytReady) yt.seekTo(yt.getDuration() * e.target.value / 1000, true);
  });

  /* keep the filter bar right under the site header (its height varies) */
  function dock(){
    const head = document.querySelector('.topbar');
    if (head) document.querySelector('.bt-bar').style.top = Math.round(head.getBoundingClientRect().bottom + 8) + 'px';
  }
  window.addEventListener('load', dock); window.addEventListener('resize', dock); setTimeout(dock, 300);

  $('btLicGrid').innerHTML = licHtml(null);
  genres(); filter();
})();
