#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace K2k;

K2000AudioProcessor::K2000AudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createLayout())
{
    ptrs.bind (apvts);
    for (int i = 0; i < 16; ++i) { voices[(size_t) i] = new KVoice (ptrs, shared); synth.addVoice (voices[(size_t) i]); }
    synth.addSound (new KSound());
    noMidi.ensureSize (64);
}

void K2000AudioProcessor::prepareToPlay (double sr, int)
{
    synth.setCurrentPlaybackSampleRate (sr);
    chorusFx.setSampleRate (sr); delayFx.setSampleRate (sr); reverbFx.setSampleRate (sr);
    volSm.reset (sr, 0.02); volSm.setCurrentAndTargetValue (ptrs (volume));
    lastMode = -1; stackSize = 0;
    for (auto* v : voices) v->kill();   // repartir d'un etat silencieux
    chorusFx.reset(); delayFx.reset(); reverbFx.reset(); limiter.reset();
    prepared = sr > 0.0;
}

void K2000AudioProcessor::applyPreset (int index)
{
    const auto& list = presets();
    if (index < 0 || index >= (int) list.size()) return;
    for (int i = 0; i < COUNT; ++i)   // valeurs par defaut, puis preset
        if (auto* p = apvts.getParameter (ids[i]))
            if (i != bend && i != modw) p->setValueNotifyingHost (p->getDefaultValue());
    for (auto& kv : list[(size_t) index].v)
        if (auto* p = apvts.getParameter (kv.first))
            p->setValueNotifyingHost (p->convertTo0to1 (kv.second));
    presetIndex = index;
    presetName = list[(size_t) index].name;
}

void K2000AudioProcessor::allNotesOff()
{
    stackSize = 0;
    for (auto* v : voices) v->release();
    synth.allNotesOff (0, true);
}

void K2000AudioProcessor::monoOn (int note, float vel)
{
    const bool legato = (int) ptrs (vmode) == 3 && stackSize > 0;
    // retire un doublon eventuel puis empile
    for (int i = 0; i < stackSize; ++i) if (noteStack[(size_t) i] == note) { for (int j = i; j < stackSize - 1; ++j) { noteStack[(size_t) j] = noteStack[(size_t) j + 1]; velStack[(size_t) j] = velStack[(size_t) j + 1]; } --stackSize; break; }
    if (stackSize < (int) noteStack.size()) { noteStack[(size_t) stackSize] = note; velStack[(size_t) stackSize] = vel; ++stackSize; }
    voices[0]->monoNote (note, vel, legato);
}

void K2000AudioProcessor::monoOff (int note)
{
    int idx = -1;
    for (int i = 0; i < stackSize; ++i) if (noteStack[(size_t) i] == note) { idx = i; break; }
    if (idx < 0) return;
    const bool wasTop = idx == stackSize - 1;
    for (int j = idx; j < stackSize - 1; ++j) { noteStack[(size_t) j] = noteStack[(size_t) j + 1]; velStack[(size_t) j] = velStack[(size_t) j + 1]; }
    --stackSize;
    if (stackSize == 0) voices[0]->release();
    else if (wasTop) voices[0]->monoNote (noteStack[(size_t) stackSize - 1], velStack[(size_t) stackSize - 1], (int) ptrs (vmode) == 3);
}

void K2000AudioProcessor::handleEvent (const juce::MidiMessage& m, int mode)
{
    const bool mono = mode >= 2;
    const int ch = juce::jmax (1, m.getChannel());
    if (m.isNoteOn()) { if (mono) monoOn (m.getNoteNumber(), m.getFloatVelocity()); else synth.noteOn (ch, m.getNoteNumber(), m.getFloatVelocity()); }
    else if (m.isNoteOff()) { if (mono) monoOff (m.getNoteNumber()); else synth.noteOff (ch, m.getNoteNumber(), m.getFloatVelocity(), true); }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) { allNotesOff(); if (m.isAllSoundOff()) synth.allNotesOff (ch, false); }
    else if (m.isPitchWheel()) { const float b = (m.getPitchWheelValue() - 8192) / 8192.f; for (auto* v : voices) v->setMidiBend (b); synth.handlePitchWheel (ch, m.getPitchWheelValue()); }
    else if (m.isController())
    {
        if (m.getControllerNumber() == 1) { const float x = m.getControllerValue() / 127.f; shared.lastMod.store (x); for (auto* v : voices) v->setMidiMod (x); }
        else if (m.getControllerNumber() == 121) { shared.lastMod.store (0.f); for (auto* v : voices) v->setMidiMod (0.f); }   // reset all controllers
        else if (m.getControllerNumber() == 64 && ! mono) synth.handleSustainPedal (ch, m.getControllerValue() >= 64);
    }
}

void K2000AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (n == 0 || buffer.getNumChannels() < 1 || ! prepared) return;   // nothing to do / prepareToPlay not called yet
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    const int mode = juce::jlimit (0, 3, (int) ptrs (vmode));
    if (mode != lastMode)
    {
        if (lastMode >= 0) allNotesOff();
        lastMode = mode;
        for (int i = 0; i < 16; ++i) voices[(size_t) i]->enabled = mode == 1 ? true : mode == 0 ? i < 8 : i == 0;
    }

    int pos = 0;
    for (auto m : midi)
    {
        const int at = juce::jlimit (pos, n, m.samplePosition);
        if (at > pos) { synth.renderNextBlock (buffer, noMidi, pos, at - pos); pos = at; }
        handleEvent (m.getMessage(), mode);
    }
    if (pos < n) synth.renderNextBlock (buffer, noMidi, pos, n - pos);

    // chaine d'effets maitre : distorsion -> chorus -> delay -> reverb -> volume + limiteur
    masterDistortion.setDrive (ptrs (dist));
    const float cdep = juce::jlimit (0.f, 1.f, ptrs (chorus) + (((int) ptrs (lfoDst) == 4) ? 0.5f * shared.lfoChorus.load() : 0.f) + 0.25f * ptrs (modw));
    chorusFx.setDepth (cdep);
    delayFx.setMix (ptrs (delayMix)); delayFx.setDelayTime (ptrs (delayTime)); delayFx.setFeedback (ptrs (delayFb));
    reverbFx.setMix (ptrs (reverb));
    volSm.setTargetValue (ptrs (volume));

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : L;
    for (int i = 0; i < n; ++i)
    {
        float l = L[i], r = R[i];
        masterDistortion.process (l, r);
        chorusFx.process (l, r);
        delayFx.process (l, r);
        reverbFx.process (l, r);
        limiter.process (l, r, volSm.getNextValue());
        L[i] = l; if (R != L) R[i] = r;
    }
}

void K2000AudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", presetIndex, nullptr);
    state.setProperty ("presetName", presetName, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void K2000AudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            presetIndex = vt.getProperty ("presetIndex", -1);
            presetName = vt.getProperty ("presetName", "K2000 - Init Patch").toString();
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* K2000AudioProcessor::createEditor() { return new K2000AudioProcessorEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new K2000AudioProcessor(); }
