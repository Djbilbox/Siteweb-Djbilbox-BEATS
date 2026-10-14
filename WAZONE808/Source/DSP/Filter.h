#pragma once

#include <JuceHeader.h>
#include <cmath>

// Topology-preserving-transform state variable filter (Zavalishin).
//
// Replaces a Chamberlin SVF whose coefficient had to be clamped for stability,
// capping the usable cutoff at about 3.5kHz: every preset asking for more got
// the same muffled corner, which is why bright patches were neither bright nor
// distinguishable from each other. This form stays exact and stable all the way
// to Nyquist, and self-oscillates cleanly at high resonance instead of blowing up.
class StateVariableFilter
{
public:
    enum class FilterType
    {
        LowPass = 0,
        BandPass = 1,
        HighPass = 2
    };

    StateVariableFilter();
    ~StateVariableFilter() = default;

    void setSampleRate(double sr);
    void setCutoffFrequency(double freqHz);
    void setResonance(double q);
    void setFilterType(FilterType type);
    void reset();

    double tick(double inputSample);
    void fillBuffer(double* input, double* output, int numSamples);

    double getLowPassOutput() const { return lowPass; }
    double getBandPassOutput() const { return bandPass; }
    double getHighPassOutput() const { return highPass; }

    // Modulation
    void setModulationDepth(double depthOctaves);
    void modulateCutoff(double amountOctaves);

private:
    double sampleRate = 44100.0;
    double cutoffFrequency = 1000.0;
    double resonance = 1.0;
    FilterType filterType = FilterType::LowPass;

    double lowPass = 0.0, bandPass = 0.0, highPass = 0.0;

    // TPT coefficients
    double g = 0.0, k = 1.0, a1 = 0.0, a2 = 0.0, a3 = 0.0;
    // Integrator state
    double ic1eq = 0.0, ic2eq = 0.0;

    double modulationDepth = 0.0;
    double baseFrequency = 1000.0;

    void updateCoefficients();
};
