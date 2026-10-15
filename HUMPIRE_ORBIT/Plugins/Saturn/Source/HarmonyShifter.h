#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <algorithm>

// Pitch shift "2 prises" adapte de Plugins/Solar/Source/PitchShifter.h (HUMPIRE SOLAR) pour 4 voix :
//  - une seule ligne a retard mono partagee par toutes les voix (signal + residu LPC pour le mode Formant),
//  - chaque voix ne garde que sa phase de balayage (deux prises en quadrature, fondu sin^2),
//  - fenetre W synchrone au pitch : W/2 = k * periode, donc les deux prises restent en phase (pas de peigne),
//  - retard moyen des prises = latence fixe annoncee (le sec est aligne exactement dessus).
// Aucune allocation hors prepare().
namespace saturn
{
    /** Ligne a retard mono partagee : x = voix d'entree, e = residu LPC (excitation sans formants). */
    class SharedDelayRing
    {
    public:
        void prepare (int minSize)
        {
            int sz = 1; while (sz < minSize) sz <<= 1;
            mask = sz - 1;
            x.assign ((size_t) sz, 0.0f);
            e.assign ((size_t) sz, 0.0f);
            reset();
        }

        void reset()
        {
            std::fill (x.begin(), x.end(), 0.0f);
            std::fill (e.begin(), e.end(), 0.0f);
            wr = 0;
        }

        bool isReady() const noexcept { return mask > 0; }

        /** Ecrit l'echantillon courant (lu ensuite par les voix avec un retard >= 2), puis advance(). */
        void write (float xs, float es) noexcept { x[(size_t) wr] = xs; e[(size_t) wr] = es; }
        void advance() noexcept { wr = (wr + 1) & mask; }

        float readX (double delay) const noexcept { return read (x, delay); }
        float readE (double delay) const noexcept { return read (e, delay); }

        /** Retard maximal lisible sans repli (echantillons). */
        double maxDelay() const noexcept { return (double) mask - 4.0; }

    private:
        /** Lecture Hermite cubique a `delay` echantillons (>= 2) derriere l'ecriture courante (comme SOLAR). */
        float read (const std::vector<float>& b, double delay) const noexcept
        {
            const double rp = (double) wr - delay;
            const double fl = std::floor (rp);
            const int i0 = (int) fl;
            const float f = (float) (rp - fl);
            const float y0 = b[(size_t) ((i0 - 1) & mask)], y1 = b[(size_t) (i0 & mask)],
                        y2 = b[(size_t) ((i0 + 1) & mask)], y3 = b[(size_t) ((i0 + 2) & mask)];
            const float c1 = 0.5f * (y2 - y0);
            const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            return ((c3 * f + c2) * f + c1) * f + y1;
        }

        std::vector<float> x, e;
        int mask = 0, wr = 0;
    };

    //==========================================================================
    /** Longueur de fenetre commune aux voix, synchronisee sur la periode detectee. */
    class WindowControl
    {
    public:
        void prepare (double sr)
        {
            wMax = 0.024 * sr;
            wDefault = 0.020 * sr;
            W = Wtarget = wDefault;
            slew = 1.0 / (0.05 * sr);          // lissage ~50 ms (pas de saut de retard)
        }

        void reset() noexcept { W = Wtarget = wDefault; }

        /** Latence fixe (retard moyen des prises) : W/2 max + 2 echantillons de marge Hermite. */
        int latencySamples() const noexcept { return (int) std::ceil (0.5 * wMax) + 2; }

        /** W = plus grand multiple de 2 x periode <= wMax (les deux prises, separees de W/2, restent en phase). */
        void setPeriod (double periodSamples) noexcept
        {
            if (periodSamples <= 1.0) return;
            const double k = std::floor (wMax / (2.0 * periodSamples));
            Wtarget = k >= 1.0 ? 2.0 * k * periodSamples : wMax;
        }

        void tick() noexcept { W += (Wtarget - W) * slew; }
        double get() const noexcept { return W; }

    private:
        double wMax = 1152.0, wDefault = 960.0, W = 960.0, Wtarget = 960.0, slew = 0.0004;
    };

    //==========================================================================
    /** Etat "deux prises" d'une voix (approche SOLAR) : phase de balayage a la vitesse (1 - ratio),
        prises A et B decalees d'une demi-fenetre, gains sin^2 complementaires. Au repos (ratio ~ 1),
        la phase revient doucement a 0 ou 0.5 : une seule prise active, au retard exact du signal sec. */
    struct TwoTapState
    {
        double phase = 0.0, pB = 0.5;
        float gA = 0.0f, gB = 1.0f;

        void reset() noexcept { phase = 0.0; pB = 0.5; gA = 0.0f; gB = 1.0f; }

        void step (double ratio, double W) noexcept
        {
            const double rate = 1.0 - ratio;
            if (std::abs (rate) < 5e-4)
            {
                const double home = std::round (phase * 2.0) * 0.5;
                const double stepMax = 0.01 / W;
                phase += std::max (-stepMax, std::min (stepMax, home - phase));
            }
            else
                phase += rate / W;
            phase -= std::floor (phase);

            pB = phase + 0.5; pB -= std::floor (pB);
            const double sA = std::sin (3.14159265358979323846 * phase), sB = std::sin (3.14159265358979323846 * pB);
            gA = (float) (sA * sA);
            gB = (float) (sB * sB);
        }
    };
}
