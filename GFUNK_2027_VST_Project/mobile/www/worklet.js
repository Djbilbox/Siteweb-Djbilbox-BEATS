// GFUNK 2027 mobile engine (AudioWorklet). Port of the plug-in DSP: 2 osc + sub + noise with unison,
// TPT state-variable filter, 2 ADSR, 2 LFO, arpeggiator, tape, phaser-less light FX chain, delay, reverb.
const TWO_PI = Math.PI * 2;
const NVOICE = 8;

function fastTanh (x) { if (x > 3) return 1; if (x < -3) return -1; const x2 = x * x; return x * (27 + x2) / (27 + 9 * x2); }
function blep (t, dt) {
  if (t < dt) { t /= dt; return t + t - t * t - 1; }
  if (t > 1 - dt) { t = (t - 1) / dt; return t * t + t + t + 1; }
  return 0;
}
function shape (i, ph, dt, pw) {
  switch (i) {
    case 0: return Math.sin(ph * TWO_PI);
    case 1: return 2 * Math.abs(2 * ph - 1) - 1;
    case 2: return 2 * ph - 1 - blep(ph, dt);
    default: { let p2 = ph - pw; if (p2 < 0) p2 += 1;
      return (ph < pw ? 1 : -1) + blep(ph, dt) - blep(p2, dt) - (2 * pw - 1); }
  }
}
function oscWave (ph, dt, w, pw) {
  const i = Math.max(0, Math.min(3, w | 0)), fr = w - i, a = shape(i, ph, dt, pw);
  if (fr < 1e-3 || i >= 3) return a;
  return a + (shape(i + 1, ph, dt, pw) - a) * fr;
}
const mtof = n => 440 * Math.pow(2, (n - 69) / 12);

class Voice {
  constructor () {
    this.active = false; this.gate = false; this.note = -1; this.vel = 0; this.age = 0;
    this.freq = 440; this.target = 440; this.subPh = 0;
    this.ph1 = new Float32Array(7); this.ph2 = new Float32Array(7);
    this.aSt = 0; this.aV = 0; this.fSt = 0; this.fV = 0;     // 0 idle 1 atk 2 dec 3 sus 4 rel
    this.ic = new Float32Array(8);                             // [chan][stage][ic1, ic2]
    this.rng = 12345;
  }
}

class Comb { constructor (n) { this.b = new Float32Array(n); this.i = 0; this.s = 0; } }
class Allpass { constructor (n) { this.b = new Float32Array(n); this.i = 0; } }

class GFunk extends AudioWorkletProcessor {
  constructor () {
    super();
    this.P = null; this.names = [];
    this.voices = Array.from({ length: NVOICE }, () => new Voice());
    this.age = 0; this.held = []; this.arpHeld = []; this.arpSeq = []; this.arpPlaying = -1;
    this.arpCounter = 0; this.arpStep = 0; this.arpGate = 0; this.arpVel = 0.8; this.bpm = 100;
    this.lfo1 = 0; this.lfo2 = 0; this.sh = 0;
    this.bend = 0;
    const L = Math.floor(sampleRate * 2);
    this.dly = [new Float32Array(L), new Float32Array(L)]; this.dw = 0; this.dlyCur = sampleRate * 0.3; this.dlyLp = [0, 0];
    this.chBuf = [new Float32Array(2048), new Float32Array(2048)]; this.chW = 0; this.chPh = 0;
    this.toneLp = [0, 0];
    const k = sampleRate / 44100;
    const ct = [1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617], at = [556, 441, 341, 225];
    this.combs = [0, 1].map(c => ct.map(t => new Comb(Math.floor((t + c * 23) * k))));
    this.aps = [0, 1].map(c => at.map(t => new Allpass(Math.floor((t + c * 23) * k))));
    this.funk = 0;
    this.port.onmessage = e => this.onMsg(e.data);
  }
  onMsg (m) {
    if (m.type === 'params') { this.P = m.values; this.names = m.names; this.idx = {}; m.names.forEach((n, i) => this.idx[n] = i); this.arpDirty = true; }
    else if (m.type === 'set') { if (this.P) this.P[this.idx[m.id]] = m.value; }
    else if (m.type === 'on') this.noteOnIn(m.note, m.vel);
    else if (m.type === 'off') this.noteOffIn(m.note);
    else if (m.type === 'panic') this.allOff();
    else if (m.type === 'bend') this.bend = m.value;
    else if (m.type === 'bpm') this.bpm = m.value;
  }
  p (id) { return this.P[this.idx[id]]; }

