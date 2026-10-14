#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FactoryPresets.h"

namespace humpire
{

ChaabiProcessor::ChaabiProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "HUMPIRE_CHAABI", pid::createLayout())
{
    initPhraseBank();
    phrasePlayer = std::make_unique<PhrasePlayer> (seqGrid);

    auto raw = [this] (const juce::String& id) { return apvts.getRawParameterValue (id); };
    pSelected = raw (pid::selected);  pMulti = raw (pid::multi);      pPoly = raw (pid::poly);
    pAuth = raw (pid::authent);       pMaster = raw (pid::master);    pRevSize = raw (pid::revSize);
    pRevDamp = raw (pid::revDamp);    pDlyTime = raw (pid::dlyTime);  pDlyFb = raw (pid::dlyFb);
    pTempo = raw (pid::tempo);        pBend = raw (pid::bendRange);   pSeqOn = raw (pid::seqOn);
    pSeqSwing = raw (pid::seqSwing);  pSeqRate = raw (pid::seqRate);

    for (int i = 0; i < kNumInstruments; ++i)
    {
        auto& p = slots[(size_t) i].p;
        p.vol = raw (pid::inst (i, "vol"));       p.pan = raw (pid::inst (i, "pan"));
        p.rev = raw (pid::inst (i, "rev"));       p.dly = raw (pid::inst (i, "dly"));
        p.filter = raw (pid::inst (i, "filter")); p.pitch = raw (pid::inst (i, "pitch"));
        p.atk = raw (pid::inst (i, "atk"));       p.rel = raw (pid::inst (i, "rel"));
        p.lo = raw (pid::inst (i, "lo"));         p.hi = raw (pid::inst (i, "hi"));
        p.on = raw (pid::inst (i, "on"));
    }

    for (auto* id : { &pid::selected, &pid::multi })
        apvts.addParameterListener (*id, this);
    for (int i = 0; i < kNumInstruments; ++i)
        apvts.addParameterListener (pid::inst (i, "on"), this);
    apvts.state.addListener (this);

    // Etat initial : grille du sequenceur
    if (! apvts.state.hasProperty (pid::seqGrid))
        apvts.state.setProperty (pid::seqGrid, defaultSeqGrid(), nullptr);
    decodeSeqGrid (apvts.state.getProperty (pid::seqGrid).toString());

    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("HUMPIRE").getChildFile ("Chaabi Orchestra").getChildFile ("Presets");
    presets = std::make_unique<PresetManager> (apvts, undoManager, dir, makeFactoryPresets());

    triggerAsyncUpdate();
}

ChaabiProcessor::~ChaabiProcessor()
{
    cancelPendingUpdate();
    apvts.state.removeListener (this);
    for (auto* id : { &pid::selected, &pid::multi })
        apvts.removeParameterListener (*id, this);
    for (int i = 0; i < kNumInstruments; ++i)
        apvts.removeParameterListener (pid::inst (i, "on"), this);

    for (auto& s : slots) s.generation++;           // annule les chargements en cours
    loaderPool.removeAllJobs (true, 15000);
}

bool ChaabiProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ------------------------------------------------------------------ preparation
void ChaabiProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    maxBlock = juce::jmax (samplesPerBlock, 4096);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 2 };
    for (auto& s : slots)
    {
        s.synth.setCurrentPlaybackSampleRate (sampleRate);
        s.buf.setSize (2, maxBlock);
        s.filter.prepare (spec);
        s.filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        s.filter.setResonance (0.707f);
        s.filter.reset();
        for (auto* sm : { &s.vol, &s.pan, &s.rev, &s.dly }) sm->reset (sampleRate, 0.02);
        s.cutoff.reset (sampleRate, 0.03);
        s.vol.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (s.p.vol->load(), -60.f));
        s.pan.setCurrentAndTargetValue (s.p.pan->load());
        s.rev.setCurrentAndTargetValue (s.p.rev->load());
        s.dly.setCurrentAndTargetValue (s.p.dly->load());
        s.cutoff.setCurrentAndTargetValue (s.p.filter->load());
        s.wasActive = false;
    }
    for (auto& m : slotMidi) m.ensureSize (8192);

    bus.setSize (5, maxBlock);
    reverb.setSampleRate (sampleRate);
    reverb.reset();

    juce::dsp::ProcessSpec mono { sampleRate, (juce::uint32) maxBlock, 1 };
    delayL.prepare (mono); delayR.prepare (mono);
    delayL.reset(); delayR.reset();
    masterGain.reset (sampleRate, 0.02);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pMaster->load()));
    delaySamples.reset (sampleRate, 0.15);
    delaySamples.setCurrentAndTargetValue ((float) (pDlyTime->load() * 0.001 * sampleRate));
    delayFb.reset (sampleRate, 0.02);
    delayFb.setCurrentAndTargetValue (pDlyFb->load());

    phrasePlayer->prepare (sampleRate);
    setLatencySamples (0);                 // aucun look-ahead : latence nulle
    refreshLoads();
}

