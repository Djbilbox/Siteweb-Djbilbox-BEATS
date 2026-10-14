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
    Waveform currentWaveform = Waveform::Sine;

    // Running integrator for the band-limited triangle (integrated square).
    double triangleState = 0.0;

    double processNoise();

    // Correction pulse that rounds off the instant a naive waveform jumps.
    // Those vertical edges are what alias — an ideal saw or square needs
    // infinite bandwidth, so sampling one directly folds everything above
    // Nyquist back down as inharmonic grit. Oversampling with a gentle
    // one-pole lowpass, as this did before, barely touched it; subtracting a
    // BLEP at each discontinuity removes it at the source.
    static double polyBlep(double t, double dt);
};