  allOff () { this.held = []; this.arpHeld = []; this.arpPlaying = -1; for (const v of this.voices) if (v.active) this.release(v); }
  release (v) { v.gate = false; if (v.aSt) v.aSt = 4; if (v.fSt) v.fSt = 4; }
  start (v, note, vel, retrig) {
    const was = v.active;
    v.note = note; v.vel = vel; v.target = mtof(note);
    if (!was) {
      v.freq = v.target;
      for (let k = 0; k < 7; k++) { v.ph1[k] = Math.random(); v.ph2[k] = Math.random(); }
      v.subPh = 0; v.ic.fill(0);
    }
    v.gate = true; v.active = true; v.age = ++this.age;
    if (retrig) { v.aSt = 1; v.fSt = 1; }
    this.funk += 0.4 + 0.6 * vel;
  }
  voiceOn (note, vel) {
    const mode = this.p('mode') | 0;
    if (mode === 0) {
      let v = this.voices.find(x => x.active && x.note === note) || this.voices.find(x => !x.active);
      if (!v) { v = this.voices[0]; for (const x of this.voices) if ((!x.gate && v.gate) || (x.gate === v.gate && x.age < v.age)) v = x; }
      this.start(v, note, vel, true);
    } else {
      this.held = this.held.filter(n => n !== note); this.held.push(note);
      const v = this.voices[0];
      if (v.active && v.gate && mode === 2) { v.target = mtof(note); v.note = note; v.vel = vel; }
      else this.start(v, note, vel, true);
    }
  }
  voiceOff (note) {
    const mode = this.p('mode') | 0;
    if (mode === 0) { for (const v of this.voices) if (v.active && v.gate && v.note === note) this.release(v); }
    else {
      this.held = this.held.filter(n => n !== note);
      const v = this.voices[0];
      if (!this.held.length) this.release(v);
      else if (v.active) { v.target = mtof(this.held[this.held.length - 1]); v.note = this.held[this.held.length - 1]; if (mode === 1) { v.aSt = 1; v.fSt = 1; } }
    }
  }
  noteOnIn (note, vel) {
    if (!this.P) return;
    if ((this.p('arp') | 0) > 0) {
      this.arpVel = vel;
      if (!this.arpHeld.includes(note)) { this.arpHeld.push(note); this.arpHeld.sort((a, b) => a - b); this.arpDirty = true; }
      if (this.arpHeld.length === 1) { this.arpStep = 0; this.arpCounter = 0; }
    } else this.voiceOn(note, vel);
  }
  noteOffIn (note) {
    if (!this.P) return;
    if ((this.p('arp') | 0) > 0) {
      const i = this.arpHeld.indexOf(note); if (i < 0) return;
      this.arpHeld.splice(i, 1); this.arpDirty = true;
      if (!this.arpHeld.length && this.arpPlaying >= 0) { this.voiceOff(this.arpPlaying); this.arpPlaying = -1; }
    } else this.voiceOff(note);
  }
  arpTick (stepLen, arp) {
    if (this.arpGate > 0 && --this.arpGate === 0 && this.arpPlaying >= 0) { this.voiceOff(this.arpPlaying); this.arpPlaying = -1; }
    if (!this.arpHeld.length) return;
    if (this.arpCounter <= 0) {
      if (this.arpDirty) {
        this.arpSeq = []; const octs = Math.max(1, Math.min(4, this.p('arpoct') | 0));
        for (let o = 0; o < octs; o++) for (const n of this.arpHeld) if (n + 12 * o < 128) this.arpSeq.push(n + 12 * o);
        this.arpDirty = false;
      }
      const n = this.arpSeq.length;
      if (n) {
        let idx;
        if (arp === 1) idx = this.arpStep % n;
        else if (arp === 2) idx = n - 1 - this.arpStep % n;
        else if (arp === 3) { const per = Math.max(1, 2 * n - 2), q = this.arpStep % per; idx = q < n ? q : 2 * n - 2 - q; }
        else idx = Math.floor(Math.random() * n);
        this.arpStep++;
        if (this.arpPlaying >= 0) this.voiceOff(this.arpPlaying);
        this.arpPlaying = this.arpSeq[idx]; this.voiceOn(this.arpPlaying, this.arpVel);
        this.arpGate = Math.max(1, Math.floor(stepLen * this.p('arpgate')));
      }
      this.arpCounter = stepLen;
    }
    this.arpCounter--;
  }
  tickEnv (st, v, c) {
    switch (st) {
      case 1: v += c.atk; if (v >= 1) { v = 1; st = 2; } break;
      case 2: v = c.sus + (v - c.sus) * c.dec; if (Math.abs(v - c.sus) < 1e-4) { v = c.sus; st = 3; } break;
      case 3: v = c.sus; break;
      case 4: v *= c.rel; if (v < 1e-4) { v = 0; st = 0; } break;
      default: v = 0;
    }
    return [st, v];
  }