// ------------------------------------------------------------------ audio
void ChaabiProcessor::pickupBank (Slot& s)
{
    if (s.pendingLock.tryEnter())
    {
        if (s.pending != nullptr && s.retired == nullptr)
            s.retired = s.synth.swapBank (std::move (s.pending));   // l'ancienne banque est liberee cote message
        s.pendingLock.exit();
    }
}

void ChaabiProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    midi.clear();
    buffer.clear();
    for (auto& s : slots) { s.synth.allNotesOff (0, false); s.wasActive = false; }
    phrasePlayer->reset();
}

void ChaabiProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (n == 0 || n > maxBlock || buffer.getNumChannels() < 2) return;

    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    for (auto& s : slots) pickupBank (s);

    // ---- parametres globaux
    const bool multi = pMulti->load() > 0.5f;
    const int selected = juce::jlimit (0, kNumInstruments - 1, (int) pSelected->load());
    const int poly = (int) pPoly->load();
    const float auth = pAuth->load();

    activeNow.fill (false);
    if (! multi)
        activeNow[(size_t) selected] = true;
    else
    {
        int cnt = 0;
        for (int i = 0; i < kNumInstruments && cnt < kMaxMultiInstruments; ++i)
            if (slots[(size_t) i].p.on->load() > 0.5f) { activeNow[(size_t) i] = true; ++cnt; }
        if (cnt == 0) activeNow[(size_t) selected] = true;
    }

    // ---- transport hote
    double bpm = (double) pTempo->load(), ppq = 0.0;
    bool playing = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            playing = pos->getIsPlaying();
            if (auto p = pos->getPpqPosition()) ppq = *p;
        }
    lastBpm.store (bpm);

    // ---- routage MIDI -> slots
    for (auto& m : slotMidi) m.clear();
    for (size_t i = 0; i < slots.size(); ++i)
    {
        auto& s = slots[i];
        if (! activeNow[i] && s.wasActive) s.synth.allNotesOff (0, false);
        s.wasActive = activeNow[i];
    }

    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        const int t = meta.samplePosition;

        if (m.isNoteOn())
        {
            const int note = m.getNoteNumber();
            midiActivity.store (juce::jmax (midiActivity.load(), m.getFloatVelocity()));

            if (note >= kPhraseLo && note <= kPhraseHi) { phrasePlayer->trigger (note, activeNow, t); continue; }

            for (size_t i = 0; i < slots.size(); ++i)
            {
                if (! activeNow[i]) continue;
                const bool isKeyswitch = note >= kKeyswitchLo && note <= kKeyswitchHi;
                const bool inRange = ! multi || (note >= (int) slots[i].p.lo->load() && note <= (int) slots[i].p.hi->load());
                if (isKeyswitch || inRange) slotMidi[i].addEvent (m, t);
            }
        }
        else if (m.isNoteOff())
        {
            const int note = m.getNoteNumber();
            if (note >= kPhraseLo && note <= kPhraseHi) { phrasePlayer->release (note); continue; }
            for (size_t i = 0; i < slots.size(); ++i)
                if (activeNow[i]) slotMidi[i].addEvent (m, t);
        }
        else
        {
            for (size_t i = 0; i < slots.size(); ++i)   // pitch bend, CC (sustain), aftertouch, all notes off
                if (activeNow[i]) slotMidi[i].addEvent (m, t);
        }
    }

    // ---- phrases + sequenceur
    phrasePlayer->process (slotMidi, n, bpm, playing, ppq, pSeqOn->load() > 0.5f,
                           (double) pSeqSwing->load(), pSeqRate->load() < 0.5f ? 2 : 4, activeNow);

    // ---- rendu des instruments
    bus.clear (0, n);
    float* dryL = bus.getWritePointer (0); float* dryR = bus.getWritePointer (1);
    float* revL = bus.getWritePointer (2); float* revR = bus.getWritePointer (3);
    float* dlyIn = bus.getWritePointer (4);
    constexpr float sqrt2 = juce::MathConstants<float>::sqrt2;

    for (size_t si = 0; si < slots.size(); ++si)
    {
        if (! activeNow[si]) continue;
        auto& s = slots[si];

        s.synth.setPolyLimit (poly);
        s.synth.params.pitchSemis.store (s.p.pitch->load());
        s.synth.params.attack.store (s.p.atk->load());
        s.synth.params.release.store (s.p.rel->load());
        s.synth.params.authenticity.store (auth);
        s.synth.params.bendRange.store (pBend->load());

        s.buf.clear (0, n);
        s.synth.renderNextBlock (s.buf, slotMidi[si], 0, n);

        s.vol.setTargetValue (juce::Decibels::decibelsToGain (s.p.vol->load(), -60.f));
        s.pan.setTargetValue (s.p.pan->load());
        s.rev.setTargetValue (s.p.rev->load());
        s.dly.setTargetValue (s.p.dly->load());
        s.cutoff.setTargetValue (s.p.filter->load());

        float* l = s.buf.getWritePointer (0);
        float* r = s.buf.getWritePointer (1);

        // Filtre passe-bas, parametre lisse par sous-blocs de 32 echantillons
        for (int off = 0; off < n; off += 32)
        {
            const int c = juce::jmin (32, n - off);
            const bool smoothing = s.cutoff.isSmoothing();
            const float cut = s.cutoff.skip (c);
            if (smoothing || cut < 19900.f)
            {
                s.filter.setCutoffFrequency (juce::jlimit (60.f, (float) (sr * 0.45), cut));
                float* chans[2] { l + off, r + off };
                juce::dsp::AudioBlock<float> blk (chans, 2, (size_t) c);
                juce::dsp::ProcessContextReplacing<float> ctx (blk);
                s.filter.process (ctx);
            }
        }

        for (int i = 0; i < n; ++i)
        {
            const float g = s.vol.getNextValue();
            const float a = (s.pan.getNextValue() + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
            const float rv = s.rev.getNextValue(), dl = s.dly.getNextValue();
            const float outL = l[i] * g * std::cos (a) * sqrt2;
            const float outR = r[i] * g * std::sin (a) * sqrt2;
            dryL[i] += outL;  dryR[i] += outR;
            revL[i] += outL * rv;  revR[i] += outR * rv;
            dlyIn[i] += (outL + outR) * 0.5f * dl;
        }
    }

    // ---- effets globaux : reverb (la macro Authenticite ouvre la reverb), delay ping-pong
    reverb.setParameters ({ pRevSize->load(), pRevDamp->load(), 1.0f, 0.0f, 1.0f, 0.0f });
    reverb.processStereo (revL, revR, n);
    const float revReturn = 0.7f + 0.8f * auth;

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (pMaster->load()));
    delaySamples.setTargetValue (juce::jlimit (1.0f, (float) (sr * 1.5), (float) (pDlyTime->load() * 0.001 * sr)));
    delayFb.setTargetValue (pDlyFb->load());

    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getWritePointer (1);
    float pkL = 0.f, pkR = 0.f;

    for (int i = 0; i < n; ++i)
    {
        const float d = delaySamples.getNextValue(), fb = delayFb.getNextValue();
        delayL.setDelay (d); delayR.setDelay (d);
        const float dL = delayL.popSample (0), dR = delayR.popSample (0);
        delayL.pushSample (0, dlyIn[i] + dR * fb);
        delayR.pushSample (0, dL * fb);

        const float mg = masterGain.getNextValue();
        const float l = (dryL[i] + revL[i] * revReturn + dL) * mg;
        const float r = (dryR[i] + revR[i] * revReturn + dR) * mg;
        outL[i] = l; outR[i] = r;
        pkL = juce::jmax (pkL, std::abs (l));
        pkR = juce::jmax (pkR, std::abs (r));
    }

    meterL.store (juce::jmax (pkL, meterL.load() * 0.8f));
    meterR.store (juce::jmax (pkR, meterR.load() * 0.8f));
    midiActivity.store (midiActivity.load() * 0.9f);
    midi.clear();
}

