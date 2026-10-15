#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SaturnPresets.h"

using namespace saturn;

SaturnAudioProcessor::SaturnAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "HUMPIRE_SATURN", createLayout()),
      presets (apvts, "Saturn")
{
    for (auto& a : uiVoiceShift) a.store (0.0f);
    for (auto& a : uiVoiceLevel) a.store (0.0f);

    pMode = apvts.getRawParameterValue (id::mode);           pKey = apvts.getRawParameterValue (id::key);
    pScale = apvts.getRawParameterValue (id::scale);         pFormant = apvts.getRawParameterValue (id::formant);
    pGlide = apvts.getRawParameterValue (id::glide);         pCorrection = apvts.getRawParameterValue (id::correction);
    pHumanize = apvts.getRawParameterValue (id::humanize);   pWidth = apvts.getRawParameterValue (id::width);
    pMix = apvts.getRawParameterValue (id::mix);             pIn = apvts.getRawParameterValue (id::inGain);
    pOut = apvts.getRawParameterValue (id::outGain);         pBypass = apvts.getRawParameterValue (id::bypass);
    for (int v = 0; v < kNumVoices; ++v)
        for (int k = 0; k < kNumVoiceParams; ++k)
            pVoice[(size_t) v][(size_t) k] = apvts.getRawParameterValue (voiceId (v, (VoiceParam) k));
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id::bypass));

    // La tonalite est celle du morceau : un preset ne la change pas.
    presets.setPreservedParameters ({ id::bypass, id::simple, id::inGain, id::outGain, id::key });
    presets.setFactoryPresets (makeSaturnFactoryPresets());

    // Pre-preparation : processBlock reste sur meme si l'hote l'appelle avant prepareToPlay.
    prepareToPlay (48000.0, 512);
}

bool SaturnAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo()) && o == l.getMainInputChannelSet();
}

void SaturnAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate > 0.0 ? sampleRate : 48000.0;
    engine.setSettings (readSettings());
    engine.prepare (sr);
    setLatencySamples (engine.latencySamples());

    const float m = *pBypass > 0.5f ? 0.0f : pMix->load() * 0.01f;
    mixSm.reset (sr, 0.02);  mixSm.setCurrentAndTargetValue (m);
    inSm.reset (sr, 0.02);   inSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outSm.reset (sr, 0.02);  outSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    waveCounter = 0; wavePeak = 0.0f;
}

EngineSettings SaturnAudioProcessor::readSettings() const noexcept
{
    EngineSettings s;
    s.degreeMode = pMode->load() > 0.5f;
    s.key        = juce::jlimit (0, 11, (int) pKey->load());
    s.scale      = juce::jlimit (0, (int) NumScales - 1, (int) pScale->load());
    s.formant    = pFormant->load() > 0.5f;
    s.glideMs    = pGlide->load();
    s.correction = pCorrection->load() * 0.01f;
    s.humanize   = pHumanize->load() * 0.01f;
    const float width = pWidth->load() * 0.01f;
    for (int v = 0; v < kNumVoices; ++v)
    {
        const auto& p = pVoice[(size_t) v];
        auto& vs = s.voice[(size_t) v];
        const float lvl = p[vLevel]->load();
        vs.on          = p[vOn]->load() > 0.5f;
        vs.semi        = (int) std::lround (p[vSemi]->load());
        vs.deg         = (int) std::lround (p[vDeg]->load());
        vs.gain        = lvl <= -59.9f ? 0.0f : juce::Decibels::decibelsToGain (lvl);
        vs.pan         = juce::jlimit (-1.0f, 1.0f, p[vPan]->load() * 0.01f * width);
        vs.detuneCents = p[vDetune]->load();
        vs.delayMs     = p[vDelay]->load();
    }
    return s;
}

void SaturnAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int nCh = juce::jmin (HarmonyEngine::maxChannels, buffer.getNumChannels());
    for (int c = getTotalNumInputChannels(); c < juce::jmin (getTotalNumOutputChannels(), buffer.getNumChannels()); ++c)
        buffer.clear (c, 0, n);
    if (nCh == 0 || n <= 0 || ! engine.isPrepared()) return;

    engine.setSettings (readSettings());
    inSm.setTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outSm.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    mixSm.setTargetValue (*pBypass > 0.5f ? 0.0f : pMix->load() * 0.01f);

    float* ch[HarmonyEngine::maxChannels] = { buffer.getWritePointer (0), nCh > 1 ? buffer.getWritePointer (1) : nullptr };
    float peakIn = 0.0f, peakOut = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float g = inSm.getNextValue();
        float in[HarmonyEngine::maxChannels] {}, dry[HarmonyEngine::maxChannels] {}, wet[HarmonyEngine::maxChannels] {};
        for (int c = 0; c < nCh; ++c) { in[c] = ch[c][i] * g; peakIn = juce::jmax (peakIn, std::abs (in[c])); }

        engine.process (in, dry, wet, nCh);

        // Mix "balance" : 0 % = sec seul, 50 % = sec + harmonies a plein niveau, 100 % = harmonies seules.
        const float m = mixSm.getNextValue(), og = outSm.getNextValue();
        const float dg = juce::jmin (1.0f, 2.0f * (1.0f - m)), wg = juce::jmin (1.0f, 2.0f * m);
        float mono = 0.0f;
        for (int c = 0; c < nCh; ++c)
        {
            const float o = (dry[c] * dg + wet[c] * wg) * og;
            ch[c][i] = o;
            peakOut = juce::jmax (peakOut, std::abs (o));
            mono += o;
        }
        wavePeak = juce::jmax (wavePeak, std::abs (mono) / (float) nCh);
        if (++waveCounter >= 1024) { waveform.push (wavePeak); wavePeak = 0.0f; waveCounter = 0; }
    }

    // --- donnees UI ---
    const float decay = std::exp ((float) (-n / (0.3 * sr)));
    inLevel.store (juce::jmax (peakIn, inLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    outLevel.store (juce::jmax (peakOut, outLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    uiInMidi.store (engine.isVoiced() ? engine.getDetectedMidi() : -1.0f, std::memory_order_relaxed);
    uiTrackedNote.store (engine.getTrackedNote(), std::memory_order_relaxed);
    for (int v = 0; v < kNumVoices; ++v)
    {
        uiVoiceShift[(size_t) v].store (engine.getVoiceShift (v), std::memory_order_relaxed);
        const float pk = engine.takeVoicePeak (v);
        uiVoiceLevel[(size_t) v].store (juce::jmax (pk, uiVoiceLevel[(size_t) v].load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    }
}

//==============================================================================
void SaturnAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void SaturnAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* SaturnAudioProcessor::createEditor() { return new SaturnEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SaturnAudioProcessor(); }
