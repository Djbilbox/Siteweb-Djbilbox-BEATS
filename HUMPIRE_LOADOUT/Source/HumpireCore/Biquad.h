#pragma once
#include <cmath>
#include <complex>
#include <algorithm>

// Biquad RBJ (TDF2), sans allocation : calcul des coefficients en place (thread audio OK).
namespace humpire
{
struct Biquad
{
    enum class Type { lowShelf, highShelf, peak, highPass, lowPass };

    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;

    void set (Type t, double fs, double f, double q, double gainDb)
    {
        f = std::min (f, fs * 0.49);  q = std::max (q, 0.05);
        const double A = std::pow (10.0, gainDb / 40.0), w = 6.283185307179586 * f / fs;
        const double cw = std::cos (w), sw = std::sin (w), al = sw / (2.0 * q);
        double B0 = 1, B1 = 0, B2 = 0, A0 = 1, A1 = 0, A2 = 0;
        switch (t)
        {
            case Type::peak:      B0 = 1 + al * A; B1 = -2 * cw; B2 = 1 - al * A; A0 = 1 + al / A; A1 = -2 * cw; A2 = 1 - al / A; break;
            case Type::lowShelf:  { const double s = 2 * std::sqrt (A) * al;
                                    B0 = A * ((A + 1) - (A - 1) * cw + s); B1 = 2 * A * ((A - 1) - (A + 1) * cw); B2 = A * ((A + 1) - (A - 1) * cw - s);
                                    A0 = (A + 1) + (A - 1) * cw + s; A1 = -2 * ((A - 1) + (A + 1) * cw); A2 = (A + 1) + (A - 1) * cw - s; break; }
            case Type::highShelf: { const double s = 2 * std::sqrt (A) * al;
                                    B0 = A * ((A + 1) + (A - 1) * cw + s); B1 = -2 * A * ((A - 1) + (A + 1) * cw); B2 = A * ((A + 1) + (A - 1) * cw - s);
                                    A0 = (A + 1) - (A - 1) * cw + s; A1 = 2 * ((A - 1) - (A + 1) * cw); A2 = (A + 1) - (A - 1) * cw - s; break; }
            case Type::highPass:  B0 = (1 + cw) / 2; B1 = -(1 + cw); B2 = (1 + cw) / 2; A0 = 1 + al; A1 = -2 * cw; A2 = 1 - al; break;
            case Type::lowPass:   B0 = (1 - cw) / 2; B1 = 1 - cw;    B2 = (1 - cw) / 2; A0 = 1 + al; A1 = -2 * cw; A2 = 1 - al; break;
        }
        b0 = (float) (B0 / A0); b1 = (float) (B1 / A0); b2 = (float) (B2 / A0);
        a1 = (float) (A1 / A0); a2 = (float) (A2 / A0);
    }

    float process (float x) noexcept
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void reset() noexcept { z1 = z2 = 0; }

    /** |H(e^jw)| pour dessin / design FIR. */
    double magnitude (double fs, double f) const
    {
        const double w = 6.283185307179586 * f / fs;
        const std::complex<double> z1c = std::polar (1.0, -w), z2c = std::polar (1.0, -2 * w);
        return std::abs ((double) b0 + (double) b1 * z1c + (double) b2 * z2c) / std::abs (1.0 + (double) a1 * z1c + (double) a2 * z2c);
    }
};
}
