#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
#include "SaturnParams.h"

namespace saturn
{
    /** Real-time YIN pitch detector (de Cheveigne & Kawahara), ported from HUMPIRE SOLAR.
        The input is low-passed and decimated to about 16 kHz; one estimate every 10 ms; range 70-1000 Hz.
        Every buffer is allocated in prepare(); nothing is allocated while audio runs. */
    class PitchDetector
    {
    public:
        static constexpr float minHz = 70.0f, maxHz = 1000.0f;

        void prepare (double sampleRate)
        {
            decim = std::max (1, (int) (sampleRate / 16000.0));
            dsr = sampleRate / (double) decim;
            tauMin = std::max (2, (int) std::floor (dsr / (double) maxHz));
            tauMax = (int) std::ceil (dsr / (double) minHz);
            win = (int) std::lround (0.030 * dsr);
            hop = (int) std::lround (0.010 * dsr);
            need = win + tauMax + 2;
            int sz = 1;
            while (sz < need * 2) sz <<= 1;
            mask = sz - 1;
            ring.assign ((size_t) sz, 0.0f);
            lin.assign ((size_t) need, 0.0f);
            cmnd.assign ((size_t) tauMax + 2, 1.0f);

            // two cascaded second-order low-pass sections (4th-order Butterworth) below the decimated Nyquist
            makeLowpass (lp[0], sampleRate, 0.4 * dsr, 0.5412);
            makeLowpass (lp[1], sampleRate, 0.4 * dsr, 1.3066);
            reset();
        }

        void reset()
        {
            std::fill (ring.begin(), ring.end(), 0.0f);
            for (auto& b : lp) b.z1 = b.z2 = 0.0f;
            wr = 0;
            sinceHop = 0;
            filled = 0;
            counter = 0;
            f0Hz = 0.0f;
            conf = 0.0f;
            hist[0] = hist[1] = hist[2] = 0.0f;
        }

        /** Number of input samples between two estimates. */
        int hopInputSamples() const noexcept { return hop * decim; }

        /** Pushes one input sample; returns true when a new estimate is available. */
        bool pushSample (float x) noexcept
        {
            x = lp[1].process (lp[0].process (x));
            if (++counter < decim) return false;
            counter = 0;
            ring[(size_t) wr] = x;
            wr = (wr + 1) & mask;
            if (filled < need) ++filled;                 // saturates: no signed overflow on long sessions
            if (++sinceHop >= hop && filled >= need)
            {
                sinceHop = 0;
                analyse();
                return true;
            }
            return false;
        }

        float getF0() const noexcept         { return f0Hz; }          // 0 = unvoiced
        float getConfidence() const noexcept { return conf; }

    private:
        struct Biquad
        {
            float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f, z1 = 0.0f, z2 = 0.0f;

            float process (float x) noexcept
            {
                const float y = b0 * x + z1;
                z1 = b1 * x - a1 * y + z2;
                z2 = b2 * x - a2 * y;
                return y;
            }
        };

        static void makeLowpass (Biquad& b, double fs, double fc, double q)
        {
            const double w0 = 2.0 * kPi * fc / fs;
            const double c = std::cos (w0), a = std::sin (w0) / (2.0 * q);
            const double a0 = 1.0 + a;
            b.b0 = (float) ((1.0 - c) * 0.5 / a0);
            b.b1 = (float) ((1.0 - c) / a0);
            b.b2 = b.b0;
            b.a1 = (float) (-2.0 * c / a0);
            b.a2 = (float) ((1.0 - a) / a0);
        }

        void analyse() noexcept
        {
            const int start = wr - need;
            for (int i = 0; i < need; ++i) lin[(size_t) i] = ring[(size_t) ((start + i) & mask)];

            // energy gate (about -54 dBFS RMS)
            float e = 0.0f;
            for (int i = 0; i < win; ++i) e += lin[(size_t) i] * lin[(size_t) i];
            if (std::sqrt (e / (float) win) < 0.002f) { publish (0.0f, 0.0f); return; }

            // cumulative mean normalised difference function (CMNDF)
            cmnd[0] = 1.0f;
            float running = 0.0f;
            for (int tau = 1; tau <= tauMax; ++tau)
            {
                float d = 0.0f;
                const float* a = lin.data();
                const float* b = lin.data() + tau;
                for (int j = 0; j < win; ++j)
                {
                    const float t = a[j] - b[j];
                    d += t * t;
                }
                running += d;
                cmnd[(size_t) tau] = running > 1e-12f ? d * (float) tau / running : 1.0f;
            }

            int tau = -1;
            for (int t = tauMin; t <= tauMax; ++t)
                if (cmnd[(size_t) t] < 0.15f)
                {
                    while (t + 1 <= tauMax && cmnd[(size_t) t + 1] < cmnd[(size_t) t]) ++t;
                    tau = t;
                    break;
                }

            if (tau < 0)   // fallback: global minimum, if it is clean enough
            {
                float best = 1.0f;
                for (int t = tauMin; t <= tauMax; ++t)
                    if (cmnd[(size_t) t] < best) { best = cmnd[(size_t) t]; tau = t; }
                if (best > 0.25f) { publish (0.0f, 0.0f); return; }
            }

            // parabolic interpolation around the minimum
            float tauF = (float) tau;
            if (tau > 1 && tau < tauMax)
            {
                const float s0 = cmnd[(size_t) tau - 1], s1 = cmnd[(size_t) tau], s2 = cmnd[(size_t) tau + 1];
                const float den = s0 + s2 - 2.0f * s1;
                if (std::abs (den) > 1e-9f) tauF += clampUnit (0.5f * (s0 - s2) / den);
            }
            const float f = (float) dsr / tauF;
            publish ((f >= minHz && f <= maxHz) ? f : 0.0f, 1.0f - cmnd[(size_t) tau]);
        }

        static float clampUnit (float v) noexcept { return std::max (-1.0f, std::min (1.0f, v)); }

        /** Median of three frames: rejects isolated octave errors. */
        void publish (float f, float c) noexcept
        {
            hist[0] = hist[1];
            hist[1] = hist[2];
            hist[2] = f;
            if (hist[0] > 0.0f && hist[1] > 0.0f && hist[2] > 0.0f)
            {
                const float a = hist[0], b = hist[1], d = hist[2];
                f0Hz = std::max (std::min (a, b), std::min (std::max (a, b), d));
            }
            else
                f0Hz = f;
            conf = c;
        }

        int decim = 1, tauMin = 2, tauMax = 100, win = 400, hop = 160, need = 600, mask = 1023;
        int wr = 0, sinceHop = 0, filled = 0, counter = 0;
        double dsr = 16000.0;
        std::vector<float> ring, lin, cmnd;
        Biquad lp[2];
        float f0Hz = 0.0f, conf = 0.0f, hist[3] {};
    };
}