// ------------------------------------------------------------------ chargement des banques
juce::File ChaabiProcessor::getLibraryRoot() const
{
    auto prop = apvts.state.getProperty (pid::libraryRoot).toString();
    if (prop.isNotEmpty()) return juce::File (prop);
    if (auto env = juce::SystemStats::getEnvironmentVariable ("HUMPIRE_SAMPLES", {}); env.isNotEmpty())
        return juce::File (env);
    return juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
        .getChildFile ("HUMPIRE").getChildFile ("Chaabi Orchestra").getChildFile ("Samples");
}

void ChaabiProcessor::setLibraryRoot (const juce::File& f)
{
    apvts.state.setProperty (pid::libraryRoot, f.getFullPathName(), nullptr);
}

juce::File ChaabiProcessor::resolveSfz (int i) const
{
    auto ov = apvts.state.getProperty (pid::sfzOverride (i)).toString();
    if (ov.isNotEmpty()) return juce::File (ov);
    return getLibraryRoot().getChildFile (kInstruments[i].folder).getChildFile (kInstruments[i].sfz);
}

bool ChaabiProcessor::isSlotActive (int slot) const
{
    if (pMulti->load() < 0.5f) return slot == (int) pSelected->load();
    return slots[(size_t) slot].p.on->load() > 0.5f || slot == (int) pSelected->load();
}

