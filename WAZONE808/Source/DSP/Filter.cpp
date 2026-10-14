#include "Filter.h"

StateVariableFilter::StateVariableFilter()
{
    setSampleRate(44100.0);
    setCutoffFrequency(1000.0);
    setResonance(1.0);
    reset();
}

void StateVariableFilter::setSampleRate(double sr)
{
    sampleRate = juce::jlimit(8000.0, 192000.0, sr);
    updateCoefficients();
}

void StateVariableFilter::setCutoffFrequency(double freqHz)
{
    cutoffFrequency = juce::jlimit(20.0, 20000.0, freqHz);
    baseFrequency = cutoffFrequency;
    updateCoefficients();
}

void StateVariableFilter::setResonance(double q)
{
    resonance = juce::jlimit(0.5, 40.0, q);
    updateCoefficients();
}

void StateVariableFilter::setFilterType(FilterType type)
{
    filterType = type;
}

void StateVariableFilter::reset()
{
    lowPass = bandPass = highPass = 0.0;
    ic1eq = ic2eq = 0.0;
}

double StateVariableFilter::tick(double inputSample)
{
    // Zavalishin's TPT SVF: one trapezoidal-integrator step, no clamping needed.
    const double v3 = inputSample - ic2eq;
    const double v1 = a1 * ic1eq + a2 * v3;
    const double v2 = ic2eq + a2 * ic1eq + a3 * v3;

    ic1eq = 2.0 * v1 - ic1eq;
    ic2eq = 2.0 * v2 - ic2eq;

    lowPass  = v2;
    bandPass = v1;
    highPass = inputSample - k * v1 - v2;

    // Keep a runaway (or a denormal) from poisoning the state permanently.
    if (!std::isfinite(ic1eq) || !std::isfinite(ic2eq))
        reset();

    if (std::abs(lowPass) < 1e-15) lowPass = 0.0;
    if (std::abs(bandPass) < 1e-15) bandPass = 0.0;
    if (std::abs(highPass) < 1e-15) highPass = 0.0;

    switch (filterType)
    {
        case FilterType::LowPass:  return lowPass;
        case FilterType::BandPass: return bandPass;
        case FilterType::HighPass: return highPass;
        default:                   return lowPass;
    }
}

void StateVariableFilter::fillBuffer(double* input, double* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        output[i] = tick(input[i]);
}

void StateVariableFilter::setModulationDepth(double depthOctaves)
{
    modulationDepth = depthOctaves;
}

void StateVariableFilter::modulateCutoff(double amountOctaves)
{
    cutoffFrequency = juce::jlimit(20.0, 20000.0, baseFrequency * std::pow(2.0, amountOctaves));
    updateCoefficients();
}

void StateVariableFilter::updateCoefficients()
{
    // Prewarped so the analogue cutoff lands exactly where it is asked for,
    // right up to just below Nyquist.
    const double nyquist = sampleRate * 0.5;
    const double fc = juce::jlimit(10.0, nyquist * 0.99, cutoffFrequency);

    g = std::tan(juce::MathConstants<double>::pi * fc / sampleRate);
    k = 1.0 / juce::jlimit(0.5, 40.0, resonance);

    a1 = 1.0 / (1.0 + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}
