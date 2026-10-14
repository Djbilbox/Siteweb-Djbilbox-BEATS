#include "SynthEngine.h"
#include <algorithm>
#include <cmath>

// Voice Implementation

Voice::Voice()
{
    setSampleRate(44100.0);
    // Fixed unity scale: modulateCutoff() is always fed a value already expressed
    // in octaves (envelope depth + LFO combined), so the filter's internal
    // "depth" multiplier stays at 1.0 and all scaling happens in Voice::render().
    filter.setModulationDepth(1.0);
    filterRight.setModulationDepth(1.0);
    updateUnisonLayout();
}

void Voice::setSampleRate(double sr)
{
    sampleRate = sr;
    for (auto& o : unisonOsc)
        o.setSampleRate(sr);
    filter.setSampleRate(sr);
    filterRight.setSampleRate(sr);
    envelope.setSampleRate(sr);
    filterEnv.setSampleRate(sr);
    lfo.setSampleRate(sr);
}

void Voice::setUnison(int voiceCount, double detuneCents, double stereoWidth)
{
    unisonCount = juce::jlimit(1, kMaxUnison, voiceCount);
    unisonDetuneCents = juce::jlimit(0.0, 60.0, detuneCents);
    unisonWidth = juce::jlimit(0.0, 1.0, stereoWidth);
    updateUnisonLayout();
}

void Voice::updateUnisonLayout()
{
    for (int i = 0; i < kMaxUnison; ++i)
    {
        // Spread the copies evenly either side of the played pitch, and place
        // each one at the matching point across the image, so detune and
        // position line up the way a supersaw's do.
        const double t = unisonCount <= 1 ? 0.0
                       : (2.0 * i / (double) (unisonCount - 1)) - 1.0;   // -1..+1

        unisonDetuneRatio[i] = std::pow(2.0, (t * unisonDetuneCents) / 1200.0);

        const double pan = t * unisonWidth;                              // -1..+1
        unisonPanL[i] = std::cos((pan + 1.0) * juce::MathConstants<double>::pi * 0.25);
        unisonPanR[i] = std::sin((pan + 1.0) * juce::MathConstants<double>::pi * 0.25);
    }

    // Detuned copies drift in and out of phase, so they periodically line up
    // and sum far above what an uncorrelated sqrt(n) would predict — measured
    // peaks of 1.67 for a 7-voice stack, which then hit the master saturator on
    // every note. The slightly steeper exponent keeps the worst-case alignment
    // just under full scale.
    unisonGain = 1.0 / std::pow((double) unisonCount, 0.8);
}

void Voice::noteOn(int midiNote, double velocity)
{
    currentNote = juce::jlimit(0, 127, midiNote);
    currentVelocity = juce::jlimit(0.0, 1.0, velocity);

    // Calculate target pitch in Hz
    targetPitch = 440.0 * std::pow(2.0, (currentNote - 69) / 12.0);

    if (glideTime <= 0.0)
    {
        currentPitch = targetPitch;
        for (int u = 0; u < unisonCount; ++u)
            unisonOsc[u].setFrequency(currentPitch * unisonDetuneRatio[u]);
    }
    else
    {
        glideCounter = 0.0;
    }

    held = true;
    envelope.noteOn();
    filterEnv.noteOn();
}

void Voice::glideTo(int midiNote)
{
    currentNote = juce::jlimit(0, 127, midiNote);
    targetPitch = 440.0 * std::pow(2.0, (currentNote - 69) / 12.0);
    // No envelope retrigger: the amp/filter envelopes keep running and only
    // the pitch glides — classic mono-legato 808 behaviour.
}

void Voice::noteOff()
{
    held = false;
    envelope.noteOff();
    filterEnv.noteOff();
}

