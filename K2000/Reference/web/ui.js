// K2000 Synthesizer — High-Fidelity UI Controller & Event Animator

document.addEventListener('DOMContentLoaded', () => {
  const dsp = new K2000DSP();
  let currentPresetIndex = 0;

  // Initialize DSP on first user interaction
  const initAudioOnFirstGesture = async () => {
    await dsp.init();
    document.removeEventListener('pointerdown', initAudioOnFirstGesture);
    document.removeEventListener('keydown', initAudioOnFirstGesture);
  };
  document.addEventListener('pointerdown', initAudioOnFirstGesture, { once: true });
  document.addEventListener('keydown', initAudioOnFirstGesture, { once: true });

  // =========================================================================
  // 1. ROTARY KNOBS BINDING & REAL-TIME ANIMATION
  // =========================================================================
  const knobs = document.querySelectorAll('.knob');
  
  knobs.forEach(knob => {
    const paramId = knob.dataset.param;
    const min = parseFloat(knob.dataset.min || 0);
    const max = parseFloat(knob.dataset.max || 1);
    const def = parseFloat(knob.dataset.default || min);
    const isLog = knob.dataset.curve === 'log';
    
    let currentVal = dsp.params[paramId] !== undefined ? dsp.params[paramId] : def;

    const valToNorm = (v) => {
      if (isLog) {
        return (Math.log(v) - Math.log(min)) / (Math.log(max) - Math.log(min));
      }
      return (v - min) / (max - min);
    };

    const normToVal = (n) => {
      const clamped = Math.max(0, Math.min(n, 1));
      if (isLog) {
        return Math.exp(Math.log(min) + clamped * (Math.log(max) - Math.log(min)));
      }
      return min + clamped * (max - min);
    };

    const updateKnobVisual = (val) => {
      const norm = valToNorm(val);
      const angle = -135 + norm * 270;
      
      // Rotate inner knob notch
      const body = knob.querySelector('.knob-body');
      if (body) {
        body.style.transform = `rotate(${angle}deg)`;
      }

      // Update SVG Cyan Arc
      const arcVal = knob.querySelector('.knob-arc-val');
      if (arcVal) {
        const fullLength = 74; // dasharray length for 270 deg
        const offset = 98 - (norm * fullLength);
        arcVal.style.strokeDashoffset = offset;
      }
    };

    updateKnobVisual(currentVal);

    // Pointer Drag Interaction
    let startY = 0;
    let startVal = 0;

    const onPointerMove = (e) => {
      const deltaY = startY - e.clientY;
      const sensitivity = 0.005;
      const currentNorm = valToNorm(startVal);
      const newNorm = Math.max(0, Math.min(1, currentNorm + deltaY * sensitivity));
      const newVal = normToVal(newNorm);
      
      dsp.setParam(paramId, newVal);
      updateKnobVisual(newVal);
    };

    const onPointerUp = () => {
      window.removeEventListener('pointermove', onPointerMove);
      window.removeEventListener('pointerup', onPointerUp);
    };

    knob.addEventListener('pointerdown', (e) => {
      startY = e.clientY;
      startVal = dsp.params[paramId] !== undefined ? dsp.params[paramId] : def;
      window.addEventListener('pointermove', onPointerMove);
      window.addEventListener('pointerup', onPointerUp);
    });

    // Mouse Wheel
    knob.addEventListener('wheel', (e) => {
      e.preventDefault();
      const currentNorm = valToNorm(dsp.params[paramId]);
      const step = e.deltaY < 0 ? 0.04 : -0.04;
      const newNorm = Math.max(0, Math.min(1, currentNorm + step));
      const newVal = normToVal(newNorm);
      dsp.setParam(paramId, newVal);
      updateKnobVisual(newVal);
    });

    // Double Click Reset
    knob.addEventListener('dblclick', () => {
      dsp.setParam(paramId, def);
      updateKnobVisual(def);
    });
  });

  // =========================================================================
  // 2. VERTICAL FADERS BINDING & ANIMATION
  // =========================================================================
  const faders = document.querySelectorAll('.fader-track-wrap');

  faders.forEach(wrap => {
    const paramId = wrap.dataset.param;
    const min = parseFloat(wrap.dataset.min || 0);
    const max = parseFloat(wrap.dataset.max || 1);
    const def = parseFloat(wrap.dataset.default || min);
    const cap = wrap.querySelector('.fader-cap');
    const ticks = wrap.querySelectorAll('.fader-tick');

    const updateFaderVisual = (val) => {
      const norm = (val - min) / (max - min);
      const clamped = Math.max(0, Math.min(norm, 1));
      
      // Position cap from 100% (bottom) to 0% (top)
      const topPercent = (1 - clamped) * 100;
      if (cap) cap.style.top = `${topPercent}%`;

      // Illuminate LED Ticks below cap
      const numTicks = ticks.length;
      ticks.forEach((tick, i) => {
        const tickNorm = 1 - (i / (numTicks - 1));
        if (tickNorm <= clamped + 0.05) {
          tick.classList.add('lit');
        } else {
          tick.classList.remove('lit');
        }
      });
    };

    const initialVal = dsp.params[paramId] !== undefined ? dsp.params[paramId] : def;
    updateFaderVisual(initialVal);

    const handleFaderEvent = (e) => {
      const rect = wrap.getBoundingClientRect();
      const offsetY = e.clientY - rect.top;
      const norm = 1 - (offsetY / rect.height);
      const clamped = Math.max(0, Math.min(1, norm));
      const val = min + clamped * (max - min);

      dsp.setParam(paramId, val);
      updateFaderVisual(val);
    };

    let isFaderDragging = false;

    wrap.addEventListener('pointerdown', (e) => {
      isFaderDragging = true;
      handleFaderEvent(e);
      window.addEventListener('pointermove', onFaderMove);
      window.addEventListener('pointerup', onFaderUp);
    });

    const onFaderMove = (e) => {
      if (isFaderDragging) handleFaderEvent(e);
    };

    const onFaderUp = () => {
      isFaderDragging = false;
      window.removeEventListener('pointermove', onFaderMove);
      window.removeEventListener('pointerup', onFaderUp);
    };
  });

  // =========================================================================
  // 3. PITCH TOGGLE SWITCH BINDING
  // =========================================================================
  const pitchSwitch = document.querySelector('.switch-group');
  if (pitchSwitch) {
    pitchSwitch.addEventListener('click', () => {
      const bat = pitchSwitch.querySelector('.switch-bat');
      const led = pitchSwitch.querySelector('.switch-led');
      const current = dsp.params.lfo_pitch === 1;
      const next = current ? 0 : 1;
      dsp.setParam('lfo_pitch', next);

      if (next === 1) {
        bat.classList.remove('off');
        bat.classList.add('on');
        led.classList.add('active');
      } else {
        bat.classList.remove('on');
        bat.classList.add('off');
        led.classList.remove('active');
      }
    });
  }

  // =========================================================================
  // 4. PERFORMANCE WHEELS (PITCH BEND & MODULATION)
  // =========================================================================
  const pitchDrum = document.getElementById('pitch-drum');
  const modDrum = document.getElementById('mod-drum');

  // Pitch Bend (Spring-back on release)
  if (pitchDrum) {
    const indexLine = pitchDrum.querySelector('.wheel-index-line');
    let startY = 0;
    let isPitchDragging = false;

    const onPitchMove = (e) => {
      if (!isPitchDragging) return;
      const deltaY = startY - e.clientY;
      const norm = Math.max(-1, Math.min(1, deltaY / 40));
      dsp.setPitchBend(norm);
      indexLine.style.top = `${50 - norm * 35}%`;
    };

    const onPitchUp = () => {
      isPitchDragging = false;
      dsp.setPitchBend(0);
      indexLine.style.transition = 'top 0.15s cubic-bezier(0.2, 0.9, 0.4, 1)';
      indexLine.style.top = '50%';
      setTimeout(() => { indexLine.style.transition = ''; }, 150);
      window.removeEventListener('pointermove', onPitchMove);
      window.removeEventListener('pointerup', onPitchUp);
    };

    pitchDrum.addEventListener('pointerdown', (e) => {
      isPitchDragging = true;
      startY = e.clientY;
      window.addEventListener('pointermove', onPitchMove);
      window.addEventListener('pointerup', onPitchUp);
    });
  }

  // Modulation Wheel (Latching)
  if (modDrum) {
    const indexLine = modDrum.querySelector('.wheel-index-line');
    let startY = 0;
    let startVal = 0;
    let isModDragging = false;

    const onModMove = (e) => {
      if (!isModDragging) return;
      const deltaY = startY - e.clientY;
      const newVal = Math.max(0, Math.min(1, startVal + deltaY / 80));
      dsp.setModWheel(newVal);
      indexLine.style.top = `${90 - newVal * 80}%`;
    };

    const onModUp = () => {
      isModDragging = false;
      window.removeEventListener('pointermove', onModMove);
      window.removeEventListener('pointerup', onModUp);
    };

    modDrum.addEventListener('pointerdown', (e) => {
      isModDragging = true;
      startY = e.clientY;
      startVal = dsp.modWheel;
      window.addEventListener('pointermove', onModMove);
      window.addEventListener('pointerup', onModUp);
    });
  }

  // =========================================================================
  // 5. CYBER DROPDOWNS BINDING
  // =========================================================================
  const selects = document.querySelectorAll('.cyber-select');
  selects.forEach(select => {
    select.addEventListener('change', (e) => {
      const paramId = select.dataset.param;
      dsp.setParam(paramId, parseInt(e.target.value, 10));
    });
  });

  // =========================================================================
  // 6. PATCH BROWSER & MODAL
  // =========================================================================
  const patchDisplay = document.getElementById('patch-title');
  const prevBtn = document.getElementById('btn-prev-patch');
  const nextBtn = document.getElementById('btn-next-patch');
  const browseBtn = document.getElementById('btn-browse-patch');
  const saveBtn = document.getElementById('btn-save-patch');
  const cassetteBay = document.getElementById('cassette-bay');

  const presetModal = document.getElementById('preset-modal');
  const presetListContainer = document.getElementById('preset-list-container');
  const modalCloseBtn = document.getElementById('modal-close-btn');

  const updateAllUIControls = () => {
    // Update Knobs
    knobs.forEach(knob => {
      const paramId = knob.dataset.param;
      const min = parseFloat(knob.dataset.min || 0);
      const max = parseFloat(knob.dataset.max || 1);
      const isLog = knob.dataset.curve === 'log';
      const val = dsp.params[paramId];

      const valToNorm = (v) => {
        if (isLog) return (Math.log(v) - Math.log(min)) / (Math.log(max) - Math.log(min));
        return (v - min) / (max - min);
      };

      const norm = valToNorm(val);
      const angle = -135 + norm * 270;
      const body = knob.querySelector('.knob-body');
      if (body) body.style.transform = `rotate(${angle}deg)`;
      const arcVal = knob.querySelector('.knob-arc-val');
      if (arcVal) {
        arcVal.style.strokeDashoffset = 98 - (norm * 74);
      }
    });

    // Update Faders
    faders.forEach(wrap => {
      const paramId = wrap.dataset.param;
      const min = parseFloat(wrap.dataset.min || 0);
      const max = parseFloat(wrap.dataset.max || 1);
      const val = dsp.params[paramId];
      const norm = (val - min) / (max - min);
      const cap = wrap.querySelector('.fader-cap');
      if (cap) cap.style.top = `${(1 - norm) * 100}%`;
      const ticks = wrap.querySelectorAll('.fader-tick');
      ticks.forEach((tick, i) => {
        const tickNorm = 1 - (i / (ticks.length - 1));
        if (tickNorm <= norm + 0.05) tick.classList.add('lit');
        else tick.classList.remove('lit');
      });
    });

    // Update Selects
    selects.forEach(select => {
      const paramId = select.dataset.param;
      select.value = dsp.params[paramId];
    });

    // Update Switch
    if (pitchSwitch) {
      const bat = pitchSwitch.querySelector('.switch-bat');
      const led = pitchSwitch.querySelector('.switch-led');
      if (dsp.params.lfo_pitch === 1) {
        bat.classList.remove('off'); bat.classList.add('on'); led.classList.add('active');
      } else {
        bat.classList.remove('on'); bat.classList.add('off'); led.classList.remove('active');
      }
    }
  };

  const loadPresetByIndex = (idx) => {
    currentPresetIndex = (idx + K2000_PRESETS.length) % K2000_PRESETS.length;
    dsp.loadPreset(currentPresetIndex);
    patchDisplay.textContent = K2000_PRESETS[currentPresetIndex].name;
    updateAllUIControls();
  };

  if (prevBtn) prevBtn.addEventListener('click', () => loadPresetByIndex(currentPresetIndex - 1));
  if (nextBtn) nextBtn.addEventListener('click', () => loadPresetByIndex(currentPresetIndex + 1));

  const openPresetModal = () => {
    presetListContainer.innerHTML = '';
    K2000_PRESETS.forEach((preset, idx) => {
      const item = document.createElement('div');
      item.className = `preset-item ${idx === currentPresetIndex ? 'active' : ''}`;
      item.innerHTML = `<span>${preset.name}</span><span style="color:#00f0ff;font-size:10px;">${preset.category}</span>`;
      item.addEventListener('click', () => {
        loadPresetByIndex(idx);
        presetModal.classList.remove('open');
      });
      presetListContainer.appendChild(item);
    });
    presetModal.classList.add('open');
  };

  if (browseBtn) browseBtn.addEventListener('click', openPresetModal);
  if (cassetteBay) cassetteBay.addEventListener('click', openPresetModal);
  if (modalCloseBtn) modalCloseBtn.addEventListener('click', () => presetModal.classList.remove('open'));

  // Save / Export Preset
  if (saveBtn) {
    saveBtn.addEventListener('click', () => {
      const currentPresetData = {
        name: prompt("Nom du Preset à sauvegarder :", patchDisplay.textContent) || "Custom Patch",
        category: "User",
        params: { ...dsp.params }
      };
      const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(currentPresetData, null, 2));
      const dl = document.createElement('a');
      dl.setAttribute("href", dataStr);
      dl.setAttribute("download", `${currentPresetData.name.replace(/\s+/g, '_')}.json`);
      dl.click();
    });
  }

  // =========================================================================
  // 7. 25-KEY VIRTUAL KEYBOARD & MIDI
  // =========================================================================
  const keys = document.querySelectorAll('.key-white, .key-black');

  keys.forEach(key => {
    const note = parseInt(key.dataset.note, 10);

    key.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      key.classList.add('active');
      dsp.noteOn(note, 110);
    });

    key.addEventListener('pointerup', () => {
      key.classList.remove('active');
      dsp.noteOff(note);
    });

    key.addEventListener('pointerleave', () => {
      if (key.classList.contains('active')) {
        key.classList.remove('active');
        dsp.noteOff(note);
      }
    });
  });

  // Computer QWERTY Keyboard Mapping
  const keyboardMap = {
    'KeyA': 48, // C3
    'KeyW': 49, // C#3
    'KeyS': 50, // D3
    'KeyE': 51, // D#3
    'KeyD': 52, // E3
    'KeyF': 53, // F3
    'KeyT': 54, // F#3
    'KeyG': 55, // G3
    'KeyY': 56, // G#3
    'KeyH': 57, // A3
    'KeyU': 58, // A#3
    'KeyJ': 59, // B3
    'KeyK': 60, // C4
    'KeyO': 61, // C#4
    'KeyL': 62, // D4
    'KeyP': 63, // D#4
    'Semicolon': 64 // E4
  };

  const activeKeys = new Set();

  window.addEventListener('keydown', (e) => {
    if (e.repeat) return;
    const note = keyboardMap[e.code];
    if (note !== undefined && !activeKeys.has(e.code)) {
      activeKeys.add(e.code);
      const keyEl = document.querySelector(`[data-note="${note}"]`);
      if (keyEl) keyEl.classList.add('active');
      dsp.noteOn(note, 110);
    }
  });

  window.addEventListener('keyup', (e) => {
    const note = keyboardMap[e.code];
    if (note !== undefined) {
      activeKeys.delete(e.code);
      const keyEl = document.querySelector(`[data-note="${note}"]`);
      if (keyEl) keyEl.classList.remove('active');
      dsp.noteOff(note);
    }
  });

  // Web MIDI API Auto-connect
  if (navigator.requestMIDIAccess) {
    navigator.requestMIDIAccess().then(midiAccess => {
      const inputs = midiAccess.inputs.values();
      for (const input of inputs) {
        input.onmidimessage = (msg) => {
          const [status, data1, data2] = msg.data;
          const cmd = status >> 4;
          if (cmd === 9 && data2 > 0) { // Note On
            const keyEl = document.querySelector(`[data-note="${data1}"]`);
            if (keyEl) keyEl.classList.add('active');
            dsp.noteOn(data1, data2);
          } else if (cmd === 8 || (cmd === 9 && data2 === 0)) { // Note Off
            const keyEl = document.querySelector(`[data-note="${data1}"]`);
            if (keyEl) keyEl.classList.remove('active');
            dsp.noteOff(data1);
          } else if (cmd === 14) { // Pitch Bend
            const bendVal = ((data2 << 7) + data1 - 8192) / 8192;
            dsp.setPitchBend(bendVal);
          } else if (cmd === 11 && data1 === 1) { // CC1 Mod Wheel
            dsp.setModWheel(data2 / 127);
          }
        };
      }
    }).catch(() => {});
  }
});
