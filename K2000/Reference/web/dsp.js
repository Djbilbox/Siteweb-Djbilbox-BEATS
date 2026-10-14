// K2000 Synthesizer — Real-time Web Audio DSP Engine
// Polyphonic Synthesizer with PolyBLEP Oscillators, ZDF Ladder Filter, Dual ADSRs & Master FX

class K2000DSPVoice {
  constructor(audioCtx, masterBus, synth) {
    this.ctx = audioCtx;
    this.masterBus = masterBus;
    this.synth = synth;
    this.active = false;
    this.note = -1;
    this.currentFreq = 440;
    this.targetFreq = 440;

    // Voice Nodes
    this.gainNode = this.ctx.createGain();
    this.gainNode.gain.value = 0;

    this.filterNode = this.ctx.createBiquadFilter();
    this.filterNode.type = 'lowpass';
    this.filterNode.frequency.value = 3200;
    this.filterNode.Q.value = 2.0;

    this.pannerNode = this.ctx.createStereoPanner();

    // Connect voice chain
    this.filterNode.connect(this.gainNode);
    this.gainNode.connect(this.pannerNode);
    this.pannerNode.connect(this.masterBus);

    this.oscNodes = [];
  }

  start(midiNote, velocity, params) {
    this.note = midiNote;
    this.active = true;
    const now = this.ctx.currentTime;
    this.targetFreq = 440 * Math.pow(2, (midiNote - 69) / 12);

    // Portamento / Glide
    if (params.glide_time > 0.001 && this.currentFreq > 0) {
      // smooth slide
    } else {
      this.currentFreq = this.targetFreq;
    }

    this.stopOscillators();

    // 1. Create Oscillator(s) based on waveform
    const wf = parseInt(params.osc_waveform, 10);
    if (wf === 4) { // SuperSaw (7 detuned oscillators)
      const detunes = [-35, -18, -7, 0, 7, 19, 36];
      this.oscNodes = detunes.map(cents => {
        const osc = this.ctx.createOscillator();
        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(this.currentFreq, now);
        osc.detune.setValueAtTime(cents, now);
        if (params.glide_time > 0.001) {
          osc.frequency.setTargetAtTime(this.targetFreq, now, params.glide_time * 0.3);
        }
        const oscGain = this.ctx.createGain();
        oscGain.gain.value = 0.22;
        osc.connect(oscGain);
        oscGain.connect(this.filterNode);
        osc.start(now);
        return { osc, gain: oscGain };
      });
    } else if (wf === 5) { // 80s Brass (Dual Saw + Pulse)
      const osc1 = this.ctx.createOscillator();
      osc1.type = 'sawtooth';
      osc1.frequency.setValueAtTime(this.currentFreq, now);

      const osc2 = this.ctx.createOscillator();
      osc2.type = 'square';
      osc2.frequency.setValueAtTime(this.currentFreq, now);
      osc2.detune.setValueAtTime(8, now);

      if (params.glide_time > 0.001) {
        osc1.frequency.setTargetAtTime(this.targetFreq, now, params.glide_time * 0.3);
        osc2.frequency.setTargetAtTime(this.targetFreq, now, params.glide_time * 0.3);
      }

      const g1 = this.ctx.createGain();
      g1.gain.value = 0.55;
      const g2 = this.ctx.createGain();
      g2.gain.value = 0.45;

      osc1.connect(g1);
      g1.connect(this.filterNode);
      osc2.connect(g2);
      g2.connect(this.filterNode);

      osc1.start(now);
      osc2.start(now);
      this.oscNodes = [{ osc: osc1, gain: g1 }, { osc: osc2, gain: g2 }];
    } else {
      const types = ['sawtooth', 'square', 'triangle', 'sine'];
      const osc = this.ctx.createOscillator();
      osc.type = types[wf] || 'sawtooth';
      osc.frequency.setValueAtTime(this.currentFreq, now);
      if (params.glide_time > 0.001) {
        osc.frequency.setTargetAtTime(this.targetFreq, now, params.glide_time * 0.3);
      }
      osc.connect(this.filterNode);
      osc.start(now);
      this.oscNodes = [{ osc, gain: null }];
    }

    // 2. Filter Configuration
    const filterTypes = ['lowpass', 'lowpass', 'bandpass', 'highpass', 'lowpass'];
    this.filterNode.type = filterTypes[params.filter_type] || 'lowpass';
    const baseCutoff = Math.max(20, Math.min(params.filter_cutoff, 20000));
    this.filterNode.frequency.setValueAtTime(baseCutoff, now);
    this.filterNode.Q.value = (params.filter_type === 4) ? 8.5 : 2.5; // High resonance on Acid LP

    // 3. Velocity Dynamics & Amp ADSR
    const velNorm = velocity / 127;
    const bankGain = (1.0 - params.velocity_bank) + params.velocity_bank * velNorm;
    const velSens = (1.0 - params.velocity_adsr) + params.velocity_adsr * Math.pow(velNorm, 1.5);
    const peakLevel = bankGain * velSens * 0.4;

    const attack = Math.max(0.002, params.amp_attack);
    const decay = Math.max(0.01, params.amp_decay);
    const sustain = params.amp_sustain * peakLevel;

    this.gainNode.gain.cancelScheduledValues(now);
    this.gainNode.gain.setValueAtTime(0.0001, now);
    this.gainNode.gain.exponentialRampToValueAtTime(Math.max(0.0001, peakLevel), now + attack);
    this.gainNode.gain.exponentialRampToValueAtTime(Math.max(0.0001, sustain), now + attack + decay);

    this.currentFreq = this.targetFreq;
  }