void Voice::render(double* left, double* right, int numSamples)
{
    if (!isActive())
    {
        std::fill(left, left + numSamples, 0.0);
        std::fill(right, right + numSamples, 0.0);
        return;
    }

    const bool lfoToPitch  = (lfoDestination == LFO::Destination::Pitch);
    const bool lfoToFilter = (lfoDestination == LFO::Destination::FilterCutoff);
    const bool lfoToAmp    = (lfoDestination == LFO::Destination::Amplitude);

    for (int i = 0; i < numSamples; ++i)
    {
        // 1) Glide toward target pitch
        if (glideTime > 0.0 && std::abs(currentPitch - targetPitch) > 0.01)
        {
            double glideRateSamples = juce::jmax(1.0, (glideTime / 1000.0) * sampleRate);
            currentPitch += (targetPitch - currentPitch) / glideRateSamples;
        }
        else
        {
            currentPitch = targetPitch;
        }

        // 2) LFO — single tick per sample, shared across whichever destination is active
        const double lfoRaw = lfo.tick(); // -1..+1

        // 3) Oscillator (pitch modulated by LFO if routed there)
        double pitchHz = currentPitch;

        // Pitch wheel: +/- 2 semitones, classic synth range
        if (pitchBendSemitones != 0.0)
            pitchHz *= std::pow(2.0, pitchBendSemitones / 12.0);

        // Mod wheel adds LFO depth on top of the Amount knob
        const double effectiveLfoAmount = juce::jlimit(0.0, 1.0, lfoAmountValue + modWheelAmount);

        // Vibrato depth, in semitones. This used to be a whole octave at full
        // amount, so a preset asking for a touch of movement (0.2-0.5) got 2 to
        // 6 semitones of wobble — not vibrato but audible detuning, which is
        // what made held notes sound out of tune whatever the patch.
        constexpr double kMaxVibratoSemitones = 2.0;
        if (lfoToPitch && effectiveLfoAmount > 0.0)
            pitchHz *= std::pow(2.0, (lfoRaw * effectiveLfoAmount * kMaxVibratoSemitones) / 12.0);
        // 3b) Unison stack — each copy detuned around the played pitch and
        // placed at its own spot across the image.
        double oscL = 0.0, oscR = 0.0;
        for (int u = 0; u < unisonCount; ++u)
        {
            unisonOsc[u].setFrequency(pitchHz * unisonDetuneRatio[u]);
            const double s = unisonOsc[u].tick();
            oscL += s * unisonPanL[u];
            oscR += s * unisonPanR[u];
        }
        oscL *= unisonGain;
        oscR *= unisonGain;

        // 4) Envelope
        const double envSample = envelope.tick();
        const double filterEnvSample = filterEnv.tick();

        // 5) Filter cutoff modulation: envelope depth (always active) + LFO (if routed there)
        // Filter sweep depth, in octaves — 12 octaves at full amount, as this
        // was, slams the cutoff into both end stops on every LFO cycle.
        double filterModOctaves = filterEnvSample * filterEnvDepthOctaves;
        if (lfoToFilter)
            filterModOctaves += lfoRaw * effectiveLfoAmount * 3.0;
        filter.modulateCutoff(filterModOctaves);
        filterRight.modulateCutoff(filterModOctaves);
        const double filteredL = filter.tick(oscL);
        const double filteredR = filterRight.tick(oscR);

        // 6) Amplitude (envelope always, LFO tremolo if routed there)
        double amp = envSample;
        if (lfoToAmp && effectiveLfoAmount > 0.0)
            amp *= 1.0 + (lfoRaw * effectiveLfoAmount * 0.5);

        const double velGain = 1.0 - velocitySensitivity + velocitySensitivity * currentVelocity;
        const double gain = amp * velGain;
        left[i]  = filteredL * gain;
        right[i] = filteredR * gain;
    }
}

void Voice::setPitchBend(double semitones)
{
    pitchBendSemitones = juce::jlimit(-12.0, 12.0, semitones);
}

void Voice::setModWheel(double amount01)
{
    modWheelAmount = juce::jlimit(0.0, 1.0, amount01);
}

void Voice::setOscillatorWaveform(Oscillator::Waveform wf)
{
    for (auto& o : unisonOsc)
        o.setWaveform(wf);
}

void Voice::setFilterCutoff(double hz)
{
    filter.setCutoffFrequency(hz);
    filterRight.setCutoffFrequency(hz);
}

void Voice::setFilterResonance(double q)
{
    filter.setResonance(q);
    filterRight.setResonance(q);
}

void Voice::setFilterType(StateVariableFilter::FilterType type)
{
    filter.setFilterType(type);
    filterRight.setFilterType(type);
}

void Voice::setFilterEnvelopeDepth(double octaves)
{
    filterEnvDepthOctaves = octaves;
}

void Voice::setAttack(double ms)
{
    envelope.setAttackTime(ms);
}

void Voice::setDecay(double ms)
{
    envelope.setDecayTime(ms);
}

void Voice::setSustain(double level)
{
    envelope.setSustainLevel(level);
}

