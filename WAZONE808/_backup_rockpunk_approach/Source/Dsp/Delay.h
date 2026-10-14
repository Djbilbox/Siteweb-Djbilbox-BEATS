#pragma once

#include <cmath>
#include <vector>

namespace wz
{

//==============================================================================
// Stereo ping-pong delay with feedback damping (one-pole LP) and dry/wet mix.
// Buffers are allocated in prepare() (never in the audio callback).
//==============================================================================
class Delay
{
public:
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void prepare (int maxDelayMs = 2000) noexcept
    {
        const int maxSamples = (int) (sampleRate * 0.001 * maxDelayMs) + 2;
        if ((int) lineL.size() < maxSamples)
        {
            lineL.resize ((size_t) maxSamples, 0.0f);
            lineR.resize ((size_t) maxSamples, 0.0f);
        }
        writePos = 0;
        lpL = 0.0f;
        lpR = 0.0f;
    }

    void reset() noexcept
    {
        std::fill (lineL.begin(), lineL.end(), 0.0f);
        std::fill (lineR.begin(), lineR.end(), 0.0f);
        writePos = 0;
        lpL = 0.0f;
        lpR = 0.0f;
    }

    void process (const float* inL, const float* inR,
                  float* outL, float* outR, int numSamples,
                  float timeMs, float feedback, float mix, float lpHz, bool active) noexcept
    {
        const int size = (int) lineL.size();

        if (! active || size < 2)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                outL[i] = inL[i];
                outR[i] = inR[i];
            }
            return;
        }

        const float fb = std::min (0.95f, std::max (0.0f, feedback));
        const float m  = std::min (1.0f, std::max (0.0f, mix));

        int delaySamples = (int) (timeMs * 0.001f * (float) sampleRate);
        if (delaySamples < 1)      delaySamples = 1;
        if (delaySamples >= size)  delaySamples = size - 1;

        const float fc = std::min (0.48f * (float) sampleRate, std::max (100.0f, lpHz));
        const float k = 1.0f - std::exp (-6.28318530718f * fc / (float) sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            int rp = writePos - delaySamples;
            if (rp < 0) rp += size;

            const float dryL = inL[i];
            const float dryR = inR[i];

            // ping-pong: read from the opposite line
            const float wetL = lineR[(size_t) rp];
            const float wetR = lineL[(size_t) rp];

            lineL[(size_t) writePos] = dryL + wetL * fb;
            lineR[(size_t) writePos] = dryR + wetR * fb;

            lpL += (lineL[(size_t) writePos] - lpL) * k;
            lpR += (lineR[(size_t) writePos] - lpR) * k;
            lineL[(size_t) writePos] = lpL;
            lineR[(size_t) writePos] = lpR;

            outL[i] = dryL * (1.0f - m) + wetL * m;
            outR[i] = dryR * (1.0f - m) + wetR * m;

            ++writePos;
            if (writePos >= size) writePos = 0;
        }
    }

private:
    double sampleRate = 48000.0;
    std::vector<float> lineL, lineR;
    int writePos = 0;
    float lpL = 0.0f;
    float lpR = 0.0f;
};

} // namespace wz