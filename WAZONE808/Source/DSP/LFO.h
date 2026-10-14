#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"

class LFO : public Oscillator
{
public:
    enum class Destination
    {
        Pitch = 0,
        FilterCutoff = 1,
        Amplitude = 2,
        Pan = 3
    };

    LFO();
    ~LFO() = default;

    void setAmount(double amt); // 0-1
    void setDestination(Destination dest);
    void setWaveformType(Waveform wf);

    double getAmount() const { return amount; }
    Destination getDestination() const { return destination; }

private:
    double amount = 0.0; // 0-1
    Destination destination = Destination::Pitch;
};
