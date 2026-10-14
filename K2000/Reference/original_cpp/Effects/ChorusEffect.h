#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

class ChorusEffect
{
public:
    ChorusEffect();
    ~ChorusEffect() = default;

    void setSampleRate(double sr);
    void setRate(double hz);
    void setAmount(double amt);
    void setMix(double mix);

    void process(double* left, double* right, int numSamples);

private:
    double sampleRate = 44100.0;
    double rate = 1.5;
    double amount = 0.5;
    double mix = 0.3;

    std::vector<double> delayBufferL, delayBufferR;
    int writePos = 0;
    double lfoPhase = 0.0;
    double baseDelayMs = 12.0;
};