void Voice::setRelease(double ms)
{
    envelope.setReleaseTime(ms);
    filterEnv.setReleaseTime(ms);
}

void Voice::setFilterAttack(double ms)  { filterEnv.setAttackTime(ms); }
void Voice::setFilterDecay(double ms)   { filterEnv.setDecayTime(ms); }
void Voice::setFilterSustain(double level) { filterEnv.setSustainLevel(level); }
void Voice::setVelocitySensitivity(double amount01) { velocitySensitivity = juce::jlimit(0.0, 1.0, amount01); }

void Voice::setGlideTime(double ms)
{
    glideTime = ms;
}

void Voice::setLFORate(double hz)
{
    lfo.setFrequency(hz);
}

void Voice::setLFOAmount(double amt)
{
    lfoAmountValue = juce::jlimit(0.0, 1.0, amt);
    lfo.setAmount(lfoAmountValue);
}

void Voice::setLFODestination(LFO::Destination dest)
{
    lfoDestination = dest;
    lfo.setDestination(dest);
}

void Voice::setLFOWaveform(Oscillator::Waveform wf)
{
    lfo.setWaveformType(wf);
}

// SynthEngine Implementation

SynthEngine::SynthEngine()
{
    setSampleRate(44100.0);

    // Create voices
    for (int i = 0; i < 16; ++i)
    {
        voices.push_back(std::make_unique<Voice>());
    }
    setBlockSize(512);

    // Create effects
    distortion = std::make_unique<DistortionEffect>();
    chorus = std::make_unique<ChorusEffect>();
    delay = std::make_unique<DelayEffect>();
    reverb = std::make_unique<ReverbEffect>();
}

void SynthEngine::setSampleRate(double sr)
{
    sampleRate = sr;

    for (auto& voice : voices)
    {
        voice->setSampleRate(sr);
    }

    if (distortion) distortion->setSampleRate(sr);
    if (chorus) chorus->setSampleRate(sr);
    if (delay) delay->setSampleRate(sr);
    if (reverb) reverb->setSampleRate(sr);
}

void SynthEngine::setBlockSize(int blockSize)
{
    mixBuffer.assign((size_t) blockSize, 0.0);
    mixBufferR.assign((size_t) blockSize, 0.0);
    sideBuffer.assign((size_t) blockSize, 0.0);
    effectBuffer.assign((size_t) blockSize, 0.0);
    voiceL.assign((size_t) blockSize, 0.0);
    voiceR.assign((size_t) blockSize, 0.0);
}

void SynthEngine::noteOn(int midiNote, double velocity)
{
    const double velNormalized = velocity / 127.0;
    if (!isPolyphonic)
    {
        // Mono, last-note priority. A note played while another is held glides
        // (legato, envelopes keep running); a note played from silence retriggers.
        heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), midiNote), heldNotes.end());
        const bool legato = !heldNotes.empty() && voices[0]->isActive() && voices[0]->isHeld();
        heldNotes.push_back(midiNote);
        if (legato) voices[0]->glideTo(midiNote);
        else        voices[0]->noteOn(midiNote, velNormalized);
        return;
    }
    Voice* voice = allocateVoice(midiNote);
    if (voice)
        voice->noteOn(midiNote, velNormalized);
}

void SynthEngine::noteOff(int midiNote)
{
    if (!isPolyphonic)
    {
        const bool wasTop = !heldNotes.empty() && heldNotes.back() == midiNote;
        heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), midiNote), heldNotes.end());
        if (heldNotes.empty()) { for (auto& v : voices) v->noteOff(); }
        else if (wasTop)       voices[0]->glideTo(heldNotes.back());   // retombe sur la note encore tenue
        return;
    }
    for (auto& voice : voices)
        if (voice->isActive() && voice->isHeld() && voice->getCurrentNote() == midiNote)
            voice->noteOff();
}

void SynthEngine::allNotesOff()
{
    heldNotes.clear();
    for (auto& voice : voices)
        voice->noteOff();
}

void SynthEngine::reset()
{
    heldNotes.clear();
    for (auto& v : voices) v->hardReset();
    setSampleRate(sampleRate);   // reconstruit/vide les buffers des effets
}

bool SynthEngine::anyVoiceActive() const
{
    for (auto& v : voices) if (v->isActive()) return true;
    return false;
}

