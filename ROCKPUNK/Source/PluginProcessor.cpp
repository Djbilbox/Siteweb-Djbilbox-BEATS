#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace rk
{

namespace
{
    constexpr const char* kPresetNameProperty = "presetName";
    constexpr const char* kUiWidthProperty    = "uiWidth";
    constexpr const char* kUiHeightProperty   = "uiHeight";
}

PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "ROCKPUNK_GUITAR_STATE", createGuitarParameterLayout())
{
    fillGuitarParams (params, apvts);

    prepareUserSampleFolder();
    bank.openDefault();
    seedFactoryPresets();
    refreshPresetList();

    synth.addSound (new GuitarSound());
    synth.setNoteStealingEnabled (true);
    sampleSynth.addSound (new SamplerSound());
    sampleSynth.setNoteStealingEnabled (true);

    // The engine is fully built here, at a default rate, and not only in
    // prepareToPlay: a host (or a test) that sends a note before it has called
    // prepareToPlay must find voices with allocated string buffers, not empty
    // ones.
    setupEngine (48000.0);

    loader.startThread (juce::Thread::Priority::background);
}

PluginProcessor::~PluginProcessor()
{
    loader.stopThread (4000);
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void PluginProcessor::setupEngine (double sampleRate)
{
    preparedRate = sampleRate > 0.0 ? sampleRate : 48000.0;
    const double sr = preparedRate;

    synth.clearVoices();
    guitarVoices.clear();
    for (int i = 0; i < 8; ++i)
    {
        auto* voice = new GuitarVoice();
        voice->setSampleRate (sr);
        guitarVoices.push_back (voice);
        synth.addVoice (voice);
    }
    synth.setCurrentPlaybackSampleRate (sr);

    sampleSynth.clearVoices();
    samplerVoices.clear();
    for (int i = 0; i < 8; ++i)
    {
        auto* sv = new SamplerVoice();
        sv->setSampleRate (sr);
        samplerVoices.push_back (sv);
        sampleSynth.addVoice (sv);
    }
    sampleSynth.setCurrentPlaybackSampleRate (sr);

    amp.setSampleRate (sr);
    drive.setSampleRate (sr);
    delay.setSampleRate (sr);
    delay.prepare (Delay::kMaxDelayMs);

    amp.reset();
    drive.reset();
    delay.reset();

    reverb.setSampleRate (sr);
    reverb.reset();

    lfo.setSampleRate (sr);
    lfo.reset();
    lfoValue = 0.0f;

    // Cut-off around 20 Hz: below the first partial of a guitar, so inaudible on
    // the useful sound.
    dcCoeff = (float) std::exp (-2.0 * juce::MathConstants<double>::pi * 20.0 / sr);
    dcInL = dcOutL = dcInR = dcOutR = 0.0f;

    filterL.setSampleRate (sr);
    filterR.setSampleRate (sr);
    filterL.reset();
    filterR.reset();

    filterEnv.setSampleRate (sr);
    filterEnv.reset();
    anyNoteHeld = false;

    // 40 ms of line: room for the chorus's mean delay and its excursion at any
    // sample rate.
    const int chorusLen = (int) (sr * 0.04) + 4;
    chorusL.assign ((size_t) chorusLen, 0.0f);
    chorusR.assign ((size_t) chorusLen, 0.0f);
    chorusWrite = 0;
    chorusPhase = 0.0f;

    masterVolSmooth.reset (sr, 0.03);
    widthSmooth.reset (sr, 0.03);
    lfoGainSmooth.reset (sr, 0.01);
    ampGainSmooth.reset (sr, 0.03);
    ampDistSmooth.reset (sr, 0.03);
    ampLevelSmooth.reset (sr, 0.03);
    driveSmooth.reset (sr, 0.03);
    cutoffSmooth.reset (sr, 0.03);

    masterVolSmooth.setCurrentAndTargetValue (dbToGain (params.masterVol.get()));
    widthSmooth.setCurrentAndTargetValue (params.outWidth.get01());
    lfoGainSmooth.setCurrentAndTargetValue (1.0f);
    ampGainSmooth.setCurrentAndTargetValue (params.ampGain.get());
    ampDistSmooth.setCurrentAndTargetValue (params.ampDist.get01());
    ampLevelSmooth.setCurrentAndTargetValue (params.ampLevel.get());
    driveSmooth.setCurrentAndTargetValue (params.driveAmt.get01());
    cutoffSmooth.setCurrentAndTargetValue (juce::jmax (20.0f, params.fltCutoff.get()));

    currentBpm = 120.0f;
    delayTimeMs = params.delayTime.get();

    // Scratch space reserved here so that processBlock never allocates.
    chunkMidi.ensureSize (8192);
    monoMidi.ensureSize (8192);
    hostSliceMidi.ensureSize (8192);
    hostMonoL.assign ((size_t) kHostMonoChunk, 0.0f);
    hostMonoR.assign ((size_t) kHostMonoChunk, 0.0f);

    noteBits[0] = 0;
    noteBits[1] = 0;
    monoNote = -1;
    audioSampleVersion = -1;      // make the audio thread pick the sample up again
    bypassFlushed = false;
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    setupEngine (sampleRate);
    pollSampleSelection();
}

void PluginProcessor::releaseResources()
{
}

// Silence every voice and clear the effect tails. Used when the host bypasses
// the plug-in and when a non-finite value shows up in the signal.
void PluginProcessor::silenceEngine()
{
    synth.allNotesOff (0, false);
    sampleSynth.allNotesOff (0, false);

    amp.reset();
    drive.reset();
    delay.reset();
    reverb.reset();
    filterL.reset();
    filterR.reset();
    filterEnv.reset();
    anyNoteHeld = false;
    dcInL = dcOutL = dcInR = dcOutR = 0.0f;
    std::fill (chorusL.begin(), chorusL.end(), 0.0f);
    std::fill (chorusR.begin(), chorusR.end(), 0.0f);

    noteBits[0] = 0;
    noteBits[1] = 0;
    monoNote = -1;
}

//==============================================================================
void PluginProcessor::updateVoices()
{
    const float mix  = params.layerMix.get01();
    const float toneA = 0.08f + 0.92f * params.laTone.get01();
    const float toneB = 0.08f + 0.92f * params.lbTone.get01();
    const float gA    = 0.98f + 0.0195f * params.laSustain.get01();
    const float gB    = 0.98f + 0.0195f * params.lbSustain.get01();
    const float pickA = 0.15f + 0.85f * params.laPick.get01();
    const float pickB = 0.15f + 0.85f * params.lbPick.get01();

    const float envA = params.envA.get();
    const float envD = params.envD.get();
    const float envS = params.envS.get01();
    const float envR = params.envR.get();
    const float glide = params.glideMs.get();
    const float velAmt = params.velAmt.get01();
    const int   pick = params.pickShape.getInt();

    // Pitch: LFO vibrato (the PITCH knob, opened further by the MODULATION
    // wheel or by MIDI CC1) plus the PITCH BEND wheel and MIDI pitch bend. Two
    // semitones of vibrato at most, which is still a guitar and not a siren.
    const float vibratoDepth = params.lfoPitch.get01() + juce::jmax (params.modDepth.get01(), midiMod);
    const float pitchOffset = params.pbend.get() + 2.0f * midiBend + 2.0f * vibratoDepth * lfoValue;

    for (auto* gv : guitarVoices)
    {
        gv->setLayerA (params.laOn.getBool(), toneA, gA, params.laBlend.get01(),
                       params.laBody.get01(), params.laLevel.get(), pickA);
        gv->setLayerB (params.lbOn.getBool(), toneB, gB, params.lbBlend.get01(),
                       params.lbBody.get01(), params.lbLevel.get(), pickB);
        gv->setLayerMix (mix);
        gv->setEnvelope (envA, envD, envS, envR);
        gv->setGlideMs (glide);
        gv->setPitchOffset (pitchOffset);
        gv->setVelocityAmount (velAmt);
        gv->setPickShape (pick);
    }
}

//==============================================================================
void PluginProcessor::sliceMidi (const juce::MidiBuffer& src, juce::MidiBuffer& dst, int start, int length,
                                 bool isFirst, bool isLast)
{
    // The events that fall in [start, start + length). Nothing may be lost at the
    // ends: an event stamped before the block (first piece) or at or beyond its
    // end (last piece) is played at the nearest valid position, because a
    // dropped note-off is a stuck note.
    dst.clear();
    auto it = isFirst ? src.cbegin() : src.findNextSamplePosition (start);
    for (; it != src.cend(); ++it)
    {
        const auto meta = *it;
        if (! isLast && meta.samplePosition >= start + length)
            break;
        dst.addEvent (meta.data, meta.numBytes, juce::jlimit (0, length - 1, meta.samplePosition - start));
    }
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    bypassFlushed = false;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numChannels <= 0 || numSamples <= 0)
        return;

    buffer.clear();

    if (numChannels >= 2)
    {
        renderBlock (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples, midi);
        return;
    }

    // A host that insists on a mono output: render in stereo into scratch space,
    // in pieces that fit it, and fold down. The effects run on two channels and
    // must never be handed the same memory twice.
    float* mono = buffer.getWritePointer (0);
    const int maxChunk = (int) hostMonoL.size();
    if (maxChunk <= 0)
        return;

    for (int start = 0; start < numSamples; start += maxChunk)
    {
        const int n = juce::jmin (maxChunk, numSamples - start);
        sliceMidi (midi, hostSliceMidi, start, n, start == 0, start + n >= numSamples);

        std::fill (hostMonoL.begin(), hostMonoL.begin() + n, 0.0f);
        std::fill (hostMonoR.begin(), hostMonoR.begin() + n, 0.0f);
        renderBlock (hostMonoL.data(), hostMonoR.data(), n, hostSliceMidi);

        for (int i = 0; i < n; ++i)
            mono[start + i] = 0.5f * (hostMonoL[(size_t) i] + hostMonoR[(size_t) i]);
    }
}

void PluginProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    // Keys clicked on the on-screen keyboard while bypassed are consumed here
    // and thrown away with the host's events: left queued, they would all fire
    // at once on the first block after the bypass is lifted.
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);
    midi.clear();

    if (! bypassFlushed)
    {
        silenceEngine();
        bypassFlushed = true;
    }
}

void PluginProcessor::renderBlock (float* left, float* right, int numSamples, juce::MidiBuffer& midi)
{
    // ---- host tempo for the synced delay ----
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())
                currentBpm = (float) juce::jmax (1.0, *bpm);
        }
    }

    // ---- delay time (free ms or tempo-synced) ----
    {
        float target = params.delayTime.get();
        if (params.delaySync.getBool())
            target = divisionToMs (currentBpm, params.delayDiv.getInt());
        delayTimeMs = target;
    }

    // Merge the on-screen keyboard into the buffer, then read the controllers and
    // follow every note (external MIDI and piano clicks alike) so the UI can
    // light the keys.
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
    scanMidiControllers (midi);

    if (params.monoMode.getBool())
        applyMonoMode (midi);
    else
        monoNote = -1;

    trackNotes (midi);
    updateSamplerVoices();

    // Rendered in short pieces so that vibrato, LFO and the controls follow
    // smoothly whatever buffer size the host uses. Each piece only receives the
    // MIDI events that fall inside it: juce::Synthesiser handles every event
    // that remains in the buffer it is given.
    for (int start = 0; start < numSamples; start += kSubBlock)
    {
        const int n = juce::jmin (kSubBlock, numSamples - start);

        static const int waveToShape[] = { 0, 1, 3, 4 };   // sine / triangle / square / S&H
        lfo.setFrequency (params.lfoRate.get());
        lfo.setShape (waveToShape[juce::jlimit (0, 3, params.lfoWave.getInt())]);
        for (int i = 0; i < n; ++i)
            lfoValue = lfo.process();

        updateVoices();

        float* chans[2] = { left + start, right + start };
        juce::AudioBuffer<float> view (chans, 2, n);

        sliceMidi (midi, chunkMidi, start, n, start == 0, start + n >= numSamples);
        synth.renderNextBlock (view, chunkMidi, 0, n);
        sampleSynth.renderNextBlock (view, chunkMidi, 0, n);

        applyMasterFx (chans[0], chans[1], n);
    }
}

