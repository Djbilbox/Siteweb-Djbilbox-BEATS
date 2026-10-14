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
    void setSustainLevel(double level); // 0-1
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
    double attackTime = 0.05;
    double decayTime = 0.5;
    double sustainLevel = 0.7;
    double releaseTime = 1.0;

    State currentState = State::Idle;
    double output = 0.0;
    double stateCounter = 0.0;

    // Levels each stage starts from, captured when that stage begins. Without
    // them attack always restarted at 0 and release always fell from the
    // sustain level, so retriggering a sounding note or letting go before
    // sustain was reached made the output jump — the clicks on fast passages
    // and on every short, percussive patch.
    double attackStartLevel = 0.0;
    double releaseStartLevel = 0.0;

    // Precomputed rate (samples per ms)
    double samplesPerMs = 0.0;

    void updateRates();
};
