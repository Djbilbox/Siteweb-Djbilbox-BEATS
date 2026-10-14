#include "DelayEffect.h"

DelayEffect::DelayEffect()
{
    setSampleRate(44100.0);
    setTime(250.0);
    setFeedback(0.4);
    setMix(0.2);
}

void DelayEffect::setSampleRate(double sr)
{
    sampleRate = sr;
    int bufferSize = (int)((1.0 / 1000.0) * sr * 1000.0) + 1; // 1 second
    delayBuffer.assign((size_t) bufferSize, 0.0);
    writePos = 0; smoothDelay = 0.0;
}

void DelayEffect::setTime(double ms)
{
    time = juce::jlimit(50.0, 1000.0, ms);
}

void DelayEffect::setFeedback(double newFeedback)
{
    feedback = juce::jlimit(0.0, 0.95, newFeedback);
}

void DelayEffect::setMix(double newMix)
{
    mix = juce::jlimit(0.0, 1.0, newMix);
}

void DelayEffect::process(double* buffer, int numSamples)
{
    const double target = juce::jlimit(1.0, (double) delayBuffer.size() - 2.0, (time / 1000.0) * sampleRate);
    if (smoothDelay <= 0.0) smoothDelay = target;
    const int size = (int) delayBuffer.size();

    for (int i = 0; i < numSamples; ++i)
    {
        smoothDelay += (target - smoothDelay) * 0.0008;   // glisse vers le nouveau temps sans clic
        double rp = (double) writePos - smoothDelay;
        while (rp < 0.0) rp += (double) size;
        const int i0 = (int) rp % size, i1 = (i0 + 1) % size;
        const double fr = rp - std::floor(rp);
        const double delayOut = delayBuffer[(size_t) i0] * (1.0 - fr) + delayBuffer[(size_t) i1] * fr;

        // Write with feedback
        delayBuffer[writePos] = buffer[i] + delayOut * feedback;

        // Mix
        buffer[i] = buffer[i] * (1.0 - mix) + delayOut * mix;

        // Update write position
        writePos = (writePos + 1) % delayBuffer.size();
    }
}
