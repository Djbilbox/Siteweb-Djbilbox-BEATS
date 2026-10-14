#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

/** Detection de hauteur YIN temps reel (algorithme de Cheveigne & Kawahara, reimplemente).
    Entree decimee vers ~16-22 kHz apres filtre passe-bas ; une estimation toutes les ~10 ms.
    Aucune allocation hors prepare(). */
class PitchDetector
{
public:
    static constexpr float minHz = 70.0f, maxHz = 1000.0f;

    void prepare (double sampleRate)
    {
        decim = std::max (1, (int) (sampleRate / 16000.0));
        dsr = sampleRate / decim;
        tauMin = std::max (2, (int) std::floor (dsr / maxHz));
        tauMax = (int) std::ceil (dsr / minHz);
        win = (int) std::lround (0.030 * dsr);
        hop = (int) std::lround (0.010 * dsr);
        need = win + tauMax + 2;
        int sz = 1; while (sz < need * 2) sz <<= 1;
        mask = sz - 1;
        ring.assign ((size_t) sz, 0.0f);
        lin.assign ((size_t) need, 0.0f);
        cmnd.assign ((size_t) tauMax + 2, 1.0f);

        // 2 biquads passe-bas (Butterworth 4e ordre) a ~0.4 * fs_decime
        makeLowpass (lp[0], sampleRate, 0.4 * dsr, 0.5412);
        makeLowpass (lp[1], sampleRate, 0.4 * dsr, 1.3066);
        reset();
    }

    void reset()
    {
        std::fill (ring.begin(), ring.end(), 0.0f);
        for (auto& b : lp) b.z1 = b.z2 = 0.0f;
        wr = 0; sinceHop = 0; filled = 0; counter = 0;
        f0Hz = 0.0f; conf = 0.0f; hist[0] = hist[1] = hist[2] = 0.0f;
    }

    /** Duree entre deux estimations, en echantillons d'entree. */
    int hopInputSamples() const noexcept { return hop * decim; }

    /** Pousse un echantillon ; renvoie true quand une nouvelle estimation est disponible. */
    bool pushSample (float x) noexcept
    {
        x = lp[1].process (lp[0].process (x));
        if (++counter < decim) return false;
        counter = 0;
        ring[(size_t) wr] = x;
        wr = (wr + 1) & mask;
        ++filled;
        if (++sinceHop >= hop && filled >= need)
        {
            sinceHop = 0;
            analyse();
            return true;
        }
        return false;
    }

    float getF0() const noexcept         { return f0Hz; }          // 0 = non voise
    float getConfidence() const noexcept { return conf; }

private:
    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
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
        const double w0 = 2.0 * 3.14159265358979323846 * fc / fs, c = std::cos (w0), a = std::sin (w0) / (2.0 * q);
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

        // Porte d'energie (~ -54 dBFS)
        float e = 0.0f;
        for (int i = 0; i < win; ++i) e += lin[(size_t) i] * lin[(size_t) i];
        if (std::sqrt (e / (float) win) < 0.002f) { publish (0.0f, 0.0f); return; }

        // Fonction de difference + moyenne cumulee normalisee (CMNDF)
        cmnd[0] = 1.0f;
        float running = 0.0f;
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            float d = 0.0f;
            const float* a = lin.data();
            const float* b = lin.data() + tau;
            for (int j = 0; j < win; ++j) { const float t = a[j] - b[j]; d += t * t; }
            running += d;
            cmnd[(size_t) tau] = running > 1e-12f ? d * (float) tau / running : 1.0f;
        }

        int tau = -1;
        for (int t = tauMin; t <= tauMax; ++t)
            if (cmnd[(size_t) t] < 0.15f)
            {
                while (t + 1 <= tauMax && cmnd[(size_t) t + 1] < cmnd[(size_t) t]) ++t;
                tau = t; break;
            }
        if (tau < 0)   // repli : minimum global s'il est assez net
        {
            float best = 1.0f;
            for (int t = tauMin; t <= tauMax; ++t) if (cmnd[(size_t) t] < best) { best = cmnd[(size_t) t]; tau = t; }
            if (best > 0.25f) { publish (0.0f, 0.0f); return; }
        }

        // Interpolation parabolique
        float tauF = (float) tau;
        if (tau > 1 && tau < tauMax)
        {
            const float s0 = cmnd[(size_t) tau - 1], s1 = cmnd[(size_t) tau], s2 = cmnd[(size_t) tau + 1];
            const float den = s0 + s2 - 2.0f * s1;
            if (std::abs (den) > 1e-9f) tauF += juce_clamp (0.5f * (s0 - s2) / den, -1.0f, 1.0f);
        }
        const float f = (float) dsr / tauF;
        publish ((f >= minHz && f <= maxHz) ? f : 0.0f, 1.0f - cmnd[(size_t) tau]);
    }

    static float juce_clamp (float v, float lo, float hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    /** Median de 3 trames voisees : rejette les erreurs d'octave isolees. */
    void publish (float f, float c) noexcept
    {
        hist[0] = hist[1]; hist[1] = hist[2]; hist[2] = f;
        if (hist[0] > 0.0f && hist[1] > 0.0f && hist[2] > 0.0f)
        {
            float a = hist[0], b = hist[1], d = hist[2];
            f0Hz = std::max (std::min (a, b), std::min (std::max (a, b), d));
        }
        else f0Hz = f;
        conf = c;
    }

    int decim = 1, tauMin = 2, tauMax = 100, win = 400, hop = 160, need = 600, mask = 1023;
    int wr = 0, sinceHop = 0, filled = 0, counter = 0;
    double dsr = 16000.0;
    std::vector<float> ring, lin, cmnd;
    Biquad lp[2];
    float f0Hz = 0.0f, conf = 0.0f, hist[3] {};
};
