#include "Oscillator.h"
#include <random>

Oscillator::Oscillator()
{
    setSampleRate(44100.0);
    setFrequency(440.0);
}

void Oscillator::setSampleRate(double sr)
{
    sampleRate = sr;
    setFrequency(frequency); // Recalculate phase increment
}

void Oscillator::setFrequency(double freqHz)
{
    frequency = juce::jlimit(0.01, 20000.0, freqHz);
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

//==============================================================================
// PolyBLEP: over the one sample either side of a jump, replace the naive step
// with a short polynomial that carries the same discontinuity but no energy
// above Nyquist.
double Oscillator::polyBlep(double t, double dt)
{
    if (dt <= 0.0)
        return 0.0;

    if (t < dt)                 // just after the jump
    {
        t /= dt;
        return t + t - t * t - 1.0;
    }

    if (t > 1.0 - dt)           // just before the next one
    {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }

    return 0.0;                 // nowhere near an edge
}

double Oscillator::tick()
{
    const double dt = phaseIncrement;
    double sample = 0.0;

    switch (currentWaveform)
    {
        case Waveform::Sine:
            sample = std::sin(2.0 * juce::MathConstants<double>::pi * phase);
            break;

        case Waveform::Sawtooth:
            sample = 2.0 * phase - 1.0;
            sample -= polyBlep(phase, dt);
            break;

        case Waveform::Square:
            sample = phase < 0.5 ? 1.0 : -1.0;
            sample += polyBlep(phase, dt);                              // rising edge
            sample -= polyBlep(std::fmod(phase + 0.5, 1.0), dt);        // falling edge
            break;

        case Waveform::Triangle:
        {
            // Integrating a band-limited square yields a band-limited triangle.
            double square = phase < 0.5 ? 1.0 : -1.0;
            square += polyBlep(phase, dt);
            square -= polyBlep(std::fmod(phase + 0.5, 1.0), dt);

            triangleState += 4.0 * dt * square;
            triangleState *= 0.9995;   // bleeds off the DC the integrator collects
            sample = triangleState;
            break;
        }

        case Waveform::Noise:
            sample = processNoise();
            break;

        default:
            break;
    }

    phase += dt;
    if (phase >= 1.0)
        phase -= 1.0;

    if (std::abs(sample) < 1e-10)
        sample = 0.0;

    return sample;
}

void Oscillator::fillBuffer(double* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = tick();
}

double Oscillator::processNoise()
{
    // Each voice gets its own generator (not a shared static) — a static RNG
    // here would have every playing voice pull from one sequential stream,
    // correlating them instead of giving each its own independent noise.
    thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dis(-1.0, 1.0);
    return dis(gen);
}