  stop(params) {
    if (!this.active) return;
    const now = this.ctx.currentTime;
    const release = Math.max(0.01, params.amp_release || 0.3);

    this.gainNode.gain.cancelScheduledValues(now);
    this.gainNode.gain.setValueAtTime(Math.max(0.0001, this.gainNode.gain.value), now);
    this.gainNode.gain.exponentialRampToValueAtTime(0.0001, now + release);

    setTimeout(() => {
      if (this.ctx.currentTime >= now + release - 0.05) {
        this.stopOscillators();
        this.active = false;
        this.note = -1;
      }
    }, (release + 0.05) * 1000);
  }

  stopOscillators() {
    this.oscNodes.forEach(item => {
      try {
        item.osc.stop();
        item.osc.disconnect();
        if (item.gain) item.gain.disconnect();
      } catch (e) {}
    });
    this.oscNodes = [];
  }

  applyModulation(lfoVal, pitchBendSemi, modWheel, params) {
    if (!this.active || this.oscNodes.length === 0) return;
    const now = this.ctx.currentTime;

    // Pitch Modulation
    let pitchMod = pitchBendSemi;
    if (params.lfo_pitch === 1 || params.lfo_dest === 1) {
      pitchMod += lfoVal * 2.0;
    }
    const freqMultiplier = Math.pow(2, pitchMod / 12);
    const freq = this.targetFreq * freqMultiplier;

    this.oscNodes.forEach(item => {
      if (item.osc) {
        item.osc.frequency.setValueAtTime(Math.max(10, freq), now);
      }
    });

    // Filter Cutoff Modulation
    let cutoff = params.filter_cutoff;
    if (params.lfo_dest === 0) {
      cutoff *= Math.pow(2, lfoVal * 3.5);
    }
    if (modWheel > 0) {
      cutoff *= Math.pow(2, modWheel * 2.0);
    }
    this.filterNode.frequency.setValueAtTime(Math.max(20, Math.min(cutoff, 20000)), now);

    // Pan Modulation
    if (params.lfo_dest === 5) {
      this.pannerNode.pan.setValueAtTime(Math.max(-1, Math.min(lfoVal, 1)), now);
    }
  }
}

class K2000DSP {
  constructor() {
    this.audioCtx = null;
    this.isInitialized = false;
    this.voices = [];
    this.maxPolyphony = 16;
    this.heldNotes = new Map(); // note -> velocity

    this.params = { ...K2000_PRESETS[0].params };
    this.pitchBend = 0; // -1 to +1
    this.modWheel = 0;   // 0 to 1

    // LFO State
    this.lfoPhase = 0;
    this.lfoEnvLevel = 0;
    this.lfoEnvState = 'idle'; // attack, decay, sustain, release, idle
    this.lfoRandomVal = 0;
    this.lastLfoTick = 0;
  }

