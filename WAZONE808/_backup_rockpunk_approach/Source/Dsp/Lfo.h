#pragma once

#include <cmath>

namespace wz
{

//==============================================================================
// Global LFO with its own attack / decay / sustain / release envelope that
// scales the modulation depth (LFO ADSR block, left side of the panel).
// Sources: 0 = sine, 1 = triangle, 2 = square, 3 = sample & hold random.
//==============================================================================
class Lfo
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        rnd = 0.5f;
    }

    void reset() noexcept
    {
        phase = 0.0f;
        env = 0.0f;
        envStage = Idle;
        rnd = 0.5f;
        held = 0.5f;
    }

    // start / stop the LFO envelope with the note (retrig per note-on).
    void noteOn() noexcept
    {
        phase = 0.0f;
        env = 0.0f;
        envStage = Attack;
    }

    void noteOff() noexcept
    {
        if (envStage != Idle)
            envStage = Release;
    }

    // Advances one sample. a/d/r in seconds, s 0..1. Returns the shaped
    // modulator in -1..+1 (already scaled by envelope 0..1).
    float process (float rateHz, int source,
                   float aSec, float dSec, float sLevel, float rSec) noexcept
    {
        // ---- oscillator ----
        phase += rateHz / (float) sampleRate;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            if (source == 3)
                held = rnd;
        }

        float osc = 0.0f;
        switch (source)
        {
            case 0:  osc = std::sin (2.0f * juce::MathConstants<float>::pi * phase); break;
            case 1:  osc = 2.0f * phase - 1.0f; break;
            case 2:  osc = phase < 0.5f ? 1.0f : -1.0f; break;
            case 3:  osc = held * 2.0f - 1.0f; break;
            default: osc = 0.0f;
        }

        // ---- envelope on depth ----
        switch (envStage)
        {
            case Attack:
                env += 1.0f / ((float) sampleRate * juce::jmax (0.001f, aSec));
                if (env >= 1.0f) { env = 1.0f; envStage = Decay; }
                break;
            case Decay:
                env += (sLevel - env) / ((float) sampleRate * juce::jmax (0.001f, dSec) * 0.2f + 1.0f);
                if (env <= sLevel + 0.0005f) { env = sLevel; envStage = Sustain; }
                break;
            case Sustain:
                env = sLevel;
                break;
            case Release:
                env -= 1.0f / ((float) sampleRate * juce::jmax (0.001f, rSec));
                if (env <= 0.0f) { env = 0.0f; envStage = Idle; }
                break;
            case Idle:
            default:
                break;
        }

        return osc * juce::jlimit (0.0f, 1.0f, env);
    }

private:
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    double sampleRate = 48000.0;
    float phase = 0.0f, env = 0.0f;
    float rnd = 0.5f, held = 0.5f;
    Stage envStage = Idle;
};

} // namespace wz
