#pragma once

#include <cmath>
#include <cstdint>

namespace rk
{

//==============================================================================
// Low-frequency oscillator. Shapes: 0 = sine, 1 = triangle, 2 = saw, 3 = square, 4 = S&H.
// Outputs -1..+1.
//==============================================================================
class Lfo
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        updateInc();
    }

    void reset() noexcept
    {
        phase = 0.0f;
        wrapped = false;
        rng = 0x9E3779B9u;
        heldRandom = randUnit();
    }

    void setFrequency (float f) noexcept
    {
        freq = std::max (0.0001f, f);
        updateInc();
    }

    void setShape (int s) noexcept { shape = s; }

    float process() noexcept
    {
        // Sample & hold: a new random value at the start of every cycle. The
        // phase is wrapped at the end of each call, so it can never be read
        // outside [0, 1) here - the wrap itself has to be what triggers it.
        if (wrapped)
        {
            wrapped = false;
            heldRandom = randUnit();
        }

        float out = 0.0f;

        switch (shape)
        {
            case 0: out = std::sin (6.28318530718f * phase); break;
            case 1: out = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase); break;
            case 2: out = 2.0f * phase - 1.0f; break;
            case 3: out = (phase < 0.5f) ? 1.0f : -1.0f; break;
            case 4: out = heldRandom; break;
            default: out = 0.0f; break;
        }

        phase += phaseInc;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            wrapped = true;
        }
        else if (phase < 0.0f)
        {
            phase += 1.0f;
            wrapped = true;
        }

        return out;
    }

private:
    void updateInc() noexcept { phaseInc = freq / (float) sampleRate; }

    float randUnit() noexcept
    {
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
        return (float) ((int32_t) rng) * (1.0f / 2147483648.0f);
    }

    double sampleRate = 48000.0;
    float  freq = 1.0f;
    float  phase = 0.0f;
    float  phaseInc = 0.0f;
    float  heldRandom = 0.0f;
    bool   wrapped = false;
    int    shape = 0;
    uint32_t rng = 0x9E3779B9u;
};

} // namespace rk