#include "HumpireProcessor.h"

namespace humpire
{
juce::AudioProcessorValueTreeState::ParameterLayout HumpireProcessor::withCommon (juce::AudioProcessorValueTreeState::ParameterLayout l)
{
    l.add (mkBool ("bypass", "Bypass", false));
    l.add (mkBool ("simple", "UI Simple", false, false));
    l.add (mkBool ("clean",  "UI Clean",  false, false));
    return l;
}

HumpireProcessor::HumpireProcessor (const juce::String& n, const juce::String& key,
                                    juce::AudioProcessorValueTreeState::ParameterLayout layout,
                                    std::vector<FactoryPreset> factory, bool wantsMidi)
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undo, "HUMPIRE_" + key, withCommon (std::move (layout))),
      presets (apvts, key, std::move (factory)),
      name (n), midi (wantsMidi)
{
    for (auto& a : aux) a.store (0.0f);
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter ("bypass"));
}

bool HumpireProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo()) && o == l.getMainInputChannelSet();
}

void HumpireProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& m)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c) buffer.clear (c, 0, n);
    if (buffer.getNumChannels() == 0 || n == 0) return;
    if (! isPrepared) { buffer.clear(); return; }

    const float inMag = buffer.getMagnitude (0, n);
    atomicMax (inPeak, inMag);

    // Bypass : passage direct du signal (le wrapper VST3 appelle processBlock dans tous les cas).
    if (bypassParam != nullptr && bypassParam->get()) { atomicMax (outPeak, inMag); return; }

    // Un hote peut envoyer des blocs plus grands que celui annonce dans prepareToPlay : on decoupe.
    float* const* ptrs = buffer.getArrayOfWritePointers();
    const int nch = buffer.getNumChannels();
    juce::MidiBuffer noMidi;
    for (int start = 0; start < n; start += preparedBlock)
    {
        const int len = juce::jmin (preparedBlock, n - start);
        juce::AudioBuffer<float> chunk (ptrs, nch, start, len);
        process (chunk, start == 0 ? m : noMidi);
    }
    atomicMax (outPeak, buffer.getMagnitude (0, n));
}

void HumpireProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    saveExtraState (state);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void HumpireProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            loadExtraState (vt);
            apvts.replaceState (vt);
        }
}
} // namespace humpire
