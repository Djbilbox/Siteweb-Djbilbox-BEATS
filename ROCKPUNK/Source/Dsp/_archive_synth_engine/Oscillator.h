#pragma once

#include <cmath>

namespace rk
{

//==============================================================================
// Two-sided PolyBLEP residual (standard implementation).
// t = phase (0..1), dt = phase increment per sample.
//==============================================================================
inline float polyBlep (float t, float dt) noexcept
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }

    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }

    return 0.0f;
}

//==============================================================================
// Virtual-analog oscillator with PolyBLEP anti-aliasing.
// Shapes: 0 = saw, 1 = square, 2 = pulse (PWM), 3 = triangle.
//==============================================================================
class Oscillator
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        updatePhaseInc();
    }

    void reset() noexcept { phase = 0.0f; }

    void setFrequency (float f) noexcept
    {
        freq = std::max (0.001f, f);
        updatePhaseInc();
    }

    void setShape (int s) noexcept { shape = s; }
    void setPulseWidth (float pw) noexcept { pulseWidth = std::min (0.95f, std::max (0.05f, pw)); }

    float getFrequency() const noexcept { return freq; }

    float process() noexcept
    {
        const float dt = phaseInc;
        float out = 0.0f;

        switch (shape)
        {
            case 0: // saw
                out = 2.0f * phase - 1.0f - polyBlep (phase, dt);
                break;

            case 1: // square
            {
                const float sq = (phase < 0.5f) ? 1.0f : -1.0f;
                out = sq + polyBlep (phase, dt) - polyBlep (std::fmod (phase + 0.5f, 1.0f), dt);
                break;
            }

            case 2: // pulse (PWM)
            {
                const float pl = (phase < pulseWidth) ? 1.0f : -1.0f;
                out = pl + polyBlep (phase, dt) - polyBlep (std::fmod (phase - pulseWidth + 1.0f, 1.0f), dt);
                break;
            }

            case 3: // triangle
            default:
                out = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
                break;
        }

        phase += phaseInc;
        if (phase >= 1.0f)       phase -= 1.0f;
        else if (phase < 0.0f)   phase += 1.0f;

        return out;
    }

private:
    void updatePhaseInc() noexcept
    {
        if (sampleRate > 0.0)
            phaseInc = freq / (float) sampleRate;
    }

    double sampleRate = 48000.0;
    float  freq = 440.0f;
    float  phase = 0.0f;
    float  phaseInc = 0.0f;
    float  pulseWidth = 0.5f;
    int    shape = 1;
};

} // namespace rk