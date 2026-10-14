#include "LFO.h"
#include <algorithm>

LFO::LFO()
{
    osc.setWaveform(Oscillator::Waveform::Triangle);
    osc.setFrequency(rate);
}

void LFO::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        osc.setSampleRate(sr);
    }
}

void LFO::setRate(double rateHz)
{
    rate = std::max(0.01, std::min(rateHz, 40.0));
    osc.setFrequency(rate);
}

void LFO::setAmount(double amt)
{
    amount = std::max(0.0, std::min(amt, 1.0));
}

void LFO::setDestination(Destination dest)
{
    currentDestination = dest;
}

void LFO::setWaveform(Oscillator::Waveform wf)
{
    osc.setWaveform(wf);
}

void LFO::reset()
{
    osc.reset();
    output = 0.0;
}

double LFO::tick()
{
    if (amount <= 0.0001)
    {
        output = 0.0;
        return 0.0;
    }

    output = osc.tick() * amount;
    return output;
}

void LFO::fillBuffer(double* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = tick();
}
