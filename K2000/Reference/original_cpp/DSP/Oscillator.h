#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <array>

class Oscillator
{
public:
    enum class Waveform
    {
        Sine = 0,
        Square = 1,
        Sawtooth = 2,
        Triangle = 3,
        Noise = 4
    };

    Oscillator();
    ~Oscillator() = default;

    void setSampleRate(double sr);
    void setFrequency(double freqHz);
    void setWaveform(Waveform wf);
    void reset();

    double tick();
    void fillBuffer(double* buffer, int numSamples);

    double getPhase() const { return phase; }
    void setPhase(double p) { phase = std::fmod(p, 1.0); }

private:
    double phase = 0.0;
    double phaseIncrement = 0.0;
    double sampleRate = 44100.0;
    double frequency = 440.0;
    Waveform currentWaveform = Waveform::Sawtooth;

    double triangleState = 0.0;
    double processNoise();

    static double polyBlep(double t, double dt);
};
