#pragma once

#include <cmath>
#include <vector>

namespace wz
{

//==============================================================================
// Lightweight stereo chorus (2 modulated delay lines, LFO in quadrature).
// "amount" 0..1 scales both wet and modulation width. Cheap: 2 delay lines of
// 40 ms, linear interpolation.
//==============================================================================
class Chorus
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        prepare();
    }

    void prepare() noexcept
    {
        const int n = (int) (0.040f * (float) sampleRate) + 4;
        if ((int) lineL.size() < n)
        {
            lineL.assign ((size_t) n, 0.0f);
            lineR.assign ((size_t) n, 0.0f);
        }
        writePos = 0;
    }

    void reset() noexcept
    {
        std::fill (lineL.begin(), lineL.end(), 0.0f);
        std::fill (lineR.begin(), lineR.end(), 0.0f);
        writePos = 0;
        phase = 0.0f;
    }

    void process (float& l, float& r, float amount01) noexcept
    {
        const float amt = juce::jlimit (0.0f, 1.0f, amount01);
        const int size = (int) lineL.size();
        if (amt < 0.001f || size < 8)
            return;

        phase += 0.9f / (float) sampleRate; // 0.9 Hz chorus sweep
        if (phase >= 1.0f)
            phase -= 1.0f;

        const float base = 0.012f * (float) sampleRate;
        const float depth = (0.004f + 0.004f * amt) * (float) sampleRate;

        const float lfoL = std::sin (2.0f * juce::MathConstants<float>::pi * phase);
        const float lfoR = std::sin (2.0f * juce::MathConstants<float>::pi * phase + 1.5708f);

        const float posL = base + depth * lfoL;
        const float posR = base + depth * lfoR;

        const float wetL = read (lineL, posL);
        const float wetR = read (lineR, posR);

        lineL[(size_t) writePos] = l;
        lineR[(size_t) writePos] = r;
        if (++writePos >= size) writePos = 0;

        const float wet = 0.35f * amt;
        l = l * (1.0f - wet * 0.5f) + wetR * wet;
        r = r * (1.0f - wet * 0.5f) + wetL * wet;
    }

private:
    float read (const std::vector<float>& line, float posSamples) const noexcept
    {
        const int size = (int) line.size();
        int p0 = writePos - 1 - (int) posSamples;
        while (p0 < 0) p0 += size;
        const int p1 = (p0 + 1) % size;
        const float frac = posSamples - std::floor (posSamples);
        return line[(size_t) p0] * (1.0f - frac) + line[(size_t) p1] * frac;
    }

    double sampleRate = 48000.0;
    std::vector<float> lineL, lineR;
    int writePos = 0;
    float phase = 0.0f;
};

} // namespace wz
