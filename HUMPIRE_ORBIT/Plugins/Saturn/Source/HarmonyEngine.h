#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "SaturnParams.h"
#include "PitchDetector.h"
#include "HarmonyShifter.h"

namespace saturn
{
    /** Settings of one harmony voice, read once per block on the audio thread. */
    struct VoiceSettings
    {
        bool  on = false;
        int   semi = 0, deg = 0;        // interval: semitones (semitone mode) or scale steps (degree mode)
        float gain = 0.0f;              // linear level
        float pan = 0.0f;               // -1 (left) .. +1 (right)
        float detuneCents = 0.0f;
    };

    /** Global settings of the harmony engine. */
    struct EngineSettings
    {
        bool  degreeMode = true;
        int   key = 0;
        int   scale = Major;
        float glideMs = 30.0f;
        std::array<VoiceSettings, kNumVoices> voice {};
    };

    /** HUMPIRE SATURN harmony engine: four pitch-shifted voices built from the input pitch.

        Chain (per sample, audio thread):
          input -> shared delay line (L, R, mono)
                -> YIN pitch detection (every 10 ms): input note, scale degree (with 15 cent hysteresis),
                   pitch-synchronous grain length
                -> interval of each voice (scale steps or semitones, plus detune), glided in semitones
                -> two-tap shifter per voice -> level gate -> constant-power pan -> harmony bus
          dry output = input delayed by the announced latency (exact integer delay).

        Unvoiced passages keep the last intervals; the voices fade out after about 150 ms of silence. */
    class HarmonyEngine
    {
    public:
        void prepare (double sampleRate)
        {
            sr = sampleRate;
            detector.prepare (sr);
            window.prepare (sr);
            latency = window.latencySamples();
            line.prepare ((int) std::ceil (0.1 * sr));     // far more than latency + half a grain
            homeStep = 1.0 / (0.1 * sr);
            levelCoef = coefFor (0.02);
            gateCoef = coefFor (0.025);
            reset();
        }

        void reset() noexcept
        {
            detector.reset();
            line.reset();
            haveIdx = false;
            lastIdx = 0;
            voiced = false;
            detMidi = 0.0f;
            unvoicedFrames = 1000;
            gateTarget = 0.0f;
            gate = 0.0f;
            computeTargets();
            for (auto& s : state)
            {
                s.tap.reset (0.0);
                s.shift = s.targetShift;
                s.lvl = s.targetLvl;
                s.gl = s.tgl;
                s.gr = s.tgr;
                s.peak = 0.0f;
            }
        }

        bool isPrepared() const noexcept { return sr > 0.0; }
        int  latencySamples() const noexcept { return latency; }

        /** New settings (read once per block). */
        void setSettings (const EngineSettings& s) noexcept
        {
            if (s.key != settings.key || s.scale != settings.scale) haveIdx = false;   // degree index changes meaning
            settings = s;
            glideCoef = s.glideMs <= 0.01f ? 1.0 : coefFor (0.001 * (double) s.glideMs);
            computeTargets();
        }

        /** One sample. dry = input delayed by latency (stereo). wet = harmony bus (stereo). */
        void process (float inL, float inR, float& dryL, float& dryR, float& wetL, float& wetR) noexcept
        {
            const float mono = 0.5f * (inL + inR);
            const float frame[SharedDelayLine::channels] = { inL, inR, mono };
            line.write (frame);

            if (detector.pushSample (mono))
                onPitchFrame (detector.getF0());

            window.tick();
            const double W = window.length();
            const double base = (double) latency - 0.5 * W;   // read delay of tap A at phase 0

            dryL = line.read (0, (double) latency);
            dryR = line.read (1, (double) latency);

            gate += (gateTarget - gate) * gateCoef;
            float wl = 0.0f, wr = 0.0f;
            for (auto& s : state)
            {
                s.shift += (s.targetShift - s.shift) * glideCoef;
                s.lvl += (s.targetLvl - s.lvl) * levelCoef;
                s.gl += (s.tgl - s.gl) * levelCoef;
                s.gr += (s.tgr - s.gr) * levelCoef;

                const double ratio = std::exp2 (s.shift / 12.0);
                s.tap.step (1.0 - ratio, W, homeStep);

                const double dA = base + s.tap.phase * W;
                const double dB = base + s.tap.pB * W;
                const float y = s.tap.gA * line.read (2, dA) + s.tap.gB * line.read (2, dB);
                const float amp = y * s.lvl * gate;

                wl += amp * s.gl;
                wr += amp * s.gr;
                s.peak = std::max (s.peak, std::abs (amp));
            }
            wetL = wl;
            wetR = wr;
            line.advance();
        }

