#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <juce_core/juce_core.h>

/** HUMPIRE TWIN - moteur d'auto-doublage.

    Principe (double track realiste, sans latence) :
    - la voix (somme mono, filtree HP/LP) est ecrite dans une ligne a retard partagee ;
    - chaque doublure est une "prise" qui lit cette ligne avec un retard de 5-50 ms ;
    - le micro-detune est obtenu en faisant glisser lentement le retard (d(retard)/dt = 1 - ratio) :
      la hauteur est decalee exactement de `ratio` ; quand la prise atteint le bord de sa fenetre de glissement
      (+/- 0,5..4 ms), une seconde prise repart de l'autre bord et on fond les deux (10-40 ms, smoothstep) ;
    - l'humanisation module lentement et aleatoirement (bruit lisse interpole) le retard, le detune et le niveau
      de chaque doublure, a la vitesse "Drift" ;
    - les doublures sont placees en stereo (loi a puissance constante) selon Width, normalisees en 1/sqrt(N).

    Les commandes sont calculees a cadence de controle (tous les 16 echantillons) avec rampes lineaires par
    echantillon : aucun zipper, aucune allocation hors prepare(), resultat independant de la taille des blocs. */
namespace twin
{
    //==========================================================================
    /** Filtre a variable d'etat TPT (Zavalishin), 2 poles, Q = 0,707 : sorties passe-bas et passe-haut. */
    struct Svf
    {
        float g = 0.0f, k = 1.41421356f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f, ic1 = 0.0f, ic2 = 0.0f;

        void setCutoff (float hz, float sampleRate) noexcept
        {
            const float fc = juce::jlimit (10.0f, 0.45f * sampleRate, hz);
            g  = std::tan (juce::MathConstants<float>::pi * fc / sampleRate);
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }
        void reset() noexcept { ic1 = ic2 = 0.0f; }

        inline void tick (float v0, float& lp, float& hp) noexcept
        {
            const float v3 = v0 - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            lp = v2;
            hp = v0 - k * v1 - v2;
        }
    };

    /** Generateur pseudo-aleatoire xorshift32 (deterministe, sans allocation). Sortie dans [-1, 1]. */
    struct Rng
    {
        std::uint32_t s = 0x9E3779B9u;
        float next() noexcept
        {
            s ^= s << 13; s ^= s >> 17; s ^= s << 5;
            return (float) (s >> 8) * (1.0f / 8388608.0f) - 1.0f;
        }
    };

    /** Bruit lisse : nouvelle cible aleatoire a chaque periode, interpolation smoothstep (derivee nulle aux noeuds). */
    struct SmoothRandom
    {
        float a = 0.0f, b = 0.0f, ph = 0.0f;
        void reset (Rng& r) noexcept { a = 0.0f; b = r.next(); ph = 0.0f; }
        float tick (float inc, Rng& r) noexcept
        {
            ph += inc;
            if (ph >= 1.0f)
            {
                ph -= std::floor (ph);
                a = b;
                b = r.next();
            }
            const float w = ph * ph * (3.0f - 2.0f * ph);
            return a + (b - a) * w;
        }
    };

    //==========================================================================
    class DoublerEngine
    {
    public:
        static constexpr int maxVoices = 4;
        static constexpr int tickLen = 16;

        struct Settings
        {
            int   voices = 2;
            float detuneCents = 12.0f, delayMs = 18.0f, spread = 0.5f, humanize = 0.4f,
                  driftHz = 0.35f, width = 0.8f, hpfHz = 120.0f, lpfHz = 14000.0f;
        };

        /** Lecture seule pour l'UI (copiee par le processeur dans des atomics). */
        struct VoiceView { float delayMs, cents, pan, env, peak; };

