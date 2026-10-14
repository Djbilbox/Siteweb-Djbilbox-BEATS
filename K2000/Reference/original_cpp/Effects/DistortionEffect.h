#pragma once

#include <JuceHeader.h>
#include <cmath>

class DistortionEffect
{
public:
    DistortionEffect();
    ~DistortionEffect() = default;

    void setSampleRate(double sr);
    void setDrive(double db);
    void setTone(double hz);
    void setMix(double mix);

    void process(double* left, double* right, int numSamples);

private:
    double sampleRate = 44100.0;
    double driveDb = 0.0;
    double toneHz = 5000.0;
    double mix = 0.0;

    double driveGain = 1.0;
    double toneCoeff = 0.5;
    double filterStateL = 0.0;
    double filterStateR = 0.0;

    void updateParameters();
};
