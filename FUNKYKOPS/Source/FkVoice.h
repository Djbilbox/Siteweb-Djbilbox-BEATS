#pragma once
#include "Params.h"
#include <cmath>

namespace Fk
{
struct GlideState
{
    std::atomic<float> lastFreq { 0.f };
    std::atomic<float> modWheel { 0.f };   // latest MIDI CC1 (0..1): voices started later must begin with it
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

class FkSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class FkVoice : public juce::SynthesiserVoice
{
public:
    FkVoice (const ParamPtrs& pp, GlideState& gs) : P (pp), glideState (gs) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<FkSound*> (s) != nullptr; }

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
        midiMod = glideState.modWheel.load();   // idle voices never received CC1, so take the latest wheel position
        midiBend = (pitchWheel - 8192) / 8192.f;
        age = 0;
        for (auto& p : ph) p = rng.nextDouble();
        std::fill (ksBuf.begin(), ksBuf.end(), 0.f);
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
    void controllerMoved (int cc, int v) override { if (cc == 1) midiMod = v / 127.f; else if (cc == 121) midiMod = 0.f; }   // CC121 = reset all controllers
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
        const float fdec = P (fdecay), fdepth = P (fenv), resoQ = 0.7f + P (reso) * 9.f;
        fadsr.setParameters ({ 0.002f, fdec, 0.f, fdec });
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

            float y = renderEngine (freq, t, (float) sr);
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

    float renderEngine (double f, const float* t, float sr)
    {
        const double dt = f / sr;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float breathT = t[0], body = t[1], bright = t[2], width = t[3], tex = t[4];
        const float noise = rng.nextFloat() * 2.f - 1.f;
        float out = 0.f;

        switch (eng)
        {
        case 0: case 6: // clavinet (0) / guitare (6) : Karplus-Strong
        {
            const int size = (int) ksBuf.size();
            const float Lraw = juce::jlimit (4.f, (float) size - 3.f, sr / (float) f);
            float inj = 0.f;
            if ((float) age < Lraw) { inj = noise * 0.6f; if (eng == 6) { exciteLP += 0.4f * (inj - exciteLP); inj = exciteLP * 1.7f; } }
            float rp = (float) ksW - Lraw; if (rp < 0) rp += (float) size;
            const int i0 = (int) rp, i1 = (i0 + 1) % size; const float fr = rp - (float) i0;
            const float y = ksBuf[(size_t) i0] * (1.f - fr) + ksBuf[(size_t) i1] * fr;
            const float damp = eng == 0 ? 0.45f + 0.5f * bright : 0.25f + 0.6f * bright;
            const float yf = damp * y + (1.f - damp) * ksPrev; ksPrev = yf;
            const float g = (eng == 0 ? 0.93f : 0.965f) + (eng == 0 ? 0.06f : 0.0325f) * body;
            ksBuf[(size_t) ksW] = inj + yf * g;
            ksW = (ksW + 1) % size;
            out = y * (eng == 0 ? 1.7f : 2.0f);
            if (eng == 0) out = std::tanh (out * (1.f + tex * 3.f)) * 0.8f;     // morsure du capteur
            break;
        }
        case 1: // rhodes : tine + cloche, tombee naturelle, "bark" a la velocite
        {
            static const float R[4] = { 1.f, 2.f, 4.f, 7.01f }, A[4] = { 1.f, .3f, .14f, .18f }, DR[4] = { 1.f, 2.2f, 4.5f, 12.f };
            const float tsec = (float) age / sr;
            const float barkAmt = 0.3f + 1.3f * bright * (0.4f + 0.6f * vel);
            for (int k = 0; k < 4; ++k)
            {
                ph[(size_t) k] += dt * R[k]; wrap (ph[(size_t) k]);
                const float rate = DR[k] / (0.5f + body * 3.f);
                out += A[k] * (k == 0 ? 1.f : barkAmt) * std::exp (-tsec * rate) * std::sin ((float) ph[(size_t) k] * twoPi);
            }
            out *= 0.7f;
            out += noise * std::exp (-tsec * 120.f) * 0.1f * (0.3f + tex);
            break;
        }
        default: // synth bass (2), brass (3), strings (4), lead (5), talkbox (7) : saws
        {
            const int n = eng == 2 ? 1 : (eng == 3 ? 3 : (eng == 4 ? 5 : (eng == 5 ? 2 : 3)));
            const double dspread = eng == 3 ? 0.006 : (eng == 4 ? 0.012 : 0.010);
            float s = 0.f;
            for (int k = 0; k < n; ++k)
            {
                const double det = 1.0 + (double) (k - (n - 1) * 0.5) * width * dspread;
                ph[(size_t) k] += dt * det; wrap (ph[(size_t) k]);
                s += saw (ph[(size_t) k], dt * det);
            }
            s /= (float) n;
            if (eng == 2)          // sub sinus + carre : basse moog
            {
                ph[5] += dt * 0.5; wrap (ph[5]);
                const float sq = s - saw (std::fmod (ph[0] + 0.5, 1.0), dt);
                s = s * (1.f - 0.5f * tex) + sq * 0.5f * tex + std::sin ((float) ph[5] * twoPi) * (0.4f + 0.6f * width);
                s *= 0.6f;
            }
            if (eng == 5)          // lead : saw + carre
            {
                ph[3] += dt * 0.5 * (1.0 + width * 0.01); wrap (ph[3]);
                s = s * 0.7f + (ph[3] < 0.5 ? 0.3f : -0.3f);
            }
            if (eng == 7)          // talkbox : voyelles
            {
                f1.set (300.f + 600.f * breathT, 8.f, sr);
                f2.set (900.f + 1500.f * breathT, 8.f, sr);
                fl.set ((float) f * (1.5f + 8.f * bright), 0.7f, sr);
                out = fl.process (s).lp * 0.5f + (f1.process (s).bp + f2.process (s).bp) * (0.4f + body) * 1.8f;
                out += noise * 0.05f * tex;
                out *= 0.5f;
                break;
            }
            fl.set ((float) f * (eng == 2 ? 3.f + 6.f * bright : 2.f + 12.f * bright), 0.7f, sr);
            out = fl.process (s).lp * (eng == 4 ? 0.5f : 0.8f);
            if (eng == 3) out += noise * 0.02f * tex;
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
    double ph[6] {}, lfoPh = 0.0;
};
}
