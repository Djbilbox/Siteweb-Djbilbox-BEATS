#include "ChaabiSynth.h"
#include <cmath>

namespace humpire
{
namespace
{
// Interpolation Hermite 4 points
inline float hermite (const float* x, double position, int last) noexcept
{
    const int i = (int) position;
    const float f = (float) (position - (double) i);
    const float y0 = x[juce::jmax (i - 1, 0)], y1 = x[i];
    const float y2 = x[juce::jmin (i + 1, last)], y3 = x[juce::jmin (i + 2, last)];
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * f + c2) * f + c1) * f + y1;
}
} // namespace

// ------------------------------------------------------------------ Voice
ChaabiVoice::ChaabiVoice (const SynthParams& p) : sp (p) { rng.setSeedRandomly(); }

void ChaabiVoice::setCurrentPlaybackSampleRate (double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
    env.setSampleRate (newRate > 0.0 ? newRate : 44100.0);
}

void ChaabiVoice::startNote (int note, float velocity, juce::SynthesiserSound* s, int pitchWheel)
{
    region = dynamic_cast<Region*> (s);
    if (region == nullptr || region->data == nullptr) { clearCurrentNote(); return; }

    const auto& d = region->def;
    data = region->data.get();
    const double sr = getSampleRate();
    const float a = sp.authenticity.load();

    // Humanisation (macro Authenticite) : velocite, accordage, micro-retard
    const float velJit = 1.0f + a * 0.12f * (rng.nextFloat() * 2.0f - 1.0f);
    const float detune = a * 8.0f * (rng.nextFloat() * 2.0f - 1.0f);                 // cents
    delayLeft = (int) (a * 0.012 * sr * (double) rng.nextFloat());                    // <= 12 ms

    const float vt = d.ampVeltrack / 100.0f;
    const float velGain = (1.0f - vt) + vt * velocity * velocity;
    gain = juce::Decibels::decibelsToGain (d.volumeDb) * velGain * velJit;

    const float p = d.pan / 100.0f;
    const float ang = (p + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
    gL = std::cos (ang) * juce::MathConstants<float>::sqrt2;
    gR = std::sin (ang) * juce::MathConstants<float>::sqrt2;

    const double semis = (double) (note - d.keycenter) * (double) d.pitchKeytrack / 100.0 + (double) d.transpose;
    step = std::pow (2.0, semis / 12.0 + ((double) d.tuneCents + (double) detune) / 1200.0) * data->sampleRate / sr;

    bendRatio = std::pow (2.0, ((double) (pitchWheel - 8192) / 8192.0) * (double) sp.bendRange.load() / 12.0);
    pressureGain = 1.0f;

    const int last = data->numFrames;
    pos = (double) juce::jlimit<juce::int64> (0, last - 1, d.offset);
    endPos = d.end >= 0 ? (double) juce::jmin<juce::int64> (d.end + 1, last) : (double) last;

    oneShot = d.loop == SfzRegionDef::Loop::oneShot || d.triggerRelease;
    looping = d.loop == SfzRegionDef::Loop::continuous || d.loop == SfzRegionDef::Loop::sustain;
    loopStart = (double) juce::jlimit<juce::int64> (0, last - 1, d.loopStart >= 0 ? d.loopStart : 0);
    loopEndF = (double) juce::jlimit<juce::int64> (1, last, d.loopEnd >= 0 ? d.loopEnd + 1 : last);
    if (loopEndF <= loopStart + 1.0) looping = false;

    released = false;
    env.setParameters ({ d.ampAttack + sp.attack.load(), d.ampDecay, d.ampSustain / 100.0f,
                         d.ampRelease + sp.release.load() });
    env.noteOn();
}

void ChaabiVoice::stopNote (float, bool allowTailOff)
{
    if (region == nullptr) { clearCurrentNote(); return; }
    if (allowTailOff)
    {
        if (! oneShot && ! released) { released = true; env.noteOff(); }
    }
    else
    {
        finish();
    }
}

void ChaabiVoice::quickRelease()
{
    if (region == nullptr) return;
    env.setParameters ({ 0.0f, 0.0f, 1.0f, 0.012f });
    released = true;
    oneShot = false;
    env.noteOff();
}

void ChaabiVoice::pitchWheelMoved (int v)
{
    bendRatio = std::pow (2.0, ((double) (v - 8192) / 8192.0) * (double) sp.bendRange.load() / 12.0);
}

void ChaabiVoice::finish()
{
    env.reset();
    clearCurrentNote();
    detach();
}

void ChaabiVoice::renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples)
{
    if (region == nullptr || data == nullptr) return;

    const int lastFrame = data->numFrames - 1;
    const float* srcL = data->audio.getReadPointer (0);
    const float* srcR = data->audio.getNumChannels() > 1 ? data->audio.getReadPointer (1) : srcL;
    float* outL = out.getWritePointer (0, startSample);
    float* outR = out.getNumChannels() > 1 ? out.getWritePointer (1, startSample) : nullptr;

    const double pitchParam = std::exp2 ((double) sp.pitchSemis.load() / 12.0);
    const double inc = step * pitchParam * bendRatio;
    const float g = gain * pressureGain;

    for (int n = 0; n < numSamples; ++n)
    {
        if (delayLeft > 0) { --delayLeft; continue; }

        const float e = env.getNextSample();
        if (! env.isActive()) { finish(); return; }

        if (looping)
        {
            if (pos >= loopEndF) pos -= (loopEndF - loopStart);
        }
        else if (pos >= endPos - 1.0)
        {
            finish();
            return;
        }

        const float l = hermite (srcL, pos, lastFrame) * e * g;
        outL[n] += l * gL;
        if (outR != nullptr)
            outR[n] += (srcR == srcL ? l : hermite (srcR, pos, lastFrame) * e * g) * gR;

        pos += inc;
    }
}

