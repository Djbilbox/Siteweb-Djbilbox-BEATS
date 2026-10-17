#pragma once

#include <cmath>

namespace rk
{

//==============================================================================
// "Urban Slayer" - HUMPIRE signature amp stage.
// Chain: pre-gain -> tone (one-pole LP) -> asymmetric saturation -> level.
// The saturation uses a dynamic gain-reduction curve with an asymmetric drive
// bias (positive swing pushed harder than negative), giving the mid-forward,
// aggressive-but-musical character expected from rock/punk guitar tones.
//==============================================================================
class Amp
{
public:
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void reset() noexcept
    {
        lpL = 0.0f;
        lpR = 0.0f;
    }

    void processStereo (float& l, float& r, float gainDb, float distAmount,
                        float toneHz, float levelDb, bool active) noexcept
    {
        const float lev = std::pow (10.0f, levelDb / 20.0f);

        if (! active)
        {
            l *= lev;
            r *= lev;
            return;
        }

        const float gain = std::pow (10.0f, gainDb / 20.0f);
        const float mix  = std::min (1.0f, std::max (0.0f, distAmount));
        const float fc   = std::min (0.48f * (float) sampleRate, std::max (100.0f, toneHz));
        const float k    = 1.0f - std::exp (-6.28318530718f * fc / (float) sampleRate);

        l = urbanSlayer (l * gain, mix, k, lpL);
        r = urbanSlayer (r * gain, mix, k, lpR);
        l *= lev;
        r *= lev;
    }

    void processStereo (float& l, float& r, float gainDb, float distAmount,
                        float toneHz, float levelDb) noexcept
    {
        processStereo (l, r, gainDb, distAmount, toneHz, levelDb, true);
    }

private:
    static float urbanSlayer (float x, float mix, float k, float& lp) noexcept
    {
        // amp tone: one-pole lowpass
        lp += (x - lp) * k;
        x = lp;

        // asymmetric drive bias
        const float bias = x > 0.0f ? 1.14f : 0.86f;
        const float ax   = std::fabs (x);
        const float g    = 1.0f / (1.0f + 0.55f * ax);       // dynamic gain reduction
        const float s    = x * (1.35f + g) * bias;
        const float t    = std::tanh (s);

        // tube saturation + a touch of 2nd harmonic colour
        const float y = t * (0.62f + 0.38f * g) + 0.06f * x * x;

        return x * (1.0f - mix) + y * mix;
    }

    double sampleRate = 48000.0;
    float  lpL = 0.0f;
    float  lpR = 0.0f;
};

} // namespace rk