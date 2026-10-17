#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "TwinPresets.h"

using namespace twin;

TwinAudioProcessor::TwinAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "HUMPIRE_TWIN", createLayout()),
      presets (apvts, "Twin")
{
    // Cached parameter pointers: the audio thread never looks parameters up by id
    pDoubles  = apvts.getRawParameterValue (id::doubles);
    pDetune   = apvts.getRawParameterValue (id::detune);
    pDelay    = apvts.getRawParameterValue (id::delay);
    pHumanize = apvts.getRawParameterValue (id::humanize);
    pDrift    = apvts.getRawParameterValue (id::drift);
    pWidth    = apvts.getRawParameterValue (id::width);
    pHp       = apvts.getRawParameterValue (id::hpFreq);
    pLp       = apvts.getRawParameterValue (id::lpFreq);
    pMix      = apvts.getRawParameterValue (id::mix);
    pOutGain  = apvts.getRawParameterValue (id::outGain);
    pBypass   = apvts.getRawParameterValue (id::bypass);
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id::bypass));

    presets.setFactoryPresets (makeTwinFactoryPresets());
}

bool TwinAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo()) && o == l.getMainInputChannelSet();
}

void TwinAudioProcessor::prepareToPlay (double sampleRate, int)
{
    prepared.store (false, std::memory_order_release);

    sr = sampleRate > 1000.0 ? sampleRate : 44100.0;
    doubler.prepare (sr);
    doubler.setSettings (readSettings());
    doubler.reset();
    setLatencySamples (0);   // the doubles are an audible delay effect, not a processing latency

    mixSm.reset (sr, 0.02);     mixSm.setCurrentAndTargetValue (targetMix());
    outSm.reset (sr, 0.02);     outSm.setCurrentAndTargetValue (targetOutGain());
    activeSm.reset (sr, 0.015); activeSm.setCurrentAndTargetValue (targetBypass());
    waveCounter = 0;
    wavePeak = 0.0f;
    inLevel.store (0.0f, std::memory_order_relaxed);
    outLevel.store (0.0f, std::memory_order_relaxed);

    prepared.store (true, std::memory_order_release);
}

DoublerSettings TwinAudioProcessor::readSettings() const noexcept
{
    DoublerSettings s;
    s.voices   = juce::jlimit (1, kMaxVoices, juce::roundToInt (pDoubles->load()));
    s.detuneCt = pDetune->load();
    s.delayMs  = pDelay->load();
    s.humanize = pHumanize->load() * 0.01f;
    s.driftHz  = pDrift->load();
    s.width    = pWidth->load() * 0.01f;
    s.hpHz     = pHp->load();
    s.lpHz     = pLp->load();
    return s;
}

float TwinAudioProcessor::targetMix() const noexcept
{
    return juce::jlimit (0.0f, 1.0f, pMix->load() * 0.01f);
}

float TwinAudioProcessor::targetOutGain() const noexcept
{
    return juce::Decibels::decibelsToGain (pOutGain->load());
}

float TwinAudioProcessor::targetBypass() const noexcept
{
    return pBypass->load() > 0.5f ? 0.0f : 1.0f;
}

void TwinAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nOut = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    if (nOut <= 0 || n <= 0) return;
    if (! prepared.load (std::memory_order_acquire)) return;   // before prepareToPlay: audio passes through untouched

    for (int c = 2; c < nOut; ++c) buffer.clear (c, 0, n);     // the bus is mono or stereo; anything else is silence

    const auto rs = reseedCounter.load (std::memory_order_relaxed);
    if (rs != lastReseed)
    {
        lastReseed = rs;
        doubler.reseed (0x2F6E2B1u + rs * 0x9E3779B1u);
    }

    doubler.setSettings (readSettings());
    mixSm.setTargetValue (targetMix());
    outSm.setTargetValue (targetOutGain());
    activeSm.setTargetValue (targetBypass());

    float* ch0 = buffer.getWritePointer (0);
    float* ch1 = nOut > 1 ? buffer.getWritePointer (1) : nullptr;
    float peakIn = 0.0f, peakOut = 0.0f;

    // The whole block is processed in one loop, so any block size works (also larger than the one
    // given to prepareToPlay). The doubler keeps running while bypassed so that leaving bypass is seamless.
    for (int i = 0; i < n; ++i)
    {
        const float dryL = ch0[i];
        const float dryR = ch1 != nullptr ? ch1[i] : dryL;
        float mono = ch1 != nullptr ? 0.5f * (dryL + dryR) : dryL;
        if (! (std::abs (mono) < 1.0e6f)) mono = 0.0f;   // NaN / Inf / absurd input must not poison the delay line and filters

        float wetL = 0.0f, wetR = 0.0f;
        doubler.process (mono, wetL, wetR);
        if (ch1 == nullptr) wetL = 0.5f * (wetL + wetR);   // mono output: fold the wet bus

        const float m = mixSm.getNextValue();
        const float g = outSm.getNextValue();
        const float a = activeSm.getNextValue();   // 1 = processed, 0 = bypassed (exact input)

        const float pL = (dryL * (1.0f - m) + wetL * m) * g;
        const float oL = a >= 1.0f ? pL : (a <= 0.0f ? dryL : dryL + (pL - dryL) * a);
        ch0[i] = oL;
        peakOut = juce::jmax (peakOut, std::abs (oL));
        if (ch1 != nullptr)
        {
            const float pR = (dryR * (1.0f - m) + wetR * m) * g;
            const float oR = a >= 1.0f ? pR : (a <= 0.0f ? dryR : dryR + (pR - dryR) * a);
            ch1[i] = oR;
            peakOut = juce::jmax (peakOut, std::abs (oR));
        }
        peakIn = juce::jmax (peakIn, std::abs (mono));

        wavePeak = juce::jmax (wavePeak, std::abs (oL));
        if (++waveCounter >= 1024)
        {
            waveform.push (wavePeak);
            wavePeak = 0.0f;
            waveCounter = 0;
        }
    }

    // Publish live values for the UI (once per block, no allocation)
    ui.voices.store (doubler.getActiveVoices(), std::memory_order_relaxed);
    for (int k = 0; k < kMaxVoices; ++k)
    {
        const auto i = (size_t) k;
        ui.delayMs[i].store (doubler.getVoiceDelayMs (k), std::memory_order_relaxed);
        ui.detuneCt[i].store (doubler.getVoiceDetuneCt (k), std::memory_order_relaxed);
        ui.pan[i].store (doubler.getVoicePan (k), std::memory_order_relaxed);
    }

    const float decay = std::exp ((float) (-(double) n / (0.3 * sr)));
    inLevel.store (juce::jmax (peakIn, inLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    outLevel.store (juce::jmax (peakOut, outLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
}

//==============================================================================
void TwinAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void TwinAudioProcessor::setStateInformation (const void* data, int size)
{
    if (data == nullptr || size <= 0) return;
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* TwinAudioProcessor::createEditor() { return new TwinEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TwinAudioProcessor(); }
