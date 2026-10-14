#pragma once

#include <cmath>

namespace wz
{

//==============================================================================
// State-variable filter (Chamberlin form, double-sampled for stability).
// Modes: 0 = LP12, 1 = LP24 (two cascaded LP12), 2 = BP, 3 = SUB-CUT (HP).
// Cutoff is clamped to [20 Hz .. 0.45 * sr] so sub material never explodes.
//==============================================================================
class Svf
{
public:
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void reset() noexcept
    {
        low = band = low2 = band2 = high = 0.0f;
    }

    // mode from FILTER TYPE, cutoffHz may be modulated by the LFO.
    float process (float x, int mode, float cutoffHz) noexcept
    {
        const float sr = (float) sampleRate;
        float fc = cutoffHz;
        if (fc < 20.0f)   fc = 20.0f;
        if (fc > 0.45f * sr) fc = 0.45f * sr;

        // clamp g to stay stable at low sample rates / high cutoffs
        const float f = 2.0f * std::sin (juce::MathConstants<float>::pi * fc / sr);
        const float g = juce::jlimit (0.0f, 1.2f, f);

        low += g * band;
        high = x - low - q * band;
        band += g * high;

        switch (mode)
        {
            case 0:  return low;                 // LP12
            case 1:                                // LP24
            {
                low2 += g * band2;
                const float h2 = low - low2 - q * band2;
                band2 += g * h2;
                return low2;
            }
            case 2:  return band;                // BP
            case 3:  return high;                // SUB-CUT (high pass)
            default: return low;
        }
    }

    void setQ (float resonance) noexcept { q = juce::jlimit (0.05f, 1.4f, resonance); }

private:
    double sampleRate = 48000.0;
    float low = 0.0f, band = 0.0f, high = 0.0f;
    float low2 = 0.0f, band2 = 0.0f;
    float q = 0.6f;
};

} // namespace wz