        void prepare (double sampleRate)
        {
            sr = sampleRate > 1000.0 ? sampleRate : 44100.0;
            int sz = 1;
            while (sz < (int) (0.12 * sr) + 8) sz <<= 1;          // > 120 ms : retard max ~ 60 ms
            buf.assign ((size_t) sz, 0.0f);
            mask = sz - 1;
            msToSamp   = sr * 0.001;
            minDelay   = std::max (2.0, 0.0005 * sr);
            maxDelay   = (double) sz - 4.0;
            jumpThresh = 0.003 * sr;
            slew       = 0.01;                                     // pente max de suivi du retard (~17 cents)
            const double tl = (double) tickLen;
            smoothCoef = (float) (1.0 - std::exp (-tl / (0.05 * sr)));
            envCoef    = (float) (1.0 - std::exp (-tl / (0.025 * sr)));
            panCoef    = (float) (1.0 - std::exp (-tl / (0.04 * sr)));
            reset();
        }

        void reset() noexcept
        {
            std::fill (buf.begin(), buf.end(), 0.0f);
            wpos = 0;
            tickPos = 0;
            hp.reset(); lp.reset();
            initialised = false;
            for (int k = 0; k < maxVoices; ++k)
            {
                auto& v = voices[(size_t) k];
                v = Voice();
                v.rng.s = 0x1234567u + 0x9E3779B9u * (std::uint32_t) (k + 1);
                v.rD.reset (v.rng); v.rP.reset (v.rng); v.rG.reset (v.rng);
            }
        }

        bool isPrepared() const noexcept { return ! buf.empty(); }

        void setSettings (const Settings& s) noexcept { target = s; }

        /** Traite un echantillon de source mono ; renvoie les doublures en stereo (L, R) et en mono (C). */
        inline void process (float src, float& outL, float& outR, float& outC) noexcept
        {
            if (tickPos == 0) controlTick();
            if (++tickPos >= tickLen) tickPos = 0;

            float lo, hi, lo2, hi2;
            hp.tick (src, lo, hi);           // passe-haut
            lp.tick (hi, lo2, hi2);          // puis passe-bas
            buf[(size_t) wpos] = lo2;

            float L = 0.0f, R = 0.0f, C = 0.0f;
            for (auto& v : voices)
            {
                if (! v.alive) continue;
                const float s = renderVoice (v);
                L += s * v.gL; R += s * v.gR; C += s * v.gC;
                v.peak = std::max (v.peak, std::abs (s * v.gC));
                v.gL += v.dL; v.gR += v.dR; v.gC += v.dC;
            }
            wpos = (wpos + 1) & mask;
            outL = L; outR = R; outC = C;
        }

        VoiceView getVoiceView (int k) const noexcept
        {
            const auto& v = voices[(size_t) juce::jlimit (0, maxVoices - 1, k)];
            const auto& t = v.tap[(size_t) v.act];
            return { v.alive ? (float) ((t.c + t.v) / msToSamp) : 0.0f, v.cents, v.pan, v.env, v.peak };
        }
        void clearPeaks() noexcept { for (auto& v : voices) v.peak = 0.0f; }

        // Constantes de placement (publiques pour l'UI)
        static constexpr float detunePattern[maxVoices] = { 1.0f, -1.0f, 0.6f, -0.6f };
        static constexpr float spreadDev[maxVoices]     = { 0.0f, -0.25f, 0.25f, -0.4f };
        static float panFor (int numVoices, int k) noexcept
        {
            static constexpr float table[maxVoices][maxVoices] = {
                { -0.6f,  0.0f,  0.0f,  0.0f  },    // 1 doublure : legerement a gauche (lead au centre)
                { -1.0f,  1.0f,  0.0f,  0.0f  },    // 2 : paire gauche / droite
                { -1.0f,  1.0f,  0.0f,  0.0f  },    // 3 : paire + centre
                { -1.0f,  1.0f, -0.45f, 0.45f },    // 4 : deux paires
            };
            return table[juce::jlimit (0, maxVoices - 1, numVoices - 1)][juce::jlimit (0, maxVoices - 1, k)];
        }

    private:
        struct Tap { double c = 0.0, v = 0.0; };    // retard = c (centre suivi) + v (glissement detune), en echantillons

