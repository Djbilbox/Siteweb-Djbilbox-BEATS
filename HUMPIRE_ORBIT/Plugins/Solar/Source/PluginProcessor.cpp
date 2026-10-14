#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SolarPresets.h"

using namespace solar;

SolarAudioProcessor::SolarAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "HUMPIRE_SOLAR", createLayout()),
      presets (apvts, "Solar")
{
    for (auto& g : graphDelta) g.store (0.0f);

    pMode = apvts.getRawParameterValue (id::mode);       pKey = apvts.getRawParameterValue (id::key);
    pScale = apvts.getRawParameterValue (id::scale);     pRetune = apvts.getRawParameterValue (id::retune);
    pFlex = apvts.getRawParameterValue (id::flex);       pHumanize = apvts.getRawParameterValue (id::humanize);
    pAmount = apvts.getRawParameterValue (id::amount);   pRefA = apvts.getRawParameterValue (id::refA);
    pVibDepth = apvts.getRawParameterValue (id::vibDepth); pVibRate = apvts.getRawParameterValue (id::vibRate);
    pVibDelay = apvts.getRawParameterValue (id::vibDelay); pPortamento = apvts.getRawParameterValue (id::portamento);
    pMix = apvts.getRawParameterValue (id::mix);         pIn = apvts.getRawParameterValue (id::inGain);
    pOut = apvts.getRawParameterValue (id::outGain);     pBypass = apvts.getRawParameterValue (id::bypass);
    for (int i = 0; i < 12; ++i) pNotes[(size_t) i] = apvts.getRawParameterValue (noteId (i));
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id::bypass));

    presets.setFactoryPresets (makeSolarFactoryPresets());
    presets.onStateRestored = [this] { loadGraphFromState(); };
}

bool SolarAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo()) && o == l.getMainInputChannelSet();
}

void SolarAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    detector.prepare (sr);
    shifter.prepare (sr);
    corrector.reset();
    setLatencySamples (shifter.latencySamples());

    mixSm.reset (sr, 0.02);  mixSm.setCurrentAndTargetValue (*pBypass > 0.5f ? 0.0f : *pMix * 0.01f);
    inSm.reset (sr, 0.02);   inSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outSm.reset (sr, 0.02);  outSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOut->load()));

    smoothedSemis = 0.0f; vibPhase = 0.0f; vibEnv = 0.0f;
    ratioCoef  = 1.0f - std::exp ((float) (-1.0 / (0.003 * sr)));    // lissage du ratio : 3 ms (anti-clic)
    vibEnvCoef = 1.0f - std::exp ((float) (-1.0 / (0.15 * sr)));     // fondu d'entree du vibrato : 150 ms
    waveCounter = 0; wavePeak = 0.0f;
}

CorrectionParams SolarAudioProcessor::readParams() const
{
    CorrectionParams p;
    p.graphMode    = pMode->load() > 0.5f;
    p.key          = (int) pKey->load();
    p.scale        = (int) pScale->load();
    p.noteMask     = 0;
    for (int i = 0; i < 12; ++i) if (pNotes[(size_t) i]->load() > 0.5f) p.noteMask |= 1u << i;
    p.retuneMs     = pRetune->load();
    p.flexCents    = pFlex->load();
    p.humanize     = pHumanize->load() * 0.01f;
    p.amount       = pAmount->load() * 0.01f;
    p.refA         = pRefA->load();
    p.portamentoMs = pPortamento->load();
    p.graph        = graphDelta.data();
    return p;
}

void SolarAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nCh = juce::jmin (PitchShifter::maxChannels, buffer.getNumChannels());
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c) buffer.clear (c, 0, n);
    if (nCh == 0) return;

    const auto cp = readParams();
    const float vibDepthSemis = pVibDepth->load() * 0.01f;
    const float vibInc = juce::MathConstants<float>::twoPi * pVibRate->load() / (float) sr;
    const float vibDelaySec = pVibDelay->load() * 0.001f;
    const double hopDt = (double) detector.hopInputSamples() / sr;

    inSm.setTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outSm.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    mixSm.setTargetValue (*pBypass > 0.5f ? 0.0f : pMix->load() * 0.01f);

    float* ch[PitchShifter::maxChannels] = { buffer.getWritePointer (0), nCh > 1 ? buffer.getWritePointer (1) : nullptr };
    float peakIn = 0.0f, peakOut = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float g = inSm.getNextValue();
        float in[PitchShifter::maxChannels] {}, wet[PitchShifter::maxChannels] {}, dry[PitchShifter::maxChannels] {};
        for (int c = 0; c < nCh; ++c) { in[c] = ch[c][i] * g; peakIn = juce::jmax (peakIn, std::abs (in[c])); }
        const float mono = nCh > 1 ? 0.5f * (in[0] + in[1]) : in[0];

        // --- analyse + decision (cadence ~100 Hz) ---
        if (detector.pushSample (mono))
        {
            const float f0 = detector.getF0();
            corrector.update (f0, hopDt, cp);
            if (f0 > 0.0f) shifter.setPeriod ((float) (sr / f0));

            const int h = historyHead.load (std::memory_order_relaxed);
            const bool v = corrector.isVoiced() && f0 > 0.0f;
            history[(size_t) h].det.store (v ? corrector.getDetectedMidi() : -1.0f, std::memory_order_relaxed);
            history[(size_t) h].out.store (v ? corrector.getDetectedMidi() + corrector.getOffsetSemitones() : -1.0f, std::memory_order_relaxed);
            historyHead.store ((h + 1) % historySize, std::memory_order_release);
            uiDetMidi.store (v ? corrector.getDetectedMidi() : -1.0f, std::memory_order_relaxed);
            uiTargetMidi.store (v ? corrector.getTargetMidi() : -1.0f, std::memory_order_relaxed);
            uiOffset.store (corrector.getOffsetSemitones(), std::memory_order_relaxed);
        }

        // --- vibrato (par echantillon) ---
        const bool vibOn = vibDepthSemis > 0.0f && corrector.isVoiced() && corrector.getVoicedTime() > vibDelaySec;
        vibEnv += ((vibOn ? 1.0f : 0.0f) - vibEnv) * vibEnvCoef;
        vibPhase += vibInc; if (vibPhase > juce::MathConstants<float>::twoPi) vibPhase -= juce::MathConstants<float>::twoPi;
        const float vib = vibEnv * vibDepthSemis * std::sin (vibPhase);

        smoothedSemis += (corrector.getOffsetSemitones() + vib - smoothedSemis) * ratioCoef;
        shifter.process (in, wet, dry, nCh, std::exp2 ((double) smoothedSemis / 12.0));

        const float m = mixSm.getNextValue(), og = outSm.getNextValue();
        for (int c = 0; c < nCh; ++c)
        {
            const float o = (dry[c] * (1.0f - m) + wet[c] * m) * og;
            ch[c][i] = o;
            peakOut = juce::jmax (peakOut, std::abs (o));
        }

        wavePeak = juce::jmax (wavePeak, std::abs (mono));
        if (++waveCounter >= 1024) { waveform.push (wavePeak); wavePeak = 0.0f; waveCounter = 0; }
    }

    const float decay = std::exp ((float) (-n / (0.3 * sr)));
    inLevel.store (juce::jmax (peakIn, inLevel.load() * decay), std::memory_order_relaxed);
    outLevel.store (juce::jmax (peakOut, outLevel.load() * decay), std::memory_order_relaxed);
}

//==============================================================================
void SolarAudioProcessor::commitGraphToState()
{
    juce::String s;
    for (auto& g : graphDelta) s << juce::String (g.load(), 3) << " ";
    apvts.state.getOrCreateChildWithName ("GRAPH", nullptr).setProperty ("d", s.trim(), nullptr);
}

void SolarAudioProcessor::loadGraphFromState()
{
    auto g = apvts.state.getChildWithName ("GRAPH");
    juce::StringArray tok;
    if (g.isValid()) tok.addTokens (g.getProperty ("d").toString(), " ", "");
    tok.removeEmptyStrings();
    for (int i = 0; i < kGraphPoints; ++i)
        graphDelta[(size_t) i].store (i < tok.size() ? juce::jlimit (-6.0f, 6.0f, tok[i].getFloatValue()) : 0.0f);
}

void SolarAudioProcessor::resetGraph()
{
    for (auto& g : graphDelta) g.store (0.0f);
    commitGraphToState();
}

void SolarAudioProcessor::snapGraphToScale()
{
    unsigned mask = 0;
    for (int i = 0; i < 12; ++i) if (pNotes[(size_t) i]->load() > 0.5f) mask |= 1u << i;
    const int key = (int) pKey->load();
    const auto deg = buildDegrees (key, (int) pScale->load(), mask);
    for (int i = 0; i < kGraphPoints; ++i)
    {
        const float pc = (float) i / 8.0f;
        graphDelta[(size_t) i].store (snapMidi (pc, key, deg) - pc);
    }
    commitGraphToState();
}

void SolarAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    commitGraphToState();
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void SolarAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            loadGraphFromState();
        }
}

juce::AudioProcessorEditor* SolarAudioProcessor::createEditor() { return new SolarEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SolarAudioProcessor(); }
