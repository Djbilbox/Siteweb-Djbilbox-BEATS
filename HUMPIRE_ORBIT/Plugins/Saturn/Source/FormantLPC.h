#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <algorithm>

// Preservation simple des formants par LPC (prediction lineaire) :
//   1) analyse : enveloppe spectrale de la voix (autocorrelation + Levinson-Durbin, trame 25 ms / pas 5 ms),
//   2) filtre inverse (treillis FIR) -> residu "e" = excitation sans formants,
//   3) les voix decalent e (pas x) avec les deux prises, puis
//   4) re-synthese (treillis tout-pole) avec l'enveloppe ORIGINALE -> harmonies au timbre naturel
//      (pas d'effet "chipmunk" vers le haut ni de voix "monstre" vers le bas).
// Coefficients de reflexion interpoles lineairement echantillon par echantillon (|k| < 1 -> treillis stable).
// Aucune allocation hors prepare().
namespace saturn
{
    class FormantLPC
    {
    public:
        static constexpr int maxOrder = 32;

        /** Etat d'un filtre de synthese (un par voix). */
        struct SynthState
        {
            std::array<float, maxOrder + 1> z {};
            void reset() noexcept { z.fill (0.0f); }
        };

        void prepare (double sr)
        {
            order = std::max (16, std::min (maxOrder, (int) (sr / 2400.0)));   // 48 kHz -> ordre 20
            frameLen = std::max (64, (int) std::lround (0.025 * sr));
            hop = std::max (16, (int) std::lround (0.005 * sr));
            int sz = 1; while (sz < frameLen + 4) sz <<= 1;
            mask = sz - 1;
            ring.assign ((size_t) sz, 0.0f);
            frame.assign ((size_t) frameLen, 0.0);
            window.assign ((size_t) frameLen, 0.0);
            const double twoPi = 6.283185307179586476925;
            for (int i = 0; i < frameLen; ++i)
                window[(size_t) i] = 0.5 - 0.5 * std::cos (twoPi * ((double) i + 0.5) / (double) frameLen);
            // fenetre de retard gaussienne (~90 Hz) : lisse l'enveloppe, evite de "coller" aux harmoniques
            for (int k = 0; k <= maxOrder; ++k)
            {
                const double a = twoPi * 90.0 * (double) k / sr;
                lagWindow[(size_t) k] = std::exp (-0.5 * a * a);
            }
            reset();
        }

        void reset() noexcept
        {
            std::fill (ring.begin(), ring.end(), 0.0f);
            k.fill (0.0f); kTarget.fill (0.0f); dk.fill (0.0f); za.fill (0.0f);
            wr = 0; sinceHop = 0; interpLeft = 0;
        }

        int getOrder() const noexcept { return order; }

        /** Pousse un echantillon d'entree, renvoie le residu (filtre inverse avec les coefficients courants). */
        float analyse (float x) noexcept
        {
            if (mask == 0) return x;
            ring[(size_t) wr] = x;
            wr = (wr + 1) & mask;
            if (++sinceHop >= hop) { sinceHop = 0; computeFrame(); }
            if (interpLeft > 0)
            {
                if (--interpLeft == 0) k = kTarget;
                else for (int m = 0; m < order; ++m) k[(size_t) m] += dk[(size_t) m];
            }

            // Treillis FIR : f_m = f_{m-1} + k_m b_{m-1}[n-1] ; b_m[n] = b_{m-1}[n-1] + k_m f_{m-1}
            float f = x, b = x;
            for (int m = 0; m < order; ++m)
            {
                const float bDel = za[(size_t) m];
                za[(size_t) m] = b;
                const float km = k[(size_t) m];
                const float fN = f + km * bDel;
                b = bDel + km * f;
                f = fN;
            }
            if (! (std::abs (f) < 64.0f)) { za.fill (0.0f); f = 0.0f; }   // garde-fou (NaN / emballement)
            return f;
        }

        /** Treillis tout-pole (inverse exact du FIR a coefficients egaux) : residu decale -> voix avec formants d'origine. */
        float synthesise (SynthState& s, float e) const noexcept
        {
            float y = e;
            for (int m = order - 1; m >= 0; --m)
            {
                const float km = k[(size_t) m];
                y -= km * s.z[(size_t) m];
                s.z[(size_t) m + 1] = s.z[(size_t) m] + km * y;
            }
            s.z[0] = y;
            if (! (std::abs (y) < 16.0f)) { s.reset(); y = 0.0f; }          // garde-fou
            return y;
        }

    private:
        void computeFrame() noexcept
        {
            // trame fenetree (pre-accentuation 0.9 : conditionnement numerique, poles reserves aux formants)
            const int start = wr - frameLen;
            float prev = ring[(size_t) ((start - 1) & mask)];
            for (int i = 0; i < frameLen; ++i)
            {
                const float s = ring[(size_t) ((start + i) & mask)];
                frame[(size_t) i] = ((double) s - 0.9 * (double) prev) * window[(size_t) i];
                prev = s;
            }

            std::array<double, maxOrder + 1> r {};
            for (int lag = 0; lag <= order; ++lag)
            {
                double acc = 0.0;
                const double* a = frame.data();
                const double* b = frame.data() + lag;
                const int len = frameLen - lag;
                for (int i = 0; i < len; ++i) acc += a[i] * b[i];
                r[(size_t) lag] = acc;
            }
            if (r[0] < 1e-9 * (double) frameLen) return;       // silence : on garde l'enveloppe precedente

            r[0] *= 1.0001;                                       // correction de bruit blanc (-40 dB)
            for (int lag = 1; lag <= order; ++lag) r[(size_t) lag] *= lagWindow[(size_t) lag];

            // Levinson-Durbin -> coefficients de reflexion
            std::array<double, maxOrder + 1> a {}, tmp {};
            std::array<double, maxOrder> kk {};
            a[0] = 1.0;
            double err = r[0];
            for (int i = 1; i <= order; ++i)
            {
                double acc = r[(size_t) i];
                for (int j = 1; j < i; ++j) acc += a[(size_t) j] * r[(size_t) (i - j)];
                double ki = -acc / err;
                ki = std::max (-0.995, std::min (0.995, ki));
                kk[(size_t) i - 1] = ki;
                for (int j = 1; j < i; ++j) tmp[(size_t) j] = a[(size_t) j] + ki * a[(size_t) (i - j)];
                for (int j = 1; j < i; ++j) a[(size_t) j] = tmp[(size_t) j];
                a[(size_t) i] = ki;
                err *= (1.0 - ki * ki);
                if (err <= r[0] * 1e-9) break;                    // les k restants valent 0
            }

            for (int m = 0; m < order; ++m)
            {
                kTarget[(size_t) m] = (float) kk[(size_t) m];
                dk[(size_t) m] = (kTarget[(size_t) m] - k[(size_t) m]) / (float) hop;
            }
            interpLeft = hop;
        }

        int order = 20, frameLen = 1200, hop = 240, mask = 0, wr = 0, sinceHop = 0, interpLeft = 0;
        std::vector<float> ring;
        std::vector<double> frame, window;
        std::array<double, maxOrder + 1> lagWindow {};
        std::array<float, maxOrder> k {}, kTarget {}, dk {};
        std::array<float, maxOrder> za {};
    };
}
