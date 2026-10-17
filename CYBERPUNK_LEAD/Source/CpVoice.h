#pragma once
#include "Params.h"
#include <cmath>

namespace Cp
{
struct GlideState
{
    std::atomic<float> lastFreq { 0.f };
    std::atomic<float> lastMod { 0.f };   // latest mod wheel (CC1) value 0..1, kept even when no voice is playing
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

class CpSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class CpVoice : public juce::SynthesiserVoice
{
public:
    CpVoice (const ParamPtrs& pp, GlideState& gs) : P (pp), glideState (gs) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<CpSound*> (s) != nullptr; }

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
        freshNote = true; dcX = dcY = 0.f; dcR = 1.f - juce::MathConstants<float>::twoPi * 12.f / (float) getSampleRate();
        fadsr.noteOn(); adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) { adsr.noteOff(); fadsr.noteOff(); }
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

            static constexpr float trim[8] = { 1.6f, 1.2f, 0.85f, 0.7f, 1.8f, 1.1f, 2.0f, 1.1f };   // equilibrage des niveaux entre moteurs
            float y = renderEngine (freq, t, (float) sr) * trim[eng];
            { const float dcOut = y - dcX + dcR * dcY; dcX = y; dcY = dcOut; y = dcOut; } // bloqueur DC

            if ((i & 7) == 0)
            {
                float fc = smCut * std::exp2 ((dst == 1 ? mod * 3.f : 0.f) + (float) std::log2 (freq / 261.6) * 0.25f + fdepth * fenvLevel * 5.f);
                voiceFilter.set (fc, resoQ, sr);
            }
            auto fo = voiceFilter.process (y);
            y = ft == 0 ? fo.lp : ft == 1 ? fo.hp : fo.bp * 1.5f;

            const float env = adsr.getNextSample();
            fenvLevel = fadsr.getNextSample();
            float amp = env * ((1.f - velSens) + velSens * vel) * smVol * 0.5f;
            if (dst == 2) amp *= 1.f - 0.5f * amt * (0.5f + 0.5f * lfo);
            y *= amp;

            L[start + i] += y * gL;
            if (R) R[start + i] += y * gR;
            if (age < 0x3fffffff) ++age;   // saturate: no signed overflow on very long held notes
            if (! adsr.isActive()) { clearCurrentNote(); break; }
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

    static float pulse (double phase, double dt, double pw)
    {
        double p2 = phase + pw; p2 -= std::floor (p2);
        const float a = (float) (2.0 * phase - 1.0) - polyBlep (phase, dt), b2 = (float) (2.0 * p2 - 1.0) - polyBlep (p2, dt);
        return a - b2;
    }