SlotStatus ChaabiProcessor::getSlotStatus (int slot) const
{
    auto& s = slots[(size_t) slot];
    const juce::ScopedLock sl (s.msgLock);
    return { s.state.load(), s.progress.load(), s.message };
}

void ChaabiProcessor::handleAsyncUpdate() { refreshLoads(); }

void ChaabiProcessor::parameterChanged (const juce::String&, float) { triggerAsyncUpdate(); }

void ChaabiProcessor::valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& id)
{
    if (id == pid::seqGrid)
        decodeSeqGrid (apvts.state.getProperty (pid::seqGrid).toString());
    else
        triggerAsyncUpdate();
}

void ChaabiProcessor::refreshLoads()
{
    for (int i = 0; i < kNumInstruments; ++i)
    {
        auto& s = slots[(size_t) i];
        if (! isSlotActive (i)) continue;
        const auto sfz = resolveSfz (i);
        juce::String loaded;
        { const juce::ScopedLock sl (s.msgLock); loaded = s.loadedPath; }
        if (loaded != sfz.getFullPathName())
            requestLoad (i, sfz);
    }
}

void ChaabiProcessor::requestLoad (int slotIndex, const juce::File& sfz)
{
    auto& s = slots[(size_t) slotIndex];
    const int gen = ++s.generation;

    { const juce::SpinLock::ScopedLockType l (s.pendingLock); s.retired.reset(); s.pending.reset(); }
    {
        const juce::ScopedLock sl (s.msgLock);
        s.loadedPath = sfz.getFullPathName();
        s.message = sfz.existsAsFile() ? juce::String::fromUTF8 ("Chargement\xE2\x80\xA6")
                                       : juce::String::fromUTF8 ("SFZ introuvable : ") + sfz.getFullPathName();
    }
    s.progress = 0.f;
    s.state = sfz.existsAsFile() ? 1 : 3;
    if (! sfz.existsAsFile()) return;

    loaderPool.addJob ([this, slotIndex, gen, sfz]
    {
        auto& slot = slots[(size_t) slotIndex];
        juce::String err;
        auto bank = Bank::load (sfz, slot.progress, [&] { return slot.generation.load() != gen; }, err);
        if (slot.generation.load() != gen) return;

        if (bank != nullptr)
        {
            const auto info = juce::String ((int) bank->regions.size()) + juce::String::fromUTF8 (" r\xC3\xA9gions \xE2\x80\xA2 ") + juce::String (bank->memoryMB, 1) + " Mo en RAM";
            {
                const juce::SpinLock::ScopedLockType l (slot.pendingLock);
                slot.pending = std::move (bank);
            }
            const juce::ScopedLock sl (slot.msgLock);
            slot.message = info;
            slot.state = 2;
        }
        else
        {
            const juce::ScopedLock sl (slot.msgLock);
            slot.message = err;
            slot.state = 3;
        }
    });
}