//==============================================================================
void PluginProcessor::loadSample()
{
    const juce::ScopedLock ll (loadLock);

    const int sel  = params.smpSel.getInt();
    const int root = params.smpRoot.getInt();
    const int previousSel = lastSampleSel.exchange (sel);
    lastSampleRoot = root;

    // Only the root offset moved: the file is already in memory.
    if (previousSel == sel)
    {
        const juce::ScopedLock sl (sampleLock);
        if (sampleData != nullptr)
        {
            sampleRootNote = sampleBaseRoot + (float) root;
            ++sampleVersion;
            return;
        }
    }

    std::shared_ptr<juce::AudioBuffer<float>> newData;
    double newRate = 44100.0;
    float  newRoot = 60.0f;
    juce::String newName;

    const int count = bank.getNumEntries();
    if (count > 0)
    {
        const int idx = juce::jlimit (0, count - 1, sel);

        SampleBank::Entry e;
        if (bank.getEntryInfo (idx, e))
            newName = e.name;

        // The fundamental was measured when the bank was built (or on first
        // load for a loose file); smpRoot only shifts it, to rescue a
        // detection that landed beside the note.
        newData = bank.load (idx, newRate, newRoot);
    }

    const juce::ScopedLock sl (sampleLock);
    sampleHistory.push_back (newData);
    while (sampleHistory.size() > 3)
        sampleHistory.erase (sampleHistory.begin());

    sampleData        = std::move (newData);
    sampleFileRate    = newRate;
    sampleBaseRoot    = newRoot;
    sampleRootNote    = newRoot + (float) root;
    currentSampleName = newName;
    ++sampleVersion;
}

