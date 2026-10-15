#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <algorithm>
#include "SaturnParams.h"
#include "PitchDetector.h"
#include "HarmonyShifter.h"
#include "FormantLPC.h"

/** HUMPIRE SATURN - moteur d'harmonies 4 voix (thread audio, traitement echantillon par echantillon).

    Chaine : entree -> somme mono -> [YIN : f0 -> note de gamme suivie (hysteresis)] -> intervalle de chaque voix
             (demi-tons fixes ou degres de la gamme, quarts de ton des maqams inclus) -> glide -> ratio
             -> deux prises sur ligne partagee (signal direct, ou residu LPC + re-synthese si Formant)
             -> niveau / pan a puissance constante -> bus harmonies stereo.
    Le sec est retarde exactement de la latence annoncee (aligne sur le retard moyen des prises). */
namespace saturn
{
    struct VoiceSettings
    {
        bool  on = false;
        int   semi = 0, deg = 0;
        float gain = 0.0f;           // lineaire
        float pan = 0.0f;            // -1..1 (deja multiplie par la largeur)
        float detuneCents = 0.0f;
        float delayMs = 0.0f;
    };

    struct EngineSettings
    {
        bool  degreeMode = true;
        int   key = 0, scale = Major;
        bool  formant = true;
        float glideMs = 25.0f, correction = 0.0f, humanize = 0.2f;
        std::array<VoiceSettings, kNumVoices> voice {};
    };

    class HarmonyEngine
    {
    public:
        static constexpr int maxChannels = 2;
        static constexpr double maxVoiceDelaySec = 0.065;

        void prepare (double sampleRate)
        {
            sr = sampleRate;
            detector.prepare (sr);
            lpc.prepare (sr);
            window.prepare (sr);
            latency = window.latencySamples();

            // ligne partagee : latence + demi-fenetre + delai de voix max + marge
            ring.prepare ((int) (2.0 * latency + maxVoiceDelaySec * sr) + 16);

            int dsz = 1; while (dsz < latency + 4) dsz <<= 1;
            dryMask = dsz - 1;
            for (auto& d : dryRing) d.assign ((size_t) dsz, 0.0f);

            gainCoef  = (float) coefFor (0.015);
            delayCoef = coefFor (0.060);
            fmCoef    = (float) coefFor (0.030);
            ctrlInc   = 32.0 / sr;
            static const double rates1[kNumVoices] = { 0.13, 0.19, 0.23, 0.29 };
            static const double rates2[kNumVoices] = { 0.71, 0.53, 0.83, 0.61 };
            for (int v = 0; v < kNumVoices; ++v) { lfoRate1[(size_t) v] = rates1[v]; lfoRate2[(size_t) v] = rates2[v]; }
            prepared = true;
            reset();
        }

        void reset() noexcept
        {
            detector.reset();
            lpc.reset();
            window.reset();
            ring.reset();
            for (auto& d : dryRing) std::fill (d.begin(), d.end(), 0.0f);
            dryWr = 0;
            haveIdx = false; lastIdx = 0; detMidi = 0.0f; voiced = false; unvoicedFrames = 0;
            lastKey = -1; lastScale = -1;
            fm = settings.formant ? 1.0f : 0.0f;
            ctrlCounter = 0;
            for (int v = 0; v < kNumVoices; ++v)
            {
                auto& s = state[(size_t) v];
                s.tap.reset(); s.synth.reset(); s.synthActive = false;
                s.lfo1 = 1.7 * (double) v; s.lfo2 = 0.9 * (double) v; s.drift = 0.0f;
                s.peak = 0.0f;
            }
            updateTargets();
            for (auto& s : state)
            {
                s.shift = s.targetShift; s.delay = s.targetDelay;
                s.gl = s.tgl; s.gr = s.tgr; s.gm = s.tgm;
            }
        }

        bool isPrepared() const noexcept { return prepared; }
        int  latencySamples() const noexcept { return latency; }

        /** Nouveaux reglages (lus par bloc) : recalcule les cibles des voix. */
        void setSettings (const EngineSettings& s) noexcept
        {
            settings = s;
            updateTargets();
        }