        struct Voice
        {
            Tap tap[2];
            int act = 0;
            bool fading = false, alive = false;
            float xf = 0.0f, xfInc = 0.0f;
            double nominal = 0.0, halfSpan = 0.0, rate = 0.0;
            float env = 0.0f, pan = 0.0f, cents = 0.0f, peak = 0.0f;
            float gL = 0.0f, gR = 0.0f, gC = 0.0f, dL = 0.0f, dR = 0.0f, dC = 0.0f;
            SmoothRandom rD, rP, rG;
            Rng rng;
        };

        //======================================================================
        void controlTick() noexcept
        {
            const Settings& s = target;
            const float kS = initialised ? smoothCoef : 1.0f;
            const float kE = initialised ? envCoef : 1.0f;
            const float kP = initialised ? panCoef : 1.0f;
            const float fsr = (float) sr;

            detuneSm += (s.detuneCents - detuneSm) * kS;
            hpLog += (std::log (juce::jlimit (10.0f, 20000.0f, s.hpfHz)) - hpLog) * kS;
            lpLog += (std::log (juce::jlimit (10.0f, 24000.0f, s.lpfHz)) - lpLog) * kS;
            hp.setCutoff (std::exp (hpLog), fsr);
            lp.setCutoff (std::exp (lpLog), fsr);

            const int N = juce::jlimit (1, maxVoices, s.voices);
            float envSum = 0.0f;
            for (int k = 0; k < maxVoices; ++k)
            {
                auto& v = voices[(size_t) k];
                const bool on = k < N;
                v.env += ((on ? 1.0f : 0.0f) - v.env) * kE;
                if (! on && v.env < 1.0e-4f) { v.env = 0.0f; v.alive = false; }
                envSum += v.env;
            }
            const float norm = 1.0f / std::sqrt (std::max (1.0f, envSum));
            const float hum = juce::jlimit (0.0f, 1.0f, s.humanize);
            const float speedMul[maxVoices] = { 1.0f, 0.83f, 1.21f, 0.93f };

            for (int k = 0; k < maxVoices; ++k)
            {
                auto& v = voices[(size_t) k];
                const bool on = k < N;
                if (! on && ! v.alive)
                {
                    v.gL = v.gR = v.gC = v.dL = v.dR = v.dC = 0.0f;
                    continue;
                }

                // --- humanisation : trois bruits lisses independants ---
                const float inc = juce::jlimit (0.01f, 3.0f, s.driftHz) * speedMul[k] * (float) tickLen / fsr;
                const float hD = v.rD.tick (inc, v.rng);
                const float hP = v.rP.tick (inc * 0.71f, v.rng);
                const float hG = v.rG.tick (inc * 1.37f, v.rng);

                // --- retard nominal + fenetre de glissement ---
                const float baseMs  = juce::jlimit (1.5f, 50.0f, s.delayMs * (1.0f + juce::jlimit (0.0f, 1.0f, s.spread) * spreadDev[k]));
                const float depthMs = hum * std::min (2.5f, 0.3f * baseMs);
                v.nominal  = (double) (baseMs + depthMs * hD) * msToSamp;
                v.halfSpan = (double) juce::jlimit (0.5f, 4.0f, (baseMs - depthMs - 1.0f) / 1.3f) * msToSamp;

                // --- micro-detune : glissement du retard a la pente (1 - ratio) ---
                v.cents = detuneSm * detunePattern[k] + hum * 5.0f * hP;
                v.rate  = 1.0 - std::exp2 ((double) v.cents / 1200.0);

                if (! v.alive)
                {
                    // (re)naissance de la doublure : prise unique centree sur le retard nominal
                    v.tap[0].c = v.nominal; v.tap[0].v = 0.0;
                    v.act = 0; v.fading = false; v.alive = true;
                    v.gL = v.gR = v.gC = 0.0f;
                }
                else if (! v.fading && std::abs (v.nominal - v.tap[(size_t) v.act].c) > jumpThresh)
                {
                    // grand changement de Delay / Spread : saut en fondu plutot qu'un glissement (evite le "wow")
                    startFade (v, v.nominal, 0.0, 0.03 * sr);
                }

                // --- placement stereo (puissance constante) et gains en rampe ---
                if (on) v.pan += (panFor (N, k) * juce::jlimit (0.0f, 1.0f, s.width) - v.pan) * kP;
                const float theta = (v.pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
                const float gain = v.env * (1.0f + 0.11f * hum * hG) * norm;
                const float tL = std::cos (theta) * gain, tR = std::sin (theta) * gain, tC = 0.70710678f * gain;
                if (initialised)
                {
                    const float inv = 1.0f / (float) tickLen;
                    v.dL = (tL - v.gL) * inv; v.dR = (tR - v.gR) * inv; v.dC = (tC - v.gC) * inv;
                }
                else
                {
                    v.gL = tL; v.gR = tR; v.gC = tC; v.dL = v.dR = v.dC = 0.0f;
                }
            }
            initialised = true;
        }

        static void startFade (Voice& v, double c, double nv, double lenSamples) noexcept
        {
            v.act ^= 1;
            v.tap[(size_t) v.act].c = c;
            v.tap[(size_t) v.act].v = nv;
            v.fading = true;
            v.xf = 0.0f;
            v.xfInc = (float) (1.0 / std::max (1.0, lenSamples));
        }

        inline double clampDelay (double d) const noexcept { return d < minDelay ? minDelay : (d > maxDelay ? maxDelay : d); }

        inline float renderVoice (Voice& v) noexcept
        {
            Tap& a = v.tap[(size_t) v.act];
            a.v += v.rate;
            a.c += juce::jlimit (-slew, slew, v.nominal - a.c);
            float s = read (clampDelay (a.c + a.v));

            if (v.fading)
            {
                Tap& b = v.tap[(size_t) (v.act ^ 1)];
                b.v += v.rate;
                v.xf += v.xfInc;
                if (v.xf >= 1.0f)
                    v.fading = false;
                else
                {
                    const float w = v.xf * v.xf * (3.0f - 2.0f * v.xf);
                    s = w * s + (1.0f - w) * read (clampDelay (b.c + b.v));
                }
            }
            else if (a.v > v.halfSpan || a.v < -v.halfSpan)
            {
                // bord de fenetre atteint : nouvelle prise a l'autre bord, fondu croise
                const double h = v.halfSpan;
                const double nv = a.v > h ? (v.rate > 0.0 ? -h : h) : (v.rate < 0.0 ? h : -h);
                const double len = juce::jlimit (0.01 * sr, 0.04 * sr, 0.9 * h / std::max (std::abs (v.rate), 1.0e-9));
                startFade (v, a.c, nv, len);
            }
            return s;
        }

        /** Lecture Hermite cubique a `delay` echantillons (>= 2) derriere l'ecriture courante. */
        inline float read (double delaySamples) const noexcept
        {
            const double rp = (double) wpos - delaySamples;
            const double fl = std::floor (rp);
            const int i0 = (int) fl;
            const float f = (float) (rp - fl);
            const float y0 = buf[(size_t) ((i0 - 1) & mask)], y1 = buf[(size_t) (i0 & mask)],
                        y2 = buf[(size_t) ((i0 + 1) & mask)], y3 = buf[(size_t) ((i0 + 2) & mask)];
            const float c1 = 0.5f * (y2 - y0);
            const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            return ((c3 * f + c2) * f + c1) * f + y1;
        }

        std::vector<float> buf;
        std::array<Voice, maxVoices> voices {};
        Settings target;
        Svf hp, lp;
        double sr = 44100.0, msToSamp = 44.1, minDelay = 22.0, maxDelay = 1000.0, jumpThresh = 132.0, slew = 0.01;
        float smoothCoef = 0.01f, envCoef = 0.01f, panCoef = 0.01f;
        float detuneSm = 0.0f, hpLog = 4.787f, lpLog = 9.547f;
        int mask = 0, wpos = 0, tickPos = 0;
        bool initialised = false;
    };
}
