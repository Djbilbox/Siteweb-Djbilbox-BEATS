#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <algorithm>

/** Pitch shifter temporel faible latence : deux prises sur une ligne a retard, balayage a la vitesse (1 - ratio),
    fenetre de fondu sin^2, longueur de fenetre = multiple entier de la periode detectee (fondu synchrone au pitch).
    Fournit aussi le signal sec aligne (retard moyen de la fenetre) pour le Mix. Aucune allocation hors prepare(). */
class PitchShifter
{
public:
    static constexpr int maxChannels = 2;

    void prepare (double sr)
    {
        sampleRate = sr;
        int sz = 1; while (sz < (int) (0.1 * sr)) sz <<= 1;
        mask = sz - 1;
        for (auto& b : buf) b.assign ((size_t) sz, 0.0f);
        wMin = 0.016 * sr;  wMax = 0.022 * sr;
        W = Wtarget = 0.5 * (wMin + wMax);
        wSlew = 1.0 / (0.05 * sr);
        reset();
    }

    void reset()
    {
        for (auto& b : buf) std::fill (b.begin(), b.end(), 0.0f);
        wr = 0;
        phase = 0.0;      // etat de repos : prise B a 0.5 -> sortie = signal sec retarde de W/2
    }

    /** Latence moyenne annoncee a l'hote (echantillons). */
    int latencySamples() const noexcept { return (int) std::lround (0.5 * 0.5 * (wMin + wMax)) + 2; }

    /** Cible de longueur de fenetre depuis la periode (en echantillons). */
    void setPeriod (float periodSamples) noexcept
    {
        if (periodSamples <= 1.0f) return;
        const double k = std::ceil (wMin / periodSamples);
        Wtarget = std::min (wMax, std::max (wMin, k * (double) periodSamples));
    }

    /** Traite un echantillon (nCh voies) : wet = voix decalee, dry = voix seche alignee. */
    void process (const float* in, float* wet, float* dry, int nCh, double ratio) noexcept
    {
        for (int c = 0; c < nCh; ++c) buf[(size_t) c][(size_t) wr] = in[c];

        W += (Wtarget - W) * wSlew;
        const double rate = 1.0 - ratio;
        if (std::abs (rate) < 5e-4)            // repos : revient doucement vers un etat sans peigne
        {
            const double home = std::round (phase * 2.0) * 0.5;
            const double step = 0.01 / W;
            phase += std::max (-step, std::min (step, home - phase));
        }
        else
            phase += rate / W;
        phase -= std::floor (phase);

        double pB = phase + 0.5; pB -= std::floor (pB);
        const double sA = std::sin (3.14159265358979323846 * phase), sB = std::sin (3.14159265358979323846 * pB);
        const float gA = (float) (sA * sA), gB = (float) (sB * sB);
        const double dA = phase * W + 2.0, dB = pB * W + 2.0, dD = 0.5 * W + 2.0;

        for (int c = 0; c < nCh; ++c)
        {
            wet[c] = gA * read (c, dA) + gB * read (c, dB);
            dry[c] = read (c, dD);
        }
        wr = (wr + 1) & mask;
    }

private:
    /** Lecture Hermite cubique a `delay` echantillons (>= 2) derriere l'ecriture courante. */
    float read (int c, double delay) const noexcept
    {
        const double rp = (double) wr - delay;
        const int i0 = (int) std::floor (rp);
        const float f = (float) (rp - (double) i0);
        const auto& b = buf[(size_t) c];
        const float y0 = b[(size_t) ((i0 - 1) & mask)], y1 = b[(size_t) (i0 & mask)],
                    y2 = b[(size_t) ((i0 + 1) & mask)], y3 = b[(size_t) ((i0 + 2) & mask)];
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + y1;
    }

    std::array<std::vector<float>, maxChannels> buf;
    double sampleRate = 44100.0, W = 800.0, Wtarget = 800.0, wMin = 700.0, wMax = 1000.0, wSlew = 0.0005, phase = 0.0;
    int mask = 0, wr = 0;
};
