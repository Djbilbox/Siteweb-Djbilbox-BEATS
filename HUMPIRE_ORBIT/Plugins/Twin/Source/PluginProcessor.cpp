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
    pVoices = apvts.getRawParameterValue (id::voices);     pDetune = apvts.getRawParameterValue (id::detune);
    pDelay = apvts.getRawParameterValue (id::delay);       pSpread = apvts.getRawParameterValue (id::spread);
    pHumanize = apvts.getRawParameterValue (id::humanize); pDrift = apvts.getRawParameterValue (id::drift);
    pWidth = apvts.getRawParameterValue (id::width);       pHpf = apvts.getRawParameterValue (id::hpf);
    pLpf = apvts.getRawParameterValue (id::lpf);           pMix = apvts.getRawParameterValue (id::mix);
    pIn = apvts.getRawParameterValue (id::inGain);         pOut = apvts.getRawParameterValue (id::outGain);
    pBypass = apvts.getRawParameterValue (id::bypass);
    bypassParam = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id::bypass));

    presets.setFactoryPresets (makeTwinFactoryPresets());

    // Etat valide des la construction : un processBlock appele avant prepareToPlay ne plante pas.
    prepareInternal (44100.0);
}

bool TwinAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto& in = l.getMainInputChannelSet();
    const auto& out = l.getMainOutputChannelSet();
    const auto mono = juce::AudioChannelSet::mono(), stereo = juce::AudioChannelSet::stereo();
    if (out != mono && out != stereo) return false;
    if (in != mono && in != stereo) return false;
    return ! (in == stereo && out == mono);          // mono->mono, stereo->stereo, mono->stereo (doublage d'une piste mono)
}

void TwinAudioProcessor::prepareToPlay (double sampleRate, int)
{
    prepareInternal (sampleRate);
}

void TwinAudioProcessor::prepareInternal (double sampleRate)
{
    sr = sampleRate > 1000.0 ? sampleRate : 44100.0;
    engine.prepare (sr);
    engine.setSettings (readSettings());
    setLatencySamples (0);          // aucun lookahead : le sec n'est pas retarde, les doublures sont des retards voulus

    const float mix = pMix->load() * 0.01f;
    inSm.reset (sr, 0.02);      inSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outSm.reset (sr, 0.02);     outSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    drySm.reset (sr, 0.03);     drySm.setCurrentAndTargetValue (juce::jmin (1.0f, 2.0f * (1.0f - mix)));
    wetSm.reset (sr, 0.03);     wetSm.setCurrentAndTargetValue (juce::jmin (1.0f, 2.0f * mix));
    activeSm.reset (sr, 0.02);  activeSm.setCurrentAndTargetValue (pBypass->load() > 0.5f ? 0.0f : 1.0f);

    corrLR = corrLL = corrRR = 0.0;
    waveCounter = 0; wavePeak = 0.0f;
}

DoublerEngine::Settings TwinAudioProcessor::readSettings() const noexcept
{
    DoublerEngine::Settings s;
    s.voices      = juce::jlimit (1, kMaxVoices, (int) std::lround (pVoices->load()));
    s.detuneCents = pDetune->load();
    s.delayMs     = pDelay->load();
    s.spread      = pSpread->load() * 0.01f;
    s.humanize    = pHumanize->load() * 0.01f;
    s.driftHz     = pDrift->load();
    s.width       = pWidth->load() * 0.01f;
    s.hpfHz       = pHpf->load();
    s.lpfHz       = pLpf->load();
    return s;
}

