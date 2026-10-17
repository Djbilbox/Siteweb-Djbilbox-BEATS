#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "../Params.h"
#include "Envelope.h"
#include "Oscillator.h"
#include "Filter.h"
#include "Lfo.h"

namespace rk
{

//==============================================================================
class RockPunkSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
class RockPunkVoice final : public juce::SynthesiserVoice
{
public:
    explicit RockPunkVoice (const RockPunkParams& p) : params (p) {}

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<RockPunkSound*> (s) != nullptr;
    }

    void prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;

        oscA.setSampleRate (sampleRate);
        oscB.setSampleRate (sampleRate);
        subOsc.setSampleRate (sampleRate);
        filter.setSampleRate (sampleRate);
        ampEnv.setSampleRate (sampleRate);
        filtEnv.setSampleRate (sampleRate);
        lfo1.setSampleRate (sampleRate);
        lfo2.setSampleRate (sampleRate);

        filterCutSmooth.reset (sampleRate, 0.02);
        filterResSmooth.reset (sampleRate, 0.02);

        reset();
    }

    void setHostBpm (float bpm) noexcept { hostBpm = bpm; }

    void reset()
    {
        oscA.reset(); oscB.reset(); subOsc.reset();
        filter.reset();
        ampEnv.reset(); filtEnv.reset();
        lfo1.reset(); lfo2.reset();

        currentPitch = 0.0f;
        targetPitch = 0.0f;
        currentVelocity = 0.8f;
        modWheel = 0.0f;
        aftertouch = 0.0f;
        glideCoef = 1.0f;
        voicePan = 0.0f;
        randomValue = 0.0f;
        randomTimer = 0.0f;
        lastCutStep = 1.0e9f;
        lastRes = 1.0e9f;
        noiseSeed = 0x5EED1234u;

        filterCutSmooth.setCurrentAndTargetValue (initialCutLog());
        filterResSmooth.setCurrentAndTargetValue (1.0f);
    }

    //==========================================================================
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* /*sound*/,
                    int /*currentPitchWheelPosition*/) override
    {
        const int mode = params.voiceMode.getInt();
        const bool isPoly   = (mode == 0);
        const bool restartEnv = (mode != 2); // legato (2) keeps the amp envelope running

        if (isPoly || ampEnv.isIdle())
            reset();

        targetPitch = (float) midiNoteNumber;
        currentVelocity = std::min (1.0f, std::max (0.0f, velocity));

        if (restartEnv || ampEnv.isIdle())
            ampEnv.noteOn();
        filtEnv.noteOn();

        const float glideMs = params.glideTime.get();
        if (glideMs > 1.0f && ! isPoly && ! ampEnv.isIdle())
            glideCoef = 1.0f - std::exp (-1.0f / (glideMs * 0.001f * (float) sr));
        else
            glideCoef = 1.0f;

        oscA.reset();
        oscB.reset();
        lfo1.reset();
        lfo2.reset();

        // deterministic pseudo-random pan spread for poly voice assignment
        noiseSeed ^= noiseSeed << 13; noiseSeed ^= noiseSeed >> 17; noiseSeed ^= noiseSeed << 5;
        const float r = (float) ((int32_t) noiseSeed) * (1.0f / 2147483648.0f);
        voicePan = isPoly ? (r * 0.6f) : 0.0f;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        ampEnv.noteOff();
        filtEnv.noteOff();

        if (! allowTailOff)
            clearCurrentNote();
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int controllerNumber, int newValue) override
    {
        if (controllerNumber == 1) // modulation wheel
            modWheel = newValue / 127.0f;
    }

    void channelPressureChanged (int newPressure) override { aftertouch = newPressure / 127.0f; }
    void aftertouchChanged (int newPressure) override      { aftertouch = newPressure / 127.0f; }

    //==========================================================================
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override
    {
        if (numSamples <= 0)
            return;

        if (glideCoef >= 1.0f)
            currentPitch = targetPitch;

        // --- per-block parameter refresh -----------------------------------
        const int shapeA  = params.oscAShape.getInt();
        const int shapeB  = params.oscBShape.getInt();
        const int subShape = params.subShape.getInt();
        const int mode    = params.voiceMode.getInt();
        const bool isPoly = (mode == 0);

        const float levelA = params.oscALevel.get();
        const float levelB = params.oscBLevel.get();
        const float subLvl = params.subLevel.get();
        const float noiseLvl = params.noiseLevel.get();
        const float pwA = params.oscAPW.get();
        const float pwB = params.oscBPW.get();
        const float mixDrive = params.mixDrive.get();
        const float filterDrive = params.filterDrive.get();
        const float keyTrk = params.filterKeyTrk.get();
        const float envAmt = params.filterEnvAmt.get();
        const int   filterType = params.filterType.getInt();
        const float lfo1Amt = params.lfo1Amt.get();
        const float lfo2Amt = params.lfo2Amt.get();
        const int   lfo1Sync = params.lfo1Sync.getBool() ? 1 : 0;
        const int   lfo2Sync = params.lfo2Sync.getBool() ? 1 : 0;
        const int   lfo1Div = params.lfo1Div.getInt();
        const int   lfo2Div = params.lfo2Div.getInt();
        const float lfo1Rate = params.lfo1Rate.get();
        const float lfo2Rate = params.lfo2Rate.get();
        const int   lfo1Shape = params.lfo1Shape.getInt();
        const int   lfo2Shape = params.lfo2Shape.getInt();

        oscA.setShape (shapeA);
        oscB.setShape (shapeB);
        subOsc.setShape (subShape);
        filter.setType (filterType);
        lfo1.setShape (lfo1Shape);
        lfo2.setShape (lfo2Shape);

        lfo1.setFrequency (lfo1Sync != 0 ? divisionToHz (hostBpm, lfo1Div) : lfo1Rate);
        lfo2.setFrequency (lfo2Sync != 0 ? divisionToHz (hostBpm, lfo2Div) : lfo2Rate);

        ampEnv.updateTimes (params.ampA.get(), params.ampD.get(), params.ampS.get(), params.ampR.get());
        filtEnv.updateTimes (params.filtA.get(), params.filtD.get(), params.filtS.get(), params.filtR.get());

        // smoothed (log-domain) cutoff target and resonance target
        filterCutSmooth.setTargetValue (initialCutLog());
        filterResSmooth.setTargetValue (params.filterRes.get());

        // matrix
        const int s1 = params.mod1Src.getInt(), d1 = params.mod1Dst.getInt(); const float a1 = params.mod1Amt.get();
        const int s2 = params.mod2Src.getInt(), d2 = params.mod2Dst.getInt(); const float a2 = params.mod2Amt.get();

        const float voiceGain = (isPoly ? 0.7f : 0.85f) * (0.6f + 0.4f * currentVelocity);

        const int numCh = outputBuffer.getNumChannels();
        const int chL = numCh > 0 ? 0 : -1;
        const int chR = numCh > 1 ? 1 : -1;

        // --- sample loop ----------------------------------------------------
        for (int i = 0; i < numSamples; ++i)
        {
            if (glideCoef < 1.0f)
                currentPitch += (targetPitch - currentPitch) * glideCoef;

            const float lfo1v = lfo1.process();
            const float lfo2v = lfo2.process();
            const float envf = filtEnv.process();
            const float ampv = ampEnv.process();

            // random source (S&H, ~50 ms)
            randomTimer -= 1.0f;
            if (randomTimer <= 0.0f)
            {
                randomTimer = 0.05f * (float) sr;
                randomValue = noiseSample();
            }

            // --- modulation sources ---
            auto sourceValue = [&] (int src) -> float
            {
                switch (src)
                {
                    case 1: return lfo1v;
                    case 2: return lfo2v;
                    case 3: return envf;
                    case 4: return modWheel;
                    case 5: return currentVelocity;
                    case 6: return aftertouch;
                    case 7: return randomValue;
                    default: return 0.0f;
                }
            };

            const float m1v = sourceValue (s1);
            const float m2v = sourceValue (s2);

            float pitchCents = lfo2v * lfo2Amt;
            float cutOct = lfo1v * (lfo1Amt / 100.0f) * 1.5f;
            float resOff = 0.0f;
            float pwOffA = 0.0f, pwOffB = 0.0f;
            float lvlOffA = 0.0f, lvlOffB = 0.0f;
            float panOff = 0.0f;

            auto applySlot = [&] (int dst, float val, float amt)
            {
                const float scale = amt / 100.0f;
                switch (dst)
                {
                    case 1: pitchCents += val * scale * 1200.0f; break;
                    case 2: cutOct    += val * scale * 1.5f;     break;
                    case 3: resOff    += val * scale * 5.0f;     break;
                    case 4: pwOffA    += val * scale * 40.0f;    break;
                    case 5: pwOffB    += val * scale * 40.0f;    break;
                    case 6: lvlOffA   += val * scale * 0.8f;     break;
                    case 7: lvlOffB   += val * scale * 0.8f;     break;
                    case 8: panOff    += val * scale * 0.5f;     break;
                    default: break;
                }
            };

            applySlot (d1, m1v, a1);
            applySlot (d2, m2v, a2);

            // --- filter cutoff (log domain) ---
            const float cutLog = filterCutSmooth.getNextValue() + cutOct;
            const float keyOct = (currentPitch - 60.0f) * (keyTrk / 100.0f) / 12.0f;
            const float envOct = envf * (envAmt / 100.0f) * 1.5f;

            float cutHz = 20.0f * std::exp2 (cutLog + keyOct + envOct);
            cutHz = std::min (20000.0f, std::max (20.0f, cutHz));

            float res = filterResSmooth.getNextValue() + resOff;
            res = std::min (10.0f, std::max (0.5f, res));

            const float cutStep = std::round (cutLog * 64.0f);
            if (cutStep != lastCutStep || std::fabs (res - lastRes) > 0.0001f)
            {
                filter.setCutoff (cutHz);
                filter.setResonance (res);
                filter.updateCoeffs();
                lastCutStep = cutStep;
                lastRes = res;
            }

            // --- oscillators ---
            const float fA = midiToFreq (currentPitch + (float) params.oscASemi.getInt())
                           * centsToRatio (params.oscACent.get() + pitchCents);
            const float fB = midiToFreq (currentPitch + (float) params.oscBSemi.getInt())
                           * centsToRatio (params.oscBCent.get() + pitchCents);
            const float fSub = midiToFreq (currentPitch) * semiToRatio (params.subSemi.getInt());

            oscA.setFrequency (fA);
            oscB.setFrequency (fB);
            subOsc.setFrequency (fSub);

            const float cA = std::min (0.95f, std::max (0.05f, (pwA + pwOffA) / 100.0f));
            const float cB = std::min (0.95f, std::max (0.05f, (pwB + pwOffB) / 100.0f));
            oscA.setPulseWidth (cA);
            oscB.setPulseWidth (cB);

            const float lA = std::min (2.0f, std::max (0.0f, levelA + lvlOffA));
            const float lB = std::min (2.0f, std::max (0.0f, levelB + lvlOffB));

            const float oa = oscA.process() * lA;
            const float ob = oscB.process() * lB;
            const float os = subOsc.process() * subLvl;
            const float on = noiseSample() * noiseLvl;

            // --- mixer + drives ---
            float mix = (oa + ob + os + on) * 0.5f;                       // -6 dB staging
            mix = std::tanh (mix * (1.0f + mixDrive * 3.0f)) / (1.0f + mixDrive);
            mix = std::tanh (mix * (1.0f + filterDrive * 4.0f));          // filter pre-drive

            const float filtered = filter.process (mix);
            const float out = filtered * ampv * voiceGain;

            // --- pan ---
            const float pan = std::min (1.0f, std::max (-1.0f, voicePan + panOff));
            const float angle = 0.78539816339f * (pan + 1.0f);
            const float l = out * std::cos (angle);
            const float r = out * std::sin (angle);

            if (chL >= 0) outputBuffer.addSample (chL, startSample + i, l);
            if (chR >= 0) outputBuffer.addSample (chR, startSample + i, r);
        }

        if (ampEnv.isIdle())
            clearCurrentNote();
    }

