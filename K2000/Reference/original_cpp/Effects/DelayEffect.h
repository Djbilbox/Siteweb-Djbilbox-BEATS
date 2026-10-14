#pragma once

#include <JuceHeader.h>
#include <vector>

class DelayEffect
{
public:
    DelayEffect();
    ~DelayEffect() = default;

    void setSampleRate(double sr);
    void setTime(double timeMs);
    void setFeedback(double fb);
    void setMix(double mix);

    void process(double* left, double* right, int numSamples);

private:
    double sampleRate = 44100.0;
    double timeMs = 375.0;
    double feedback = 0.45;
    double mix = 0.3;

    std::vector<double> bufferL, bufferR;
    int writePos = 0;
    double filterL = 0.0, filterR = 0.0;
};