        //======================================================================
        // Read-out for the UI (called on the audio thread, copied into atomics by the processor)
        bool  isVoiced() const noexcept                { return voiced; }
        float getDetectedMidi() const noexcept         { return detMidi; }
        /** Tracked scale note (MIDI-like, may be a quarter tone), -1 if none yet. */
        float getTrackedMidi() const noexcept
        {
            if (! haveIdx) return -1.0f;
            return degreeToMidi (settings.key, buildDegrees (settings.scale), lastIdx);
        }
        float getVoiceShift (int v) const noexcept     { return (float) state[(size_t) v].shift; }
        float takeVoicePeak (int v) noexcept           { auto& s = state[(size_t) v]; const float p = s.peak; s.peak = 0.0f; return p; }

    private:
        double coefFor (double seconds) const noexcept { return 1.0 - std::exp (-1.0 / (seconds * sr)); }

        void onPitchFrame (float f0) noexcept
        {
            if (f0 > 0.0f)
            {
                window.setPeriod (sr / (double) f0);
                detMidi = 69.0f + 12.0f * std::log2 (f0 / 440.0f);
                voiced = true;
                unvoicedFrames = 0;
                gateTarget = 1.0f;
                trackNote (detMidi);
            }
            else
            {
                if (unvoicedFrames < 1000) ++unvoicedFrames;
                if (unvoicedFrames > 15)        // about 150 ms without pitch: close the voices
                {
                    voiced = false;
                    gateTarget = 0.0f;
                }
            }
            computeTargets();
        }

        /** Scale degree closest to the detected pitch, with 15 cents of hysteresis. */
        void trackNote (float midi) noexcept
        {
            const Degrees deg = buildDegrees (settings.scale);
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

        /** Target interval, level and pan of every voice (control rate). */
        void computeTargets() noexcept
        {
            const Degrees deg = buildDegrees (settings.scale);
            const int idx = haveIdx ? lastIdx : 0;                  // without pitch: relative to the tonic
            const float base = degreeToMidi (settings.key, deg, idx);

            for (int v = 0; v < kNumVoices; ++v)
            {
                const VoiceSettings& vs = settings.voice[(size_t) v];
                VoiceState& s = state[(size_t) v];

                double shift = settings.degreeMode
                    ? (double) (degreeToMidi (settings.key, deg, idx + vs.deg) - base)
                    : (double) vs.semi;
                shift += 0.01 * (double) vs.detuneCents;
                s.targetShift = std::max ((double) -kMaxShiftSemitones, std::min ((double) kMaxShiftSemitones, shift));

                s.targetLvl = vs.on ? vs.gain : 0.0f;
                const float theta = (std::max (-1.0f, std::min (1.0f, vs.pan)) + 1.0f) * (float) (0.25 * kPi);
                s.tgl = std::cos (theta);
                s.tgr = std::sin (theta);
            }
        }

        struct VoiceState
        {
            TwoTap tap;
            double shift = 0.0, targetShift = 0.0;          // semitones
            float lvl = 0.0f, targetLvl = 0.0f;             // linear level
            float gl = 0.0f, gr = 0.0f, tgl = 0.0f, tgr = 0.0f;
            float peak = 0.0f;
        };

        double sr = 0.0;
        int latency = 0;
        PitchDetector detector;
        GrainWindow window;
        SharedDelayLine line;

        EngineSettings settings;
        std::array<VoiceState, kNumVoices> state {};
        double glideCoef = 1.0, homeStep = 0.0;
        float levelCoef = 0.0f, gateCoef = 0.0f;
        float gate = 0.0f, gateTarget = 0.0f;

        bool haveIdx = false, voiced = false;
        int lastIdx = 0, unvoicedFrames = 1000;
        float detMidi = 0.0f;
    };
}
