#pragma once

#include <JuceHeader.h>
#include <cmath>

class DistortionEffect
{
public:
    DistortionEffect();
    ~DistortionEffect() = default;

    void setSampleRate(double sr);
    void setDrive(double db); // 0-24dB
    void setTone(double hz);  // 100-8000Hz
    void setMix(double mix);  // 0-1

    void process(double* buffer, int numSamples);

private:
    double sampleRate = 44100.0;
    double drive = 0.0;
    double tone = 5000.0;
    double mix = 0.0;

    // Tone filter state
    double filterState = 0.0;
    double filterCoeff = 0.0;

    void updateToneFilter();
};
