#pragma once

#include <JuceHeader.h>
#include <vector>

class ReverbEffect
{
public:
    ReverbEffect();
    ~ReverbEffect() = default;

    void setSampleRate(double sr);
    void setRoomSize(double room);
    void setDamping(double damp);
    void setWidth(double w);
    void setMix(double mix);

    void process(double* left, double* right, int numSamples);

private:
    double sampleRate = 44100.0;
    double roomSize = 0.5;
    double damping = 0.5;
    double width = 1.0;
    double mix = 0.35;

    // Comb filters (4 per channel)
    std::vector<double> combL[4], combR[4];
    size_t combPosL[4] { 0, 0, 0, 0 }, combPosR[4] { 0, 0, 0, 0 };
    double combFilterL[4] { 0, 0, 0, 0 }, combFilterR[4] { 0, 0, 0, 0 };

    // Allpass filters (2 per channel)
    std::vector<double> allpassL[2], allpassR[2];
    size_t allpassPosL[2] { 0, 0 }, allpassPosR[2] { 0, 0 };

    void initBuffers();
};
