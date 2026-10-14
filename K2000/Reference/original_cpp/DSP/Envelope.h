#pragma once

#include <JuceHeader.h>
#include <cmath>

class ADSREnvelope
{
public:
    enum class State
    {
        Idle = 0,
        Attack = 1,
        Decay = 2,
        Sustain = 3,
        Release = 4
    };

    ADSREnvelope();
    ~ADSREnvelope() = default;

    void setSampleRate(double sr);
    void setAttackTime(double timeMs);
    void setDecayTime(double timeMs);
    void setSustainLevel(double level);
    void setReleaseTime(double timeMs);
    void reset();

    void noteOn();
    void noteOff();

    double tick();
    void fillBuffer(double* output, int numSamples);

    double getOutput() const { return output; }
    State getState() const { return currentState; }

    bool isActive() const { return currentState != State::Idle; }

private:
    double sampleRate = 44100.0;
    double attackTime = 10.0;
    double decayTime = 200.0;
    double sustainLevel = 0.7;
    double releaseTime = 300.0;

    State currentState = State::Idle;
    double output = 0.0;
    double stateCounter = 0.0;

    double attackStartLevel = 0.0;
    double releaseStartLevel = 0.0;
    double samplesPerMs = 0.0;

    void updateRates();
};
