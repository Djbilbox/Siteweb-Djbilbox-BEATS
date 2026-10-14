#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"
#include "../Effects/DistortionEffect.h"
#include "../Effects/ChorusEffect.h"
#include "../Effects/DelayEffect.h"
#include "../Effects/ReverbEffect.h"
#include <vector>
#include <memory>

class Voice
{
public:
    Voice();
    ~Voice() = default;

    void setSampleRate(double sr);
    void noteOn(int midiNote, double velocity);
    void noteOff();
    // Renders straight to stereo. A unison stack is only worth having if its
    // copies can sit at different places across the image, so the voice can no
    // longer be a single mono signal panned afterwards.
    void render(double* left, double* right, int numSamples);

    bool isActive() const { return envelope.isActive(); }
    bool isReleasing() const { return envelope.getState() == ADSREnvelope::State::Release; }
    int getCurrentNote() const { return currentNote; }

    // Parameter setters
    void setOscillatorWaveform(Oscillator::Waveform wf);
    void setFilterCutoff(double hz);
    void setFilterResonance(double q);
    void setFilterType(StateVariableFilter::FilterType type);
    void setFilterEnvelopeDepth(double octaves);
    // Filter envelope (right "A D ASDR" block) — its own attack/decay/sustain,
    // release shared with the amp envelope.
    void setFilterAttack(double ms);
    void setFilterDecay(double ms);
    void setFilterSustain(double level);
    // 0 = fixed level whatever the key velocity, 1 = full velocity tracking.
    void setVelocitySensitivity(double amount01);
    void setAttack(double ms);
    void setDecay(double ms);
    void setSustain(double level);
    void setRelease(double ms);
    void setGlideTime(double ms);
    // Mono legato: retarget pitch without retriggering the envelopes.
    void glideTo(int midiNote);
    void setLFORate(double hz);
    void setLFOAmount(double amt);
    void setLFODestination(LFO::Destination dest);
    void setLFOWaveform(Oscillator::Waveform wf);
    void setPitchBend(double semitones);
    void setModWheel(double amount01);
    void setUnison(int voiceCount, double detuneCents, double stereoWidth);
    void hardReset() { envelope.reset(); filterEnv.reset(); filter.reset(); filterRight.reset(); held = false; }
    bool isHeld() const { return held; }
    void setHeld(bool h) { held = h; }

    static constexpr int kMaxUnison = 7;

private:
    // Unison stack. One oscillator per copy, each detuned a little and placed
    // at its own point across the stereo field — this is what gives a saw its
    // width and body instead of the thin single-oscillator tone.
    Oscillator unisonOsc[kMaxUnison];
    double unisonDetuneRatio[kMaxUnison] = { 1.0 };
    double unisonPanL[kMaxUnison] = { 1.0 };
    double unisonPanR[kMaxUnison] = { 1.0 };
    int unisonCount = 1;
    double unisonDetuneCents = 0.0;
    double unisonWidth = 0.0;
    double unisonGain = 1.0;

    // One filter per side so the stereo spread survives the filter stage.
    StateVariableFilter filter;
    StateVariableFilter filterRight;
    ADSREnvelope envelope;
    ADSREnvelope filterEnv;   // right-block "A D ASDR" envelope sweeping the cutoff
    LFO lfo;

    double velocitySensitivity = 0.4;
    bool held = false;

    void updateUnisonLayout();

    int currentNote = 60;
    double currentVelocity = 1.0;
    double glideTime = 0.0;
    double glideCounter = 0.0;
    double targetPitch = 440.0;
    double currentPitch = 440.0;

    double filterEnvDepthOctaves = 6.0;
    double lfoAmountValue = 0.0;
    LFO::Destination lfoDestination = LFO::Destination::Pitch;

    // Performance controls (pitch/mod wheels). The mod wheel adds LFO depth on
    // top of whatever the Amount knob is set to, rather than replacing it.
    double pitchBendSemitones = 0.0;
    double modWheelAmount = 0.0;

    double sampleRate = 44100.0;

    void updatePitch();
};

class SynthEngine
{
public:
    SynthEngine();
    ~SynthEngine() = default;

    void setSampleRate(double sr);
    void setBlockSize(int blockSize);

    void noteOn(int midiNote, double velocity);
    void noteOff(int midiNote);
    void allNotesOff();
    void reset();   // vide voix, filtres et queues d'effets
    void processBlock(float* leftOut, float* rightOut, int numSamples);

    // Synth mode
    void setPolyphonyMode(bool isPolyphonic);
    void setMaxVoices(int maxVoices);
    // Mono default with last-note priority: WARZONE808 is a bass instrument,
    // one 808 line at a time, gliding between held notes.

    // Parameter routing
    void setOscillatorWaveform(Oscillator::Waveform wf);
    void setFilterCutoff(double hz);
    void setFilterResonance(double q);
    void setFilterType(StateVariableFilter::FilterType type);
    void setFilterEnvelopeDepth(double octaves);
    void setAttack(double ms);
    void setDecay(double ms);
    void setSustain(double level);
    void setRelease(double ms);
    void setGlideTime(double ms);
    void setLFORate(double hz);
    void setLFOAmount(double amt);
    void setLFODestination(LFO::Destination dest);
    void setLFOWaveform(Oscillator::Waveform wf);
    void setMasterVolume(double gainLinear);
    void setMasterPan(double panAmount); // -1 to +1

    // Filter envelope + velocity (right-hand block of the faceplate)
    void setFilterAttack(double ms);
    void setFilterDecay(double ms);
    void setFilterSustain(double level);
    void setVelocitySensitivity(double amount01);
    bool anyVoiceActive() const;

    // Performance controls
    void setPitchBend(double semitones);
    void setModWheel(double amount01);

    // Unison: how many detuned copies each note plays, how far apart they
    // sit in cents, and how wide they are spread across the image.
    void setUnison(int voiceCount, double detuneCents, double stereoWidth);

    // Effects
    void setDistortionDrive(double db);
    void setDistortionTone(double hz);
    void setDistortionMix(double mix);

    void setChorusRate(double hz);
    void setChorusAmount(double amt);
    void setChorusMix(double mix);

    void setDelayTime(double ms);
    void setDelayFeedback(double feedback);
    void setDelayMix(double mix);

    void setReverbRoom(double room);
    void setReverbDamping(double damping);
    void setReverbWidth(double width);
    void setReverbMix(double mix);

private:
    std::vector<std::unique_ptr<Voice>> voices;
    // Held-note stack for mono last-note priority (top = most recent).
    std::vector<int> heldNotes;
    int nextVoiceIndex = 0;
    bool isPolyphonic = false;
    int maxVoices = 1;

    double sampleRate = 44100.0;
    double masterVolume = 1.0;
    double masterPan = 0.0;
    double pitchBendSemitones = 0.0;
    double modWheelAmount = 0.0;
    int unisonVoices = 1;
    double unisonDetune = 0.0;
    double unisonWidth = 0.0;

    std::unique_ptr<DistortionEffect> distortion;
    std::unique_ptr<ChorusEffect> chorus;
    std::unique_ptr<DelayEffect> delay;
    std::unique_ptr<ReverbEffect> reverb;

    std::vector<double> voiceL, voiceR;   // pre-allocated render scratch (no allocation in processBlock)
    std::vector<double> mixBuffer;
    std::vector<double> mixBufferR;
    std::vector<double> sideBuffer;
    std::vector<double> effectBuffer;

    Voice* allocateVoice(int midiNote);
};
