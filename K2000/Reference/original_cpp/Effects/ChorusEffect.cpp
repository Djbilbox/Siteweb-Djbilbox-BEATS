#include "ChorusEffect.h"
#include <algorithm>

ChorusEffect::ChorusEffect()
{
    delayBufferL.resize(4096, 0.0);
    delayBufferR.resize(4096, 0.0);
}

void ChorusEffect::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        size_t size = static_cast<size_t>(sampleRate * 0.1);
        delayBufferL.assign(size, 0.0);
        delayBufferR.assign(size, 0.0);
        writePos = 0;
    }
}

void ChorusEffect::setRate(double hz)
{
    rate = std::max(0.1, std::min(hz, 10.0));
}

void ChorusEffect::setAmount(double amt)
{
    amount = std::max(0.0, std::min(amt, 1.0));
}

void ChorusEffect::setMix(double newMix)
{
    mix = std::max(0.0, std::min(newMix, 1.0));
}

void ChorusEffect::process(double* left, double* right, int numSamples)
{
    if (mix <= 0.0001 || delayBufferL.empty()) return;

    const size_t bufSize = delayBufferL.size();
    const double samplesPerMs = sampleRate / 1000.0;
    const double maxDepthMs = 5.0 * amount;

    for (int i = 0; i < numSamples; ++i)
    {
        delayBufferL[static_cast<size_t>(writePos)] = left[i];
        delayBufferR[static_cast<size_t>(writePos)] = right[i];

        // Dual quadrature LFOs (90° phase offset)
        double lfoL = std::sin(lfoPhase);
        double lfoR = std::cos(lfoPhase);

        lfoPhase += juce::MathConstants<double>::twoPi * rate / sampleRate;
        if (lfoPhase >= juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;

        double delaySamplesL = (baseDelayMs + maxDepthMs * lfoL) * samplesPerMs;
        double delaySamplesR = (baseDelayMs + maxDepthMs * lfoR) * samplesPerMs;

        auto readInterpolated = [&](const std::vector<double>& buf, double delayS) {
            double readPos = static_cast<double>(writePos) - delayS;
            while (readPos < 0.0) readPos += bufSize;
            size_t idx0 = static_cast<size_t>(readPos) % bufSize;
            size_t idx1 = (idx0 + 1) % bufSize;
            double frac = readPos - static_cast<size_t>(readPos);
            return buf[idx0] * (1.0 - frac) + buf[idx1] * frac;
        };

        double wetL = readInterpolated(delayBufferL, delaySamplesL);
        double wetR = readInterpolated(delayBufferR, delaySamplesR);

        writePos = (writePos + 1) % static_cast<int>(bufSize);

        left[i] = left[i] * (1.0 - mix * 0.4) + wetL * mix;
        right[i] = right[i] * (1.0 - mix * 0.4) + wetR * mix;
    }
}
