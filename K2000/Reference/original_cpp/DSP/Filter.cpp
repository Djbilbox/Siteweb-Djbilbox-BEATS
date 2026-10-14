#include "Filter.h"
#include <algorithm>

StateVariableFilter::StateVariableFilter()
{
    reset();
    updateCoefficients();
}

void StateVariableFilter::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        updateCoefficients();
    }
}

void StateVariableFilter::setCutoffFrequency(double freqHz)
{
    baseFrequency = freqHz;
    cutoffFrequency = std::max(10.0, std::min(freqHz, sampleRate * 0.49));
    updateCoefficients();
}

void StateVariableFilter::setResonance(double q)
{
    resonance = std::max(0.1, std::min(q, 30.0));
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

void StateVariableFilter::setModulationDepth(double depthOctaves)
{
    modulationDepth = depthOctaves;
}

void StateVariableFilter::modulateCutoff(double amountOctaves)
{
    double modHz = baseFrequency * std::pow(2.0, amountOctaves * modulationDepth);
    cutoffFrequency = std::max(10.0, std::min(modHz, sampleRate * 0.49));
    updateCoefficients();
}

void StateVariableFilter::updateCoefficients()
{
    g = std::tan(juce::MathConstants<double>::pi * cutoffFrequency / sampleRate);
    k = 1.0 / resonance;

    a1 = 1.0 / (1.0 + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

double StateVariableFilter::tick(double inputSample)
{
    double v0 = inputSample;
    double v1 = a1 * ic1eq + a2 * (v0 - ic2eq);
    double v2 = ic2eq + a3 * (v0 - ic2eq) + a2 * ic1eq;

    ic1eq = 2.0 * v1 - ic1eq;
    ic2eq = 2.0 * v2 - ic2eq;

    lowPass = v2;
    bandPass = v1;
    highPass = v0 - k * v1 - v2;

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
