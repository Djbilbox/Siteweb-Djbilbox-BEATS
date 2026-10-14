#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"

namespace K2000Audio
{
    class Voice : public juce::SynthesiserVoice
    {
    public:
        Voice();
        ~Voice() override = default;

        bool canPlaySound(juce::SynthesiserSound* sound) override;
        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int newPitchWheelValue) override;
        void controllerMoved(int controllerNumber, int newControllerValue) override;
        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

        void setCurrentSampleRate(double newRate) override;

        // Parameter Setters
        void setWaveform(WaveformType type);
        void setFilterType(FilterType type);
        void setFilterCutoff(float cutoffHz);
        void setFilterDrive(float drive);
        void setAmpAttack(float s);
        void setAmpDecay(float s);
        void setAmpSustain(float s);
        void setVelocityBank(float b);
        void setVelocityAdsr(float a);
        void setGlideTime(float s);
        void setModWheel(float mw);

        LowFrequencyOscillator& getLfo() { return lfo; }

    private:
        PolyBlepOscillator osc;
        MultimodeFilter filter;
        AdsrEnvelope ampEnvelope;
        LowFrequencyOscillator lfo;

        double sampleRate = 44100.0;
        int currentNoteNumber = -1;
        float currentVelocity = 1.0f;
        float pitchBendOffset = 0.0f; // in semitones
        float modWheelValue = 0.0f;

        // Portamento / Glide
        float currentPitchHz = 440.0f;
        float targetPitchHz = 440.0f;
        float glideTime = 0.05f;

        // Dynamics
        float velocityBank = 0.75f;
        float velocityAdsr = 0.4f;
        float filterCutoffBase = 3200.0f;

        float noteToFreq(int note) const;
        void updatePitch();
    };
}