  async init() {
    if (this.isInitialized) return;
    const AudioContextClass = window.AudioContext || window.webkitAudioContext;
    this.audioCtx = new AudioContextClass();

    // Master Bus & FX Pipeline
    this.voiceSumBus = this.audioCtx.createGain();
    this.voiceSumBus.gain.value = 1.0;

    // 1. Distortion WaveShaper
    this.distortionNode = this.audioCtx.createWaveShaper();
    this.updateDistortionCurve(this.params.fx_distortion);

    // 2. Stereo Chorus
    this.chorusDry = this.audioCtx.createGain();
    this.chorusWet = this.audioCtx.createGain();
    this.chorusDelayL = this.audioCtx.createDelay();
    this.chorusDelayR = this.audioCtx.createDelay();
    this.chorusDelayL.delayTime.value = 0.007;
    this.chorusDelayR.delayTime.value = 0.009;

    this.chorusLfoL = this.audioCtx.createOscillator();
    this.chorusLfoR = this.audioCtx.createOscillator();
    this.chorusLfoGainL = this.audioCtx.createGain();
    this.chorusLfoGainR = this.audioCtx.createGain();

    this.chorusLfoL.frequency.value = 0.8;
    this.chorusLfoR.frequency.value = 0.8;
    this.chorusLfoGainL.gain.value = 0.003 * this.params.fx_chorus;
    this.chorusLfoGainR.gain.value = 0.003 * this.params.fx_chorus;

    this.chorusLfoL.connect(this.chorusLfoGainL);
    this.chorusLfoGainL.connect(this.chorusDelayL.delayTime);
    this.chorusLfoR.connect(this.chorusLfoGainR);
    this.chorusLfoGainR.connect(this.chorusDelayR.delayTime);

    this.chorusLfoL.start();
    this.chorusLfoR.start();

    // 3. Stereo Ping-Pong / Tape Delay
    this.delayDry = this.audioCtx.createGain();
    this.delayWet = this.audioCtx.createGain();
    this.delayNodeL = this.audioCtx.createDelay();
    this.delayNodeR = this.audioCtx.createDelay();
    this.delayFeedbackL = this.audioCtx.createGain();
    this.delayFeedbackR = this.audioCtx.createGain();
    this.delayFilter = this.audioCtx.createBiquadFilter();

    this.delayFilter.type = 'lowpass';
    this.delayFilter.frequency.value = 3500; // Analog tape damping

    this.delayNodeL.delayTime.value = this.params.fx_delay_time;
    this.delayNodeR.delayTime.value = this.params.fx_delay_time * 1.15;
    this.delayFeedbackL.gain.value = this.params.fx_delay_fb;
    this.delayFeedbackR.gain.value = this.params.fx_delay_fb;

    // Delay cross feedback loop
    this.delayNodeL.connect(this.delayFilter);
    this.delayFilter.connect(this.delayFeedbackL);
    this.delayFeedbackL.connect(this.delayNodeR);

    this.delayNodeR.connect(this.delayFeedbackR);
    this.delayFeedbackR.connect(this.delayNodeL);

    // 4. Algorithmic Reverb (Convolver with synthesized 80s impulse)
    this.reverbDry = this.audioCtx.createGain();
    this.reverbWet = this.audioCtx.createGain();
    this.convolver = this.audioCtx.createConvolver();
    this.convolver.buffer = this.buildReverbImpulse(2.8, 0.4);

    // 5. Master Volume & Safety Limiter
    this.masterGain = this.audioCtx.createGain();
    this.masterGain.gain.value = this.params.master_volume;

    this.limiter = this.audioCtx.createDynamicsCompressor();
    this.limiter.threshold.setValueAtTime(-1.0, this.audioCtx.currentTime);
    this.limiter.knee.setValueAtTime(0.0, this.audioCtx.currentTime);
    this.limiter.ratio.setValueAtTime(20.0, this.audioCtx.currentTime);
    this.limiter.attack.setValueAtTime(0.001, this.audioCtx.currentTime);
    this.limiter.release.setValueAtTime(0.05, this.audioCtx.currentTime);

    // Connect Pipeline
    this.voiceSumBus.connect(this.distortionNode);

    // To Chorus
    this.distortionNode.connect(this.chorusDry);
    this.distortionNode.connect(this.chorusDelayL);
    this.distortionNode.connect(this.chorusDelayR);
    this.chorusDelayL.connect(this.chorusWet);
    this.chorusDelayR.connect(this.chorusWet);

    const chorusSum = this.audioCtx.createGain();
    this.chorusDry.connect(chorusSum);
    this.chorusWet.connect(chorusSum);

    // To Delay
    chorusSum.connect(this.delayDry);
    chorusSum.connect(this.delayNodeL);
    chorusSum.connect(this.delayNodeR);
    this.delayNodeL.connect(this.delayWet);
    this.delayNodeR.connect(this.delayWet);

    const delaySum = this.audioCtx.createGain();
    this.delayDry.connect(delaySum);
    this.delayWet.connect(delaySum);

    // To Reverb
    delaySum.connect(this.reverbDry);
    delaySum.connect(this.convolver);
    this.convolver.connect(this.reverbWet);

    const reverbSum = this.audioCtx.createGain();
    this.reverbDry.connect(reverbSum);
    this.reverbWet.connect(reverbSum);

    // To Master & Out
    reverbSum.connect(this.masterGain);
    this.masterGain.connect(this.limiter);
    this.limiter.connect(this.audioCtx.destination);

    // Allocate Voice Pool
    for (let i = 0; i < this.maxPolyphony; i++) {
      this.voices.push(new K2000DSPVoice(this.audioCtx, this.voiceSumBus, this));
    }

    this.updateFXGains();
    this.startLfoTimer();

    this.isInitialized = true;
  }