  process (inputs, outputs) {
    const out = outputs[0], L = out[0], R = out[1] || out[0], n = L.length;
    if (!this.P) return true;
    const fs = sampleRate, p = id => this.p(id);
    const ms = x => x * 0.001 * fs;
    const ampC = { atk: 1 / Math.max(1, ms(Math.max(1, p('aA')))), dec: Math.exp(-4.6 / ms(p('aD'))), sus: p('aS'), rel: Math.exp(-6.9 / ms(p('aR'))) };
    const fltC = { atk: 1 / Math.max(1, ms(Math.max(1, p('fA')))), dec: Math.exp(-4.6 / ms(p('fD'))), sus: p('fS'), rel: Math.exp(-6.9 / ms(p('fR'))) };
    const glideMs = p('glide'), glideCoef = glideMs < 1 ? 0 : Math.exp(-4 / (glideMs * 0.001 * fs));
    const uniN = Math.max(1, Math.min(7, p('uni') | 0));
    const det = p('unidetune') * 45, spread = p('unispread'), norm = 1.41421356 / Math.sqrt(uniN);
    const uniMul = [], uniL = [], uniR = [];
    for (let k = 0; k < uniN; k++) {
      const pos = uniN === 1 ? 0 : (k / (uniN - 1)) * 2 - 1;
      uniMul.push(Math.pow(2, pos * det / 1200));
      const ang = (pos * spread + 1) * Math.PI * 0.25;
      uniL.push(Math.cos(ang) * norm); uniR.push(Math.sin(ang) * norm);
    }
    const o1w = p('o1wave'), o2w = p('o2wave'), o1l = p('o1level'), o2l = p('o2level'), subL = p('sub'), noiseL = p('noise');
    const o1mul = Math.pow(2, p('o1oct')), o2mul = Math.pow(2, p('o2oct') + p('o2semi') / 12 + p('o2fine') / 1200);
    const pwBase = p('pw'), ftype = p('ftype') | 0, cutoff = p('cutoff'), reso = p('reso'), keytrack = p('keytrack'), fenvamt = p('fenvamt');
    const fdrive = p('fdrive'), dg = 1 + fdrive * 5, dinv = 1 / Math.sqrt(dg);
    const l1Inc = p('lfo1rate') / fs, l2Inc = p('lfo2rate') / fs, l1shape = p('lfo1shape') | 0;
    const l1cut = p('lfo1cut'), l1pitch = p('lfo1pitch'), l1pwm = p('lfo1pwm'), l2amp = p('lfo2amp'), l2cut = p('lfo2cut');
    const kBase = 2 * (1 - reso * 0.97);
    const arp = p('arp') | 0;
    const beats = [1, 0.5, 1 / 3, 0.25, 1 / 6, 0.125];
    const stepLen = Math.max(64, Math.floor(60 / this.bpm * fs * beats[Math.max(0, Math.min(5, p('arprate') | 0))]));
    if (this.lastArp !== arp) { if (this.lastArp !== undefined) this.allOff(); this.lastArp = arp; this.arpDirty = true; }

    for (let i = 0; i < n; i++) {
      if (arp > 0) this.arpTick(stepLen, arp);
      this.lfo1 += l1Inc; if (this.lfo1 >= 1) { this.lfo1 -= 1; this.sh = Math.random() * 2 - 1; }
      this.lfo2 += l2Inc; if (this.lfo2 >= 1) this.lfo2 -= 1;
      let l1;
      switch (l1shape) {
        case 0: l1 = Math.sin(this.lfo1 * TWO_PI); break;
        case 1: l1 = 4 * Math.abs(this.lfo1 - 0.5) - 1; break;
        case 2: l1 = 2 * this.lfo1 - 1; break;
        case 3: l1 = this.lfo1 < 0.5 ? 1 : -1; break;
        default: l1 = this.sh;
      }
      const l2 = Math.sin(this.lfo2 * TWO_PI);
      const vib = Math.min(1, l1pitch);
      const pitchMul = Math.pow(2, (l1 * vib * 2 + this.bend * 2) / 12);
      const pw = Math.max(0.05, Math.min(0.95, pwBase + l1 * l1pwm * 0.4));
      const trem = 1 - l2amp * (0.5 - 0.5 * l2);
      let oL = 0, oR = 0;
      for (const v of this.voices) {
        if (!v.active) continue;
        v.freq = v.target + (v.freq - v.target) * glideCoef;
        let r = this.tickEnv(v.aSt, v.aV, ampC); v.aSt = r[0]; v.aV = r[1];
        if (v.aSt === 0) { v.active = false; v.gate = false; continue; }
        r = this.tickEnv(v.fSt, v.fV, fltC); v.fSt = r[0]; v.fV = r[1];
        const base = v.freq * pitchMul, f1 = base * o1mul, f2 = base * o2mul;
        let l = 0, rr = 0;
        for (let k = 0; k < uniN; k++) {
          const dt1 = Math.max(1e-5, Math.min(0.45, f1 * uniMul[k] / fs)), dt2 = Math.max(1e-5, Math.min(0.45, f2 * uniMul[k] / fs));
          v.ph1[k] += dt1; if (v.ph1[k] >= 1) v.ph1[k] -= 1;
          v.ph2[k] += dt2; if (v.ph2[k] >= 1) v.ph2[k] -= 1;
          const s = oscWave(v.ph1[k], dt1, o1w, pw) * o1l + oscWave(v.ph2[k], dt2, o2w, pw) * o2l;
          l += s * uniL[k]; rr += s * uniR[k];
        }
        v.subPh += f1 * 0.5 / fs; if (v.subPh >= 1) v.subPh -= 1;
        v.rng = (Math.imul(v.rng, 1664525) + 1013904223) >>> 0;
        const nz = ((v.rng >>> 8) / 8388608 - 1) * noiseL;
        const cen = Math.sin(v.subPh * TWO_PI) * subL + nz;
        l = (l + cen) * 0.3; rr = (rr + cen) * 0.3;
        if ((i & 7) === 0 || v.a1 === undefined) {
          const oct = keytrack * Math.log2(v.freq / 261.63) + fenvamt * v.fV * 5 + (l1 * l1cut + l2 * l2cut) * 3;
          const fc = Math.max(20, Math.min(0.46 * fs, cutoff * Math.pow(2, oct)));
          const g = Math.tan(Math.PI * fc / fs);
          v.k = kBase; v.a1 = 1 / (1 + g * (g + v.k)); v.a2 = g * v.a1; v.a3 = g * v.a2;
          v.b1 = 1 / (1 + g * (g + 1.5)); v.b2 = g * v.b1; v.b3 = g * v.b2;
        }
        const inp = [l, rr], o = [0, 0];
        for (let c = 0; c < 2; c++) {
          let x = inp[c];
          if (fdrive > 0.001) x = fastTanh(x * dg) * dinv;
          const b = c * 4;
          const v3 = x - v.ic[b + 1], v1 = v.a1 * v.ic[b] + v.a2 * v3, v2 = v.ic[b + 1] + v.a2 * v.ic[b] + v.a3 * v3;
          v.ic[b] = 2 * v1 - v.ic[b]; v.ic[b + 1] = 2 * v2 - v.ic[b + 1];
          if (ftype === 0) o[c] = v2;
          else if (ftype === 1) {
            const w3 = v2 - v.ic[b + 3], w1 = v.b1 * v.ic[b + 2] + v.b2 * w3, w2 = v.ic[b + 3] + v.b2 * v.ic[b + 2] + v.b3 * w3;
            v.ic[b + 2] = 2 * w1 - v.ic[b + 2]; v.ic[b + 3] = 2 * w2 - v.ic[b + 3]; o[c] = w2;
          } else if (ftype === 2) o[c] = v1 * v.k;
          else o[c] = x - v.k * v1 - v2;
        }
        const gain = v.aV * (0.3 + 0.7 * v.vel) * trem;
        oL += o[0] * gain; oR += o[1] * gain;
      }
      L[i] = oL; R[i] = oR;
    }
    this.fx(L, R, n);
    return true;
  }

