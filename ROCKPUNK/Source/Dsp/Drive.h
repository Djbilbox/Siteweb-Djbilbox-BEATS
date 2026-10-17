#pragma once

#include <cmath>

namespace rk
{

//==============================================================================
// Post-filter waveshaper (rock drive).
// Types: 0 = soft (tanh), 1 = hard (clip), 2 = asymmetric, 3 = fuzz.
// "amount" is 0..1. A one-pole lowpass at "toneHz" darkens the top end.
//==============================================================================
class Drive
{
public:
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void reset() noexcept
    {
        toneL = 0.0f;
        toneR = 0.0f;
    }

    void processStereo (float& x, float& y, float amount, int type, float toneHz) noexcept
    {
        const float sr = (float) sampleRate;
        x = processSample (x, amount, type, toneHz, sr, toneL);
        y = processSample (y, amount, type, toneHz, sr, toneR);
    }

private:
    static float processSample (float input, float amount, int type, float toneHz,
                                float sampleRate, float& toneState) noexcept
    {
        const float amt = std::min (1.0f, std::max (0.0f, amount));
        const float wetMix = amt * amt;
        const float pre = 1.0f + amt * 6.0f;
        const float in = input * pre;

        float shaped = in;

        switch (type)
        {
            case 0: shaped = std::tanh (in); break;
            case 1: shaped = std::max (-1.0f, std::min (1.0f, in)); break;
            case 2:
            {
                const float t = std::tanh (in);
                shaped = (t + 0.3f * t * t) * 0.7f;
                break;
            }
            case 3:
            default:
                shaped = std::copysign (std::sqrt (std::min (1.0f, std::fabs (in))), in);
                shaped = std::tanh (shaped * 2.0f);
                break;
        }

        const float out = input * (1.0f - wetMix) + shaped * wetMix;

        const float fc = std::min (0.48f * sampleRate, std::max (100.0f, toneHz));
        const float k = 1.0f - std::exp (-6.28318530718f * fc / sampleRate);
        toneState += (out - toneState) * k;

        return toneState;
    }

    double sampleRate = 48000.0;
    float  toneL = 0.0f;
    float  toneR = 0.0f;
};

} // namespace rk