void PluginProcessor::pollSampleSelection()
{
    if (! params.smpOn.getBool())
        return;

    if (params.smpSel.getInt() != lastSampleSel.load() || params.smpRoot.getInt() != lastSampleRoot.load())
        loadSample();
}

juce::String PluginProcessor::getCurrentSampleName() const
{
    const juce::ScopedLock sl (sampleLock);
    return currentSampleName;
}

float PluginProcessor::getSampleRootNote() const
{
    const juce::ScopedLock sl (sampleLock);
    return sampleRootNote;
}

//==============================================================================
void PluginProcessor::updateSamplerVoices()
{
    // Pick up a newly loaded file. try-lock: the audio thread never waits, it
    // simply looks again on the next block. All sampler voices point into the
    // old buffer, so they are cut before they are re-pointed.
    {
        const juce::ScopedTryLock stl (sampleLock);
        if (stl.isLocked() && audioSampleVersion != sampleVersion)
        {
            audioSample        = sampleData;
            audioFileRate      = sampleFileRate;
            audioRoot          = sampleRootNote;
            audioSampleVersion = sampleVersion;
            sampleSynth.allNotesOff (0, false);
        }
    }

    const bool  on   = params.smpOn.getBool();
    const bool  loop = params.smpLoop.getBool();
    const float atk  = params.smpAttack.get();
    const float rel  = params.smpRelease.get();
    const float lvl  = params.smpLevel.get();

    for (auto* sv : samplerVoices)
    {
        sv->setSharedSample (on ? audioSample : nullptr, audioFileRate, audioRoot, loop, atk, rel);
        sv->setLevel (lvl);
    }
}

//==============================================================================
// [MONO/POLY] on the front panel. juce::Synthesiser is polyphonic by nature:
// mono is rendered by cutting the held note just before each new attack, which
// also lets the glide join the two pitches. Only the most recent note can be
// held, so one integer is all the state needed - and nothing is allocated.
void PluginProcessor::applyMonoMode (juce::MidiBuffer& midi)
{
    monoMidi.clear();

    for (const auto meta : midi)
    {
        const auto* d = meta.data;
        const int n = meta.numBytes;
        const int pos = meta.samplePosition;
        const int status = n > 0 ? (d[0] & 0xF0) : 0;

        if (n >= 3 && status == 0x90 && d[2] > 0)
        {
            const int note = d[1];
            if (monoNote >= 0 && monoNote != note)
            {
                const juce::uint8 off[3] = { (juce::uint8) (0x80 | (d[0] & 0x0F)), (juce::uint8) monoNote, 0 };
                monoMidi.addEvent (off, 3, pos);
            }
            monoNote = note;
        }
        else if (n >= 3 && (status == 0x80 || status == 0x90))
        {
            if (d[1] == monoNote)
                monoNote = -1;
        }
        else if (n >= 3 && status == 0xB0 && (d[1] == 120 || d[1] == 123))
        {
            monoNote = -1;
        }

        monoMidi.addEvent (d, n, pos);
    }

    midi.clear();
    midi.addEvents (monoMidi, 0, -1, 0);
}

//==============================================================================
void PluginProcessor::setNoteBit (int note, bool on) noexcept
{
    if (note < 0 || note > 127)
        return;

    const auto bit = (juce::uint64) 1 << (note & 63);
    if (on)
        noteBits[(size_t) (note >> 6)].fetch_or (bit);
    else
        noteBits[(size_t) (note >> 6)].fetch_and (~bit);
}

void PluginProcessor::trackNotes (const juce::MidiBuffer& midi)
{
    for (const auto meta : midi)
    {
        const auto* d = meta.data;
        if (meta.numBytes < 3)
            continue;

        const int status = d[0] & 0xF0;
        if (status == 0x90 && d[2] > 0)
            setNoteBit (d[1], true);
        else if (status == 0x80 || status == 0x90)
            setNoteBit (d[1], false);
    }
}

