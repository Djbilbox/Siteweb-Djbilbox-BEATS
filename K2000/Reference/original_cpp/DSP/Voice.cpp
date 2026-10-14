#include "Voice.h"
#include "SynthSound.h"

namespace K2000Audio
{
    Voice::Voice()
    {
        osc.setWaveform(WaveformType::Sawtooth);
        filter.setFilterType(FilterType::LowPass24);
    }

    bool Voice::canPlaySound(juce::SynthesiserSound* sound)
    {
        return dynamic_cast<SynthSound*>(sound) != nullptr;
    }

    void Voice::setCurrentSampleRate(double newRate)
    {
        sampleRate = newRate;
        osc.setSampleRate(newRate);
        filter.setSampleRate(newRate);
        ampEnvelope.setSampleRate(newRate);
        lfo.setSampleRate(newRate);
    }

    float Voice::noteToFreq(int note) const
    {
        return 440.0f * std::pow(2.0f, static_cast<float>(note - 69) / 12.0f);
    }

    void Voice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* /*sound*/, int currentPitchWheelPosition)
    {
        currentNoteNumber = midiNoteNumber;
        currentVelocity = velocity;
        targetPitchHz = noteToFreq(midiNoteNumber);

        if (glideTime <= 0.001f || currentPitchHz <= 0.0f)
            currentPitchHz = targetPitchHz;

        pitchBendOffset = (static_cast<float>(currentPitchWheelPosition - 8192) / 8192.0f) * 2.0f; // +/- 2 semitones

        osc.setFrequency(currentPitchHz);
        ampEnvelope.noteOn(velocity);
        lfo.trigger();
    }

    void Voice::stopNote(float /*velocity*/, bool allowTailOff)
    {
        if (allowTailOff)
        {
            ampEnvelope.noteOff();
            lfo.release();
        }
        else
        {
            clearCurrentNote();
            ampEnvelope.reset();
            lfo.reset();
        }
    }

    void Voice::pitchWheelMoved(int newPitchWheelValue)
    {
        pitchBendOffset = (static_cast<float>(newPitchWheelValue - 8192) / 8192.0f) * 2.0f;
    }

    void Voice::controllerMoved(int controllerNumber, int newControllerValue)
    {
        if (controllerNumber == 1) // Mod Wheel CC1
        {
            modWheelValue = static_cast<float>(newControllerValue) / 127.0f;
        }
    }

    void Voice::setWaveform(WaveformType type)
    {
        osc.setWaveform(type);
    }

    void Voice::setFilterType(FilterType type)
    {
        filter.setFilterType(type);
    }

    void Voice::setFilterCutoff(float cutoffHz)
    {
        filterCutoffBase = cutoffHz;
    }

    void Voice::setFilterDrive(float drive)
    {
        filter.setDrive(drive);
    }

    void Voice::setAmpAttack(float s)
    {
        ampEnvelope.setAttackTime(s);
    }

    void Voice::setAmpDecay(float s)
    {
        ampEnvelope.setDecayTime(s);
    }

    void Voice::setAmpSustain(float s)
    {
        ampEnvelope.setSustainLevel(s);
    }

    void Voice::setVelocityBank(float b)
    {
        velocityBank = std::max(0.0f, std::min(b, 1.0f));
    }

    void Voice::setVelocityAdsr(float a)
    {
        velocityAdsr = std::max(0.0f, std::min(a, 1.0f));
    }

    void Voice::setGlideTime(float s)
    {
        glideTime = std::max(0.0f, std::min(s, 1.0f));
    }

    void Voice::setModWheel(float mw)
    {
        modWheelValue = std::max(0.0f, std::min(mw, 1.0f));
    }

    void Voice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {
        if (!isVoiceActive() && !ampEnvelope.isActive())
            return;

        // Glide step rate
        float glideAlpha = (glideTime > 0.001f)
            ? 1.0f - std::exp(-1.0f / static_cast<float>(sampleRate * glideTime * 0.3f))
            : 1.0f;

        // Velocity gain scaling
        float dynGain = (1.0f - velocityBank) + velocityBank * currentVelocity;
        float velEnvSens = (1.0f - velocityAdsr) + velocityAdsr * std::pow(currentVelocity, 1.5f);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Portamento transition
            currentPitchHz += (targetPitchHz - currentPitchHz) * glideAlpha;

            // Modulation computation
            float lfoEff = lfo.getModulatedOutput();
            if (modWheelValue > 0.0f)
                lfoEff += lfo.getRawSample() * modWheelValue * (1.0f - lfo.getAmount());

            // Pitch modulation
            float pitchSemiMod = pitchBendOffset;
            if (lfo.isPitchSwitchOn() || lfo.getDestination() == LfoDestination::Pitch)
            {
                pitchSemiMod += lfoEff * 2.0f;
            }

            float finalPitch = currentPitchHz * std::pow(2.0f, pitchSemiMod / 12.0f);
            osc.setFrequency(finalPitch);

            // Filter cutoff modulation
            float finalCutoff = filterCutoffBase;
            if (lfo.getDestination() == LfoDestination::Cutoff)
            {
                finalCutoff *= std::pow(2.0f, lfoEff * 3.5f);
            }
            filter.setCutoff(finalCutoff);

            // Generate oscillator sample
            float oscSample = osc.getNextSample();

            // Filter processing
            float filteredSample = filter.processSample(oscSample);

            // Envelope calculation
            float ampEnvVal = ampEnvelope.getNextSample() * velEnvSens;
            float voiceOut = filteredSample * ampEnvVal * dynGain * 0.35f;

            // Volume modulation
            if (lfo.getDestination() == LfoDestination::Volume)
            {
                voiceOut *= (1.0f - 0.5f * std::max(0.0f, lfoEff));
            }

            // Mix to output buffer (stereo)
            outputBuffer.addSample(0, startSample + sample, voiceOut);
            outputBuffer.addSample(1, startSample + sample, voiceOut);

            if (!ampEnvelope.isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }
}
