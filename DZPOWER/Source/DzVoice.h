#pragma once
#include "Params.h"
#include <cmath>

namespace Dz
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

class DzSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class DzVoice : public juce::SynthesiserVoice
{
public:
    DzVoice (const ParamPtrs& pp, GlideState& gs) : P (pp), glideState (gs) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<DzSound*> (s) != nullptr; }

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
    void controllerMoved (int cc, int v) override { if (cc == 1) midiMod = v / 127.f; }
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

            static constexpr float trim[8] = { 1.3f, 1.3f, 0.75f, 0.75f, 1.2f, 1.0f, 1.0f, 0.6f };   // equilibrage des niveaux entre moteurs
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
        double dt = f / sr;
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float tone = t[0], body = t[1], bright = t[2], width = t[3], tex = t[4];
        const float noise = rng.nextFloat() * 2.f - 1.f;
        const float tsec = (float) age / sr;
        float out = 0.f;

        switch (eng)
        {
        case 0: case 1: // mandole (0) / qanun (1) : corde pincee ; le qanun ajoute l'octave de sympathie
        {
            const int size = (int) ksBuf.size();
            const float Lraw = juce::jlimit (4.f, (float) size - 3.f, sr / (float) f);
            float inj = 0.f;
            if ((float) age < Lraw)
            {
                exciteLP += (eng == 0 ? 0.35f + 0.5f * tone : 0.7f) * (noise * 0.6f - exciteLP);
                inj = exciteLP * (eng == 0 ? 1.8f : 1.3f);
            }
            float rp = (float) ksW - Lraw; if (rp < 0) rp += (float) size;
            const int i0 = (int) rp, i1 = (i0 + 1) % size; const float fr = rp - (float) i0;
            const float y = ksBuf[(size_t) i0] * (1.f - fr) + ksBuf[(size_t) i1] * fr;
            const float damp = eng == 0 ? 0.3f + 0.6f * bright : 0.4f + 0.55f * bright;
            const float yf = damp * y + (1.f - damp) * ksPrev; ksPrev = yf;
            ksBuf[(size_t) ksW] = inj + yf * ((eng == 0 ? 0.96f : 0.95f) + (eng == 0 ? 0.035f : 0.045f) * body);
            ksW = (ksW + 1) % size;
            out = y * 1.9f;
            if (eng == 0)   // double chœur de cordes + caisse
            {
                f1.set (220.f + 200.f * tone, 5.f, sr);
                out = out * 0.8f + f1.process (y).bp * tex * 1.2f;
            }
            else
            {
                ph[0] += dt * 2.0; wrap (ph[0]);
                out += width * 0.4f * std::exp (-tsec * 5.f) * std::sin ((float) ph[0] * twoPi);
            }
            out = std::tanh (out * 1.5f) * 0.7f;   // compression douce des transitoires du pincé
            break;
        }
        case 2: // gasba : flute de roseau, souffle + vibrato
        {
            ph[4] += 5.2 / sr; wrap (ph[4]);
            const float vib = std::sin ((float) ph[4] * twoPi) * width * 0.012f * juce::jmin (1.f, tsec * 2.f);
            dt *= 1.0 + (double) vib;
            ph[0] += dt; wrap (ph[0]);
            const float s1 = std::sin ((float) ph[0] * twoPi);
            out = s1 + 0.25f * bright * std::sin (2.f * (float) ph[0] * twoPi) + 0.12f * bright * std::sin (3.f * (float) ph[0] * twoPi);
            f1.set ((float) f * (2.f + 3.f * tone), 3.f + 4.f * body, sr);
            out = out * 0.55f + f1.process (noise).bp * (0.12f + 0.6f * tex);
            break;
        }
        case 3: // ghaita : anche double, resonance nasale
        {
            const double det = 1.0 + 0.004 * width;
            ph[0] += dt; wrap (ph[0]); ph[1] += dt * det; wrap (ph[1]);
            const float s = 0.5f * (saw (ph[0], dt) + saw (ph[1], dt * det)) + (ph[0] < 0.3 ? 0.25f : -0.1f);
            f1.set (900.f + 700.f * tone, 7.f, sr);
            f2.set (2000.f + 900.f * tone, 9.f, sr);
            fl.set ((float) f * (2.f + 10.f * bright), 0.7f, sr);
            out = fl.process (s).lp * 0.3f + (f1.process (s).bp + f2.process (s).bp * 0.7f) * (0.3f + body) * 0.9f;
            out += noise * 0.03f * tex;
            break;
        }
        case 4: // rai organ : tirettes + tremolo + clic
        {
            ph[7] += (3.0 + 4.0 * (double) width) / sr; wrap (ph[7]);
            const float trem = 1.f - width * 0.5f * (0.5f + 0.5f * std::sin ((float) ph[7] * twoPi));
            static const float H[6] = { 0.5f, 1.f, 2.f, 3.f, 4.f, 6.f };
            const float lv[6] = { tone, 0.9f, body, bright * 0.8f, bright, bright * 0.6f };
            for (int k = 0; k < 6; ++k)
            { ph[(size_t) k] += dt * H[k]; wrap (ph[(size_t) k]); out += lv[k] * std::sin ((float) ph[(size_t) k] * twoPi); }
            out = out * 0.28f * trem + noise * std::exp (-tsec * 400.f) * tex * 0.25f;
            break;
        }
        case 5: // chaabi strings : 5 saws + vibrato retarde
        {
            ph[6] += 5.4 / sr; wrap (ph[6]);
            const double vib = 1.0 + (double) (std::sin ((float) ph[6] * twoPi) * (0.002f + width * 0.006f) * juce::jmin (1.f, juce::jmax (0.f, tsec - 0.2f)));
            float s = 0.f;
            for (int k = 0; k < 5; ++k)
            {
                const double det = (1.0 + (double) (k - 2) * 0.003) * vib;
                ph[(size_t) k] += dt * det; wrap (ph[(size_t) k]);
                s += saw (ph[(size_t) k], dt * det);
            }
            s *= 0.2f;
            fl.set ((float) f * (1.5f + 9.f * bright), 0.7f, sr);
            f1.set (350.f + 500.f * tone, 2.f, sr);
            out = fl.process (s).lp * 0.8f + f1.process (s).bp * body * 0.6f + noise * 0.01f * tex;
            break;
        }
        case 6: // derbouka : sinus a chute de hauteur (doum) + claque (tak)
        {
            const float drop = 1.f + (0.3f + tone * 2.f) * std::exp (-tsec * 40.f);
            ph[0] += dt * (double) drop; wrap (ph[0]);
            const float env = std::exp (-tsec / (0.05f + body * 0.4f));
            out = std::sin ((float) ph[0] * twoPi) * (0.15f + env);
            f1.set (2500.f + 3000.f * bright, 3.f, sr);
            out += f1.process (noise).bp * std::exp (-tsec * 60.f) * (0.3f + width) * 0.8f;
            out += noise * std::exp (-tsec * 300.f) * tex * 0.3f;
            out *= 0.7f;
            break;
        }
        default: // 7 bendir sub : sub profond + grelots (snare du bendir)
        {
            const float knock = 1.f + tone * 1.5f * std::exp (-tsec * 25.f);
            ph[0] += dt * (double) knock; wrap (ph[0]);
            const float s1 = std::sin ((float) ph[0] * twoPi);
            out = std::tanh (s1 * (1.f + width * 3.f)) * 0.9f + body * 0.3f * std::sin (2.f * (float) ph[0] * twoPi);
            f1.set (6000.f, 2.f, sr);
            out += f1.process (noise).bp * std::exp (-tsec / (0.05f + bright * 0.3f)) * tex * 0.6f;
            out *= 0.8f;
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