void PluginProcessor::scanMidiControllers (const juce::MidiBuffer& midi)
{
    for (const auto meta : midi)
    {
        const auto* d = meta.data;
        if (meta.numBytes < 3)
            continue;

        const int status = d[0] & 0xF0;

        if (status == 0xB0)
        {
            const int cc = d[1];
            if (cc == 1)
            {
                midiMod = (float) d[2] / 127.0f;
            }
            else if (cc == 121)                 // reset all controllers
            {
                midiMod = 0.0f;
                midiBend = 0.0f;
            }
            else if (cc == 120 || cc == 123)    // all sound off / all notes off
            {
                // juce::Synthesiser lets both of them ring out through their
                // release. "All sound off" is the panic button and must cut at
                // once, whatever the RELEASE fader says.
                if (cc == 120)
                {
                    synth.allNotesOff (0, false);
                    sampleSynth.allNotesOff (0, false);
                }

                noteBits[0] = 0;
                noteBits[1] = 0;
            }
        }
        else if (status == 0xE0)
        {
            const int value = (d[2] << 7) | d[1];
            midiBend = juce::jlimit (-1.0f, 1.0f, (float) (value - 8192) / 8192.0f);
        }
    }
}

std::set<int> PluginProcessor::getActiveNotesSnapshot() const
{
    std::set<int> notes;
    for (int word = 0; word < 2; ++word)
    {
        const auto bits = noteBits[(size_t) word].load();
        for (int b = 0; b < 64; ++b)
            if ((bits >> b) & 1u)
                notes.insert (word * 64 + b);
    }
    return notes;
}

