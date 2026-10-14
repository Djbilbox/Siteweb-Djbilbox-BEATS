#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace Cz;

CzProcessor::CzProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "CHINTZWAZ", createLayout())
{
    ptrs.bind (apvts);
    for (int i = 0; i < 16; ++i) synth.addVoice (new CzVoice (ptrs, glideState));
    synth.addSound (new CzSound());
}

void CzProcessor::prepareToPlay (double sr, int block)
{
    sampleRate = sr;
    synth.setCurrentPlaybackSampleRate (sr);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
    chorus.prepare (spec);
    chorus.setRate (0.7f); chorus.setDepth (0.35f); chorus.setCentreDelay (9.f); chorus.setFeedback (0.1f);
    dly.prepare (sr);
    reverb.setSampleRate (sr);
    lastMonoNote = -1;
    monoMidi.ensureSize (4096);
    dcRm = 1.f - juce::MathConstants<float>::twoPi * 8.f / (float) sr; dcXl = dcYl = dcXr = dcYr = 0.f;
    distSm.reset (sr, 0.02); delaySm.reset (sr, 0.02);
    distSm.setCurrentAndTargetValue (0.f); delaySm.setCurrentAndTargetValue (0.f);
}

void CzProcessor::applyPreset (int index)
{
    const auto& list = presets();
    if (index < 0 || index >= (int) list.size()) return;
    // retour aux valeurs par defaut puis application du preset
    for (int i = 0; i < COUNT; ++i)
        if (auto* p = apvts.getParameter (ids[i]))
            if (i != bend && i != modw) p->setValueNotifyingHost (p->getDefaultValue());
    for (auto& kv : list[(size_t) index].v)
        if (auto* p = apvts.getParameter (kv.first))
            p->setValueNotifyingHost (p->convertTo0to1 (kv.second));
    presetIndex = index;
    presetName = list[(size_t) index].name;
}

void CzProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (n == 0 || buffer.getNumChannels() < 1) return;
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    if (ptrs (mono) > 0.5f) // mono : derniere note prioritaire, coupe la precedente
    {
        auto& out = monoMidi; out.clear();
        for (auto m : midi)
        {
            const auto msg = m.getMessage();
            if (msg.isNoteOn())
            {
                if (lastMonoNote >= 0) out.addEvent (juce::MidiMessage::noteOff (msg.getChannel(), lastMonoNote), m.samplePosition);
                lastMonoNote = msg.getNoteNumber();
            }
            else if (msg.isNoteOff() && msg.getNoteNumber() == lastMonoNote) lastMonoNote = -1;
            else if (msg.isAllNotesOff() || msg.isAllSoundOff()) lastMonoNote = -1;
            out.addEvent (msg, m.samplePosition);
        }
        midi.swapWith (out);
    }
    else lastMonoNote = -1;

    synth.renderNextBlock (buffer, midi, 0, n);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : L;

    // distorsion
    distSm.setTargetValue (ptrs (Cz::dist));
    if (distSm.getCurrentValue() > 0.005f || distSm.getTargetValue() > 0.005f)
    {
        for (int i = 0; i < n; ++i)
        {
            const float dist = distSm.getNextValue();
            const float drive = 1.f + dist * 14.f, wet = juce::jmin (1.f, dist * 2.f), comp = 1.f / (1.f + dist * 1.5f);
            L[i] = (L[i] * (1 - wet) + std::tanh (L[i] * drive) * 0.8f * wet) * comp;
            R[i] = (R[i] * (1 - wet) + std::tanh (R[i] * drive) * 0.8f * wet) * comp;
        }
    }

    // chorus
    const float cho = ptrs (Cz::chorus);
    if (cho > 0.005f)
    {
        chorus.setMix (cho * 0.8f);
        juce::dsp::AudioBlock<float> blk (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (blk);
        chorus.process (ctx);
    }

    // delay ping-pong
    delaySm.setTargetValue (ptrs (Cz::delay));
    if (delaySm.getCurrentValue() > 0.005f || delaySm.getTargetValue() > 0.005f)
    {
        const float fbk = ptrs (fb), targetLen = juce::jlimit (8.f, (float) dly.bl.size() - 4.f, ptrs (dtime) * (float) sampleRate);
        const int size = (int) dly.bl.size();
        for (int i = 0; i < n; ++i)
        {
            const float dmix = delaySm.getNextValue();
            dly.smoothLen += (targetLen - dly.smoothLen) * 0.0005f;
            const float dl = dly.read (dly.bl, dly.smoothLen), dr = dly.read (dly.br, dly.smoothLen);
            dly.bl[(size_t) dly.w] = L[i] + dr * fbk;
            dly.br[(size_t) dly.w] = R[i] + dl * fbk;
            dly.w = (dly.w + 1) % size;
            L[i] += dl * dmix; R[i] += dr * dmix;
        }
    }

    // reverb
    const float rv = ptrs (Cz::reverb);
    if (rv > 0.005f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.5f + 0.45f * rv; rp.damping = 0.45f; rp.wetLevel = rv * 0.4f; rp.dryLevel = 1.f; rp.width = 1.f;
        reverb.setParameters (rp);
        reverb.processStereo (L, R, n);
    }

    for (int i = 0; i < n; ++i) // bloqueur DC master puis limiteur doux
    {
        const float l = L[i], r = R[i];
        const float yl = l - dcXl + dcRm * dcYl, yr = r - dcXr + dcRm * dcYr;
        dcXl = l; dcYl = yl; dcXr = r; dcYr = yr;
        L[i] = std::tanh (yl);
        if (R != L) R[i] = std::tanh (yr);
    }
}

void CzProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", presetIndex, nullptr);
    state.setProperty ("presetName", presetName, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void CzProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            presetIndex = vt.getProperty ("presetIndex", -1);
            presetName = vt.getProperty ("presetName", "CHINTZWAZ - Init Patch").toString();
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* CzProcessor::createEditor() { return new CzEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CzProcessor(); }
