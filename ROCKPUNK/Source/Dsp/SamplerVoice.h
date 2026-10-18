#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>

#include "../Params.h"

namespace rk
{

class SamplerSound final : public juce::SynthesiserSound
{
public:
    SamplerSound() = default;

    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
// Sampled-guitar layer. All voices share one AudioBuffer (the WAV loaded from
// the Player sample library); each note is a linear-interpolated, pitch-shifted
// playback of that buffer. The sample's own root note (auto-detected by the
// processor) keeps the keyboard in tune, and every voice outputs mono with a
// note-based equal-power pan, exactly like the KS voice above.
//==============================================================================
class SamplerVoice final : public juce::SynthesiserVoice
{
public:
    SamplerVoice() = default;

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<SamplerSound*> (s) != nullptr;
    }

    // ---- shared, block-rate configuration (set by the processor) ----
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void setSharedSample (std::shared_ptr<juce::AudioBuffer<float>> data, double newFileRate,
                          float newRootNote, bool loop, float attackMs, float releaseMs) noexcept
    {
        sample         = std::move (data);
        fileRate       = newFileRate > 0.0 ? newFileRate : 44100.0;
        rootNote       = newRootNote;
        loopEnabled    = loop;
        attackTimeMs   = attackMs;
        releaseTimeMs  = releaseMs;
    }

    void setLevel (float l) noexcept { level = l; }

    // ---- SynthesiserVoice ----
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                    int /*currentPitchWheelPosition*/) override
    {
        if (sample == nullptr || sample->getNumSamples() <= 1)
        {
            clearCurrentNote();
            return;
        }
        voiceNote = midiNoteNumber;
        const float midiFreq = midiToFreq ((float) midiNoteNumber);
        const float rootFreq = midiToFreq (rootNote);
        rate                 = midiFreq / rootFreq;
        readPos              = 0.0;
        envGain              = 0.0f;
        releasing            = false;
        vel                  = juce::jlimit (0.001f, 1.0f, velocity);
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
            releasing = true;
        else
        {
            clearCurrentNote();
            envGain = 0.0f;
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    using juce::SynthesiserVoice::renderNextBlock;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample,
                          int numSamples) override
    {
        if (! isVoiceActive())
            return;

        if (sample == nullptr || sample->getNumSamples() <= 1)
        {
            clearCurrentNote();
            return;
        }

        const auto* dataL = sample->getReadPointer (0);
        const auto* dataR = sample->getNumChannels() > 1 ? sample->getReadPointer (1) : dataL;
        const int   numSrc = sample->getNumSamples();

        const float ratePerSample = (float) (rate * fileRate / sampleRate);

        const float panPos = juce::jmap ((float) (voiceNote % 12), 0.0f, 11.0f, -1.0f, 1.0f);
        const float angle  = (panPos + 1.0f) * 0.5f * 3.14159265f * 0.5f;
        const float panL   = std::cos (angle);
        const float panR   = std::sin (angle);

        const float atkStep = attackTimeMs  > 0.0f ? (1.0f / (attackTimeMs  * 0.001f * (float) sampleRate)) : 1.0f;
        const float relCoeff = releaseTimeMs > 0.0f ? std::exp (-1.0f / (releaseTimeMs * 0.001f * (float) sampleRate)) : 0.0f;

        const int numCh = outputBuffer.getNumChannels() >= 2 ? 2 : 1;

        for (int i = 0; i < numSamples; ++i)
        {
            if (releasing)
            {
                if (relCoeff > 0.0f)
                    envGain *= relCoeff;
                if (envGain < 0.0005f)
                {
                    envGain = 0.0f;
                    clearCurrentNote();
                    break;
                }
            }
            else if (envGain < 1.0f)
            {
                envGain += atkStep;
                envGain = std::min (1.0f, envGain);
            }

            double idx = readPos;
            if (idx >= (double) (numSrc - 1))
            {
                if (loopEnabled && ! releasing)
                {
                    readPos = 0.0;
                    idx     = 0.0;
                }
                else
                {
                    readPos = 0.0;
                    clearCurrentNote();
                    break;
                }
            }

            const int   i0    = (int) idx;
            const float frac  = (float) (idx - (double) i0);
            const float l     = dataL[i0] + (dataL[i0 + 1] - dataL[i0]) * frac;
            const float r     = dataR[i0] + (dataR[i0 + 1] - dataR[i0]) * frac;
            const float out   = (l + r) * 0.5f * level * vel * envGain;

            outputBuffer.addSample (0, startSample + i, out * panL);
            if (numCh >= 2)
                outputBuffer.addSample (1, startSample + i, out * panR);

            readPos += (double) ratePerSample;
        }
    }

private:
    double sampleRate = 48000.0;

    std::shared_ptr<juce::AudioBuffer<float>> sample;
    double fileRate   = 44100.0;
    float  rootNote   = 60.0f;
    bool   loopEnabled = false;
    float  attackTimeMs  = 5.0f;
    float  releaseTimeMs = 250.0f;

    float rate  = 1.0f;
    float level = 1.0f;
    float vel   = 1.0f;

    double readPos = 0.0;
    float  envGain = 0.0f;
    bool   releasing = false;
    int    voiceNote = -1;
};

} // namespace rk