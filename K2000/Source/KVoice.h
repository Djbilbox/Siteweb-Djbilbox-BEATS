#pragma once
#include "Params.h"
#include <cmath>

namespace K2k
{
// Filtre en echelle 4 poles (TPT / ZDF), tanh a l'entree : LP 24 dB, auto-oscillation controlee.
struct Ladder
{
    float G = 0.f, k = 0.f, s[4] = { 0, 0, 0, 0 };
    void set (float fc, float res, double sr)
    {
        const float wc = std::tan (juce::MathConstants<float>::pi * juce::jlimit (15.f, (float) sr * 0.45f, fc) / (float) sr);
        G = wc / (1.f + wc); k = res * 3.9f;
    }
    float process (float x)
    {
        const float a = 1.f - G;
        const float G2 = G * G, G3 = G2 * G, G4 = G3 * G;
        const float S = G3 * a * s[0] + G2 * a * s[1] + G * a * s[2] + a * s[3];
        float u = (x * (1.f + 0.5f * k) - k * S) / (1.f + k * G4);
        u = std::tanh (u);
        float y = u;
        for (int i = 0; i < 4; ++i) { const float v = (y - s[i]) * G; const float o = v + s[i]; s[i] = o + v; y = o; }
        return y;
    }
    void reset() { s[0] = s[1] = s[2] = s[3] = 0.f; }
};

// Filtre a variables d'etat 12 dB (LP / BP / HP).
struct SVF
{
    float g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
    struct Out { float lp, bp, hp; };
    void set (float fc, float q, double sr)
    {
        g = std::tan (juce::MathConstants<float>::pi * juce::jlimit (15.f, (float) sr * 0.45f, fc) / (float) sr);
        k = 1.f / juce::jmax (0.3f, q);
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

struct SharedState
{
    std::atomic<float> lastFreq { 0.f };     // portamento
    std::atomic<float> lfoChorus { 0.f };    // valeur du LFO quand la destination est Chorus
    std::atomic<float> lastMod { 0.f };      // latest mod wheel (CC1) value 0..1, kept even when no voice is playing
};

class KSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class KVoice : public juce::SynthesiserVoice
{
public:
    KVoice (const ParamPtrs& pp, SharedState& ss) : P (pp), shared (ss) {}

    bool enabled = true;     // Poly 8 : les voix 8..15 sont desactivees
    bool canPlaySound (juce::SynthesiserSound* s) override { return enabled && dynamic_cast<KSound*> (s) != nullptr; }

    void setCurrentPlaybackSampleRate (double sr) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (sr);
        amp.setSampleRate (sr); fenvAdsr.setSampleRate (sr); lfoAdsr.setSampleRate (sr);
    }

    bool isSounding() const { return amp.isActive(); }

    // ---- appels du synthetiseur (modes polyphoniques) ----
    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override
    {
        midiBend = (pitchWheel - 8192) / 8192.f;
        begin (midiNote, velocity);
    }
    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) release(); else kill();
    }
    void pitchWheelMoved (int v) override { midiBend = (v - 8192) / 8192.f; }
    void controllerMoved (int cc, int v) override { if (cc == 1) midiMod = v / 127.f; else if (cc == 121) midiMod = 0.f; }
    void aftertouchChanged (int) override {}
    void channelPressureChanged (int) override {}

