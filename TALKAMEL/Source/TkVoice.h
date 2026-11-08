#pragma once
#include "Params.h"
#include "NeonFX.h"
#include "BinaryData.h"
#include "SampleSet.h"
#include <cmath>

namespace Tk
{
struct GlideState
{
    std::atomic<float> lastFreq { 0.f };
    std::atomic<float> lastMod { 0.f };   // latest mod wheel (CC1) value 0..1, kept even when no voice is playing
    hfx::LiveTap* live = nullptr;          // what the interface shows (envelopes, LFO, cutoff of the loudest voice)
};

struct SVF
{
    float g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
    struct Out { float lp, bp, hp; };
    void set (float fc, float q, double sr)
    {
        g = std::tan (juce::MathConstants<float>::pi * juce::jlimit (10.f, (float) sr * 0.45f, fc) / (float) sr);
        k = 1.f / juce::jmax (0.1f, q);
        a1 = 1.f / (1.f + g * (g + k)); a2 = g * a1; a3 = g * a2;
    }
    Out process (float v0)
    {
        const float v3 = v0 - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2;
        return { v2, v1, v0 - k * v1 - v2 };
    }
    void reset() { ic1 = ic2 = 0; }
};

class TkSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

// The real instruments of TALKAMEL: recorded notes (see Assets/samples/CREDITS.txt) played by some engines instead of
// synthesis. Decoded once, on the message thread (processor constructor).
struct RealInstruments
{
    static const RealInstruments& get() { static const RealInstruments r; return r; }
    const hfx::SampleSet* forEngine (int) const { return nullptr; }   // every TALKAMEL engine is a synthesised voice
};

class TkVoice : public juce::SynthesiserVoice
{
public:
    TkVoice (const ParamPtrs& pp, GlideState& gs) : P (pp), glideState (gs) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<TkSound*> (s) != nullptr; }