        /** Un echantillon : in[nCh] -> dry[nCh] (aligne), wet[nCh] (bus harmonies). */
        void process (const float* in, float* dry, float* wet, int nCh) noexcept
        {
            nCh = std::max (1, std::min (maxChannels, nCh));
            const float mono = nCh > 1 ? 0.5f * (in[0] + in[1]) : in[0];

            // --- sec aligne sur la latence ---
            for (int c = 0; c < nCh; ++c)
            {
                auto& d = dryRing[(size_t) c];
                d[(size_t) dryWr] = in[c];
                dry[c] = d[(size_t) ((dryWr - latency) & dryMask)];
            }
            dryWr = (dryWr + 1) & dryMask;

            // --- analyse de hauteur (~100 Hz) ---
            if (detector.pushSample (mono)) onPitchFrame (detector.getF0());

            // --- residu LPC + ecriture dans la ligne partagee ---
            const float res = lpc.analyse (mono);
            ring.write (mono, res);
            window.tick();
            const double W = window.get();
            const double base = (double) latency - 0.5 * W;      // retard moyen des prises = latence
            const double maxD = ring.maxDelay();

            fm += ((settings.formant ? 1.0f : 0.0f) - fm) * fmCoef;
            if (fm < 1e-4f) fm = 0.0f; else if (fm > 0.9999f) fm = 1.0f;

            // --- humanisation (derive lente de hauteur, cadence de controle) ---
            if (++ctrlCounter >= 32)
            {
                ctrlCounter = 0;
                const double twoPi = 6.283185307179586476925;
                for (int v = 0; v < kNumVoices; ++v)
                {
                    auto& s = state[(size_t) v];
                    s.lfo1 += twoPi * lfoRate1[(size_t) v] * ctrlInc; if (s.lfo1 > twoPi) s.lfo1 -= twoPi;
                    s.lfo2 += twoPi * lfoRate2[(size_t) v] * ctrlInc; if (s.lfo2 > twoPi) s.lfo2 -= twoPi;
                    s.drift = humanizeSemis * (float) (0.7 * std::sin (s.lfo1) + 0.3 * std::sin (s.lfo2));
                }
            }

            for (int c = 0; c < nCh; ++c) wet[c] = 0.0f;
            for (int v = 0; v < kNumVoices; ++v)
            {
                auto& s = state[(size_t) v];
                s.gl += (s.tgl - s.gl) * gainCoef;
                s.gr += (s.tgr - s.gr) * gainCoef;
                s.gm += (s.tgm - s.gm) * gainCoef;
                if (s.tgm <= 0.0f && s.gm < 1e-6f)
                {
                    // voix muette : pas de calcul, on recale l'etat pour un depart propre
                    s.gl = s.gr = s.gm = 0.0f;
                    s.shift = s.targetShift; s.delay = s.targetDelay;
                    continue;
                }

                s.shift += (s.targetShift - s.shift) * glideCoef;
                s.delay += (s.targetDelay - s.delay) * delayCoef;
                const double ratio = std::exp2 ((double) (s.shift + s.drift) / 12.0);
                s.tap.step (ratio, W);

                const double dA = std::min (maxD, s.tap.phase * W + base + s.delay);
                const double dB = std::min (maxD, s.tap.pB * W + base + s.delay);
                float y = 0.0f;
                if (fm < 1.0f)
                    y += (1.0f - fm) * (s.tap.gA * ring.readX (dA) + s.tap.gB * ring.readX (dB));
                if (fm > 0.0f)
                {
                    const float e = s.tap.gA * ring.readE (dA) + s.tap.gB * ring.readE (dB);
                    y += fm * lpc.synthesise (s.synth, e);
                    s.synthActive = true;
                }
                else if (s.synthActive)
                {
                    s.synth.reset();             // repartira d'un etat propre au prochain passage en Formant
                    s.synthActive = false;
                }

                if (nCh > 1) { wet[0] += y * s.gl; wet[1] += y * s.gr; }
                else           wet[0] += y * s.gm;
                s.peak = std::max (s.peak, std::abs (y) * s.gm);
            }
            ring.advance();
        }

        //======================================================================
        // Lecture pour l'interface (valeurs du thread audio, copiees dans des atomiques par le processeur)
        bool  isVoiced() const noexcept              { return voiced; }
        float getDetectedMidi() const noexcept       { return detMidi; }
        float getVoiceShift (int v) const noexcept   { return state[(size_t) v].shift; }
        float takeVoicePeak (int v) noexcept         { const float p = state[(size_t) v].peak; state[(size_t) v].peak = 0.0f; return p; }
        /** Note de gamme suivie (MIDI), -1 si aucune. */
        float getTrackedNote() const noexcept
        {
            if (! haveIdx) return -1.0f;
            const auto deg = buildDegrees (settings.scale);
            return degreeToMidi (settings.key, deg, lastIdx);
        }

    private:
        double coefFor (double seconds) const noexcept { return 1.0 - std::exp (-1.0 / (seconds * sr)); }