// ------------------------------------------------------------------ Synth
ChaabiSynth::ChaabiSynth()
{
    rng.setSeedRandomly();
    for (int i = 0; i < kMaxVoices; ++i)
        addVoice (new ChaabiVoice (params));
}

std::shared_ptr<Bank> ChaabiSynth::swapBank (std::shared_ptr<Bank> newBank)
{
    for (int i = 0; i < getNumVoices(); ++i)
    {
        auto* v = static_cast<ChaabiVoice*> (getVoice (i));
        if (v->isVoiceActive()) v->stopNote (0.0f, false);
        v->detach();
    }
    auto old = std::move (bank);
    bank = std::move (newBank);
    activeSwitch = bank != nullptr ? bank->swDefault : -1;
    std::fill (std::begin (lastVelocity), std::end (lastVelocity), -1.0f);
    return old;
}

ChaabiVoice* ChaabiSynth::pickVoice()
{
    ChaabiVoice* freeVoice = nullptr;
    ChaabiVoice* victim = nullptr;
    int active = 0;

    for (int i = 0; i < getNumVoices(); ++i)
    {
        auto* v = static_cast<ChaabiVoice*> (getVoice (i));
        if (v->isVoiceActive())
        {
            ++active;
            // on prefere voler la plus ancienne voix dont la touche est relachee
            if (victim == nullptr
                || (victim->isKeyDown() && ! v->isKeyDown())
                || (victim->isKeyDown() == v->isKeyDown() && v->wasStartedBefore (*victim)))
                victim = v;
        }
        else if (freeVoice == nullptr)
            freeVoice = v;
    }
    if (freeVoice != nullptr && active < polyLimit) return freeVoice;
    return victim != nullptr ? victim : freeVoice;
}

void ChaabiSynth::startRegion (Region* r, int channel, int note, float velocity)
{
    // Groupes exclusifs (off_by) : coupe les voix concernees
    if (r->def.group != 0)
        for (int i = 0; i < getNumVoices(); ++i)
        {
            auto* v = static_cast<ChaabiVoice*> (getVoice (i));
            if (v->isVoiceActive() && v->getRegion() != nullptr && v->getRegion()->def.offBy == r->def.group)
                v->quickRelease();
        }

    if (auto* v = pickVoice())
        startVoice (v, r, channel, note, velocity);
}

void ChaabiSynth::noteOn (int channel, int note, float velocity)
{
    const juce::ScopedLock sl (lock);
    if (bank == nullptr || note < 0 || note > 127) return;

    // Keyswitch : change l'articulation, ne sonne pas
    if (bank->swLo >= 0 && note >= bank->swLo && note <= bank->swHi)
    {
        activeSwitch = note;
        return;
    }

    lastVelocity[note] = velocity;
    const int vel127 = juce::jlimit (1, 127, juce::roundToInt (velocity * 127.0f));
    const float rnd = rng.nextFloat();
    const int seq = seqCounter[note]++;

    for (auto* r : bank->regions)
        if (r->matches (note, vel127, rnd, seq, activeSwitch, false))
            startRegion (r, channel, note, velocity);
}

void ChaabiSynth::noteOff (int channel, int note, float velocity, bool allowTailOff)
{
    juce::Synthesiser::noteOff (channel, note, velocity, allowTailOff);

    const juce::ScopedLock sl (lock);
    if (bank == nullptr || ! bank->hasReleaseRegions || note < 0 || note > 127 || lastVelocity[note] < 0.0f) return;

    const int vel127 = juce::jlimit (1, 127, juce::roundToInt (lastVelocity[note] * 127.0f));
    const float rnd = rng.nextFloat();
    for (auto* r : bank->regions)
        if (r->matches (note, vel127, rnd, 0, activeSwitch, true))
            startRegion (r, channel, note, lastVelocity[note] * 0.6f);
    lastVelocity[note] = -1.0f;
}
} // namespace humpire