    void setCurrentPlaybackSampleRate (double sr) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (sr);
        ksBuf.assign ((size_t) (sr / 12.0) + 8, 0.f);
        adsr.setSampleRate (sr); fadsr.setSampleRate (sr);
    }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override
    {
        targetFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
        const float last = glideState.lastFreq.load();
        curFreq = (P (glide) > 0.001f && last > 0.f) ? last : targetFreq;
        glideState.lastFreq = targetFreq;
        vel = velocity;
        midiBend = (pitchWheel - 8192) / 8192.f;
        midiMod = glideState.lastMod.load();   // idle voices never received CC1: start from the latest wheel position
        age = 0;
        for (auto& p : ph) p = rng.nextDouble();
        std::fill (ksBuf.begin(), ksBuf.end(), 0.f); fmFb = 0.f; holdV = 0.f; holdC = 0.f;
        ksW = 0; ksPrev = 0.f;
        for (auto* s : { &breathBP, &sym1, &sym2, &f1, &f2, &fl, &voiceFilter }) s->reset();
        pan = (rng.nextFloat() - 0.5f) * 2.f;
        lfoPh = 0.0; shValue = rng.nextFloat() * 2.f - 1.f;
        freshNote = true; released = false; relSec = 0.f; robotFrom = 4; robotTo = 0; dcX = dcY = 0.f; dcR = 1.f - juce::MathConstants<float>::twoPi * 12.f / (float) getSampleRate();
        real.note = nullptr; realEngine = -1; noteKey = midiNote;   // the recording is picked when the note first sounds
        fadsr.noteOn(); adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) { adsr.noteOff(); fadsr.noteOff(); released = true; }
        else { adsr.reset(); fadsr.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int v) override { midiBend = (v - 8192) / 8192.f; }
    void controllerMoved (int cc, int v) override { if (cc == 1) midiMod = v / 127.f; else if (cc == 121) midiMod = 0.f; }
    void aftertouchChanged (int) override {}
    void channelPressureChanged (int) override {}

    using juce::SynthesiserVoice::renderNextBlock;
    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (! adsr.isActive()) { clearCurrentNote(); return; }
        const double sr = getSampleRate();
        eng = juce::jlimit (0, 7, (int) P (wave));
        const float velSens = P (velocity);
        const int src = (int) P (lfosrc), dst = (int) P (dest), ft = (int) P (ftype);
        const float semis = P (pitch), gl = P (glide);
        const float cutoff = P (filter);
        const float lfoRate = P (rate), amt = juce::jlimit (0.f, 1.f, P (amount) + 0.8f * juce::jmax (midiMod, P (modw)));
        const float uiBend = P (bend), masterVol = P (volume);
        const float tb[5] = { P (t1), P (t2), P (t3), P (t4), P (t5) };
        adsr.setParameters ({ P (atk), P (dec), P (sus), P (rel) });
        const float fdepth = P (fenv), resoQ = 0.7f + P (reso) * 9.f;
        fadsr.setParameters ({ P (fatk), P (fdec), P (fsus), P (frel) });
        if (freshNote) { smVol = masterVol; smSemi = semis; smBend = uiBend; smCut = cutoff; freshNote = false; }
        const float smK = 1.f - std::exp (-1.f / (0.006f * (float) sr));
        const float glideCoef = gl < 0.001f ? 1.f : 1.f - std::exp (-1.f / (gl * (float) sr * 0.35f));
        float* L = out.getWritePointer (0);
        float* R = out.getNumChannels() > 1 ? out.getWritePointer (1) : nullptr;
        const float gL = std::sqrt (0.5f * (1.f - pan * 0.35f)), gR = std::sqrt (0.5f * (1.f + pan * 0.35f));

        for (int i = 0; i < num; ++i)
        {
            curFreq += (targetFreq - curFreq) * glideCoef;
            smVol += (masterVol - smVol) * smK; smSemi += (semis - smSemi) * smK;
            smBend += (uiBend - smBend) * smK; smCut += (cutoff - smCut) * smK;

            lfoPh += lfoRate / sr;
            if (lfoPh >= 1.0) { lfoPh -= 1.0; shValue = rng.nextFloat() * 2.f - 1.f; }
            float lfo = 0.f; const float lp = (float) lfoPh;
            switch (src) {
                case 0: lfo = std::sin (juce::MathConstants<float>::twoPi * lp); break;
                case 1: lfo = 4.f * std::abs (lp - 0.5f) - 1.f; break;
                case 2: lfo = 2.f * lp - 1.f; break;
                case 3: lfo = lp < 0.5f ? 1.f : -1.f; break;
                default: lfo = shValue; break;
            }
            const float mod = lfo * amt;

            float semiTotal = smSemi + (midiBend + smBend) * 2.f;
            if (dst == 0) semiTotal += mod * 1.0f;
            const double freq = (double) curFreq * std::exp2 (semiTotal / 12.f);

            float t[5];
            for (int k = 0; k < 5; ++k) t[k] = juce::jlimit (0.f, 1.f, tb[k] + (dst == 3 ? mod * 0.5f : 0.f));

            static constexpr float trim[8] = { 1.0f, 1.3f, 1.2f, 0.33f, 1.0f, 1.0f, 1.7f, 0.85f };   // equilibrage des niveaux entre moteurs
            float y;
            if (const auto* set = RealInstruments::get().forEngine (eng))
                y = playReal (*set, freq, t, (float) sr);
            else
                y = renderEngine (freq, t, (float) sr) * trim[eng];
            { const float dcOut = y - dcX + dcR * dcY; dcX = y; dcY = dcOut; y = dcOut; } // bloqueur DC

            if ((i & 7) == 0)
            {
                float fc = smCut * std::exp2 ((dst == 1 ? mod * 3.f : 0.f) + (float) std::log2 (freq / 261.6) * 0.25f + fdepth * fenvLevel * 5.f);
                voiceFilter.set (fc, resoQ, sr);
                lastFc = fc;
            }
            auto fo = voiceFilter.process (y);
            y = ft == 0 ? fo.lp : ft == 1 ? fo.hp : fo.bp * 1.5f;

            const float env = adsr.getNextSample();
            lastEnv = env; lastLfo = lfo;
            fenvLevel = fadsr.getNextSample();
            float amp = env * ((1.f - velSens) + velSens * vel) * smVol * 0.5f;
            if (dst == 2) amp *= 1.f - 0.5f * amt * (0.5f + 0.5f * lfo);
            y *= amp;

            L[start + i] += y * gL;
            if (R) R[start + i] += y * gR;
            if (age < 0x3fffffff) ++age;   // saturate: no signed overflow on very long held notes
            if (! adsr.isActive()) { clearCurrentNote(); break; }
        }
        if (auto* t = glideState.live; t != nullptr && isVoiceActive())
        {
            const float secs = (float) age / (float) sr;
            t->voice (lastEnv, hfx::envStage (released, secs, P (atk), P (dec)), fenvLevel, hfx::envStage (released, secs, P (fatk), P (fdec)),
                      lastLfo, (float) lfoPh, lastFc);
        }
    }

