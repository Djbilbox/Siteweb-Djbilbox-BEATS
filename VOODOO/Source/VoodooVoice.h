#pragma once
#include "Params.h"
#include <cmath>

namespace Voodoo
{
struct GlideState { std::atomic<float> lastFreq { 0.f }; };

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

class VoodooSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class VoodooVoice : public juce::SynthesiserVoice
{
public:
    VoodooVoice (const ParamPtrs& pp, GlideState& gs) : P (pp), glideState (gs) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<VoodooSound*> (s) != nullptr; }

    void setCurrentPlaybackSampleRate (double sr) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (sr);
        ksBuf.assign ((size_t) (sr / 12.0) + 8, 0.f);
        adsr.setSampleRate (sr);
    }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override
    {
        targetFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
        const float last = glideState.lastFreq.load();
        curFreq = (P (glide) > 0.001f && last > 0.f) ? last : targetFreq;
        glideState.lastFreq = targetFreq;
        vel = velocity;
        midiBend = (pitchWheel - 8192) / 8192.f;
        age = 0;
        for (auto& p : ph) p = rng.nextDouble();
        std::fill (ksBuf.begin(), ksBuf.end(), 0.f);
        ksW = 0; ksPrev = 0.f;
        for (auto* s : { &breathBP, &sym1, &sym2, &f1, &f2, &fl, &voiceFilter }) s->reset();
        pan = (rng.nextFloat() - 0.5f) * 2.f;
        lfoPh = 0.0; shValue = rng.nextFloat() * 2.f - 1.f;
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) adsr.noteOff();
        else { adsr.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int v) override { midiBend = (v - 8192) / 8192.f; }
    void controllerMoved (int cc, int v) override { if (cc == 1) midiMod = v / 127.f; }
    void aftertouchChanged (int) override {}
    void channelPressureChanged (int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (! adsr.isActive()) { clearCurrentNote(); return; }
        const double sr = getSampleRate();
        eng = juce::jlimit (0, 7, (int) P (wave));
        const int src = (int) P (lfosrc), dst = (int) P (dest), ft = (int) P (ftype);
        const float semis = P (pitch), gl = P (glide);
        const float cutoff = P (filter);
        const float lfoRate = P (rate), amt = juce::jlimit (0.f, 1.f, P (amount) + 0.8f * juce::jmax (midiMod, P (modw)));
        const float uiBend = P (bend), masterVol = P (volume);
        const float tb[5] = { P (t1), P (t2), P (t3), P (t4), P (t5) };
        adsr.setParameters ({ P (atk), P (dec), P (sus), P (rel) });
        const float glideCoef = gl < 0.001f ? 1.f : 1.f - std::exp (-1.f / (gl * (float) sr * 0.35f));
        float* L = out.getWritePointer (0);
        float* R = out.getNumChannels() > 1 ? out.getWritePointer (1) : nullptr;
        const float gL = std::sqrt (0.5f * (1.f - pan * 0.35f)), gR = std::sqrt (0.5f * (1.f + pan * 0.35f));

        for (int i = 0; i < num; ++i)
        {
            curFreq += (targetFreq - curFreq) * glideCoef;

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

            float semiTotal = semis + (midiBend + uiBend) * 2.f;
            if (dst == 0) semiTotal += mod * 1.0f;
            const double freq = (double) curFreq * std::exp2 (semiTotal / 12.f);

            float t[5];
            for (int k = 0; k < 5; ++k) t[k] = juce::jlimit (0.f, 1.f, tb[k] + (dst == 3 ? mod * 0.5f : 0.f));

            float y = renderEngine (freq, t, (float) sr);

            if ((i & 7) == 0)
            {
                float fc = cutoff * std::exp2 ((dst == 1 ? mod * 3.f : 0.f) + (float) std::log2 (freq / 261.6) * 0.25f);
                voiceFilter.set (fc, 0.7f + tb[1] * 1.2f, sr);
            }
            auto fo = voiceFilter.process (y);
            y = ft == 0 ? fo.lp : ft == 1 ? fo.hp : fo.bp * 1.5f;

            const float env = adsr.getNextSample();
            float amp = env * (0.35f + 0.65f * vel) * masterVol * 0.5f;
            if (dst == 2) amp *= 1.f - 0.5f * amt * (0.5f + 0.5f * lfo);
            y *= amp;

            L[start + i] += y * gL;
            if (R) R[start + i] += y * gR;
            ++age;
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

    float renderEngine (double f, const float* t, float sr)
    {
        const double dt = f / sr;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float breathT = t[0], body = t[1], bright = t[2], width = t[3], tex = t[4];
        const float noise = rng.nextFloat() * 2.f - 1.f;
        float out = 0.f;

        switch (eng)
        {
        case 0: case 1: case 2: // famille souffle : flute, pan flute, shakuhachi
        {
            static const float H[3][4] = { { 1.f, .28f, .10f, .04f }, { 1.f, .03f, .16f, .02f }, { 1.f, .12f, .30f, .10f } };
            static const float BG[3] = { .30f, .35f, .60f }, BQ[3] = { 5.f, 14.f, 3.5f }, BR[3] = { 2.f, 1.f, 1.5f };
            ph[0] += dt; wrap (ph[0]);
            ph[1] += dt * (1.0 + width * 0.012); wrap (ph[1]);
            const float p0 = (float) ph[0] * twoPi, p1 = (float) ph[1] * twoPi;
            float tone = 0.f;
            for (int k = 0; k < 4; ++k)
                tone += H[eng][k] * (k == 0 ? 1.f : (0.2f + 1.6f * bright)) * (std::sin (p0 * (float) (k + 1)) + width * 0.5f * std::sin (p1 * (float) (k + 1))) ;
            tone *= 0.55f;
            breathBP.set ((float) f * BR[eng], BQ[eng] * (0.5f + body), sr);
            const float bp = breathBP.process (noise).bp * (1.f + 0.5f * BQ[eng]);
            const float chiff = std::exp (-(float) age / (0.05f * sr)) * noise * 0.4f * (0.3f + tex);
            out = tone * (1.f - 0.3f * breathT) + bp * (0.1f + breathT * 0.9f) * BG[eng] + chiff * (eng == 2 ? 1.4f : 1.f);
            break;
        }
        case 3: case 4: // cordes pincees : sitar, oud (Karplus-Strong)
        {
            const int size = (int) ksBuf.size();
            const float Lraw = juce::jlimit (4.f, (float) size - 3.f, sr / (float) f);
            float inj = 0.f;
            if ((float) age < Lraw) { inj = noise * 0.55f; if (eng == 4) { exciteLP += 0.35f * (inj - exciteLP); inj = exciteLP * 1.6f; } }
            float rp = (float) ksW - Lraw; if (rp < 0) rp += (float) size;
            const int i0 = (int) rp, i1 = (i0 + 1) % size; const float fr = rp - (float) i0;
            const float y = ksBuf[(size_t) i0] * (1.f - fr) + ksBuf[(size_t) i1] * fr;
            const float damp = eng == 3 ? 0.30f + 0.65f * bright : 0.20f + 0.60f * bright;
            const float yf = damp * y + (1.f - damp) * ksPrev; ksPrev = yf;
            const float g = 0.97f + 0.0295f * body;
            ksBuf[(size_t) ksW] = inj + yf * g;
            ksW = (ksW + 1) % size;
            out = y * (eng == 3 ? 2.f : 3.5f);
            if (eng == 3) // jawari : buzz + cordes sympathiques
            {
                out = std::tanh (out * (1.f + tex * 6.f)) * 0.5f;
                sym1.set ((float) f * 2.f, 80.f, sr); sym2.set ((float) f * 1.5f, 80.f, sr);
                out += (sym1.process (out).bp + sym2.process (out).bp) * width * 0.2f;
            }
            break;
        }
        case 5: // kalimba / mbira : modes inharmoniques
        {
            static const float R[4] = { 1.f, 2.76f, 5.40f, 8.93f }, A[4] = { 1.f, .45f, .22f, .12f };
            const float tsec = (float) age / sr;
            for (int k = 0; k < 4; ++k)
            {
                ph[(size_t) k] += dt * R[k]; wrap (ph[(size_t) k]);
                const float rate = (1.2f + (float) k * 2.2f) / (0.25f + body * 2.2f);
                out += A[k] * (k == 0 ? 1.f : 0.3f + 1.2f * bright) * std::exp (-tsec * rate) * std::sin ((float) ph[(size_t) k] * twoPi);
            }
            out *= 1.0f;
            out += noise * std::exp (-tsec * 90.f) * 0.12f * (0.3f + tex);
            break;
        }
        default: // erhu (6) / voodoo choir (7) : saws + formants
        {
            const int n = eng == 6 ? 2 : 5;
            float s = 0.f;
            for (int k = 0; k < n; ++k)
            {
                const double det = 1.0 + (double) (k - (n - 1) * 0.5) * width * (eng == 6 ? 0.012 : 0.01);
                ph[(size_t) k] += dt * det; wrap (ph[(size_t) k]);
                s += saw (ph[(size_t) k], dt * det);
            }
            s /= (float) n;
            fl.set ((float) f * (1.5f + 10.f * bright), 0.7f, sr);
            f1.set (300.f + 600.f * breathT, 6.f, sr);
            f2.set (900.f + 1500.f * breathT, 6.f, sr);
            const float base = fl.process (s).lp;
            out = base * 0.7f + (f1.process (s).bp + f2.process (s).bp) * body * 1.6f;
            out += noise * 0.06f * tex;
            out *= 0.5f;
            break;
        }
        }
        return out;
    }

    const ParamPtrs& P;
    GlideState& glideState;
    juce::ADSR adsr;
    juce::Random rng;
    SVF breathBP, sym1, sym2, f1, f2, fl, voiceFilter;
    std::vector<float> ksBuf;
    int ksW = 0, eng = 0, age = 0;
    float ksPrev = 0.f, exciteLP = 0.f, targetFreq = 440.f, curFreq = 440.f, vel = 1.f, pan = 0.f;
    float midiBend = 0.f, midiMod = 0.f, shValue = 0.f;
    double ph[6] {}, lfoPh = 0.0;
};
}
