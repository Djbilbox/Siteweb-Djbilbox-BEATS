#pragma once

#include <cmath>

namespace rk
{

//==============================================================================
// Topology-preserving transform state-variable filter (SVF), stable at high cutoff.
// 12 dB/oct. Types: 0 = lowpass, 1 = bandpass, 2 = highpass.
//==============================================================================
class SvfFilter
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        updateCoeffs();
    }

    void reset() noexcept
    {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }

    void setType (int t) noexcept { type = t < 0 ? 0 : (t > 2 ? 2 : t); }

    // Les coefficients sont recalcules ici et non laisses a l'appelant : avec
    // les valeurs neutres du depart (a1 = 1, a2 = a3 = 0) l'etat interne reste
    // a zero et le filtre ne rend que du silence - un piege invisible tant que
    // personne ne pense a appeler updateCoeffs().
    //
    // Le recalcul est saute si la coupure n'a pas bouge de facon audible : la
    // frequence est modulee a chaque echantillon par l'enveloppe, et une
    // tangente par echantillon et par canal n'apporterait rien.
    void setCutoff (float fc) noexcept
    {
        if (std::abs (fc - cutoff) < 0.5f)
            return;
        cutoff = fc;
        updateCoeffs();
    }

    void setResonance (float q) noexcept
    {
        const float clamped = std::max (0.1f, q);
        if (std::abs (clamped - res) < 0.001f)
            return;
        res = clamped;
        updateCoeffs();
    }

    void updateCoeffs() noexcept
    {
        const float maxFc = 0.49f * (float) sampleRate;
        const float fc = std::min (std::max (10.0f, cutoff), maxFc);
        const float g = std::tan (3.14159265358979f * fc / (float) sampleRate);

        // k etait une variable locale du meme nom que le membre : le passe-haut
        // lisait donc un k reste a 1 quelle que soit la resonance.
        k = 1.0f / res;

        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    float process (float v0) noexcept
    {
        const float v3 = v0 - ic2eq;
        const float v1 = a1 * ic1eq + a2 * v3;
        const float v2 = ic2eq + a2 * ic1eq + a3 * v3;

        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        switch (type)
        {
            case 1: return v1;              // bandpass
            case 2: return v0 - k * v1 - v2; // highpass
            default: return v2;              // lowpass
        }
    }

private:
    double sampleRate = 48000.0;
    int    type = 0;
    float  cutoff = 4000.0f;
    float  res = 1.0f;
    float  ic1eq = 0.0f;
    float  ic2eq = 0.0f;
    float  a1 = 1.0f;
    float  a2 = 0.0f;
    float  a3 = 0.0f;
    float  k = 1.0f;
};

} // namespace rk