void SynthEngine::setFilterAttack(double ms)    { for (auto& v : voices) v->setFilterAttack(ms); }
void SynthEngine::setFilterDecay(double ms)     { for (auto& v : voices) v->setFilterDecay(ms); }
void SynthEngine::setFilterSustain(double lvl)  { for (auto& v : voices) v->setFilterSustain(lvl); }
void SynthEngine::setVelocitySensitivity(double a) { for (auto& v : voices) v->setVelocitySensitivity(a); }

void SynthEngine::processBlock(float* leftOut, float* rightOut, int numSamples)
{
    if (numSamples > (int) mixBuffer.size()) setBlockSize(numSamples);   // garde-fou (hors temps reel normal)
    // Clear buffers
    std::fill(mixBuffer.begin(), mixBuffer.begin() + numSamples, 0.0);
    std::fill(mixBufferR.begin(), mixBufferR.begin() + numSamples, 0.0);

    // Render all active voices into a stereo accumulator
    int activeVoiceCount = 0;
    for (auto& voice : voices)
    {
        if (voice->isActive())
        {
            voice->render(voiceL.data(), voiceR.data(), numSamples);
            ++activeVoiceCount;
            for (int i = 0; i < numSamples; ++i)
            {
                mixBuffer[i]  += voiceL[i];
                mixBufferR[i] += voiceR[i];
            }
        }
    }

    // Normalize by sqrt(active voices) — a single note stays at full strength,
    // while dense chords/unison are tempered without needlessly quieting typical playing.
    double norm = 1.0 / std::sqrt((double) std::max(1, activeVoiceCount));
    for (int i = 0; i < numSamples; ++i)
    {
        mixBuffer[i]  *= norm;
        mixBufferR[i] *= norm;
    }

    // Split to mid/side before the effects. The effect chain is a single mono
    // path, so feeding it the summed signal would collapse the stereo image the
    // unison stack just created. Running it on the mid and letting the side
    // through dry keeps that width while the effects still act on everything
    // that is common to both channels.
    for (int i = 0; i < numSamples; ++i)
    {
        effectBuffer[i] = (mixBuffer[i] + mixBufferR[i]) * 0.5;   // mid
        sideBuffer[i]   = (mixBuffer[i] - mixBufferR[i]) * 0.5;   // side
    }

    // Distortion
    if (distortion)
    {
        distortion->process(effectBuffer.data(), numSamples);
    }

    // Chorus
    if (chorus)
    {
        chorus->process(effectBuffer.data(), numSamples);
    }

    // Delay
    if (delay)
    {
        delay->process(effectBuffer.data(), numSamples);
    }

    // Reverb
    if (reverb)
    {
        reverb->process(effectBuffer.data(), numSamples);
    }

    // Master volume + soft limiting
    for (int i = 0; i < numSamples; ++i)
    {
        // Smooth saturation instead of the hard clip this used to do. Squaring
        // peaks off at exactly +/-1.0 tore the top off any note that reached
        // full scale — which, with effects adding gain on top of a voice that
        // already peaks near 1.0, was most of them. tanh is transparent at
        // normal levels and bends rather than breaks when a peak runs hot.
        double mid  = effectBuffer[i];
        double side = sideBuffer[i];

        double l = (mid + side) * masterVolume;
        double r = (mid - side) * masterVolume;

        l *= (1.0 - std::max(0.0, masterPan));
        r *= (1.0 + std::min(0.0, masterPan));

        leftOut[i]  = static_cast<float>(std::tanh(l));
        rightOut[i] = static_cast<float>(std::tanh(r));
    }
}

void SynthEngine::setPolyphonyMode(bool isPolyphonic)
{
    this->isPolyphonic = isPolyphonic;
}

void SynthEngine::setMaxVoices(int maxVoices)
{
    this->maxVoices = juce::jlimit(1, 16, maxVoices);
}

Voice* SynthEngine::allocateVoice(int midiNote)
{
    if (!isPolyphonic)
        return voices[0].get();

    // Poly mode: find free voice or steal oldest
    for (auto& voice : voices)
    {
        if (!voice->isActive())
        {
            return voice.get();
        }
    }

    // No free voice: steal one already releasing, else the oldest round-robin
    for (auto& voice : voices)
        if (voice->isReleasing())
            return voice.get();
    Voice* victim = voices[(size_t) nextVoiceIndex].get();
    nextVoiceIndex = (nextVoiceIndex + 1) % (int) voices.size();
    return victim;
}

// Parameter routing methods
void SynthEngine::setOscillatorWaveform(Oscillator::Waveform wf)
{
    for (auto& voice : voices) voice->setOscillatorWaveform(wf);
}

