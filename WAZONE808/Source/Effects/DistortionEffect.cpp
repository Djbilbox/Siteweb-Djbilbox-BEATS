#include "DistortionEffect.h"

DistortionEffect::DistortionEffect()
{
    setSampleRate(44100.0);
    setDrive(0.0);
    setTone(5000.0);
    setMix(0.0);
}

void DistortionEffect::setSampleRate(double sr)
{
    sampleRate = sr;
    updateToneFilter();
}

void DistortionEffect::setDrive(double db)
{
    drive = juce::jlimit(0.0, 24.0, db);
}

void DistortionEffect::setTone(double hz)
{
    tone = juce::jlimit(100.0, 8000.0, hz);
    updateToneFilter();
}

void DistortionEffect::setMix(double newMix)
{
    mix = juce::jlimit(0.0, 1.0, newMix);
}

void DistortionEffect::process(double* buffer, int numSamples)
{
    // Convert dB to linear
    double preGain = std::pow(10.0, drive / 20.0);

    for (int i = 0; i < numSamples; ++i)
    {
        // Pre-gain
        double sample = buffer[i] * preGain;

        // Soft-clip (tanh saturation)
        double clipped = std::tanh(sample);

        // Tone filter (low-pass)
        filterState = filterState + filterCoeff * (clipped - filterState);
        double filtered = filterState;

        // Mix
        buffer[i] = buffer[i] * (1.0 - mix) + filtered * mix;
    }
}

void DistortionEffect::updateToneFilter()
{
    // Simple 1-pole low-pass
    double normalizedCutoff = tone / sampleRate;
    normalizedCutoff = juce::jlimit(0.0001, 0.5, normalizedCutoff);
    filterCoeff = 2.0 * juce::MathConstants<double>::pi * normalizedCutoff;
    filterCoeff = juce::jlimit(0.0, 1.0, filterCoeff);
}
