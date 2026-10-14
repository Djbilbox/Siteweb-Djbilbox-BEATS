#include "DelayEffect.h"
#include <algorithm>
#include <cmath>

DelayEffect::DelayEffect()
{
    bufferL.resize(96000, 0.0);
    bufferR.resize(96000, 0.0);
}

void DelayEffect::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        size_t size = static_cast<size_t>(sampleRate * 2.0);
        bufferL.assign(size, 0.0);
        bufferR.assign(size, 0.0);
        writePos = 0;
    }
}

void DelayEffect::setTime(double ms)
{
    timeMs = std::max(10.0, std::min(ms, 1500.0));
}

void DelayEffect::setFeedback(double fb)
{
    feedback = std::max(0.0, std::min(fb, 0.95));
}

void DelayEffect::setMix(double newMix)
{
    mix = std::max(0.0, std::min(newMix, 1.0));
}

void DelayEffect::process(double* left, double* right, int numSamples)
{
    if (mix <= 0.0001 || bufferL.empty()) return;

    const size_t bufSize = bufferL.size();
    const double samplesPerMs = sampleRate / 1000.0;
    const size_t delayL = static_cast<size_t>(timeMs * samplesPerMs);
    const size_t delayR = static_cast<size_t>(timeMs * 1.15 * samplesPerMs); // ping-pong offset

    for (int i = 0; i < numSamples; ++i)
    {
        size_t readPosL = (static_cast<size_t>(writePos) + bufSize - (delayL % bufSize)) % bufSize;
        size_t readPosR = (static_cast<size_t>(writePos) + bufSize - (delayR % bufSize)) % bufSize;

        double dL = bufferL[readPosL];
        double dR = bufferR[readPosR];

        // Analog tape lowpass filter (damping at ~3.5kHz)
        filterL += (dL - filterL) * 0.45;
        filterR += (dR - filterR) * 0.45;

        // Feedback saturation (tanh)
        double fbL = std::tanh((filterR * 0.8 + filterL * 0.2) * feedback);
        double fbR = std::tanh((filterL * 0.8 + filterR * 0.2) * feedback);

        bufferL[static_cast<size_t>(writePos)] = left[i] + fbL;
        bufferR[static_cast<size_t>(writePos)] = right[i] + fbR;

        writePos = (writePos + 1) % static_cast<int>(bufSize);

        left[i] = left[i] * (1.0 - mix * 0.4) + dL * mix;
        right[i] = right[i] * (1.0 - mix * 0.4) + dR * mix;
    }
}