        void onPitchFrame (float f0) noexcept
        {
            if (f0 > 0.0f)
            {
                window.setPeriod (sr / (double) f0);
                detMidi = 69.0f + 12.0f * std::log2 (f0 / 440.0f);
                voiced = true; unvoicedFrames = 0;
                trackNote (detMidi);
            }
            else if (++unvoicedFrames > 6)
                voiced = false;                      // les harmonies gardent leur dernier intervalle (consonnes)
            updateTargets();
        }

        /** Degre de gamme le plus proche, avec hysteresis de 15 cents (pas de flottement entre deux degres). */
        void trackNote (float midi) noexcept
        {
            if (settings.key != lastKey || settings.scale != lastScale) { haveIdx = false; lastKey = settings.key; lastScale = settings.scale; }
            const auto deg = buildDegrees (settings.scale);
            int idx = nearestDegree (midi, settings.key, deg);
            if (haveIdx && std::abs (idx - lastIdx) == 1)
            {
                const float dLast = std::abs (midi - degreeToMidi (settings.key, deg, lastIdx));
                const float dNew  = std::abs (midi - degreeToMidi (settings.key, deg, idx));
                if (dLast <= dNew + 0.15f) idx = lastIdx;
            }
            lastIdx = idx;
            haveIdx = true;
        }

        void updateTargets() noexcept
        {
            if (settings.key != lastKey || settings.scale != lastScale) { haveIdx = false; lastKey = settings.key; lastScale = settings.scale; }
            const auto deg = buildDegrees (settings.scale);
            const int baseIdx = haveIdx ? lastIdx : 5 * deg.n;          // sans detection : relatif a la tonique
            const float inNote = degreeToMidi (settings.key, deg, baseIdx);
            const float corr = haveIdx ? settings.correction * (inNote - detMidi) : 0.0f;

            for (int v = 0; v < kNumVoices; ++v)
            {
                const auto& vs = settings.voice[(size_t) v];
                auto& s = state[(size_t) v];
                float shift = settings.degreeMode
                            ? degreeToMidi (settings.key, deg, baseIdx + vs.deg) - inNote + corr
                            : (float) vs.semi;
                shift += vs.detuneCents * 0.01f;
                s.targetShift = std::max (-25.0f, std::min (25.0f, shift));
                s.targetDelay = std::max (0.0, std::min (maxVoiceDelaySec, (double) vs.delayMs * 0.001)) * sr;

                const float g = vs.on ? std::max (0.0f, vs.gain) : 0.0f;
                const float theta = (std::max (-1.0f, std::min (1.0f, vs.pan)) + 1.0f) * 0.78539816339744830962f;
                s.tgl = g * std::cos (theta);
                s.tgr = g * std::sin (theta);
                s.tgm = g;
            }
            const double tau = std::max (0.002, (double) settings.glideMs * 0.001 / 3.0);
            glideCoef = (float) coefFor (tau);
            humanizeSemis = std::max (0.0f, std::min (1.0f, settings.humanize)) * 0.12f;   // +/- 12 cents max
        }

        struct VoiceState
        {
            TwoTapState tap;
            FormantLPC::SynthState synth;
            float targetShift = 0.0f, shift = 0.0f, drift = 0.0f;
            double targetDelay = 0.0, delay = 0.0;
            float tgl = 0.0f, tgr = 0.0f, tgm = 0.0f, gl = 0.0f, gr = 0.0f, gm = 0.0f;
            double lfo1 = 0.0, lfo2 = 0.0;
            float peak = 0.0f;
            bool synthActive = false;
        };

        double sr = 48000.0;
        bool prepared = false;
        int latency = 0;
        PitchDetector detector;
        FormantLPC lpc;
        WindowControl window;
        SharedDelayRing ring;
        std::array<std::vector<float>, maxChannels> dryRing;
        int dryMask = 0, dryWr = 0;

        EngineSettings settings;
        std::array<VoiceState, kNumVoices> state {};
        std::array<double, kNumVoices> lfoRate1 {}, lfoRate2 {};
        double delayCoef = 0.001, ctrlInc = 0.0007;
        float gainCoef = 0.001f, fmCoef = 0.001f;
        float glideCoef = 0.01f, humanizeSemis = 0.0f, fm = 1.0f;
        int ctrlCounter = 0;

        bool haveIdx = false, voiced = false;
        int lastIdx = 0, unvoicedFrames = 0, lastKey = -1, lastScale = -1;
        float detMidi = 0.0f;
    };
}
