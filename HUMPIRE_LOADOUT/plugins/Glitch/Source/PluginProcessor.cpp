#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <HumpireCore/HumpireTheme.h>

GlitchAudioProcessor::GlitchAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undo, "HUMPIRE_GLITCH", glitch::createLayout()),
      presets (apvts, "GLITCH", glitch::factoryPresets())
{
    using namespace glitch::id;
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (bypass));
    pBits    = apvts.getRawParameterValue (bits);
    pRate    = apvts.getRawParameterValue (rate);
    pDrive   = apvts.getRawParameterValue (drive);
    pSpeaker = apvts.getRawParameterValue (speaker);
    pTone    = apvts.getRawParameterValue (tone);
    pDither  = apvts.getRawParameterValue (dither);
    pMix     = apvts.getRawParameterValue (mix);
    pOut     = apvts.getRawParameterValue (out);
}

bool GlitchAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo())
           && o == l.getMainInputChannelSet();
}

void GlitchAudioProcessor::prepareToPlay (double sr, int maxBlock)
{
    dsp.prepare (sr, maxBlock);
    prepared = true;
    scopeStride = juce::jmax (1, (int) (sr / 50.0));       // 50 colonnes/s -> ~1.3 s d'historique
    setLatencySamples (0);                                  // aucun lookahead
}

void GlitchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);

    if (! prepared || buffer.getNumChannels() == 0) { buffer.clear(); return; }
    humpire::atomicMax (inPeak, buffer.getMagnitude (0, n));
    if (bypassParam != nullptr && bypassParam->get()) { humpire::atomicMax (outPeak, buffer.getMagnitude (0, n)); return; }

    glitch::DspParams p;
    p.bits    = pBits->load();      p.rate  = pRate->load();     p.drive = pDrive->load();
    p.tone    = pTone->load();      p.mix   = pMix->load();      p.outDb = pOut->load();
    p.speaker = pSpeaker->load() > 0.5f;
    p.dither  = juce::roundToInt (pDither->load());
    dsp.setParams (p);
    dsp.process (buffer, &scope, scopeStride);

    humpire::atomicMax (outPeak, buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* GlitchAudioProcessor::createEditor() { return new GlitchEditor (*this); }

void GlitchAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("presetName", presets.getCurrentName(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void GlitchAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GlitchAudioProcessor(); }
