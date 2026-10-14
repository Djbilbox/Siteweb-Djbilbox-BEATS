#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Wz;

WarzoneProcessor::WarzoneProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "WARZONE808", createLayout())
{
    ptrs.bind (apvts);
    for (auto& d : factoryPresets())
    {
        Entry e { d.bank, d.name, {} };
        for (auto& kv : d.v) e.v.emplace_back (kv.first, kv.second);
        entries.push_back (std::move (e));
    }
    ExpansionPack pack;
    if (loadExpansionPackFromString (juce::String::fromUTF8 (BinaryData::EXPANSION_WZ_VOL1_wz808exp, BinaryData::EXPANSION_WZ_VOL1_wz808expSize), pack))
        loadExpansion (pack);
    // packs utilisateur deposes dans Documents/DJBILBOX BEATS/WARZONE808/Expansions
    for (auto& f : userDir ("Expansions").findChildFiles (juce::File::findFiles, false, "*.wz808exp"))
        loadExpansionFile (f);
}

juce::File WarzoneProcessor::userDir (const juce::String& sub)
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("WARZONE808").getChildFile (sub);
    d.createDirectory();
    return d;
}

bool WarzoneProcessor::loadExpansionFile (const juce::File& f)
{
    ExpansionPack pack;
    if (! loadExpansionPackFromFile (f, pack)) return false;
    loadExpansion (pack);
    return true;
}

void WarzoneProcessor::loadExpansion (const ExpansionPack& pack)
{
    for (auto& r : pack.presets)
    {
        Entry e { pack.name, r.name, {} };
        auto add = [&] (int id, double v) { e.v.emplace_back (ids[id], (float) v); };
        add (waveform, r.oscWaveform); add (filter, r.filterCutoffHz); add (reso, r.filterResonance); add (ftype, r.filterType);
        add (envdepth, juce::jmax (0.0, r.filterEnvDepthOct));
        add (attack, r.attackMs); add (decay, r.decayMs); add (sustain, r.sustainLevel); add (release, r.releaseMs); add (glide, r.glideMs);
        add (fattack, r.filterAttackMs); add (fdecay, r.filterDecayMs > 0 ? r.filterDecayMs : r.decayMs);
        add (fsustain, r.filterSustain >= 0 ? r.filterSustain : r.sustainLevel); add (velocity, r.velocity);
        add (rate, r.lfoRateHz); add (amount, r.lfoAmount); add (dest, juce::jlimit (0, 2, r.lfoDestination));
        add (dist, juce::jmax (r.distortionDriveDb / 24.0, r.distortionMix * 0.5)); add (dtone, r.distortionToneHz);
        add (crate, r.chorusRateHz); add (camount, r.chorusAmount); add (chorus, r.chorusMix);
        add (dtime, r.delayTimeMs); add (feedback, r.delayFeedback); add (dmix, r.delayMix);
        add (rroom, r.reverbRoom); add (rdamp, r.reverbDamping); add (rwidth, r.reverbWidth); add (reverb, r.reverbMix);
        add (volume, r.masterVolume); add (unison, r.unisonVoices); add (detune, r.unisonDetune); add (uwidth, r.unisonWidth);
        add (mono, r.unisonVoices > 0 && r.category.containsIgnoreCase ("pad") ? 0 : (r.category.containsIgnoreCase ("key") || r.category.containsIgnoreCase ("pluck") ? 0 : 1));
        entries.push_back (std::move (e));
    }
    expansionName = pack.name;
}

void WarzoneProcessor::prepareToPlay (double sr, int block)
{
    engine.setSampleRate (sr);
    engine.setBlockSize (juce::jmax (64, block));
    engine.reset();
    sustainedNotes.clear(); sustainDown = false;
    volSm.reset (sr, 0.02); cutSm.reset (sr, 0.02); pitchSm.reset (sr, 0.02);
    volSm.setCurrentAndTargetValue (ptrs (volume)); cutSm.setCurrentAndTargetValue (ptrs (filter)); pitchSm.setCurrentAndTargetValue (ptrs (pitch));
    lastPoly = ptrs (mono) < 0.5f; engine.setPolyphonyMode (lastPoly);
    dcR = 1.f - juce::MathConstants<float>::twoPi * 6.f / (float) sr; dcXl = dcYl = dcXr = dcYr = 0.f;
}

void WarzoneProcessor::applyPreset (int index)
{
    if (index < 0 || index >= (int) entries.size()) return;
    for (int i = 0; i < COUNT; ++i)   // defaut puis preset
        if (i != bend && i != modw)
            if (auto* p = apvts.getParameter (ids[i])) p->setValueNotifyingHost (p->getDefaultValue());
    // le defaut des parametres d'usine est mono/sub : on applique les valeurs du preset
    for (auto& kv : entries[(size_t) index].v)
        if (auto* p = apvts.getParameter (kv.first))
            p->setValueNotifyingHost (p->convertTo0to1 (kv.second));
    presetIndex = index;
    presetName = entries[(size_t) index].name;
}

void WarzoneProcessor::syncMode()
{
    // doit passer AVANT le traitement MIDI : sinon les premieres notes sont jouees dans l'ancien mode
    const bool poly = ptrs (mono) < 0.5f;
    if (poly != lastPoly) { engine.allNotesOff(); lastPoly = poly; sustainedNotes.clear(); }
    engine.setPolyphonyMode (poly);
}

