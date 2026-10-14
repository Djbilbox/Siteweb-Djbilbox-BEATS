#include "Oscillator.h"

Oscillator::Oscillator()
{
    reset();
}

void Oscillator::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        phaseIncrement = frequency / sampleRate;
    }
}

void Oscillator::setFrequency(double freqHz)
{
    frequency = std::max(1.0, std::min(freqHz, sampleRate * 0.49));
    phaseIncrement = frequency / sampleRate;
}

void Oscillator::setWaveform(Waveform wf)
{
    currentWaveform = wf;
}

void Oscillator::reset()
{
    phase = 0.0;
    triangleState = 0.0;
}

double Oscillator::polyBlep(double t, double dt)
{
    if (dt <= 0.0) return 0.0;

    // Discontinuity at 0 / 1
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0;
    }
    else if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }
    return 0.0;
}

double Oscillator::processNoise()
{
    return (2.0 * ((double) std::rand() / (double) RAND_MAX)) - 1.0;
}

double Oscillator::tick()
{
    double out = 0.0;
    const double dt = phaseIncrement;

    switch (currentWaveform)
    {
        case Waveform::Sine:
        {
            out = std::sin(phase * juce::MathConstants<double>::twoPi);
            break;
        }

        case Waveform::Square:
        {
            out = (phase < 0.5) ? 1.0 : -1.0;
            out += polyBlep(phase, dt);
            out -= polyBlep(std::fmod(phase + 0.5, 1.0), dt);
            break;
        }

        case Waveform::Sawtooth:
        {
            out = 2.0 * phase - 1.0;
            out -= polyBlep(phase, dt);
            break;
        }

        case Waveform::Triangle:
        {
            // Leaky integrated square wave
            double sq = (phase < 0.5) ? 1.0 : -1.0;
            sq += polyBlep(phase, dt);
            sq -= polyBlep(std::fmod(phase + 0.5, 1.0), dt);

            triangleState = triangleState * 0.999 + sq * dt * 4.0;
            out = triangleState;
            break;
        }

        case Waveform::Noise:
        {
            out = processNoise();
            break;
        }
    }

    phase += phaseIncrement;
    if (phase >= 1.0)
        phase -= 1.0;

    return out;
}

void Oscillator::fillBuffer(double* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = tick();
}