//==============================================================================
void PluginProcessor::applyMasterFx (float* dataL, float* dataR, int numSamples)
{
    const float ampTone  = params.ampTone.get();
    const bool  ampOn    = params.ampOn.getBool();

    const int   driveType = params.driveType.getInt();
    const float driveTone = params.driveTone.get();

    const bool  delayActive = params.delayOn.getBool();
    const float delayFb     = params.delayFb.get01();
    const float delayMix    = params.delayMix.get01();
    const float delayLP     = params.delayLP.get();

    masterVolSmooth.setTargetValue (dbToGain (params.masterVol.get()));
    widthSmooth.setTargetValue (params.outWidth.get01());
    ampGainSmooth.setTargetValue (params.ampGain.get());
    ampDistSmooth.setTargetValue (params.ampDist.get01());
    ampLevelSmooth.setTargetValue (params.ampLevel.get());
    cutoffSmooth.setTargetValue (juce::jmax (20.0f, params.fltCutoff.get()));

    const bool limiterOn = params.outLimiter.getBool();

    // ---- LFO towards the destinations other than pitch ----
    const int   lfoDest = params.lfoDest.getInt();
    const float lfoAmt  = params.lfoAmount.get01();

    float driveAmtMod = params.driveAmt.get01();
    float lfoGain = 1.0f;
    float cutoffMod = 1.0f;

    switch (lfoDest)
    {
        case 1:  cutoffMod   = std::pow (2.0f, 2.5f * lfoAmt * lfoValue); break;  // filter
        case 2:  lfoGain     = 1.0f - 0.5f * lfoAmt * (0.5f + 0.5f * lfoValue); break; // volume
        case 3:  driveAmtMod = juce::jlimit (0.0f, 1.0f, driveAmtMod + lfoAmt * lfoValue); break;
        default: break;   // 0 = pitch, already applied in updateVoices
    }

    driveSmooth.setTargetValue (driveAmtMod);
    lfoGainSmooth.setTargetValue (lfoGain);

    // ---- amp + drive (per sample, in place) ----
    for (int i = 0; i < numSamples; ++i)
    {
        amp.processStereo (dataL[i], dataR[i], ampGainSmooth.getNextValue(), ampDistSmooth.getNextValue(),
                           ampTone, ampLevelSmooth.getNextValue(), ampOn);
        drive.processStereo (dataL[i], dataR[i], driveSmooth.getNextValue(), driveType, driveTone);

        // Removes the DC produced by the two saturation stages.
        dcOutL = dataL[i] - dcInL + dcCoeff * dcOutL;
        dcInL  = dataL[i];
        dataL[i] = dcOutL;

        dcOutR = dataR[i] - dcInR + dcCoeff * dcOutR;
        dcInR  = dataR[i];
        dataR[i] = dcOutR;
    }

    // ---- filter (FILTER knob + [FILTER TYPE] selector) ----
    // Type 3 takes the filter out of the path: without it, a low-pass at 14 kHz
    // would stay in the chain and bring nothing.
    const int filterType = params.fltType.getInt();
    if (filterType < 3)
    {
        juce::ADSR::Parameters fp;
        fp.attack  = params.fenvA.get() * 0.001f;
        fp.decay   = params.fenvD.get() * 0.001f;
        fp.sustain = params.fenvS.get01();
        fp.release = params.fenvR.get() * 0.001f;
        filterEnv.setParameters (fp);

        // The envelope follows the held notes: it attacks on the first and
        // releases when the keyboard is released.
        const bool held = anyNoteActive();
        if (held && ! anyNoteHeld)
            filterEnv.noteOn();
        else if (! held && anyNoteHeld)
            filterEnv.noteOff();
        anyNoteHeld = held;

        const float reso = params.fltReso.get();

        filterL.setType (filterType);
        filterR.setType (filterType);
        filterL.setResonance (reso);
        filterR.setResonance (reso);

        const float nyquist = 0.45f * (float) preparedRate;

        for (int i = 0; i < numSamples; ++i)
        {
            // The envelope opens the filter from a quarter of the cut-off up to
            // it: the sweep the faders draw.
            const float e = filterEnv.getNextSample();
            const float fc = juce::jlimit (40.0f, nyquist,
                                           cutoffSmooth.getNextValue() * cutoffMod * (0.25f + 0.75f * e));
            filterL.setCutoff (fc);
            filterR.setCutoff (fc);

            dataL[i] = filterL.process (dataL[i]);
            dataR[i] = filterR.process (dataR[i]);
        }
    }
    else
    {
        cutoffSmooth.skip (numSamples);
        anyNoteHeld = anyNoteActive();
    }

    // ---- delay (block, in place) ----
    delay.process (dataL, dataR, dataL, dataR, numSamples,
                   delayTimeMs, delayFb, delayMix, delayLP, delayActive);

    // ---- chorus ----
    const float chorusAmount = params.chorAmt.get01();
    if (chorusAmount > 0.001f && ! chorusL.empty())
    {
        const int   len = (int) chorusL.size();
        const float rate = 0.55f;                       // Hz, slow and thick
        const float baseDelay = 0.011f * (float) preparedRate;      // 11 ms
        const float depth = 0.004f * (float) preparedRate * chorusAmount;
        const float phaseInc = rate / (float) preparedRate;

        for (int i = 0; i < numSamples; ++i)
        {
            chorusL[(size_t) chorusWrite] = dataL[i];
            chorusR[(size_t) chorusWrite] = dataR[i];

            auto tap = [&] (const std::vector<float>& buf, float phase)
            {
                const float d = baseDelay + depth * (0.5f + 0.5f * std::sin (6.2831853f * phase));
                float pos = (float) chorusWrite - d;
                if (! std::isfinite (pos))
                    return 0.0f;
                while (pos < 0.0f)
                    pos += (float) len;

                const int   i0 = juce::jlimit (0, len - 1, (int) pos);
                const float fr = pos - (float) i0;
                const int   i1 = (i0 + 1) % len;
                return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
            };

            // The two channels are modulated in opposite phase: when one side
            // lengthens the other shortens, which genuinely opens the image. In
            // quadrature the two moved together too much and the chorus was
            // barely wider than the dry signal.
            const float wetL = tap (chorusL, chorusPhase);
            const float wetR = tap (chorusR, chorusPhase + 0.5f);

            const float blend = 0.7f * chorusAmount;
            dataL[i] += (wetL - dataL[i]) * blend;
            dataR[i] += (wetR - dataR[i]) * blend;

            chorusWrite = (chorusWrite + 1) % len;
            chorusPhase += phaseInc;
            if (chorusPhase >= 1.0f)
                chorusPhase -= 1.0f;
        }
    }

    // ---- reverb ----
    {
        juce::Reverb::Parameters rp;
        rp.roomSize   = juce::jlimit (0.0f, 1.0f, params.revSize.get01());
        rp.damping    = 0.45f;
        rp.width      = 1.0f;
        rp.wetLevel   = params.revMix.get01();
        rp.dryLevel   = 1.0f - 0.35f * params.revMix.get01();
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        if (rp.wetLevel > 0.001f)
            reverb.processStereo (dataL, dataR, numSamples);
    }

    // ---- width + master volume + limiter (per sample) ----
    bool nonFinite = false;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = dataL[i];
        float r = dataR[i];

        // mid/side width
        const float width = widthSmooth.getNextValue();
        const float m = (l + r) * 0.5f;
        const float s = (l - r) * 0.5f * width;
        l = m + s;
        r = m - s;

        // One step of the smoothers per sample, shared by both channels.
        const float gain = masterVolSmooth.getNextValue() * lfoGainSmooth.getNextValue();
        l *= gain;
        r *= gain;

        if (limiterOn)
        {
            l = std::tanh (l);
            r = std::tanh (r);
        }

        if (! (std::isfinite (l) && std::isfinite (r)))
        {
            nonFinite = true;
            l = r = 0.0f;
        }

        dataL[i] = l;
        dataR[i] = r;
    }

    // A NaN would otherwise live on in every recursive stage (amp, filter, delay
    // line, reverb) and silence the plug-in until the project is reloaded.
    if (nonFinite)
    {
        ++nonFiniteEvents;

        // Whatever produced it may be a voice: its loop would keep the bad value
        // for as long as the note lasts.
        synth.allNotesOff (0, false);
        sampleSynth.allNotesOff (0, false);

        amp.reset();
        drive.reset();
        delay.reset();
        reverb.reset();
        filterL.reset();
        filterR.reset();
        dcInL = dcOutL = dcInR = dcOutR = 0.0f;
        std::fill (chorusL.begin(), chorusL.end(), 0.0f);
        std::fill (chorusR.begin(), chorusR.end(), 0.0f);
    }
}

