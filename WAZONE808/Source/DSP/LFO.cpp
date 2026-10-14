#include "LFO.h"

LFO::LFO() : Oscillator()
{
    // LFO defaults
    setSampleRate(44100.0);
    setFrequency(2.0); // 2Hz default
    setWaveform(Waveform::Sine);
    setAmount(0.5);
    setDestination(Destination::Pitch);
}

void LFO::setAmount(double amt)
{
    amount = juce::jlimit(0.0, 1.0, amt);
}

void LFO::setDestination(Destination dest)
{
    destination = dest;
}

void LFO::setWaveformType(Waveform wf)
{
    setWaveform(wf);
}
