#pragma once

#include <JuceHeader.h>
#include <vector>

class DelayEffect
{
public:
    DelayEffect();
    ~DelayEffect() = default;

    void setSampleRate(double sr);
    void setTime(double ms);
    void setFeedback(double feedback);
    void setMix(double mix);

    void process(double* buffer, int numSamples);

private:
    double sampleRate = 44100.0;
    double time = 250.0;
    double feedback = 0.4;
    double mix = 0.2;

    std::vector<double> delayBuffer;
    int writePos = 0;
    double smoothDelay = 0.0;
};
