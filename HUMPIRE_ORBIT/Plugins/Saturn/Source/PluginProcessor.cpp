#include <algorithm>
#include <cmath>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SaturnPresets.h"

using namespace saturn;

namespace
{
    float dbToGain (float db) noexcept { return juce::Decibels::decibelsToGain (db, -100.0f); }
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
    const bool bypassed = pBypass->load() > 0.5f;
    mixSm.reset (sampleRate, 0.02);
    mixSm.setCurrentAndTargetValue (bypassed ? 0.0f : 0.01f * pMix->load());
    inSm.reset (sampleRate, 0.02);
    inSm.setCurrentAndTargetValue (dbToGain (pIn->load()));
    outSm.reset (sampleRate, 0.02);
    outSm.setCurrentAndTargetValue (dbToGain (pOut->load()));
}

void SaturnAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    engine.prepare (sr);
    setLatencySamples (engine.latencySamples());
    setupSmoothers (sr);
    engine.setSettings (readSettings());
    waveCounter = 0;
    wavePeak = 0.0f;
    inLevel.store (0.0f);
    outLevel.store (0.0f);
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
        vs.gain = dbToGain (p[vLevel]->load());
        vs.pan = 0.01f * p[vPan]->load();
        vs.detuneCents = p[vDetune]->load();
    }
    return s;
}

void SaturnAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
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
    for (int c = 2; c < nCh; ++c) buffer.clear (c, 0, n);
    if (n <= 0 || nCh <= 0) return;

    engine.setSettings (readSettings());
    inSm.setTargetValue (dbToGain (pIn->load()));
    outSm.setTargetValue (dbToGain (pOut->load()));
    const bool bypassed = hostBypassed || pBypass->load() > 0.5f;
    mixSm.setTargetValue (bypassed ? 0.0f : 0.01f * pMix->load());

    float* left = buffer.getWritePointer (0);
    float* right = nCh > 1 ? buffer.getWritePointer (1) : nullptr;
    float peakIn = 0.0f, peakOut = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float ig = inSm.getNextValue();
        const float inL = left[i] * ig;
        const float inR = right != nullptr ? right[i] * ig : inL;

        float dryL = 0.0f, dryR = 0.0f, wetL = 0.0f, wetR = 0.0f;
        engine.process (inL, inR, dryL, dryR, wetL, wetR);

        // Mix: 0 % = dry only (aligned with the harmony voices), 100 % = dry + harmony bus.
        const float m = mixSm.getNextValue();
        const float og = outSm.getNextValue();
        if (right != nullptr)
        {
            left[i] = (dryL + m * wetL) * og;
            right[i] = (dryR + m * wetR) * og;
            peakOut = std::max (peakOut, std::max (std::abs (left[i]), std::abs (right[i])));
        }
        else
        {
            left[i] = (dryL + m * 0.5f * (wetL + wetR)) * og;
            peakOut = std::max (peakOut, std::abs (left[i]));
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
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* SaturnAudioProcessor::createEditor() { return new SaturnEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SaturnAudioProcessor(); }
