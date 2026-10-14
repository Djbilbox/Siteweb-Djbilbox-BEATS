#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"

class LFO
{
public:
    enum class Destination
    {
        Pitch = 0,
        Cutoff = 1,
        Amplitude = 2,
        Pan = 3
    };

    LFO();
    ~LFO() = default;

    void setSampleRate(double sr);
    void setRate(double rateHz);
    void setAmount(double amt); // 0-1
    void setDestination(Destination dest);
    void setWaveform(Oscillator::Waveform wf);
    void reset();

    double tick();
    void fillBuffer(double* buffer, int numSamples);

    double getOutput() const { return output; }
    Destination getDestination() const { return currentDestination; }
    double getAmount() const { return amount; }

private:
    Oscillator osc;
    double sampleRate = 44100.0;
    double rate = 2.0;
    double amount = 0.0;
    Destination currentDestination = Destination::Pitch;
    double output = 0.0;
};