void WarzoneProcessor::pushParamsToEngine (int n)
{
    auto& e = engine;
    syncMode();

    volSm.setTargetValue (ptrs (volume)); cutSm.setTargetValue (ptrs (filter)); pitchSm.setTargetValue (ptrs (pitch));
    e.setMasterVolume (volSm.skip (n) * 0.45);   // le moteur sature (tanh) des 0 dBFS : marge pour garder de la dynamique
    e.setFilterCutoff (cutSm.skip (n));
    e.setPitchBend (pitchSm.skip (n) + (midiBend + ptrs (bend)) * 2.0);

    e.setOscillatorWaveform ((Oscillator::Waveform) juce::jlimit (0, 4, (int) ptrs (waveform)));
    e.setFilterResonance (ptrs (reso));
    e.setFilterType ((StateVariableFilter::FilterType) juce::jlimit (0, 2, (int) ptrs (ftype)));
    e.setFilterEnvelopeDepth (ptrs (envdepth));
    e.setAttack (ptrs (attack)); e.setDecay (ptrs (decay)); e.setSustain (ptrs (sustain)); e.setRelease (ptrs (release));
    e.setFilterAttack (ptrs (fattack)); e.setFilterDecay (ptrs (fdecay)); e.setFilterSustain (ptrs (fsustain));
    e.setVelocitySensitivity (ptrs (velocity));
    e.setGlideTime (ptrs (glide));
    e.setLFORate (ptrs (rate)); e.setLFOAmount (ptrs (amount));
    e.setLFODestination ((LFO::Destination) juce::jlimit (0, 2, (int) ptrs (dest)));
    e.setLFOWaveform ((Oscillator::Waveform) juce::jlimit (0, 4, (int) ptrs (lfosrc)));
    e.setModWheel (juce::jmax (midiMod, ptrs (modw)));
    e.setUnison ((int) ptrs (unison), ptrs (detune), ptrs (uwidth));

    const float d = ptrs (dist);
    e.setDistortionDrive (d * 24.0); e.setDistortionMix (juce::jmin (1.f, d * 2.f)); e.setDistortionTone (ptrs (dtone));
    e.setChorusRate (ptrs (crate)); e.setChorusAmount (ptrs (camount)); e.setChorusMix (ptrs (chorus));
    e.setDelayTime (ptrs (dtime)); e.setDelayFeedback (ptrs (feedback)); e.setDelayMix (ptrs (dmix));
    e.setReverbRoom (ptrs (rroom)); e.setReverbDamping (ptrs (rdamp)); e.setReverbWidth (ptrs (rwidth)); e.setReverbMix (ptrs (reverb));
}

void WarzoneProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn()) { sustainedNotes.erase (std::remove (sustainedNotes.begin(), sustainedNotes.end(), m.getNoteNumber()), sustainedNotes.end()); engine.noteOn (m.getNoteNumber(), m.getVelocity()); }
    else if (m.isNoteOff())
    {
        if (sustainDown) { if (sustainedNotes.size() < 128) sustainedNotes.push_back (m.getNoteNumber()); }
        else engine.noteOff (m.getNoteNumber());
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) { engine.allNotesOff(); sustainedNotes.clear(); }
    else if (m.isPitchWheel()) midiBend = (m.getPitchWheelValue() - 8192) / 8192.f;
    else if (m.isControllerOfType (1)) midiMod = m.getControllerValue() / 127.f;
    else if (m.isControllerOfType (64))
    {
        const bool down = m.getControllerValue() >= 64;
        if (sustainDown && ! down) { for (int n : sustainedNotes) engine.noteOff (n); sustainedNotes.clear(); }
        sustainDown = down;
    }
}

void WarzoneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (n == 0 || buffer.getNumChannels() < 1) return;
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : buffer.getWritePointer (0);
    constexpr int kSub = 64;           // sous-blocs : MIDI au sample pres + lissage des parametres
    int pos = 0;
    auto it = midi.begin();
    while (pos < n)
    {
        syncMode();
        while (it != midi.end() && (*it).samplePosition <= pos) { handleMidi ((*it).getMessage()); ++it; }
        int len = juce::jmin (kSub, n - pos);
        if (it != midi.end()) len = juce::jmin (len, juce::jmax (1, (*it).samplePosition - pos));
        pushParamsToEngine (len);
        engine.processBlock (L + pos, R + pos, len);
        pos += len;
    }
    for (; it != midi.end(); ++it) handleMidi ((*it).getMessage());
    for (int i = 0; i < n; ++i)   // bloqueur DC (~6 Hz) : distorsion asymetrique et saw saturee decalent la tension continue
    {
        const float l = L[i], r = R[i];
        const float yl = l - dcXl + dcR * dcYl, yr = r - dcXr + dcR * dcYr;
        dcXl = l; dcYl = yl; dcXr = r; dcYr = yr;
        // genou doux au-dela de 0.9 : le retrait du DC peut decaler un pic asymetrique au-dela de 1.0
        auto soft = [] (float v) { const float a = std::abs (v); return a < 0.9f ? v : std::copysign (0.9f + 0.1f * std::tanh ((a - 0.9f) / 0.1f), v); };
        L[i] = soft (yl); if (R != L) R[i] = soft (yr);
    }
}

void WarzoneProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", presetIndex, nullptr);
    state.setProperty ("presetName", presetName, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void WarzoneProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            presetIndex = juce::jlimit (-1, (int) entries.size() - 1, (int) vt.getProperty ("presetIndex", -1));
            presetName = vt.getProperty ("presetName", "WARZONE808 - Init Patch").toString();
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* WarzoneProcessor::createEditor() { return new WarzoneEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new WarzoneProcessor(); }