//==============================================================================
juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

        // The index belongs to the sound that was loaded before: the name saved
        // with the project now says which preset this is.
        currentPreset = -1;
        if (onPresetListChanged)
            onPresetListChanged();
    }
}

juce::File PluginProcessor::getPresetDirectory() const
{
    // Users save their cartridges here, next to the factory presets: one folder,
    // in Documents, writable without elevation unlike the bundle itself.
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("DJBILBOX")
        .getChildFile ("ROCKPUNK")
        .getChildFile ("Presets");

    if (! dir.isDirectory())
        dir.createDirectory();

    return dir;
}

//==============================================================================
void PluginProcessor::refreshPresetList()
{
    const auto previous = currentPreset >= 0 && currentPreset < presetFiles.size()
        ? presetFiles[currentPreset] : juce::File();

    presetFiles.clear();
    for (const auto& f : getPresetDirectory().findChildFiles (juce::File::findFiles, false, "*.rpk"))
        presetFiles.add (f);

    // Sorted by name: the factory presets are numbered, and the order shown must
    // be that one rather than the file system's.
    presetFiles.sort();

    currentPreset = previous != juce::File() ? presetFiles.indexOf (previous) : -1;

    if (onPresetListChanged)
        onPresetListChanged();
}

juce::String PluginProcessor::getCurrentPresetName() const
{
    if (currentPreset >= 0 && currentPreset < presetFiles.size())
        return presetFiles[currentPreset].getFileNameWithoutExtension();

    // A project saved with a preset loaded remembers its name.
    const auto remembered = apvts.state.getProperty (kPresetNameProperty).toString();
    if (remembered.isNotEmpty())
        return remembered;

    return "ROCKPUNK - Init Patch";
}

int PluginProcessor::getCurrentPresetIndex() const
{
    if (currentPreset >= 0 && currentPreset < presetFiles.size())
        return currentPreset;

    const auto remembered = apvts.state.getProperty (kPresetNameProperty).toString();
    if (remembered.isNotEmpty())
        for (int i = 0; i < presetFiles.size(); ++i)
            if (presetFiles[i].getFileNameWithoutExtension() == remembered)
                return i;

    return -1;
}

bool PluginProcessor::loadPreset (const juce::File& f)
{
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;

    // replaceState also replaces the root properties: keep the window size.
    const auto width  = apvts.state.getProperty (kUiWidthProperty);
    const auto height = apvts.state.getProperty (kUiHeightProperty);

    apvts.replaceState (juce::ValueTree::fromXml (*xml));

    if (! width.isVoid())  apvts.state.setProperty (kUiWidthProperty, width, nullptr);
    if (! height.isVoid()) apvts.state.setProperty (kUiHeightProperty, height, nullptr);
    apvts.state.setProperty (kPresetNameProperty, f.getFileNameWithoutExtension(), nullptr);

    currentPreset = presetFiles.indexOf (f);

    if (onPresetListChanged)
        onPresetListChanged();
    return true;
}

bool PluginProcessor::loadPreset (int index)
{
    if (index < 0 || index >= presetFiles.size())
        return false;
    return loadPreset (presetFiles[index]);
}

void PluginProcessor::stepPreset (int delta)
{
    if (presetFiles.isEmpty())
        return;

    // From "no preset loaded", a step forward must give the first and a step
    // back the last - not index -1 or 1.
    const int current = getCurrentPresetIndex();
    int next = current < 0 ? (delta > 0 ? 0 : presetFiles.size() - 1)
                           : current + delta;

    while (next < 0)
        next += presetFiles.size();
    next %= presetFiles.size();

    loadPreset (next);
}