    // ---- appels du processeur (modes mono) ----
    void monoNote (int midiNote, float velocity, bool legato)
    {
        if (legato && amp.isActive()) { targetFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote); vel = velocity; shared.lastFreq = targetFreq; note = midiNote; return; }
        begin (midiNote, velocity);
    }
    void release() { amp.noteOff(); fenvAdsr.noteOff(); lfoAdsr.noteOff(); held = false; }
    void kill() { amp.reset(); fenvAdsr.reset(); lfoAdsr.reset(); held = false; if (isVoiceActive()) clearCurrentNote(); }
    void setMidiBend (float b) { midiBend = b; }
    void setMidiMod (float m) { midiMod = m; }
    int currentNote() const { return note; }

    using juce::SynthesiserVoice::renderNextBlock;
    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (! amp.isActive()) { if (isVoiceActive()) clearCurrentNote(); return; }
        const double sr = getSampleRate();
        const int wv = (int) P (wave), src = (int) P (lfoSrc), dst = (int) P (lfoDst), ft = (int) P (ftype);
        const float lfoRate = P (lfoRate), amount = P (lfoAmount), mw = juce::jmax (midiMod, P (modw));
        const bool lfoToPitch = P (lfoPitch) > 0.5f;
        const float cut = P (cutoff), resA = P (res), envAmt = P (fenv), drv = P (drive);
        const float detuneAmt = P (detune), gl = P (glide), uiBend = P (bend);
        const float bank = P (velBank), vsens = P (velAdsr);
        amp.setParameters ({ P (ampAtk), P (ampDec), P (ampSus), P (ampRel) });
        fenvAdsr.setParameters ({ 0.002f, juce::jlimit (0.03f, 3.f, P (ampDec)), 0.f, 0.1f });
        lfoAdsr.setParameters ({ P (lfoAtk), P (lfoDec), P (lfoSus), P (lfoRel) });
        if (fresh) { smCut = cut; fresh = false; }
        const float smK = 1.f - std::exp (-1.f / (0.006f * (float) sr));
        const float glideCoef = gl < 0.002f ? 1.f : 1.f - std::exp (-1.f / (gl * (float) sr * 0.3f));
        const float dynGain = (1.f - bank) + bank * vel;
        const float envSens = (1.f - vsens) + vsens * std::pow (vel, 1.5f);
        float* L = out.getWritePointer (0);
        float* R = out.getNumChannels() > 1 ? out.getWritePointer (1) : nullptr;

        for (int i = 0; i < num; ++i)
        {
            curFreq += (targetFreq - curFreq) * glideCoef;
            smCut += (cut - smCut) * smK;

            // LFO (source x enveloppe LFO x (amount + molette))
            lfoPh += lfoRate / sr;
            if (lfoPh >= 1.0) { lfoPh -= 1.0; shVal = rng.nextFloat() * 2.f - 1.f; }
            const float p = (float) lfoPh; float raw;
            switch (src) {
                case 0: raw = std::sin (juce::MathConstants<float>::twoPi * p); break;
                case 1: raw = 4.f * std::abs (p - 0.5f) - 1.f; break;
                case 2: raw = p < 0.5f ? 1.f : -1.f; break;
                case 3: raw = 2.f * p - 1.f; break;
                case 4: raw = 1.f - 2.f * p; break;
                default: raw = shVal; break;
            }
            const float lfoEff = raw * lfoAdsr.getNextSample() * juce::jlimit (0.f, 1.f, amount + mw * (1.f - amount));
            if (dst == 4) shared.lfoChorus = lfoEff;

            float semis = (uiBend + midiBend) * 2.f;
            if (dst == 1) semis += lfoEff;
            if (lfoToPitch) semis += lfoEff * 0.5f;
            const double freq = (double) curFreq * std::exp2 (semis / 12.f);
            const double dt = freq / sr;

            // oscillateur
            const double pw = dst == 2 ? 0.5 + 0.45 * (double) lfoEff : 0.5;
            float s = oscillator (wv, dt, pw, detuneAmt);

            // filtre
            const float fe = fenvAdsr.getNextSample();
            if ((i & 7) == 0)
            {
                const float fc = smCut * std::exp2 ((dst == 0 ? lfoEff * 4.f : 0.f) + envAmt * fe * 5.f + mw * 0.8f);
                ladder.set (fc, ft == 4 ? juce::jmin (1.f, resA * 1.1f + 0.25f) : resA, sr);
                svf.set (fc, 0.6f + resA * 9.f, sr);
            }
            float x = s * (1.f + drv * 3.f);
            if (drv > 0.01f || ft == 4) x = std::tanh (x * (ft == 4 ? 1.8f : 1.f));
            float y;
            switch (ft) {
                case 0: y = ladder.process (x); break;
                case 1: y = svf.process (x).lp; break;
                case 2: y = svf.process (x).bp * 1.5f; break;
                case 3: y = svf.process (x).hp; break;
                default: y = std::tanh (ladder.process (x) * 1.4f) * 0.8f; break;
            }
            { const float o = y - dcX + dcR * dcY; dcX = y; dcY = o; y = o; }   // bloqueur DC

            float a = amp.getNextSample() * envSens * dynGain;
            if (dst == 3) a *= 1.f - 0.5f * juce::jmax (0.f, lfoEff);
            y *= a * 0.85f;

            const float pn = dst == 5 ? lfoEff : pan;
            const float gL = std::cos (juce::MathConstants<float>::pi * 0.25f * (1.f + pn)), gR = std::sin (juce::MathConstants<float>::pi * 0.25f * (1.f + pn));
            L[start + i] += y * gL;
            if (R) R[start + i] += y * gR;
            if (! amp.isActive()) { if (isVoiceActive()) clearCurrentNote(); break; }
        }
    }