private:
    float initialCutLog() const noexcept
    {
        return std::log2 (std::max (20.0f, params.filterCut.get()) / 20.0f);
    }

    float noiseSample() noexcept
    {
        noiseSeed ^= noiseSeed << 13; noiseSeed ^= noiseSeed >> 17; noiseSeed ^= noiseSeed << 5;
        return (float) ((int32_t) noiseSeed) * (1.0f / 2147483648.0f);
    }

    //==========================================================================
    const RockPunkParams& params;

    double sr = 48000.0;
    float  hostBpm = 120.0f;

    Oscillator oscA, oscB, subOsc;
    SvfFilter  filter;
    Adsr       ampEnv, filtEnv;
    Lfo        lfo1, lfo2;

    float currentPitch = 0.0f;
    float targetPitch = 0.0f;
    float currentVelocity = 0.8f;
    float modWheel = 0.0f;
    float aftertouch = 0.0f;
    float glideCoef = 1.0f;
    float voicePan = 0.0f;

    float randomValue = 0.0f;
    float randomTimer = 0.0f;
    uint32_t noiseSeed = 0x5EED1234u;

    float lastCutStep = 1.0e9f;
    float lastRes = 1.0e9f;

    juce::SmoothedValue<float> filterCutSmooth;
    juce::SmoothedValue<float> filterResSmooth;
};

} // namespace rk