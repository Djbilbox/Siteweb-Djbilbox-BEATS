#include "ChorusEffect.h"

ChorusEffect::ChorusEffect()
{
    setSampleRate(44100.0);
    setRate(1.5);
    setAmount(0.5);
    setMix(0.3);
}

void ChorusEffect::setSampleRate(double sr)
{
    sampleRate = sr;
    int bufferSize = (int)((30.0 / 1000.0) * sr) + 1;
    delayBufferL.assign((size_t) bufferSize, 0.0);
    delayBufferR.assign((size_t) bufferSize, 0.0);
    writePos = 0; lfoPhase = 0.0;
}

void ChorusEffect::setRate(double hz)
{
    rate = juce::jlimit(0.1, 5.0, hz);
}

void ChorusEffect::setAmount(double amt)
{
    amount = juce::jlimit(0.0, 1.0, amt);
}

void ChorusEffect::setMix(double newMix)
{
    mix = juce::jlimit(0.0, 1.0, newMix);
}

void ChorusEffect::process(double* buffer, int numSamples)
{
    double lfoFreq = rate;
    double lfoPhaseIncrement = lfoFreq / sampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        // LFO (sine wave)
        double lfoL = std::sin(2.0 * juce::MathConstants<double>::pi * lfoPhase);
        double lfoR = std::sin(2.0 * juce::MathConstants<double>::pi * (lfoPhase + 0.25)); // 90° offset

        // Modulated delay time (5-30ms)
        double delayTimeL = basDelayMs + lfoL * amount * 10.0;
        double delayTimeR = basDelayMs + lfoR * amount * 10.0;
        delayTimeL = juce::jlimit(5.0, 30.0, delayTimeL);
        delayTimeR = juce::jlimit(5.0, 30.0, delayTimeR);

        // Convert to samples
        int delaySamplesL = (int)((delayTimeL / 1000.0) * sampleRate);
        int delaySamplesR = (int)((delayTimeR / 1000.0) * sampleRate);

        // Read delayed sample
        int readPosL = (writePos - delaySamplesL + delayBufferL.size()) % delayBufferL.size();
        int readPosR = (writePos - delaySamplesR + delayBufferR.size()) % delayBufferR.size();

        double delayOutL = delayBufferL[readPosL];
        double delayOutR = delayBufferR[readPosR];

        // Write to buffer with feedback
        delayBufferL[writePos] = buffer[i] + delayOutL * 0.3;
        delayBufferR[writePos] = buffer[i] + delayOutR * 0.3;

        // Stereo output
        buffer[i] = buffer[i] * (1.0 - mix) + ((delayOutL + delayOutR) * 0.5) * mix;

        // Update write position
        writePos = (writePos + 1) % delayBufferL.size();

        // Update LFO phase
        lfoPhase += lfoPhaseIncrement;
        if (lfoPhase >= 1.0)
            lfoPhase -= 1.0;
    }
}
