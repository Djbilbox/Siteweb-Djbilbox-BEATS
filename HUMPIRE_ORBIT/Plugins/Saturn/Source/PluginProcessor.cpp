#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SaturnPresets.h"

using namespace saturn;

namespace
{
    float dbToGain (float db) noexcept { return juce::Decibels::decibelsToGain (db, -100.0f); }

    /** Voice level in dB -> linear gain. The minimum of the parameter ("-inf dB") is true silence. */
    float levelToGain (float db) noexcept { return db <= kMinLevelDb + 0.05f ? 0.0f : dbToGain (db); }

    /** NaN and infinity become silence: one bad sample from the host must not poison the pitch-detector filters
        for good. Bit test instead of std::isfinite, so it also holds with fast-math style compiler options. */
    float finiteOrZero (float x) noexcept
    {
        std::uint32_t bits = 0;
        std::memcpy (&bits, &x, sizeof bits);
        return (bits & 0x7f800000u) == 0x7f800000u ? 0.0f : x;
    }
}

SaturnAudioProcessor::SaturnAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "HUMPIRE_SATURN", createLayout()),
      presets (apvts, "Saturn")
{
    pMode = apvts.getRawParameterValue (id::mode);
    pKey = apvts.getRawParameterValue (id::key);
    pScale = apvts.getRawParameterValue (id::scale);
    pGlide = apvts.getRawParameterValue (id::glide);
    pMix = apvts.getRawParameterValue (id::mix);
    pIn = apvts.getRawParameterValue (id::inGain);
    pOut = apvts.getRawParameterValue (id::outGain);
    pBypass = apvts.getRawParameterValue (id::bypass);
    for (int v = 0; v < kNumVoices; ++v)
        for (int k = 0; k < kNumVoiceParams; ++k)
            pVoice[(size_t) v][(size_t) k] = apvts.getRawParameterValue (voiceId (v, (VoiceParam) k));
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id::bypass));

    presets.setFactoryPresets (makeSaturnFactoryPresets());

    // Usable before prepareToPlay(): the engine and the smoothers are set up at 44.1 kHz here.
    engine.prepare (sr);
    bypassDelay.prepare (engine.latencySamples());
    setupSmoothers (sr);
    engine.setSettings (readSettings());
    setLatencySamples (engine.latencySamples());
}

bool SaturnAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == layouts.getMainInputChannelSet();
}

void SaturnAudioProcessor::setupSmoothers (double sampleRate)
{
    // Every smoother starts at the current parameter value: no fade when the plugin is loaded or re-prepared.
    mixSm.reset (sampleRate, 0.02);
    mixSm.setCurrentAndTargetValue (0.01f * pMix->load());
    inSm.reset (sampleRate, 0.02);
    inSm.setCurrentAndTargetValue (dbToGain (pIn->load()));
    outSm.reset (sampleRate, 0.02);
    outSm.setCurrentAndTargetValue (dbToGain (pOut->load()));
    bypassSm.reset (sampleRate, 0.01);
    bypassSm.setCurrentAndTargetValue (pBypass->load() > 0.5f ? 1.0f : 0.0f);
}

void SaturnAudioProcessor::resetDisplayData() noexcept
{
    waveCounter = 0;
    wavePeak = 0.0f;
    inLevel.store (0.0f);
    outLevel.store (0.0f);
    uiDetMidi.store (-1.0f);
    uiTrackedMidi.store (-1.0f);
    for (int v = 0; v < kNumVoices; ++v)
    {
        uiVoiceShift[(size_t) v].store (0.0f);
        uiVoiceLevel[(size_t) v].store (0.0f);
    }
}

void SaturnAudioProcessor::prepareToPlay (double sampleRate, int)
{
    // No block-sized buffer exists: processing is sample by sample, so any block length is fine.
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    engine.prepare (sr);                              // allocates; clears the detector and the delay line
    bypassDelay.prepare (engine.latencySamples());    // zeroed
    setLatencySamples (engine.latencySamples());
    setupSmoothers (sr);
    engine.setSettings (readSettings());
    engine.reset();                                   // voices start from the current settings, not from the previous run
    resetDisplayData();
}

EngineSettings SaturnAudioProcessor::readSettings() const noexcept
{
    EngineSettings s;
    s.degreeMode = pMode->load() < 0.5f;
    s.key = juce::jlimit (0, 11, (int) pKey->load());
    s.scale = juce::jlimit (0, (int) NumScales - 1, (int) pScale->load());
    s.glideMs = pGlide->load();
    for (int v = 0; v < kNumVoices; ++v)
    {
        const auto& p = pVoice[(size_t) v];
        auto& vs = s.voice[(size_t) v];
        vs.on = p[vOn]->load() > 0.5f;
        vs.semi = juce::jlimit (-24, 24, (int) std::lround (p[vSemi]->load()));
        vs.deg = juce::jlimit (-14, 14, (int) std::lround (p[vDeg]->load()));
        vs.gain = levelToGain (p[vLevel]->load());
        vs.pan = 0.01f * p[vPan]->load();
        vs.detuneCents = p[vDetune]->load();
    }
    return s;
}

void SaturnAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // The VST3 wrapper always lands here, even when the host bypasses the plugin: the bypass parameter is
    // handled inside processInternal (latency-aligned pass-through with a short crossfade).
    processInternal (buffer, false);
}

void SaturnAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    processInternal (buffer, true);
}

void SaturnAudioProcessor::processInternal (juce::AudioBuffer<float>& buffer, bool hostBypassed)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buffer.getNumSamples();
    const int nCh = buffer.getNumChannels();
    if (n <= 0 || nCh <= 0) return;
    for (int c = 2; c < nCh; ++c) buffer.clear (c, 0, n);

    engine.setSettings (readSettings());
    inSm.setTargetValue (dbToGain (pIn->load()));
    outSm.setTargetValue (dbToGain (pOut->load()));
    mixSm.setTargetValue (0.01f * pMix->load());
    bypassSm.setTargetValue (hostBypassed || pBypass->load() > 0.5f ? 1.0f : 0.0f);

    float* left = buffer.getWritePointer (0);
    float* right = nCh > 1 ? buffer.getWritePointer (1) : nullptr;
    float peakIn = 0.0f, peakOut = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float xl = finiteOrZero (left[i]);
        const float xr = right != nullptr ? finiteOrZero (right[i]) : xl;

        // bypass path: the untouched input, delayed by the same latency as the processed path
        float rawL = 0.0f, rawR = 0.0f;
        bypassDelay.process (xl, xr, rawL, rawR);
        const float b = bypassSm.getNextValue();       // 0 = processed, 1 = bypassed (exactly)

        const float ig = inSm.getNextValue();
        const float inL = xl * ig;
        const float inR = xr * ig;

        float dryL = 0.0f, dryR = 0.0f, wetL = 0.0f, wetR = 0.0f;
        engine.process (inL, inR, dryL, dryR, wetL, wetR, b < 1.0f);

        // Mix: 0 % = dry only (aligned with the harmony voices), 100 % = dry + harmony bus.
        const float m = mixSm.getNextValue();
        const float og = outSm.getNextValue();

        float outL, outR = 0.0f;
        if (b >= 1.0f)
        {
            outL = rawL;
            outR = rawR;
        }
        else
        {
            float procL, procR = 0.0f;
            if (right != nullptr)
            {
                procL = (dryL + m * wetL) * og;
                procR = (dryR + m * wetR) * og;
            }
            else
                procL = (dryL + m * 0.5f * (wetL + wetR)) * og;

            outL = b > 0.0f ? procL + b * (rawL - procL) : procL;
            outR = b > 0.0f ? procR + b * (rawR - procR) : procR;
        }

        left[i] = outL;
        peakOut = std::max (peakOut, std::abs (outL));
        if (right != nullptr)
        {
            right[i] = outR;
            peakOut = std::max (peakOut, std::abs (outR));
        }

        peakIn = std::max (peakIn, std::max (std::abs (inL), std::abs (inR)));
        wavePeak = std::max (wavePeak, std::abs (0.5f * (inL + inR)));
        if (++waveCounter >= 1024)
        {
            waveform.push (wavePeak);
            wavePeak = 0.0f;
            waveCounter = 0;
        }
    }

    // display data
    const float decay = std::exp (-(float) n / (0.3f * (float) sr));
    inLevel.store (std::max (peakIn, inLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    outLevel.store (std::max (peakOut, outLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    const bool voiced = engine.isVoiced();
    uiDetMidi.store (voiced ? engine.getDetectedMidi() : -1.0f, std::memory_order_relaxed);
    uiTrackedMidi.store (voiced ? engine.getTrackedMidi() : -1.0f, std::memory_order_relaxed);
    for (int v = 0; v < kNumVoices; ++v)
    {
        uiVoiceShift[(size_t) v].store (engine.getVoiceShift (v), std::memory_order_relaxed);
        const float pk = engine.takeVoicePeak (v);
        uiVoiceLevel[(size_t) v].store (std::max (pk, uiVoiceLevel[(size_t) v].load (std::memory_order_relaxed) * decay),
                                        std::memory_order_relaxed);
    }
}

//==============================================================================
void SaturnAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void SaturnAudioProcessor::setStateInformation (const void* data, int size)
{
    if (data == nullptr || size <= 0) return;
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* SaturnAudioProcessor::createEditor() { return new SaturnEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SaturnAudioProcessor(); }