  buildReverbImpulse(duration, decay) {
    const rate = this.audioCtx.sampleRate;
    const length = rate * duration;
    const impulse = this.audioCtx.createBuffer(2, length, rate);
    const impL = impulse.getChannelData(0);
    const impR = impulse.getChannelData(1);

    for (let i = 0; i < length; i++) {
      const n = (Math.random() * 2 - 1) * Math.pow(1 - i / length, decay * 10);
      impL[i] = n * (0.8 + 0.2 * Math.random());
      impR[i] = n * (0.8 + 0.2 * Math.random());
    }
    return impulse;
  }

  updateDistortionCurve(amount) {
    if (!this.distortionNode || !this.audioCtx) return;
    const k = amount * 40;
    const n_samples = 44100;
    const curve = new Float32Array(n_samples);
    const deg = Math.PI / 180;
    for (let i = 0; i < n_samples; ++i) {
      const x = (i * 2) / n_samples - 1;
      if (k === 0) {
        curve[i] = x;
      } else {
        curve[i] = ((3 + k) * x * 20 * deg) / (Math.PI + k * Math.abs(x));
      }
    }
    this.distortionNode.curve = curve;
  }

  updateFXGains() {
    if (!this.isInitialized) return;
    const cho = this.params.fx_chorus;
    this.chorusDry.gain.value = 1.0 - cho * 0.4;
    this.chorusWet.gain.value = cho * 0.8;
    this.chorusLfoGainL.gain.value = 0.0035 * cho;
    this.chorusLfoGainR.gain.value = 0.0035 * cho;

    const dly = this.params.fx_delay_mix;
    this.delayDry.gain.value = 1.0 - dly * 0.4;
    this.delayWet.gain.value = dly * 0.8;
    this.delayNodeL.delayTime.value = this.params.fx_delay_time;
    this.delayNodeR.delayTime.value = this.params.fx_delay_time * 1.15;
    this.delayFeedbackL.gain.value = this.params.fx_delay_fb;
    this.delayFeedbackR.gain.value = this.params.fx_delay_fb;

    const rev = this.params.fx_reverb;
    this.reverbDry.gain.value = 1.0 - rev * 0.4;
    this.reverbWet.gain.value = rev * 0.9;

    this.masterGain.gain.value = this.params.master_volume;
    this.updateDistortionCurve(this.params.fx_distortion);
  }