  fx (l, r, n) {
    const fs = sampleRate, p = id => this.p(id);
    const drive = p('drive'), dk = 1 + drive * 9, bias = 0.12 * drive, biasOut = Math.tanh(bias), comp = 1 / Math.sqrt(dk), blend = Math.min(1, drive * 4);
    const chMix = p('chorus') * 0.6;
    const dlyBeats = [0.25, 1 / 3, 0.5, 0.75, 1];
    const dlyTarget = Math.max(64, Math.min(this.dly[0].length - 4, 60 / this.bpm * fs * dlyBeats[Math.max(0, Math.min(4, p('dlytime') | 0))]));
    const dlyFb = p('dlyfb'), dlyMix = p('dlymix');
    const rv = p('revmix'), room = 0.3 + 0.65 * p('revsize'), damp = 0.45, wet = rv * 0.4, dry = 1 - rv * 0.25;
    const toneT = p('tone') * 0.7, toneC = 1 - Math.exp(-TWO_PI * 1200 / fs), vol = p('volume') * 2;
    const dm = this.dly[0].length, cm = 2048;
    for (let i = 0; i < n; i++) {
      const ch = [l[i], r[i]];
      for (let c = 0; c < 2; c++) {                       // tape saturation
        const x = ch[c], s = (Math.tanh(x * dk + bias) - biasOut) * comp;
        ch[c] = x + (s - x) * blend;
      }
      if (chMix > 0.001) {                                // chorus
        this.chPh += 0.8 / fs; if (this.chPh >= 1) this.chPh -= 1;
        for (let c = 0; c < 2; c++) {
          this.chBuf[c][this.chW] = ch[c];
          const d = (7 + 3 * Math.sin((this.chPh + c * 0.25) * TWO_PI)) * 0.001 * fs, rp = this.chW - d, i0 = Math.floor(rp), fr = rp - i0;
          const a = this.chBuf[c][(i0 + cm) % cm], b = this.chBuf[c][(i0 + 1 + cm) % cm];
          ch[c] = ch[c] * (1 - chMix * 0.5) + (a + (b - a) * fr) * chMix;
        }
        this.chW = (this.chW + 1) % cm;
      }
      this.dlyCur += (dlyTarget - this.dlyCur) * 0.0004;   // ping-pong delay
      const rd = [0, 0];
      for (let c = 0; c < 2; c++) {
        const rp = this.dw - this.dlyCur, i0 = Math.floor(rp), fr = rp - i0, buf = this.dly[c];
        const a = buf[((i0 % dm) + dm) % dm], b = buf[(((i0 + 1) % dm) + dm) % dm];
        rd[c] = a + (b - a) * fr;
      }
      this.dlyLp[0] += 0.4 * (rd[1] - this.dlyLp[0]); this.dlyLp[1] += 0.4 * (rd[0] - this.dlyLp[1]);
      this.dly[0][this.dw] = 0.5 * (ch[0] + ch[1]) + this.dlyLp[0] * dlyFb; this.dly[1][this.dw] = this.dlyLp[1] * dlyFb;
      this.dw = (this.dw + 1) % dm;
      ch[0] += rd[0] * dlyMix; ch[1] += rd[1] * dlyMix;
      if (rv > 0.001) {                                   // reverb (Freeverb style)
        const inp = (ch[0] + ch[1]) * 0.015, w = [0, 0];
        for (let c = 0; c < 2; c++) {
          let sum = 0;
          for (const cb of this.combs[c]) {
            const o = cb.b[cb.i]; cb.s = o * (1 - damp) + cb.s * damp; cb.b[cb.i] = inp + cb.s * room;
            if (++cb.i >= cb.b.length) cb.i = 0; sum += o;
          }
          for (const ap of this.aps[c]) {
            const bo = ap.b[ap.i], o = bo - sum; ap.b[ap.i] = sum + bo * 0.5;
            if (++ap.i >= ap.b.length) ap.i = 0; sum = o;
          }
          w[c] = sum;
        }
        ch[0] = ch[0] * dry + w[0] * wet * 3; ch[1] = ch[1] * dry + w[1] * wet * 3;
      }
      for (let c = 0; c < 2; c++) {                       // tone tilt + volume + soft limiter
        let x = ch[c]; this.toneLp[c] += toneC * (x - this.toneLp[c]);
        const lo = this.toneLp[c], hi = x - lo;
        x = lo * (1 - toneT) + hi * (1 + toneT);
        ch[c] = fastTanh(x * vol * 1.2) / 1.2;
      }
      l[i] = ch[0]; r[i] = ch[1];
    }
  }
}
registerProcessor('gfunk', GFunk);