// ------------------------------------------------------------------ sequenceur (grille 2 x 5 x 16)
juce::String ChaabiProcessor::defaultSeqGrid()
{
    std::string g ((size_t) (kSeqInstruments * kSeqLanes * kSeqSteps), '0');
    auto set = [&] (int inst, int lane, std::initializer_list<int> steps, char v = '1')
    {
        for (int st : steps) g[(size_t) ((inst * kSeqLanes + lane) * kSeqSteps + st)] = v;
    };
    // Derbouka : maqsum sur 16 pas (1/8)
    set (0, 0, { 8 }); set (0, 0, { 0 }, '2');
    set (0, 1, { 2, 6, 12 }); set (0, 2, { 14 }); set (0, 4, { 10 });
    // Tar
    set (1, 0, { 0, 8 }); set (1, 1, { 4, 12 }); set (1, 3, { 2, 6, 10, 14 });
    return juce::String (g);
}

void ChaabiProcessor::decodeSeqGrid (const juce::String& s)
{
    const int total = kSeqInstruments * kSeqLanes * kSeqSteps;
    for (int idx = 0; idx < total; ++idx)
    {
        const int v = idx < s.length() ? juce::jlimit (0, 2, (int) (s[idx] - '0')) : 0;
        const int inst = idx / (kSeqLanes * kSeqSteps), lane = (idx / kSeqSteps) % kSeqLanes, step = idx % kSeqSteps;
        seqGrid[(size_t) inst][(size_t) lane][(size_t) step].store ((std::uint8_t) v);
    }
}

int ChaabiProcessor::getSeqStep (int inst, int lane, int step) const
{
    return seqGrid[(size_t) inst][(size_t) lane][(size_t) step].load();
}

void ChaabiProcessor::setSeqStep (int inst, int lane, int step, int value)
{
    auto s = apvts.state.getProperty (pid::seqGrid).toString();
    const int idx = (inst * kSeqLanes + lane) * kSeqSteps + step;
    if (idx < 0 || idx >= s.length()) return;
    undoManager.beginNewTransaction ("Pattern");
    apvts.state.setProperty (pid::seqGrid, s.replaceSection (idx, 1, juce::String::charToString ((juce::juce_wchar) ('0' + value))), &undoManager);
}

// ------------------------------------------------------------------ etat
void ChaabiProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void ChaabiProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.isValid() && tree.hasType (apvts.state.getType()))
        {
            apvts.replaceState (tree);
            if (! apvts.state.hasProperty (pid::seqGrid))
                apvts.state.setProperty (pid::seqGrid, defaultSeqGrid(), nullptr);
            decodeSeqGrid (apvts.state.getProperty (pid::seqGrid).toString());
            triggerAsyncUpdate();
        }
    }
}

juce::AudioProcessorEditor* ChaabiProcessor::createEditor() { return new ChaabiEditor (*this); }
} // namespace humpire

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new humpire::ChaabiProcessor(); }