bool PluginProcessor::savePreset (const juce::File& target)
{
    auto f = target.hasFileExtension (".rpk") ? target : target.withFileExtension (".rpk");

    // The window size and the preset name belong to the session, not to the sound.
    auto state = apvts.copyState();
    state.removeProperty (kUiWidthProperty, nullptr);
    state.removeProperty (kUiHeightProperty, nullptr);
    state.removeProperty (kPresetNameProperty, nullptr);

    if (! f.replaceWithText (state.toXmlString()))
        return false;

    apvts.state.setProperty (kPresetNameProperty, f.getFileNameWithoutExtension(), nullptr);

    refreshPresetList();
    currentPreset = presetFiles.indexOf (f);

    if (onPresetListChanged)
        onPresetListChanged();
    return true;
}

void PluginProcessor::initPatch()
{
    for (auto* p : getParameters())
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->getDefaultValue());
        p->endChangeGesture();
    }

    apvts.state.removeProperty (kPresetNameProperty, nullptr);
    currentPreset = -1;

    if (onPresetListChanged)
        onPresetListChanged();
}

//==============================================================================
void PluginProcessor::rescanSamples()
{
    bank.openDefault();
    lastSampleSel = -1;          // force a reload: the indices have moved
    pollSampleSelection();
}

void PluginProcessor::prepareUserSampleFolder() const
{
    // ROCKPUNK ships no recording: this layer plays what the user drops here.
    // The folder is created empty, with the instructions inside - otherwise the
    // sample list stays empty with no explanation.
    auto dir = SampleBank::getUserSampleDirectory();
    if (! dir.isDirectory() && ! dir.createDirectory().wasOk())
        return;

    auto readme = dir.getChildFile ("README.txt");
    if (readme.existsAsFile())
        return;

    readme.replaceWithText (
        "ROCKPUNK GUITAR - your own guitars\r\n"
        "\r\n"
        "Drop your .wav, .aif or .flac files in this folder. They appear in the\r\n"
        "\"Your samples\" list of the EXPANSION menu (click the cassette at the top\r\n"
        "left of the plug-in) and can be played across the whole keyboard.\r\n"
        "\r\n"
        "ROCKPUNK measures the note of every file by itself to tune the keyboard.\r\n"
        "If the measurement lands beside the note (chords and riffs are ambiguous\r\n"
        "by nature), use the \"Sample Root Offset\" parameter to correct it by a few\r\n"
        "semitones. It is in the parameter list of your DAW, together with the\r\n"
        "level, loop, attack and release of the sampled layer.\r\n"
        "\r\n"
        "A sub-folder becomes a category in the list. For example:\r\n"
        "  Samples/Rhythm/palm-mute-E.wav\r\n"
        "  Samples/Solos/bend-A.wav\r\n"
        "\r\n"
        "The list is sorted by folder and name, so adding files can change the\r\n"
        "position of the ones you already picked.\r\n"
        "\r\n"
        "ROCKPUNK's two strings need no file at all: they are entirely calculated.\r\n"
        "The 76 factory presets rely only on them and work with this folder empty.\r\n"
        "\r\n"
        "DJBILBOX BEATS - support@djbilboxbeats.com\r\n");
}

void PluginProcessor::seedFactoryPresets() const
{
    auto dest = getPresetDirectory();
    if (! dest.isDirectory())
        return;

    // The Windows installer runs elevated: the Documents folder it sees is the
    // administrator's, not the user's. The factory presets therefore travel in
    // the bundle and are copied here on first launch.
    const auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    const juce::File sources[] =
    {
        exe.getParentDirectory().getParentDirectory()
           .getChildFile ("Resources").getChildFile ("Presets"),
        exe.getParentDirectory().getChildFile ("Presets"),
        juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
           .getChildFile ("DJBILBOX").getChildFile ("ROCKPUNK").getChildFile ("Presets"),
    };

    for (const auto& src : sources)
    {
        if (! src.isDirectory())
            continue;

        // Copy only what is missing: a factory preset that the user edited and
        // saved under the same name must not come back.
        for (const auto& f : src.findChildFiles (juce::File::findFiles, false, "*.rpk"))
        {
            const auto target = dest.getChildFile (f.getFileName());
            if (! target.existsAsFile())
                f.copyFileTo (target);
        }
        return;
    }
}

} // namespace rk

//==============================================================================
// Entry point shared by every JUCE plugin wrapper (VST3, standalone, ...).
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new rk::PluginProcessor();
}