private:
    static float polyBlep (double t, double dt)
    {
        if (t < dt) { t /= dt; return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.f;
    }
    float saw (double phase, double dt) const { return (float) (2.0 * phase - 1.0) - polyBlep (phase, dt); }
    static void wrap (double& p) { p -= std::floor (p); }

    static float sawBL (double ph, double dt) { return (float) (2.0 * ph - 1.0) - polyBlep (ph, dt); }

    static float pulse (double phase, double dt, double pw)
    {
        double p2 = phase + pw; p2 -= std::floor (p2);
        const float a = (float) (2.0 * phase - 1.0) - polyBlep (phase, dt), b2 = (float) (2.0 * p2 - 1.0) - polyBlep (p2, dt);
        return a - b2;
    }
    static void vowel (float v, float& F1, float& F2, float& F3)
    {
        static const float T[4][3] = { { 800.f, 1200.f, 2600.f }, { 500.f, 900.f, 2500.f }, { 320.f, 800.f, 2400.f }, { 300.f, 2300.f, 3000.f } };
        const float x = juce::jlimit (0.f, 0.9999f, v) * 3.f; const int i = (int) x; const float fr = x - (float) i;
        F1 = T[i][0] + (T[i + 1][0] - T[i][0]) * fr; F2 = T[i][1] + (T[i + 1][1] - T[i][1]) * fr; F3 = T[i][2] + (T[i + 1][2] - T[i][2]) * fr;
    }

    // the five vowels of a (male) mouth, formants F1 F2 F3 in Hz: A E I O U
    static void vowelAt (int v, float& F1, float& F2, float& F3)
    {
        static const float T[5][3] = { { 730.f, 1090.f, 2440.f }, { 530.f, 1840.f, 2480.f }, { 290.f, 2250.f, 3010.f }, { 570.f, 840.f, 2410.f }, { 320.f, 870.f, 2240.f } };
        F1 = T[v][0]; F2 = T[v][1]; F3 = T[v][2];
    }
    // the mouth moving from vowel a to vowel b (x = 0..1)
    static void vowelGlide (int a, int b, float x, float& F1, float& F2, float& F3)
    {
        float a1, a2, a3, b1, b2, b3; vowelAt (a, a1, a2, a3); vowelAt (b, b1, b2, b3);
        x = juce::jlimit (0.f, 1.f, x);
        F1 = a1 + (b1 - a1) * x; F2 = a2 + (b2 - a2) * x; F3 = a3 + (b3 - a3) * x;
    }
    // the talkbox: the synth's sound through the mouth (three resonances), a little throat and breath
    float mouthOut (float carrier, float F1, float F2, float F3, float body, float bright, float tex, float noise, float sr)
    {
        const float shift = 0.85f + 0.3f * bright;   // BRIGHT: a smaller (higher) or bigger (lower) mouth
        f1.set (F1 * shift, 7.f, sr); f2.set (F2 * shift, 9.f, sr); sym1.set (F3 * shift, 10.f, sr);
        const float in = carrier + noise * 0.05f * tex;
        const float o = f1.process (in).bp * 1.6f + f2.process (in).bp * 1.1f + sym1.process (in).bp * 0.55f;
        return (o * 0.28f * (0.55f + 0.6f * body) + carrier * 0.03f) * talkTrim[eng];
    }
    // how open the mouth is: opens with the note, closes when the key is released, the mod wheel opens it more
    float mouthOpen (float tsec, float speed, float sr)
    {
        if (released) relSec += 1.f / sr;
        float m = 1.f - std::exp (-tsec / speed);
        if (released) m *= std::exp (-relSec / 0.09f);
        return juce::jmax (m, midiMod);
    }

    float renderEngine (double f, const float* t, float sr)
    {
        double dt = f / sr;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float tone = t[0], body = t[1], bright = t[2], width = t[3], tex = t[4];
        const float noise = rng.nextFloat() * 2.f - 1.f;
        const float tsec = (float) age / sr;
        const float speed = 0.04f + 0.45f * width;   // WIDTH: how slowly the mouth opens
        float out = 0.f, F1 = 0.f, F2 = 0.f, F3 = 0.f;

        // the talkbox carrier: saw + square, TONE moves from a hollow square to a full saw
        auto carrier = [&] (double d)
        {
            ph[0] += d; wrap (ph[0]);
            return sawBL (ph[0], d) * (0.45f + 0.35f * tone) + pulse (ph[0], d, 0.5) * (0.4f - 0.3f * tone);
        };

        switch (eng)
        {
        case 0: // Ooh-Wah: the mouth opens from "ooh" to "wah"
        {
            const float m = mouthOpen (tsec, speed, sr);
            vowelGlide (4, 0, m, F1, F2, F3);
            out = mouthOut (carrier (dt), F1, F2, F3, body, bright, tex, noise, sr);
            break;
        }
        case 1: // Yeah: "ee" -> "eh" -> "ah"
        {
            const float m = mouthOpen (tsec, speed, sr);
            if (m < 0.5f) vowelGlide (2, 1, m * 2.f, F1, F2, F3); else vowelGlide (1, 0, m * 2.f - 1.f, F1, F2, F3);
            out = mouthOut (carrier (dt), F1, F2, F3, body, bright, tex, noise, sr);
            break;
        }
        case 2: // Ay: "ah" closing to "ee", the classic sung talkbox line
        {
            const float m = mouthOpen (tsec, speed * 1.6f, sr);
            vowelGlide (0, 2, m, F1, F2, F3);
            out = mouthOut (carrier (dt), F1, F2, F3, body, bright, tex, noise, sr);
            break;
        }
        case 3: // classic talkbox: TONE picks the vowel (oo-oh-ah-eh-ee), WIDTH makes the mouth wobble
        {
            ph[7] += (0.5 + 5.0 * (double) width) / sr; wrap (ph[7]);
            const float v = juce::jlimit (0.f, 0.999f, tone + 0.2f * width * std::sin ((float) ph[7] * twoPi) + 0.5f * midiMod) * 4.f;
            static const int path[5] = { 4, 3, 0, 1, 2 };
            const int i = (int) v;
            vowelGlide (path[i], path[juce::jmin (4, i + 1)], v - (float) i, F1, F2, F3);
            out = mouthOut (carrier (dt), F1, F2, F3, body, bright, tex, noise, sr);
            break;
        }
        case 4: // Talk bass: the carrier an octave down with a sub, "oh" opening to "ah"
        {
            const float m = mouthOpen (tsec, speed * 0.7f, sr);
            vowelGlide (3, 0, m, F1, F2, F3);
            const float c = carrier (dt * 0.5);
            ph[1] += dt * 0.5; wrap (ph[1]);
            out = mouthOut (c, F1 * 0.85f, F2 * 0.85f, F3, body, bright, tex, noise, sr) + std::sin ((float) ph[1] * twoPi) * 0.45f * body;
            break;
        }
        case 5: // Vocoder choir: five voices slightly apart singing "oh-ah"
        {
            float c = 0.f;
            for (int k = 0; k < 5; ++k)
            {
                const double det = 1.0 + (double) (k - 2) * (0.003 + 0.008 * (double) width);
                ph[(size_t) (k + 1)] += dt * det; wrap (ph[(size_t) (k + 1)]);
                c += sawBL (ph[(size_t) (k + 1)], dt * det);
            }
            c *= 0.22f;
            vowelGlide (3, 0, 0.3f + 0.6f * tone + 0.3f * midiMod, F1, F2, F3);
            out = mouthOut (c, F1, F2, F3, body, bright, tex, noise, sr) * 0.9f;
            break;
        }
        case 6: // Robot talk: the mouth jumps from vowel to vowel in rhythm (WIDTH: how fast)
        {
            ph[7] += (2.0 + 10.0 * (double) width) / sr;
            if (ph[7] >= 1.0) { ph[7] -= 1.0; robotFrom = robotTo; robotTo = (robotTo + 1 + (int) (rng.nextFloat() * 4.f)) % 5; }
            const float x = juce::jlimit (0.f, 1.f, (float) ph[7] * (3.f - 2.5f * tone));
            vowelGlide (robotFrom, robotTo, x, F1, F2, F3);
            out = mouthOut (carrier (dt), F1, F2, F3, body, bright, tex, noise, sr);
            break;
        }
        default: // 7 G-funk whistle: sine + soft saw, vibrato that settles in
        {
            ph[4] += 5.4 / sr; wrap (ph[4]);
            const float vib = std::sin ((float) ph[4] * twoPi) * width * 0.02f * juce::jmin (1.f, juce::jmax (0.f, tsec - 0.15f) * 2.f);
            dt *= 1.0 + (double) vib;
            ph[0] += dt; wrap (ph[0]); ph[1] += dt * 2.0; wrap (ph[1]);
            const float sn = std::sin ((float) ph[0] * twoPi);
            out = sn * (0.6f + body * 0.4f) + sawBL (ph[0], dt) * 0.35f * tone + std::sin ((float) ph[1] * twoPi) * tex * 0.4f;
            fl.set ((float) f * (2.f + 9.f * bright), 0.8f, sr);
            out = fl.process (out).lp * 0.8f;
            break;
        }
        }
        return out;
    }

    const ParamPtrs& P;
    GlideState& glideState;
    juce::ADSR adsr, fadsr;
    juce::Random rng;
    SVF breathBP, sym1, sym2, f1, f2, fl, voiceFilter, realTone;
    std::vector<float> ksBuf;
    int ksW = 0, eng = 0, age = 0;
    float ksPrev = 0.f, exciteLP = 0.f, targetFreq = 440.f, curFreq = 440.f, vel = 1.f, pan = 0.f;
    float midiBend = 0.f, midiMod = 0.f, shValue = 0.f;
    float smVol = 0.f, smSemi = 0.f, smBend = 0.f, smCut = 9000.f;
    bool freshNote = true, released = false;
    float relSec = 0.f;                       // time since the key was released (the mouth closes)
    float talkTrim[8] { 0.74f, 0.62f, 0.67f, 2.56f, 1.05f, 2.26f, 0.50f, 1.00f };   // per-engine level, measured so every voice sits around -16 dB like the other synths
    int robotFrom = 4, robotTo = 0;           // Robot talk: the vowels it moves between
    float fenvLevel = 0.f, lastEnv = 0.f, lastLfo = 0.f, lastFc = 9000.f;   // for the interface
    float dcX = 0.f, dcY = 0.f, dcR = 0.998f;
    double ph[8] {}, lfoPh = 0.0;
    hfx::SamplePlayer real;      // the real instrument's recording this note plays
    int realEngine = -1, noteKey = 60;
    float realGain[8] { 1.00f, 1.00f, 1.00f, 1.00f, 1.88f, 1.00f, 1.96f, 1.80f };   // levels of the recordings, matched to the synthesised engines (set_levels)

    // a recorded note, coloured by the engine's knobs: TONE darkens, BRIGHT adds air, BODY warms, TEXTURE adds breath
    float playReal (const hfx::SampleSet& set, double f, const float* t, float sr)
    {
        if (realEngine != eng || ! real.active())
        {
            int key = noteKey;
            if (false) key = 36 + ((noteKey - 36) % 24 + 24) % 24;   // drums: every octave plays the strokes
            real.start (set.pick (key, vel)); realEngine = eng;
        }
        float x = real.next (f, (double) sr);
        const float tone = t[0], body = t[1], bright = t[2], tex = t[4];
        realTone.set (juce::jlimit (200.f, sr * 0.45f, real.isDrum() ? 2500.f + 14000.f * (1.f - tone) : (float) f * (4.f + 14.f * (1.f - tone))), 0.7f, sr);
        const auto o = realTone.process (x);
        x = o.lp * (0.65f + 0.35f * (1.f - bright)) + o.hp * (0.1f + 0.7f * bright) + o.bp * 0.3f * body;
        return (x * 1.7f + (real.isDrum() ? 0.f : (rng.nextFloat() * 2.f - 1.f) * 0.01f * tex)) * realGain[eng];
    }
    float fmFb = 0.f, holdV = 0.f, holdC = 0.f;
};
}