private:
    static float blep (double t, double dt)
    {
        if (t < dt) { t /= dt; return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.f;
    }
    static float sawBL (double ph, double dt) { return (float) (2.0 * ph - 1.0) - blep (ph, dt); }
    static float pulseBL (double ph, double dt, double pw)
    {
        double p2 = ph + (1.0 - pw); p2 -= std::floor (p2);
        return sawBL (ph, dt) - sawBL (p2, dt);
    }
    static void wrap (double& p) { p -= std::floor (p); }

    float oscillator (int wv, double dt, double pw, float detuneAmt)
    {
        const float twoPi = juce::MathConstants<float>::twoPi;
        float s = 0.f;
        switch (wv)
        {
            case 0: ph[0] += dt; wrap (ph[0]); s = sawBL (ph[0], dt); break;
            case 1: ph[0] += dt; wrap (ph[0]); s = pulseBL (ph[0], dt, pw) * 0.7f; break;
            case 2: ph[0] += dt; wrap (ph[0]); s = 4.f * (float) std::abs (ph[0] - 0.5) - 1.f; break;
            case 3: ph[0] += dt; wrap (ph[0]); s = std::sin ((float) ph[0] * twoPi); break;
            case 4:
            {
                static const double off[7] = { -1.0, -0.62, -0.28, 0.0, 0.31, 0.64, 1.0 };
                for (int k = 0; k < 7; ++k)
                {
                    const double r = 1.0 + off[k] * (double) detuneAmt * 0.02;
                    ph[(size_t) k] += dt * r; wrap (ph[(size_t) k]);
                    s += sawBL (ph[(size_t) k], dt * r) * (k == 3 ? 1.f : 0.7f);
                }
                s *= 0.28f; break;
            }
            default: // 80s brass : saw + impulsion asymetrique + harmonique paire
            {
                ph[0] += dt; wrap (ph[0]); ph[1] += dt * (1.0 + 0.003 * (double) detuneAmt); wrap (ph[1]);
                s = 0.45f * sawBL (ph[0], dt) + 0.35f * pulseBL (ph[1], dt, juce::jlimit (0.15, 0.85, pw - 0.15)) + 0.2f * std::sin (2.f * (float) ph[0] * twoPi);
                break;
            }
        }
        return s;
    }

    void begin (int midiNote, float velocity)
    {
        note = midiNote; held = true; vel = velocity;
        midiMod = shared.lastMod.load();   // idle voices may have missed CC1: start from the latest wheel position
        targetFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
        const float last = shared.lastFreq.load();
        curFreq = (P (glide) > 0.002f && last > 0.f) ? last : targetFreq;
        shared.lastFreq = targetFreq;
        for (auto& p : ph) p = rng.nextDouble();
        lfoPh = rng.nextDouble() * 0.0; shVal = rng.nextFloat() * 2.f - 1.f;
        pan = (rng.nextFloat() - 0.5f) * 0.4f;
        fresh = true; dcX = dcY = 0.f; dcR = 1.f - juce::MathConstants<float>::twoPi * 12.f / (float) getSampleRate();
        ladder.reset(); svf.reset();
        amp.noteOn(); fenvAdsr.noteOn(); lfoAdsr.noteOn();
    }

    const ParamPtrs& P;
    SharedState& shared;
    juce::ADSR amp, fenvAdsr, lfoAdsr;
    juce::Random rng;
    Ladder ladder; SVF svf;
    double ph[7] {}, lfoPh = 0.0;
    float targetFreq = 440.f, curFreq = 440.f, vel = 1.f, pan = 0.f, shVal = 0.f;
    float midiBend = 0.f, midiMod = 0.f, smCut = 3200.f;
    float dcX = 0.f, dcY = 0.f, dcR = 0.998f;
    int note = -1;
    bool fresh = true, held = false;
};
}