void SynthEngine::setFilterCutoff(double hz)
{
    for (auto& voice : voices) voice->setFilterCutoff(hz);
}

void SynthEngine::setFilterResonance(double q)
{
    for (auto& voice : voices) voice->setFilterResonance(q);
}

void SynthEngine::setFilterType(StateVariableFilter::FilterType type)
{
    for (auto& voice : voices) voice->setFilterType(type);
}

void SynthEngine::setFilterEnvelopeDepth(double octaves)
{
    for (auto& voice : voices) voice->setFilterEnvelopeDepth(octaves);
}

void SynthEngine::setAttack(double ms)
{
    for (auto& voice : voices) voice->setAttack(ms);
}

void SynthEngine::setDecay(double ms)
{
    for (auto& voice : voices) voice->setDecay(ms);
}

void SynthEngine::setSustain(double level)
{
    for (auto& voice : voices) voice->setSustain(level);
}

void SynthEngine::setRelease(double ms)
{
    for (auto& voice : voices) voice->setRelease(ms);
}

void SynthEngine::setGlideTime(double ms)
{
    for (auto& voice : voices) voice->setGlideTime(ms);
}

void SynthEngine::setLFORate(double hz)
{
    for (auto& voice : voices) voice->setLFORate(hz);
}

void SynthEngine::setLFOAmount(double amt)
{
    for (auto& voice : voices) voice->setLFOAmount(amt);
}

void SynthEngine::setLFODestination(LFO::Destination dest)
{
    for (auto& voice : voices) voice->setLFODestination(dest);
}

void SynthEngine::setLFOWaveform(Oscillator::Waveform wf)
{
    for (auto& voice : voices) voice->setLFOWaveform(wf);
}

void SynthEngine::setMasterVolume(double gainLinear)
{
    masterVolume = juce::jlimit(0.0, 2.0, gainLinear);
}

void SynthEngine::setMasterPan(double panAmount)
{
    masterPan = juce::jlimit(-1.0, 1.0, panAmount);
}

void SynthEngine::setUnison(int voiceCount, double detuneCents, double stereoWidth)
{
    unisonVoices = juce::jlimit(1, Voice::kMaxUnison, voiceCount);
    unisonDetune = juce::jlimit(0.0, 60.0, detuneCents);
    unisonWidth  = juce::jlimit(0.0, 1.0, stereoWidth);
    for (auto& v : voices)
        v->setUnison(unisonVoices, unisonDetune, unisonWidth);
}

void SynthEngine::setPitchBend(double semitones)
{
    pitchBendSemitones = juce::jlimit(-12.0, 12.0, semitones);
    for (auto& v : voices)
        v->setPitchBend(pitchBendSemitones);
}

void SynthEngine::setModWheel(double amount01)
{
    modWheelAmount = juce::jlimit(0.0, 1.0, amount01);
    for (auto& v : voices)
        v->setModWheel(modWheelAmount);
}

// Effects routing
void SynthEngine::setDistortionDrive(double db)
{
    if (distortion) distortion->setDrive(db);
}

void SynthEngine::setDistortionTone(double hz)
{
    if (distortion) distortion->setTone(hz);
}

void SynthEngine::setDistortionMix(double mix)
{
    if (distortion) distortion->setMix(mix);
}

void SynthEngine::setChorusRate(double hz)
{
    if (chorus) chorus->setRate(hz);
}

void SynthEngine::setChorusAmount(double amt)
{
    if (chorus) chorus->setAmount(amt);
}

void SynthEngine::setChorusMix(double mix)
{
    if (chorus) chorus->setMix(mix);
}

void SynthEngine::setDelayTime(double ms)
{
    if (delay) delay->setTime(ms);
}

void SynthEngine::setDelayFeedback(double feedback)
{
    if (delay) delay->setFeedback(feedback);
}

void SynthEngine::setDelayMix(double mix)
{
    if (delay) delay->setMix(mix);
}

void SynthEngine::setReverbRoom(double room)
{
    if (reverb) reverb->setRoomSize(room);
}

void SynthEngine::setReverbDamping(double damping)
{
    if (reverb) reverb->setDamping(damping);
}

void SynthEngine::setReverbWidth(double width)
{
    if (reverb) reverb->setWidth(width);
}

void SynthEngine::setReverbMix(double mix)
{
    if (reverb) reverb->setMix(mix);
}