void TwinAudioProcessor::process (juce::AudioBuffer<float>& buffer, bool forceBypass) noexcept
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int nBuf = buffer.getNumChannels();
    const int nIn  = juce::jmin (2, getTotalNumInputChannels(), nBuf);
    const int nOut = juce::jmin (2, getTotalNumOutputChannels(), nBuf);
    for (int c = juce::jmax (nIn, nOut); c < nBuf; ++c) buffer.clear (c, 0, n);   // voies hors traitement
    if (n <= 0 || nOut <= 0) return;
    if (nIn <= 0 || ! engine.isPrepared())
    {
        for (int c = 0; c < nOut; ++c) buffer.clear (c, 0, n);
        return;
    }

    // --- lecture des parametres (une fois par bloc, sans allocation) ---
    engine.setSettings (readSettings());
    const float mix = pMix->load() * 0.01f;
    const bool bypassed = forceBypass || pBypass->load() > 0.5f;
    inSm.setTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    outSm.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    drySm.setTargetValue (juce::jmin (1.0f, 2.0f * (1.0f - mix)));
    wetSm.setTargetValue (juce::jmin (1.0f, 2.0f * mix));
    activeSm.setTargetValue (bypassed ? 0.0f : 1.0f);

    float* L = buffer.getWritePointer (0);
    float* R = nOut > 1 ? buffer.getWritePointer (1) : nullptr;
    const bool stereoIn = nIn > 1 && R != nullptr;
    engine.clearPeaks();

    float peakIn = 0.0f, peakOut = 0.0f;
    double sLR = 0.0, sLL = 0.0, sRR = 0.0;

    for (int i = 0; i < n; ++i)
    {
        const float xl = L[i];
        const float xr = stereoIn ? R[i] : xl;               // mono -> stereo : le sec est duplique
        peakIn = juce::jmax (peakIn, std::abs (xl), std::abs (xr));

        const float g = inSm.getNextValue();
        const float dl = xl * g, dr = xr * g;
        const float src = stereoIn ? 0.5f * (dl + dr) : dl;

        float wl, wr, wc;
        engine.process (src, wl, wr, wc);

        const float dg = drySm.getNextValue(), wg = wetSm.getNextValue();
        const float og = outSm.getNextValue(), a = activeSm.getNextValue();

        if (R == nullptr)
        {
            const float o = (dl * dg + wc * wg) * og;
            const float y = xl + a * (o - xl);
            L[i] = y;
            peakOut = juce::jmax (peakOut, std::abs (y));
        }
        else
        {
            const float ol = (dl * dg + wl * wg) * og;
            const float orr = (dr * dg + wr * wg) * og;
            const float yl = xl + a * (ol - xl), yr = xr + a * (orr - xr);
            L[i] = yl; R[i] = yr;
            peakOut = juce::jmax (peakOut, std::abs (yl), std::abs (yr));
            sLR += (double) yl * (double) yr; sLL += (double) yl * (double) yl; sRR += (double) yr * (double) yr;
        }

        wavePeak = juce::jmax (wavePeak, std::abs (src));
        if (++waveCounter >= 1024) { waveform.push (wavePeak); wavePeak = 0.0f; waveCounter = 0; }
    }

    // --- mesures pour l'UI ---
    const float decay = std::exp ((float) (-n / (0.3 * sr)));
    inLevel.store (juce::jmax (peakIn, inLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    outLevel.store (juce::jmax (peakOut, outLevel.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);

    if (R != nullptr)
    {
        const double k = std::exp (-(double) n / (0.25 * sr));
        corrLR = corrLR * k + sLR; corrLL = corrLL * k + sLL; corrRR = corrRR * k + sRR;
        const double den = std::sqrt (corrLL * corrRR);
        correlation.store (den > 1.0e-9 ? (float) juce::jlimit (-1.0, 1.0, corrLR / den) : 1.0f, std::memory_order_relaxed);
    }
    else
        correlation.store (1.0f, std::memory_order_relaxed);

    for (int k = 0; k < kMaxVoices; ++k)
    {
        const auto v = engine.getVoiceView (k);
        auto& u = voiceUi[(size_t) k];
        u.delayMs.store (v.delayMs, std::memory_order_relaxed);
        u.cents.store (v.cents, std::memory_order_relaxed);
        u.pan.store (v.pan, std::memory_order_relaxed);
        u.env.store (v.env, std::memory_order_relaxed);
        u.level.store (juce::jmax (v.peak, u.level.load (std::memory_order_relaxed) * decay), std::memory_order_relaxed);
    }
}

//==============================================================================
void TwinAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void TwinAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* TwinAudioProcessor::createEditor() { return new TwinEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TwinAudioProcessor(); }
