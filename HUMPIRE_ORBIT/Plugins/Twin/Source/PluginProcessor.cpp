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
    sr = sampleRate > 1000.0 ? sampleRate : 44100.0;
    doubler.prepare (sr);
    doubler.setSettings (readSettings());
    doubler.reset();
    setLatencySamples (0);   // the doubles are an audible delay effect, not a processing latency

    mixSm.reset (sr, 0.02);  mixSm.setCurrentAndTargetValue (targetMix());
    outSm.reset (sr, 0.02);  outSm.setCurrentAndTargetValue (targetOutGain());
    waveCounter = 0;
    wavePeak = 0.0f;
    prepared = true;
}

DoublerSettings TwinAudioProcessor::readSettings() const
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

float TwinAudioProcessor::targetMix() const
{
    if (pBypass->load() > 0.5f) return 0.0f;
    return juce::jlimit (0.0f, 1.0f, pMix->load() * 0.01f);
}

float TwinAudioProcessor::targetOutGain() const
{
    return juce::Decibels::decibelsToGain (pOutGain->load());
}

void TwinAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nOut = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    if (! prepared || nOut <= 0 || n <= 0) return;   // before prepareToPlay: audio passes through

    for (int c = 2; c < nOut; ++c) buffer.clear (c, 0, n);
    const int nCh = juce::jmin (2, nOut);

    const auto rs = reseedCounter.load (std::memory_order_relaxed);
    if (rs != lastReseed)
    {
        lastReseed = rs;
        doubler.reseed (0x2F6E2B1u + rs * 0x9E3779B1u);
    }

    doubler.setSettings (readSettings());
    mixSm.setTargetValue (targetMix());
    outSm.setTargetValue (targetOutGain());

    float* ch0 = buffer.getWritePointer (0);
    float* ch1 = nCh > 1 ? buffer.getWritePointer (1) : nullptr;
    float peakIn = 0.0f, peakOut = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float dryL = ch0[i];
        const float dryR = ch1 != nullptr ? ch1[i] : dryL;
        const float mono = ch1 != nullptr ? 0.5f * (dryL + dryR) : dryL;

        float wetL = 0.0f, wetR = 0.0f;
        doubler.process (mono, wetL, wetR);
        if (ch1 == nullptr) wetL = 0.5f * (wetL + wetR);   // mono output: fold the wet bus

        const float m = mixSm.getNextValue();
        const float g = outSm.getNextValue();

        const float oL = (dryL * (1.0f - m) + wetL * m) * g;
        ch0[i] = oL;
        peakOut = juce::jmax (peakOut, std::abs (oL));
        if (ch1 != nullptr)
        {
            const float oR = (dryR * (1.0f - m) + wetR * m) * g;
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

    const float decay = std::exp ((float) (-n / (0.3 * sr)));
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
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* TwinAudioProcessor::createEditor() { return new TwinEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TwinAudioProcessor(); }
