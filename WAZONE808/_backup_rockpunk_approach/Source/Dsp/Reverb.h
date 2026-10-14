#pragma once

#include <cmath>
#include <vector>

namespace wz
{

//==============================================================================
// Cheap but stable Schroeder reverb (4 parallel feedback comb filters +
// 2 series allpass), tuned dark so it does not brighten sub material.
// "amount" 0..1 is a pure wet/dry crossfade; the tank always runs at
// bounded gain so extreme settings cannot run away.
//==============================================================================
class Reverb
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        prepare();
    }

    void prepare() noexcept
    {
        const double sizesMs[] = { 29.7, 37.1, 41.1, 43.7, 5.2, 6.7, 1.7 };
        float* lines[] = { &combA[0], &combB[0], &combC[0], &combD[0],
                           &apA[0], &apB[0], &apC[0] };
        const int n = 7;
        for (int i = 0; i < n; ++i)
        {
            lengths[i] = juce::jmax (4, (int) (sizesMs[i] * 0.001 * sampleRate));
            lines[i]->assign ((size_t) lengths[i] + 1, 0.0f);
            idx[i] = 0;
        }
    }

    void reset() noexcept
    {
        for (auto* line : { &combA, &combB, &combC, &combD, &apA, &apB, &apC })
            std::fill (line->begin(), line->end(), 0.0f);
        for (int i = 0; i < 7; ++i) idx[i] = 0;
    }

    void process (float& l, float& r, float amount01) noexcept
    {
        const float wet = juce::jlimit (0.0f, 1.0f, amount01);
        if (wet < 0.001f)
            return;

        const float in = (l + r) * 0.5f;

        // parallel combs (dark feedback ~0.77)
        const float cA = comb (combA, 0, in);
        const float cB = comb (combB, 1, in);
        const float cC = comb (combC, 2, in);
        const float cD = comb (combD, 3, in);

        float s = (cA + cB + cC + cD) * 0.25f;

        // series allpasses
        s = allpass (apA, 4, s);
        s = allpass (apB, 5, s);
        s = allpass (apC, 6, s);

        // slight stereo spread on the wet signal
        const float wl = s * 0.9f;
        const float wr = s * 1.1f * 0.9f;

        const float dry = 1.0f - wet * 0.7f;
        l = l * dry + wl * wet;
        r = r * dry + wr * wet;
    }

private:
    using Line = std::vector<float>;

    float comb (Line& buf, int slot, float in) noexcept
    {
        const int len = lengths[slot];
        const size_t p = (size_t) idx[slot];
        const float out = buf[p];
        buf[p] = in + out * 0.77f;
        if (++idx[slot] >= len) idx[slot] = 0;
        return out;
    }

    float allpass (Line& buf, int slot, float in) noexcept
    {
        const int len = lengths[slot];
        const size_t p = (size_t) idx[slot];
        const float delayed = buf[p];
        const float out = -in + delayed;
        buf[p] = in + delayed * 0.7f;
        if (++idx[slot] >= len) idx[slot] = 0;
        return out;
    }

    double sampleRate = 48000.0;
    Line combA, combB, combC, combD, apA, apB, apC;
    int lengths[7] = { 8, 8, 8, 8, 8, 8, 8 };
    int idx[7] = { 0, 0, 0, 0, 0, 0, 0 };
};

} // namespace wz
