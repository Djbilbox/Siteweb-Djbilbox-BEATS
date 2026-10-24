// GFUNK 2027 Free - mobile UI (Android APK via Capacitor, iPhone via PWA)
(async function () {
  const $ = id => document.getElementById(id);
  const data = await (await fetch('presets.json')).json();
  const names = data.params.map(p => p.id);
  const pdef = Object.fromEntries(data.params.map((p, i) => [p.id, Object.assign({ i }, p)]));
  const skew = { cutoff: 0.25, aA: 0.35, aR: 0.35 };
  let ctx = null, node = null, cur = 0, base = 48;      // base = lowest key (C3)
  const values = data.params.map(p => p.def);

  function send (m) { if (node) node.port.postMessage(m); }
  function setParam (id, v) { values[pdef[id].i] = v; send({ type: 'set', id, value: v }); }

  // ---- macro sliders
  const macros = [['cutoff', 'CUTOFF'], ['reso', 'RESO'], ['aA', 'ATTACK'], ['aR', 'RELEASE'],
                  ['revmix', 'REVERB'], ['dlymix', 'DELAY'], ['tone', 'TONE'], ['volume', 'VOLUME']];
  const sliders = {};
  for (const [id, label] of macros) {
    const d = pdef[id], sk = skew[id] || 1, l = document.createElement('label');
    l.textContent = label;
    const r = document.createElement('input');
    r.type = 'range'; r.min = 0; r.max = 1; r.step = 0.001;
    r.oninput = () => setParam(id, d.min + (d.max - d.min) * Math.pow(+r.value, 1 / sk));
    sliders[id] = () => { r.value = Math.pow(Math.max(0, (values[d.i] - d.min) / (d.max - d.min)), sk); };
    l.appendChild(r); $('macros').appendChild(l);
  }

  function showName () {
    const p = data.presets[cur];
    $('pname').textContent = p.name;
    $('pcat').textContent = (cur + 1) + ' / ' + data.presets.length + '  ·  ' + data.categories[p.cat];
  }

  // ---- presets
  function loadPreset (i) {
    cur = (i + data.presets.length) % data.presets.length;
    data.presets[cur].v.forEach((v, k) => values[k] = v);
    send({ type: 'panic' });
    send({ type: 'params', names, values: values.slice() });
    showName();
    Object.values(sliders).forEach(f => f());
    try { localStorage.setItem('gfunkLast', cur); } catch (e) {}
    document.querySelectorAll('#list div').forEach(e => e.classList.toggle('cur', +e.dataset.i === cur));
  }
  $('prev').onclick = () => loadPreset(cur - 1);
  $('next').onclick = () => loadPreset(cur + 1);
  $('dice').onclick = () => loadPreset(Math.floor(Math.random() * data.presets.length));

  // ---- preset browser
  let cat = 0;
  function buildList () {
    const cats = $('cats'); cats.innerHTML = '';
    data.categories.forEach((c, k) => {
      const b = document.createElement('button'); b.textContent = c; b.className = k === cat ? 'on' : '';
      b.onclick = () => { cat = k; buildList(); }; cats.appendChild(b);
    });
    const list = $('list'); list.innerHTML = '';
    data.presets.forEach((p, i) => {
      if (p.cat !== cat) return;
      const d = document.createElement('div'); d.dataset.i = i; d.className = i === cur ? 'cur' : '';
      const a = document.createElement('b'), s = document.createElement('span');
      a.style.fontWeight = 500; a.textContent = p.name; s.textContent = String(i + 1).padStart(3, '0');
      d.append(a, s);
      d.onclick = () => { loadPreset(i); $('sheet').classList.remove('open'); };
      list.appendChild(d);
    });
  }
  $('menu').onclick = () => { cat = data.presets[cur].cat; buildList(); $('sheet').classList.add('open'); };
  $('close').onclick = () => $('sheet').classList.remove('open');

  // ---- keyboard (2 octaves + 1 key, multi-touch with glissando)
  const WH = [0, 2, 4, 5, 7, 9, 11], BLK = { 0: 1, 1: 3, 3: 6, 4: 8, 5: 10 };   // black key after white index
  const keys = $('keys'), keyEls = {};
  function buildKeys () {
    keys.innerHTML = ''; Object.keys(keyEls).forEach(k => delete keyEls[k]);
    const whites = 15;
    for (let w = 0; w < whites; w++) {
      const note = base + Math.floor(w / 7) * 12 + WH[w % 7];
      const k = document.createElement('div'); k.className = 'k'; k.dataset.n = note; keys.appendChild(k); keyEls[note] = k;
      if (BLK[w % 7] !== undefined && w < whites - 1) {
        const b = document.createElement('div'); b.className = 'bk';
        const bn = base + Math.floor(w / 7) * 12 + BLK[w % 7];
        b.dataset.n = bn; b.style.left = ((w + 1) / whites * 100 - 3.1) + '%'; keys.appendChild(b); keyEls[bn] = b;
      }
    }
    $('octl').textContent = 'C' + (base / 12 - 1) + '–C' + ((base + 24) / 12 - 1);
  }
  const down = new Map();      // pointerId -> note
  function noteAt (x, y) { const e = document.elementFromPoint(x, y); return e && e.dataset && e.dataset.n !== undefined ? +e.dataset.n : null; }
  function release (id) {
    if (!down.has(id)) return;
    const n = down.get(id); down.delete(id);
    if (![...down.values()].includes(n)) { if (keyEls[n]) keyEls[n].classList.remove('on'); send({ type: 'off', note: n }); }
  }
  function press (id, n) {
    if (down.get(id) === n) return;
    release(id);
    if (n === null) return;
    down.set(id, n); if (keyEls[n]) keyEls[n].classList.add('on'); send({ type: 'on', note: n, vel: 0.85 });
  }
  keys.addEventListener('pointerdown', e => { e.preventDefault(); keys.setPointerCapture(e.pointerId); press(e.pointerId, noteAt(e.clientX, e.clientY)); });
  keys.addEventListener('pointermove', e => { if (down.has(e.pointerId)) press(e.pointerId, noteAt(e.clientX, e.clientY)); });
  for (const t of ['pointerup', 'pointercancel', 'lostpointercapture']) keys.addEventListener(t, e => release(e.pointerId));
  $('octd').onclick = () => { if (base > 24) { send({ type: 'panic' }); base -= 12; buildKeys(); } };
  $('octu').onclick = () => { if (base < 84) { send({ type: 'panic' }); base += 12; buildKeys(); } };
  const bend = $('bend');
  bend.oninput = () => send({ type: 'bend', value: +bend.value });
  for (const t of ['pointerup', 'touchend']) bend.addEventListener(t, () => { bend.value = 0; send({ type: 'bend', value: 0 }); });

  // ---- audio start (needs a user gesture, especially on iOS)
  async function startAudio () {
    if (ctx) return;
    const AC = window.AudioContext || window.webkitAudioContext;
    ctx = new AC({ latencyHint: 'interactive' });
    await ctx.resume();
    await ctx.audioWorklet.addModule('worklet.js');
    node = new AudioWorkletNode(ctx, 'gfunk', { numberOfInputs: 0, outputChannelCount: [2] });
    node.connect(ctx.destination);
    loadPreset(cur);
    window.__gfunk = { ctx, node };
  }
  $('go').onclick = async () => { try { await startAudio(); } catch (e) { alert('Audio: ' + e); } $('start').style.display = 'none'; };
  document.addEventListener('visibilitychange', () => { if (ctx && !document.hidden && ctx.state !== 'running') ctx.resume(); });

  buildKeys();
  let last = 0;
  try { last = +(localStorage.getItem('gfunkLast') || 0); } catch (e) {}
  cur = Math.max(0, Math.min(data.presets.length - 1, last));
  showName();
  Object.values(sliders).forEach(f => f());
  if ('serviceWorker' in navigator && location.protocol.startsWith('http')) navigator.serviceWorker.register('sw.js').catch(() => {});
})();
