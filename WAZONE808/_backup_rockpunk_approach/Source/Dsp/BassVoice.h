#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <cmath>

#include "Svf.h"

namespace wz
{

//==============================================================================
// WARZONE808 bass voice.
// ---------------------
// Two oscillators: a pure sine SUB (the 808 body) and a secondary
// saturable oscillator (the grain). WAVEFORM selects the engine:
//   0 PURE SUB     - sine only, clean 808
//   1 SATURATED    - sine + tanh-saturated sine (harmonics, keeps the sub)
//   2 TRIANGLE     - sub + triangle grit
//   3 SAW DRILL    - sub + saw (PolyBLEP-free band-limited-ish ramp)
// Glide: exponential portamento between notes (trap 808 slides).
// Amp envelope A/D/S, fixed smoothed release (see PARAMETER_MAP).
// Velocity scales the envelope peak (velSens 0..1).
// The global LFO modulator is fed per-sample by the processor: pitch (semitones),
// cutoff offset (Hz multiplier) and volume (0..1 multiplier).
//==============================================================================
class BassVoice : public juce::SynthesiserVoice
{
public:
    //==========================================================================
    struct Settings
    {
        int   waveform = 0;       // engine selection
        float glideMs = 80.0f;    // 0 = instant
        float filterHz = 6000.0f; // base cutoff
        int   filterMode = 0;     // Svf mode
        float ampA = 0.005f, ampD = 0.4f, ampS = 0.7f; // seconds / 0..1
        float velSens = 0.4f;
        float lfoPitchSemi = 0.0f;  // pre-scaled LFO pitch depth (semitones)
        float modWheelPitchSemi = 0.0f; // mod wheel adds vibrato depth
        float bendSemi = 0.0f;     // pitch bend wheel, -2..+2
    };

    bool canPlaySound (juce::SynthesiserSound*) override { return true; }

    void setSampleRate (double sr)
    {
        SynthesiserVoice::setCurrentPlaybackSampleRate (sr);
        filter.setSampleRate (sr);
        filter.reset();
    }

    void setSettings (const Settings& s) noexcept { settings = s; }

    //==========================================================================
    void startVoice (int midiNote, float velocity)
    {
        level = velocity;
        currentNote = midiNote;

        // glide starts from wherever the last note left the pitch
        targetFreq = midiToFreq ((float) midiNote);
        if (! voiceActive)
            currentFreq = targetFreq;

        envPos = 0.0f;
        envStage = Attack;
        voiceActive = true;

        filter.reset();
    }

    void stopVoice (float releaseSec)
    {
        releaseSecVal = juce::jmax (0.01f, releaseSec);
        envStage = Release;
        envReleaseStart = envLevel;
        envPos = 0.0f;
    }

    void glideTo (int midiNote)
    {
        currentNote = midiNote;
        targetFreq = midiToFreq ((float) midiNote);
    }

    bool isVoiceActive() const noexcept { return voiceActive; }
    int getCurrentNote() const noexcept { return currentNote; }