    float renderEngine (double f, const float* t, float sr)
    {
        double dt = f / sr;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float tone = t[0], body = t[1], bright = t[2], width = t[3], tex = t[4];
        const float noise = rng.nextFloat() * 2.f - 1.f;
        const float tsec = (float) age / sr;
        float out = 0.f;

        switch (eng)
        {
        case 0: // neon saw : supersaw 5 voix + sub + octave
        {
            float s = 0.f;
            for (int k = 0; k < 5; ++k)
            {
                const double det = 1.0 + (double) (k - 2) * (0.002 + body * 0.012);
                ph[(size_t) k] += dt * det; wrap (ph[(size_t) k]);
                s += saw (ph[(size_t) k], dt * det);
            }
            s *= 0.22f;
            ph[5] += dt * 0.5; wrap (ph[5]);
            ph[6] += dt * 2.0; wrap (ph[6]);
            s += std::sin ((float) ph[5] * twoPi) * tone * 0.5f + saw (ph[6], dt * 2.0) * width * 0.15f;
            fl.set ((float) f * (2.f + 14.f * bright), 0.7f, sr);
            out = fl.process (s).lp * 0.9f + noise * 0.02f * tex;
            break;
        }
        case 1: // chrome PWM : deux pulses, largeur modulee
        {
            ph[7] += (0.3 + 5.0 * (double) width) / sr; wrap (ph[7]);
            const double pw = 0.5 - 0.4 * (0.2 + 0.8 * (double) tone) * (0.5 + 0.5 * std::sin (ph[7] * (double) twoPi));
            const double det = 1.0 + 0.002 + 0.01 * (double) body;
            ph[0] += dt; wrap (ph[0]); ph[1] += dt * det; wrap (ph[1]);
            float s = 0.5f * (pulse (ph[0], dt, pw) + pulse (ph[1], dt * det, pw)) * 0.5f;
            ph[5] += dt * 0.5; wrap (ph[5]);
            s += std::sin ((float) ph[5] * twoPi) * tex * 0.4f;
            fl.set ((float) f * (2.f + 12.f * bright), 0.7f, sr);
            out = fl.process (s).lp;
            break;
        }
        case 2: // glitch FM : 2 operateurs, indice a enveloppe, feedback
        {
            const float ratio = 0.5f + tone * 3.5f;
            const float idx = bright * 5.f * (0.15f + std::exp (-tsec / (0.04f + body * 1.2f))) ;
            ph[1] += dt * (double) ratio; wrap (ph[1]);
            const float m = std::sin ((float) ph[1] * twoPi + fmFb * width * 1.5f);
            fmFb = m;
            ph[0] += dt; wrap (ph[0]);
            out = std::sin ((float) ph[0] * twoPi + idx * m) * 0.8f + noise * std::exp (-tsec * 80.f) * tex * 0.3f;
            break;
        }
        case 3: // reese : saws detunees graves + sub + saturation
        {
            float s = 0.f;
            for (int k = 0; k < 3; ++k)
            {
                const double det = 1.0 + (double) (k - 1) * (0.001 + body * 0.012);
                ph[(size_t) k] += dt * det; wrap (ph[(size_t) k]);
                s += saw (ph[(size_t) k], dt * det);
            }
            s *= 0.33f;
            ph[5] += dt * 0.5; wrap (ph[5]);
            s += std::sin ((float) ph[5] * twoPi) * (0.2f + 0.6f * tone);
            fl.set ((float) f * (1.5f + 8.f * bright), 0.9f, sr);
            out = std::tanh (fl.process (s).lp * (1.f + width * 4.f)) * 0.7f + noise * 0.01f * tex;
            break;
        }
        case 4: // laser pluck : saw + plongeon de hauteur + filtre qui se ferme
        {
            const double dive = 1.0 + (double) (tone * 3.f) * std::exp ((double) (-tsec * 40.f));
            ph[0] += dt * dive; wrap (ph[0]);
            ph[1] += dt * dive * (1.0 + 0.004 * (double) width); wrap (ph[1]);
            const float s = 0.5f * (saw (ph[0], dt * dive) + saw (ph[1], dt * dive));
            fl.set ((float) f * (1.5f + 20.f * bright * std::exp (-tsec / (0.03f + body * 0.5f))), 1.2f, sr);
            out = fl.process (s).lp + noise * std::exp (-tsec * 150.f) * tex * 0.25f;
            break;
        }
        case 5: // neon brass : saws, filtre qui s'ouvre
        {
            float s = 0.f;
            for (int k = 0; k < 3; ++k)
            {
                const double det = 1.0 + (double) (k - 1) * (0.002 + width * 0.01);
                ph[(size_t) k] += dt * det; wrap (ph[(size_t) k]);
                s += saw (ph[(size_t) k], dt * det);
            }
            s *= 0.33f;
            const float open = 1.f - std::exp (-tsec / (0.02f + tone * 0.4f));
            fl.set ((float) f * (1.5f + (2.f + 10.f * bright) * open), 0.8f, sr);
            out = fl.process (s).lp * (0.6f + body * 0.4f) + noise * 0.02f * tex;
            break;
        }
        case 6: // data crush : saw reechantillonnee + reduction de bits
        {
            ph[0] += dt; wrap (ph[0]);
            const float s = saw (ph[0], dt) * 0.6f + (ph[0] < 0.5 ? 0.3f : -0.3f) * width;
            holdC -= 1.f;
            if (holdC <= 0.f) { holdC += 2.f + (1.f - tone) * 40.f; holdV = s; }
            const float levels = std::pow (2.f, 2.f + body * 10.f);
            const float q = std::round (holdV * levels) / levels;
            fl.set ((float) f * (2.f + 12.f * bright), 0.7f, sr);
            out = fl.process (q).lp * 0.9f + noise * 0.05f * tex;
            break;
        }
        default: // 7 hollow glass : FM cloche + paire detunee
        {
            const float ratio = 1.4f + tone * 3.1f;
            const float idx = bright * 3.5f * std::exp (-tsec / (0.2f + body * 2.f));
            ph[1] += dt * (double) ratio; wrap (ph[1]);
            const float m = std::sin ((float) ph[1] * twoPi);
            ph[0] += dt; wrap (ph[0]); ph[2] += dt * (1.0 + 0.003 * (double) width); wrap (ph[2]);
            out = 0.5f * (std::sin ((float) ph[0] * twoPi + idx * m) + std::sin ((float) ph[2] * twoPi + idx * m)) * std::exp (-tsec / (0.4f + body * 3.f) * 0.5f);
            out += noise * std::exp (-tsec * 200.f) * tex * 0.2f;
            break;
        }
        }
        return out;
    }

    const ParamPtrs& P;
    GlideState& glideState;
    juce::ADSR adsr, fadsr;
    juce::Random rng;
    SVF breathBP, sym1, sym2, f1, f2, fl, voiceFilter;
    std::vector<float> ksBuf;
    int ksW = 0, eng = 0, age = 0;
    float ksPrev = 0.f, exciteLP = 0.f, targetFreq = 440.f, curFreq = 440.f, vel = 1.f, pan = 0.f;
    float midiBend = 0.f, midiMod = 0.f, shValue = 0.f;
    float smVol = 0.f, smSemi = 0.f, smBend = 0.f, smCut = 9000.f;
    bool freshNote = true;
    float fenvLevel = 0.f;
    float dcX = 0.f, dcY = 0.f, dcR = 0.998f;
    double ph[8] {}, lfoPh = 0.0;
    float fmFb = 0.f, holdV = 0.f, holdC = 0.f;
};
}
