#include "DistortionEffect.h"
#include <algorithm>

DistortionEffect::DistortionEffect()
{
    updateParameters();
}

void DistortionEffect::setSampleRate(double sr)
{
    if (sr > 0.0)
    {
        sampleRate = sr;
        updateParameters();
    }
}

void DistortionEffect::setDrive(double db)
{
    driveDb = std::max(0.0, std::min(db, 36.0));
    updateParameters();
}

void DistortionEffect::setTone(double hz)
{
    toneHz = std::max(200.0, std::min(hz, sampleRate * 0.49));
    updateParameters();
}

void DistortionEffect::setMix(double newMix)
{
    mix = std::max(0.0, std::min(newMix, 1.0));
}

void DistortionEffect::updateParameters()
{
    driveGain = std::pow(10.0, driveDb / 20.0);
    double w = 2.0 * juce::MathConstants<double>::pi * toneHz / sampleRate;
    toneCoeff = w / (w + 1.0);
}

void DistortionEffect::process(double* left, double* right, int numSamples)
{
    if (mix <= 0.0001) return;

    for (int i = 0; i < numSamples; ++i)
    {
        double dryL = left[i];
        double dryR = right[i];

        // Soft-clipping asymmetric saturation
        double inL = dryL * driveGain;
        double inR = dryR * driveGain;

        auto sat = [](double x) {
            double s = x * 1.2;
            return (s + 0.15 * s * s) / (1.0 + std::abs(s) + 0.15 * s * s);
        };

        double wetL = sat(inL);
        double wetR = sat(inR);

        // One-pole lowpass tone control
        filterStateL += (wetL - filterStateL) * toneCoeff;
        filterStateR += (wetR - filterStateR) * toneCoeff;

        left[i] = dryL * (1.0 - mix) + filterStateL * mix;
        right[i] = dryR * (1.0 - mix) + filterStateR * mix;
    }
}