    //==========================================================================
    void renderNextBlock (juce::AudioBuffer<float>& output, int startSample, int numSamples) override
    {
        if (! voiceActive)
            return;

        auto* out = output.getWritePointer (0);
        auto* outR = output.getNumChannels() > 1 ? output.getWritePointer (1) : nullptr;

        const double sr = getSampleRate() > 0 ? getSampleRate() : 48000.0;
        const Settings& st = settings;

        const float glideCoef = st.glideMs <= 0.5f
            ? 1.0f
            : std::exp (-1.0f / (sr * st.glideMs * 0.001f));

        const float velGain = 1.0f - st.velSens + st.velSens * level;

        for (int i = 0; i < numSamples; ++i)
        {
            // ---- glide ----
            currentFreq += (targetFreq - currentFreq) * (1.0f - glideCoef);

            // ---- modulation (LFO + wheels), fed by the processor ----
            const float modPitch = lfoValue * (st.lfoPitchSemi + st.modWheelPitchSemi);
            const float freq = currentFreq * std::pow (2.0f, (modPitch + st.bendSemi) / 12.0f);

            // ---- oscillators ----
            phase += (float) (freq / sr);
            if (phase >= 1.0f) phase -= 1.0f;

            const float sub = std::sin (2.0f * juce::MathConstants<float>::pi * phase);

            float grain = 0.0f, grainMix = 0.0f;
            switch (st.waveform)
            {
                case 0:  grainMix = 0.0f; break;                       // pure sub
                case 1:  grain = std::tanh (sub * 2.6f) * 0.8f; grainMix = 0.55f; break;
                case 2:  grain = 2.0f * phase - 1.0f;  grainMix = 0.40f; break;
                case 3:  grain = (2.0f * phase - 1.0f) * 0.7f; grainMix = 0.45f; break;
                default: break;
            }

            float sample = sub + grain * grainMix;

            // ---- amplitude envelope ----
            envLevel = nextEnvelopeLevel (sr);

            // ---- volume modulation from the LFO (destination VOLUME) ----
            const float vca = envLevel * velGain * volMod;

            // ---- filter (with LFO cutoff modulation) ----
            float cutoff = st.filterHz * cutoffMod;
            if (cutoff < 20.0f) cutoff = 20.0f;
            sample = filter.process (sample * vca, st.filterMode, cutoff);

            // soft per-voice ceiling keeps the mix of 8 voices bounded
            sample = std::tanh (sample * 0.9f) * 0.95f;

            out[startSample + i] += sample;
            if (outR != nullptr)
                outR[startSample + i] += sample;
        }
    }

    //==========================================================================
    // Called by the processor once per block with the shared LFO value and
    // per-destination modulation paths (only the active destination != neutral).
    void setModulation (float lfo01, float pitchDepthSemi, float cutoffMultiplier, float volumeMult) noexcept
    {
        lfoValue = lfo01;
        lfoPitchDepth = pitchDepthSemi;
        cutoffMod = juce::jlimit (0.02f, 16.0f, cutoffMultiplier);
        volMod = juce::jlimit (0.0f, 1.0f, volumeMult);
        settings.lfoPitchSemi = lfoPitchDepth;
    }

    void setBend (float semitones) noexcept { settings.bendSemi = semitones; }
    void setModWheel (float semitoneDepth) noexcept { settings.modWheelPitchSemi = semitoneDepth; }

    void setCurrentPlaybackSampleRate (double sr) override
    {
        SynthesiserVoice::setCurrentPlaybackSampleRate (sr);
        filter.setSampleRate (sr);
    }

private:
    //==========================================================================
    float nextEnvelopeLevel (double sr)
    {
        switch (envStage)
        {
            case Attack:
                envPos += 1.0f / (float) (sr * juce::jmax (0.0015f, settings.ampA));
                if (envPos >= 1.0f) { envPos = 0.0f; envStage = Decay; return 1.0f; }
                return envPos * envPos; // slightly convex attack = punchy 808

            case Decay:
            {
                const float s = settings.ampS;
                envLevel += (s - envLevel) * (1.0f - std::exp (-1.0f / (sr * juce::jmax (0.005f, settings.ampD) * 0.25f)));
                if (std::fabs (envLevel - s) < 0.0008f) envStage = Sustain;
                return envLevel;
            }

            case Sustain:
                return settings.ampS;

            case Release:
                envPos += 1.0f / (float) (sr * releaseSecVal);
                if (envPos >= 1.0f) { voiceActive = false; return 0.0f; }
                return envReleaseStart * (1.0f - envPos);

            default:
                return 0.0f;
        }
    }

    static float midiToFreq (float note) noexcept
    {
        return 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
    }

    Settings settings;

    Svf filter;

    float phase = 0.0f;
    float currentFreq = 55.0f, targetFreq = 55.0f;
    int   currentNote = 33;

    enum EnvStage { Attack, Decay, Sustain, Release, Idle };
    EnvStage envStage = Idle;
    float envPos = 0.0f, envLevel = 0.0f, envReleaseStart = 0.0f;
    float releaseSecVal = 0.12f;

    float level = 1.0f;
    bool  voiceActive = false;

    float lfoValue = 0.0f, lfoPitchDepth = 0.0f;
    float cutoffMod = 1.0f, volMod = 1.0f;
};

//==============================================================================
// Trivial sound: every voice can play every note.
//==============================================================================
struct BassSound final : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

} // namespace wz
