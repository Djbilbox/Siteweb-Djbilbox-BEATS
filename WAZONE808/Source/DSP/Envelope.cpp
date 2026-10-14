#include "Envelope.h"

ADSREnvelope::ADSREnvelope()
{
    setSampleRate(44100.0);
    setAttackTime(50.0);
    setDecayTime(500.0);
    setSustainLevel(0.7);
    setReleaseTime(1000.0);
    reset();
}

void ADSREnvelope::setSampleRate(double sr)
{
    sampleRate = juce::jlimit(8000.0, 192000.0, sr);
    samplesPerMs = sampleRate / 1000.0;
}

void ADSREnvelope::setAttackTime(double timeMs)
{
    attackTime = juce::jlimit(0.0, 5000.0, timeMs);
}

void ADSREnvelope::setDecayTime(double timeMs)
{
    decayTime = juce::jlimit(0.0, 5000.0, timeMs);
}

void ADSREnvelope::setSustainLevel(double level)
{
    sustainLevel = juce::jlimit(0.0, 1.0, level);
}

void ADSREnvelope::setReleaseTime(double timeMs)
{
    releaseTime = juce::jlimit(0.0, 5000.0, timeMs);
}

void ADSREnvelope::reset()
{
    currentState = State::Idle;
    output = 0.0;
    stateCounter = 0.0;
    attackStartLevel = 0.0;
    releaseStartLevel = 0.0;
}

void ADSREnvelope::noteOn()
{
    currentState = State::Attack;
    stateCounter = 0.0;
    // Ramp up from wherever the envelope currently sits rather than snapping to
    // zero, so retriggering a note that is still sounding — or reusing a stolen
    // voice — stays continuous.
    attackStartLevel = output;
}

void ADSREnvelope::noteOff()
{
    currentState = State::Release;
    stateCounter = 0.0;
    // Release from the level actually reached. Falling from sustainLevel
    // instead meant letting go during attack or decay stepped the output
    // straight to that value first, which clicked on every short note.
    releaseStartLevel = output;
}

double ADSREnvelope::tick()
{
    switch (currentState)
    {
        case State::Idle:
            output = 0.0;
            break;

        case State::Attack:
        {
            if (attackTime <= 0.0)
            {
                output = 1.0;
                currentState = State::Decay;
                stateCounter = 0.0;
            }
            else
            {
                double attackSamples = attackTime * samplesPerMs;
                double progress = stateCounter / attackSamples;

                // Attack curve, scaled to run from wherever the envelope already
                // was up to full. The exponent was 0.3, which rises so steeply
                // out of zero that a 1ms attack put the very first sample at
                // 0.32 — a step that big at note onset is a click. At 0.85 the
                // same attack starts at 0.04 and still sounds immediate.
                output = attackStartLevel + (1.0 - attackStartLevel) * std::pow(progress, 0.85);

                stateCounter++;
                if (stateCounter >= attackSamples)
                {
                    output = 1.0;
                    currentState = State::Decay;
                    stateCounter = 0.0;
                }
            }
            break;
        }

        case State::Decay:
        {
            if (decayTime <= 0.0)
            {
                output = sustainLevel;
                currentState = State::Sustain;
                stateCounter = 0.0;
            }
            else
            {
                double decaySamples = decayTime * samplesPerMs;
                double progress = stateCounter / decaySamples;

                // Linear decay from 1 to sustain level
                output = 1.0 - progress * (1.0 - sustainLevel);

                stateCounter++;
                if (stateCounter >= decaySamples)
                {
                    output = sustainLevel;
                    currentState = State::Sustain;
                    stateCounter = 0.0;
                }
            }
            break;
        }

        case State::Sustain:
            output = sustainLevel;
            break;

        case State::Release:
        {
            if (releaseTime <= 0.0)
            {
                output = 0.0;
                currentState = State::Idle;
                stateCounter = 0.0;
            }
            else
            {
                double releaseSamples = releaseTime * samplesPerMs;
                double progress = stateCounter / releaseSamples;

                // Quadratic release (exponential-like), from the level the note
                // was actually at when it was let go.
                output = releaseStartLevel * std::pow(1.0 - progress, 2.0);

                stateCounter++;
                if (stateCounter >= releaseSamples)
                {
                    output = 0.0;
                    currentState = State::Idle;
                    stateCounter = 0.0;
                }
            }
            break;
        }

        default:
            output = 0.0;
            break;
    }

    // Prevent denormalization
    if (std::abs(output) < 1e-10)
        output = 0.0;

    return output;
}

void ADSREnvelope::fillBuffer(double* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = tick();
    }
}