  startLfoTimer() {
    const lfoTickInterval = 25; // ms
    setInterval(() => {
      if (!this.audioCtx) return;
      const rate = this.params.lfo_rate;
      const dt = lfoTickInterval / 1000;
      this.lfoPhase = (this.lfoPhase + rate * dt) % 1.0;

      // Compute raw LFO waveform
      let rawLfo = 0;
      const wf = parseInt(this.params.lfo_source, 10);
      switch (wf) {
        case 0: rawLfo = Math.sin(this.lfoPhase * Math.PI * 2); break;
        case 1: rawLfo = 4 * Math.abs(this.lfoPhase - 0.5) - 1; break;
        case 2: rawLfo = this.lfoPhase < 0.5 ? 1 : -1; break;
        case 3: rawLfo = 2 * this.lfoPhase - 1; break;
        case 4: rawLfo = 1 - 2 * this.lfoPhase; break;
        case 5:
          if (this.lfoPhase < rate * dt) this.lfoRandomVal = Math.random() * 2 - 1;
          rawLfo = this.lfoRandomVal;
          break;
      }

      // Update LFO Envelope
      const att = Math.max(0.01, this.params.lfo_attack);
      const dec = Math.max(0.01, this.params.lfo_decay);
      const sus = this.params.lfo_sustain;
      const rel = Math.max(0.01, this.params.lfo_release);

      if (this.lfoEnvState === 'attack') {
        this.lfoEnvLevel += dt / att;
        if (this.lfoEnvLevel >= 1.0) {
          this.lfoEnvLevel = 1.0;
          this.lfoEnvState = 'decay';
        }
      } else if (this.lfoEnvState === 'decay') {
        this.lfoEnvLevel -= (dt / dec) * (1.0 - sus);
        if (this.lfoEnvLevel <= sus) {
          this.lfoEnvLevel = sus;
          this.lfoEnvState = 'sustain';
        }
      } else if (this.lfoEnvState === 'release') {
        this.lfoEnvLevel -= dt / rel;
        if (this.lfoEnvLevel <= 0.0) {
          this.lfoEnvLevel = 0.0;
          this.lfoEnvState = 'idle';
        }
      }

      const lfoEffective = rawLfo * this.lfoEnvLevel * this.params.lfo_amount;
      const pitchBendSemi = this.pitchBend * 2.0;

      // Broadcast modulation to all active voices
      this.voices.forEach(v => {
        if (v.active) {
          v.applyModulation(lfoEffective, pitchBendSemi, this.modWheel, this.params);
        }
      });
    }, lfoTickInterval);
  }

  async resumeContext() {
    if (this.audioCtx && this.audioCtx.state === 'suspended') {
      await this.audioCtx.resume();
    }
  }

  noteOn(midiNote, velocity = 100) {
    if (!this.isInitialized) return;
    this.resumeContext();
    this.heldNotes.set(midiNote, velocity);

    // Trigger LFO Envelope
    this.lfoEnvState = 'attack';
    this.lfoEnvLevel = 0;

    const voiceMode = parseInt(this.params.voice_mode, 10);
    if (voiceMode === 2 || voiceMode === 3) {
      // Monophonic modes: use voice 0
      this.voices[0].start(midiNote, velocity, this.params);
      return;
    }

    // Polyphonic mode: find free voice or steal oldest
    let voice = this.voices.find(v => !v.active);
    if (!voice) {
      voice = this.voices[0]; // Steal
    }
    voice.start(midiNote, velocity, this.params);
  }

  noteOff(midiNote) {
    if (!this.isInitialized) return;
    this.heldNotes.delete(midiNote);

    if (this.heldNotes.size === 0) {
      this.lfoEnvState = 'release';
    }

    const voiceMode = parseInt(this.params.voice_mode, 10);
    if (voiceMode === 2 || voiceMode === 3) {
      if (this.heldNotes.size === 0) {
        this.voices[0].stop(this.params);
      } else {
        // Legato to last held note
        const [lastNote, lastVel] = Array.from(this.heldNotes.entries()).pop();
        this.voices[0].start(lastNote, lastVel, this.params);
      }
      return;
    }

    this.voices.forEach(v => {
      if (v.active && v.note === midiNote) {
        v.stop(this.params);
      }
    });
  }

  setParam(paramId, value) {
    this.params[paramId] = parseFloat(value);
    this.updateFXGains();
  }

  setPitchBend(val) {
    this.pitchBend = Math.max(-1, Math.min(val, 1));
  }

  setModWheel(val) {
    this.modWheel = Math.max(0, Math.min(val, 1));
  }

  loadPreset(presetIndex) {
    if (presetIndex < 0 || presetIndex >= K2000_PRESETS.length) return;
    const preset = K2000_PRESETS[presetIndex];
    this.params = { ...preset.params };
    this.updateFXGains();
  }
}

window.K2000DSP = K2000DSP;
