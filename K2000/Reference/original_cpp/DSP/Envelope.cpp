#include "Envelope.h"
#include <algorithm>

ADSREnvelope::ADSREnvelope()
{
    updateRates();
    reset();
}

void ADSREnvelope::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        updateRates();
    }
}

void ADSREnvelope::setAttackTime(double timeMs)
{
    attackTime = std::max(0.5, timeMs);
}

void ADSREnvelope::setDecayTime(double timeMs)
{
    decayTime = std::max(1.0, timeMs);
}

void ADSREnvelope::setSustainLevel(double level)
{
    sustainLevel = std::max(0.0, std::min(level, 1.0));
}

void ADSREnvelope::setReleaseTime(double timeMs)
{
    releaseTime = std::max(1.0, timeMs);
}

void ADSREnvelope::updateRates()
{
    samplesPerMs = sampleRate / 1000.0;
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
    attackStartLevel = output;
    currentState = State::Attack;
    stateCounter = 0.0;
}

void ADSREnvelope::noteOff()
{
    if (currentState != State::Idle)
    {
        releaseStartLevel = output;
        currentState = State::Release;
        stateCounter = 0.0;
    }
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
            double attackSamples = attackTime * samplesPerMs;
            if (stateCounter >= attackSamples)
            {
                output = 1.0;
                currentState = State::Decay;
                stateCounter = 0.0;
            }
            else
            {
                double progress = stateCounter / attackSamples;
                output = attackStartLevel + (1.0 - attackStartLevel) * progress;
                stateCounter += 1.0;
            }
            break;
        }

        case State::Decay:
        {
            double decaySamples = decayTime * samplesPerMs;
            if (stateCounter >= decaySamples)
            {
                output = sustainLevel;
                currentState = State::Sustain;
                stateCounter = 0.0;
            }
            else
            {
                double progress = stateCounter / decaySamples;
                output = 1.0 - (1.0 - sustainLevel) * progress;
                stateCounter += 1.0;
            }
            break;
        }

        case State::Sustain:
            output = sustainLevel;
            break;

        case State::Release:
        {
            double releaseSamples = releaseTime * samplesPerMs;
            if (stateCounter >= releaseSamples)
            {
                output = 0.0;
                currentState = State::Idle;
                stateCounter = 0.0;
            }
            else
            {
                double progress = stateCounter / releaseSamples;
                output = releaseStartLevel * (1.0 - progress);
                stateCounter += 1.0;
            }
            break;
        }
    }

    return output;
}

void ADSREnvelope::fillBuffer(double* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = tick();
}
