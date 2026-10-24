#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

using namespace GFunkSpec;

static constexpr float kPi = juce::MathConstants<float>::pi;
static constexpr float kTwoPi = juce::MathConstants<float>::twoPi;

namespace
{
inline float fastSin (float p)               // p in [0,1)
{
    const float x = p * 2.0f - 1.0f;
    float s = 4.0f * x * (1.0f - std::fabs (x));
    s = 0.225f * (s * std::fabs (s) - s) + s;
    return -s;
}

inline float fastTanh (float x)
{
    if (x > 3.0f) return 1.0f;
    if (x < -3.0f) return -1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

inline float blep (float t, float dt)
{
    if (t < dt)         { t /= dt;               return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt)  { t = (t - 1.0f) / dt;   return t * t + t + t + 1.0f; }
    return 0.0f;
}

inline float shape (int i, float ph, float dt, float pw)
{
    switch (i)
    {
        case 0:  return fastSin (ph);
        case 1:  return 2.0f * std::fabs (2.0f * ph - 1.0f) - 1.0f;
        case 2:  return 2.0f * ph - 1.0f - blep (ph, dt);
        default:
        {
            float p2 = ph - pw;
            if (p2 < 0.0f) p2 += 1.0f;
            return (ph < pw ? 1.0f : -1.0f) + blep (ph, dt) - blep (p2, dt) - (2.0f * pw - 1.0f);
        }
    }
}

inline float oscWave (float ph, float dt, float w, float pw)
{
    const int i = juce::jlimit (0, 3, (int) w);
    const float fr = w - (float) i;
    const float a = shape (i, ph, dt, pw);
    if (fr < 1.0e-3f || i >= 3)
        return a;
    return a + (shape (i + 1, ph, dt, pw) - a) * fr;
}

inline float mtof (int n) { return 440.0f * std::exp2 ((float) (n - 69) / 12.0f); }
} // namespace

//==============================================================================
juce::String GFunkProcessor::formatValue (int idx, float v)
{
    const juce::String fmt (params[idx].fmt);
    if (fmt == "pct")  return juce::String (juce::roundToInt (v * 100.0f)) + " %";
    if (fmt == "hz")   return v < 10.0f ? juce::String (v, 2) + " Hz" : v < 1000.0f ? juce::String (v, 1) + " Hz"
                                                                                    : juce::String (v / 1000.0f, 2) + " kHz";
    if (fmt == "ms")   return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " s" : juce::String (juce::roundToInt (v)) + " ms";
    if (fmt == "ct")   return (v > 0.5f ? "+" : "") + juce::String (juce::roundToInt (v)) + " ct";
    if (fmt == "st")   return (v > 0.5f ? "+" : "") + juce::String (juce::roundToInt (v)) + " st";
    if (fmt == "oct")  return (v > 0.5f ? "+" : "") + juce::String (juce::roundToInt (v));
    if (fmt == "int")  return juce::String (juce::roundToInt (v));
    if (fmt == "wave")
    {
        static const char* names[] = { "SINE", "TRI", "SAW", "SQUARE" };
        const int i = juce::jlimit (0, 3, (int) v);
        const float fr = v - (float) i;
        if (fr < 0.15f || i >= 3) return names[i];
        if (fr > 0.85f) return names[i + 1];
        return juce::String (names[i]) + ">" + names[i + 1];
    }
    return juce::String (v, 2);
}

juce::AudioProcessorValueTreeState::ParameterLayout GFunkProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < NumParams; ++i)
    {
        const auto& d = params[i];
        const juce::ParameterID pid { d.id, 1 };
        if (d.type == TypeFloat)
        {
            layout.add (std::make_unique<juce::AudioParameterFloat> (
                pid, d.label, juce::NormalisableRange<float> (d.minV, d.maxV, 0.0f, d.skew), d.defV,
                juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                    [i] (float v, int) { return GFunkProcessor::formatValue (i, v); })));
        }
        else if (d.type == TypeInt)
        {
            layout.add (std::make_unique<juce::AudioParameterInt> (
                pid, d.label, (int) d.minV, (int) d.maxV, (int) d.defV,
                juce::AudioParameterIntAttributes().withStringFromValueFunction (
                    [i] (int v, int) { return GFunkProcessor::formatValue (i, (float) v); })));
        }
        else
        {
            layout.add (std::make_unique<juce::AudioParameterChoice> (
                pid, d.label, juce::StringArray::fromTokens (d.choices, "|", ""), (int) d.defV));
        }
    }
    return layout;
}

//==============================================================================
GFunkProcessor::GFunkProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "GFUNK2027", createLayout())
{
    for (int i = 0; i < NumParams; ++i)
        raw[i] = apvts.getRawParameterValue (params[i].id);

    heldNotes.reserve (64);
    pendingOff.reserve (64);
    arpHeld.reserve (64);
    arpSeq.reserve (512);
    for (int i = 0; i < NumParams; ++i)
        P.v[i] = params[i].defV;
}

void GFunkProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    for (auto& b : wobbleBuf) b.assign (4096, 0.0f);
    for (auto& b : dlyBuf) b.assign (1 << 19, 0.0f);
    wobbleWrite = dlyWrite = 0;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    chorus.prepare (spec);
    chorus.setRate (0.8f);
    chorus.setDepth (0.4f);
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.1f);
    chorus.reset();

    reverb.setSampleRate (sampleRate);
    reverb.reset();

    for (auto& v : voices) v = Voice();
    heldNotes.clear(); pendingOff.clear(); arpHeld.clear(); arpSeq.clear();
    arpPlaying = -1; arpCounter = 0; arpStep = 0;
    sustainDown = false;
    std::fill (std::begin (apFb), std::end (apFb), 0.0f);
    for (auto& z : apZ) std::fill (std::begin (z), std::end (z), 0.0f);
}

bool GFunkProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

float GFunkProcessor::tickEnv (Env& e, const EnvCoef& c)
{
    switch (e.st)
    {
        case Env::Idle: e.v = 0.0f; break;
        case Env::Atk:  e.v += c.atk;
                        if (e.v >= 1.0f) { e.v = 1.0f; e.st = Env::Dec; }
                        break;
        case Env::Dec:  e.v = c.sus + (e.v - c.sus) * c.dec;
                        if (std::fabs (e.v - c.sus) < 1.0e-4f) { e.v = c.sus; e.st = Env::Sus; }
                        break;
        case Env::Sus:  e.v = c.sus; break;
        case Env::Rel:  e.v *= c.rel;
                        if (e.v < 1.0e-4f) { e.v = 0.0f; e.st = Env::Idle; }
                        break;
    }
    return e.v;
}

//==============================================================================
// Voice management
void GFunkProcessor::startVoice (Voice& v, int note, float vel, bool retrigger)
{
    const bool wasActive = v.active;
    v.note = note;
    v.vel = vel;
    v.target = mtof (note);
    if (! wasActive)
    {
        v.freq = v.target;
        for (int k = 0; k < 7; ++k)
        {
            v.ph1[k] = rnd.nextFloat();
            v.ph2[k] = rnd.nextFloat();
        }
        v.subPh = 0.0f;
        v.a1 = 0.0f;
        for (auto& ch : v.f)
            for (auto& s : ch) s = Svf();
    }
    v.gate = true;
    v.active = true;
    v.age = ++ageCounter;
    if (retrigger)
    {
        v.amp.st = Env::Atk;
        v.flt.st = Env::Atk;
    }
    funkAcc += 0.4f + 0.6f * vel;
    hitLocal = juce::jmax (hitLocal, vel);
}

void GFunkProcessor::releaseVoice (Voice& v)
{
    v.gate = false;
    if (v.amp.st != Env::Idle) v.amp.st = Env::Rel;
    if (v.flt.st != Env::Idle) v.flt.st = Env::Rel;
}

void GFunkProcessor::voiceNoteOn (int note, float vel)
{
    if (curMode == 0)
    {
        Voice* v = nullptr;
        for (auto& x : voices)
            if (x.active && x.note == note) { v = &x; break; }
        if (v == nullptr)
            for (auto& x : voices)
                if (! x.active) { v = &x; break; }
        if (v == nullptr)
        {
            v = &voices[0];
            for (auto& x : voices)
            {
                const bool better = (! x.gate && v->gate) || (x.gate == v->gate && x.age < v->age);
                if (better) v = &x;
            }
        }
        startVoice (*v, note, vel, true);
    }
    else
    {
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
        heldNotes.push_back (note);
        auto& v = voices[0];
        if (v.active && v.gate && curMode == 2)
        {
            v.target = mtof (note);          // legato: glide without retrigger
            v.note = note;
            v.vel = vel;
            funkAcc += 0.4f + 0.6f * vel;
            hitLocal = juce::jmax (hitLocal, vel);
        }
        else
            startVoice (v, note, vel, true);
    }
}

void GFunkProcessor::voiceNoteOff (int note)
{
    if (curMode == 0)
    {
        for (auto& v : voices)
            if (v.active && v.gate && v.note == note)
                releaseVoice (v);
    }
    else
    {
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
        auto& v = voices[0];
        if (heldNotes.empty())
            releaseVoice (v);
        else if (v.active)
        {
            v.target = mtof (heldNotes.back());
            v.note = heldNotes.back();
            if (curMode == 1)
            {
                v.amp.st = Env::Atk;
                v.flt.st = Env::Atk;
            }
        }
    }
}

void GFunkProcessor::allNotesOff()
{
    heldNotes.clear();
    pendingOff.clear();
    arpHeld.clear();
    arpPlaying = -1;
    for (auto& v : voices)
        if (v.active) releaseVoice (v);
}

//==============================================================================
// Arpeggiator
void GFunkProcessor::arpNoteOn (int note, float vel)
{
    arpVel = vel;
    if (std::find (arpHeld.begin(), arpHeld.end(), note) == arpHeld.end())
    {
        arpHeld.insert (std::upper_bound (arpHeld.begin(), arpHeld.end(), note), note);
        arpDirty = true;
    }
    if (arpHeld.size() == 1)
    {
        arpStep = 0;
        arpCounter = 0;
    }
}

void GFunkProcessor::arpNoteOff (int note)
{
    auto it = std::find (arpHeld.begin(), arpHeld.end(), note);
    if (it == arpHeld.end()) return;
    arpHeld.erase (it);
    arpDirty = true;
    if (arpHeld.empty() && arpPlaying >= 0)
    {
        voiceNoteOff (arpPlaying);
        arpPlaying = -1;
    }
}

void GFunkProcessor::arpTick()
{
    if (arpGateCount > 0 && --arpGateCount == 0 && arpPlaying >= 0)
    {
        voiceNoteOff (arpPlaying);
        arpPlaying = -1;
    }
    if (arpHeld.empty())
        return;

    if (arpCounter <= 0)
    {
        if (arpDirty)
        {
            arpSeq.clear();
            const int octs = juce::jlimit (1, 4, (int) P[P_arpoct]);
            for (int o = 0; o < octs; ++o)
                for (int n : arpHeld)
                    if (n + 12 * o < 128 && arpSeq.size() < 500)
                        arpSeq.push_back (n + 12 * o);
            arpDirty = false;
        }
        const int n = (int) arpSeq.size();
        if (n > 0)
        {
            int idx = 0;
            switch (curArp)
            {
                case 1:  idx = arpStep % n; break;
                case 2:  idx = n - 1 - arpStep % n; break;
                case 3:  { const int period = juce::jmax (1, 2 * n - 2); const int p = arpStep % period;
                           idx = p < n ? p : 2 * n - 2 - p; break; }
                default: idx = rnd.nextInt (n); break;
            }
            ++arpStep;
            if (arpPlaying >= 0) voiceNoteOff (arpPlaying);
            arpPlaying = arpSeq[(size_t) idx];
            voiceNoteOn (arpPlaying, arpVel);
            arpGateCount = juce::jmax (1, (int) ((float) arpStepLen * P[P_arpgate]));
        }
        arpCounter = arpStepLen;
    }
    --arpCounter;
}

//==============================================================================
// MIDI
void GFunkProcessor::noteOnInput (int note, float vel)
{
    pendingOff.erase (std::remove (pendingOff.begin(), pendingOff.end(), note), pendingOff.end());
    if (curArp > 0) arpNoteOn (note, vel);
    else voiceNoteOn (note, vel);
}

void GFunkProcessor::noteOffInput (int note)
{
    if (sustainDown)
    {
        if (std::find (pendingOff.begin(), pendingOff.end(), note) == pendingOff.end())
            pendingOff.push_back (note);
        return;
    }
    if (curArp > 0) arpNoteOff (note);
    else voiceNoteOff (note);
}

void GFunkProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        noteOnInput (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        noteOffInput (m.getNoteNumber());
    else if (m.isPitchWheel())
        midiBend = ((float) m.getPitchWheelValue() - 8192.0f) / 8192.0f;
    else if (m.isControllerOfType (1))
        midiMod = (float) m.getControllerValue() / 127.0f;
    else if (m.isControllerOfType (64))
    {
        const bool down = m.getControllerValue() >= 64;
        if (sustainDown && ! down)
        {
            sustainDown = false;
            auto pend = pendingOff;
            pendingOff.clear();
            for (int n : pend) noteOffInput (n);
        }
        sustainDown = down;
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        allNotesOff();
}

//==============================================================================
void GFunkProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();

    const float fs = (float) sr;
    for (int i = 0; i < NumParams; ++i)
        P.v[i] = raw[i]->load();

    keyState.processNextMidiBuffer (midi, 0, n, true);

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                bpm = juce::jlimit (30.0, 300.0, *b);

    curMode = (int) P[P_mode];
    curArp = (int) P[P_arp];
    if (curArp != lastArp)
    {
        allNotesOff();
        lastArp = curArp;
        arpDirty = true;
    }
    {
        static const float beats[] = { 1.0f, 0.5f, 1.0f / 3.0f, 0.25f, 1.0f / 6.0f, 0.125f };
        arpStepLen = juce::jmax (64, (int) (60.0 / bpm * sr * beats[juce::jlimit (0, 5, (int) P[P_arprate])]));
    }

    // Block constants
    const float glideMs = P[P_glide];
    glideCoef = glideMs < 1.0f ? 0.0f : std::exp (-4.0f / (glideMs * 0.001f * fs));
    ampC = { 1.0f / (juce::jmax (1.0f, P[P_aA]) * 0.001f * fs), std::exp (-4.6f / (P[P_aD] * 0.001f * fs)),
             P[P_aS], std::exp (-6.9f / (P[P_aR] * 0.001f * fs)) };
    fltC = { 1.0f / (juce::jmax (1.0f, P[P_fA]) * 0.001f * fs), std::exp (-4.6f / (P[P_fD] * 0.001f * fs)),
             P[P_fS], std::exp (-6.9f / (P[P_fR] * 0.001f * fs)) };

    uniN = juce::jlimit (1, 7, (int) P[P_uni]);
    {
        const float det = P[P_unidetune] * 45.0f, spread = P[P_unispread];
        const float norm = 1.41421356f / std::sqrt ((float) uniN);
        for (int k = 0; k < uniN; ++k)
        {
            const float pos = uniN == 1 ? 0.0f : ((float) k / (float) (uniN - 1)) * 2.0f - 1.0f;
            uniMul[k] = std::exp2 (pos * det / 1200.0f);
            const float ang = (pos * spread + 1.0f) * kPi * 0.25f;
            uniL[k] = std::cos (ang) * norm;
            uniR[k] = std::sin (ang) * norm;
        }
    }

    const float o1w = P[P_o1wave], o2w = P[P_o2wave];
    const float o1l = P[P_o1level], o2l = P[P_o2level], subL = P[P_sub], noiseL = P[P_noise];
    const float o1mul = std::exp2 (P[P_o1oct]);
    const float o2mul = std::exp2 (P[P_o2oct] + P[P_o2semi] / 12.0f + P[P_o2fine] / 1200.0f);
    const float pwBase = P[P_pw];
    const int ftype = (int) P[P_ftype];
    const float cutoff = P[P_cutoff], reso = P[P_reso], keytrack = P[P_keytrack], fenvamt = P[P_fenvamt];
    const float fdrive = P[P_fdrive], dg = 1.0f + fdrive * 5.0f, dinv = 1.0f / std::sqrt (dg);
    const float lfo1Inc = P[P_lfo1rate] / fs, lfo2Inc = P[P_lfo2rate] / fs;
    const int lfo1Shape = (int) P[P_lfo1shape];
    const float lfo1cut = P[P_lfo1cut], lfo1pitch = P[P_lfo1pitch], lfo1pwm = P[P_lfo1pwm];
    const float lfo2amp = P[P_lfo2amp], lfo2cut = P[P_lfo2cut];
    const float bendUi = uiPitchBend.load(), modUi = uiModWheel.load();
    const float kBase = 2.0f * (1.0f - reso * 0.97f);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    auto it = midi.begin();
    for (int i = 0; i < n; ++i)
    {
        while (it != midi.end() && (*it).samplePosition <= i)
        {
            handleMidi ((*it).getMessage());
            ++it;
        }
        if (curArp > 0)
            arpTick();

        lfo1Phase += lfo1Inc;
        if (lfo1Phase >= 1.0f)
        {
            lfo1Phase -= 1.0f;
            shValue = rnd.nextFloat() * 2.0f - 1.0f;
        }
        lfo2Phase += lfo2Inc;
        if (lfo2Phase >= 1.0f) lfo2Phase -= 1.0f;

        float lfo1;
        switch (lfo1Shape)
        {
            case 0:  lfo1 = fastSin (lfo1Phase); break;
            case 1:  lfo1 = 4.0f * std::fabs (lfo1Phase - 0.5f) - 1.0f; break;
            case 2:  lfo1 = 2.0f * lfo1Phase - 1.0f; break;
            case 3:  lfo1 = lfo1Phase < 0.5f ? 1.0f : -1.0f; break;
            default: lfo1 = shValue; break;
        }
        const float lfo2 = fastSin (lfo2Phase);

        const float bend = juce::jlimit (-1.0f, 1.0f, midiBend + bendUi);
        const float mod = juce::jlimit (0.0f, 1.0f, midiMod + modUi);
        const float vib = juce::jmin (1.0f, lfo1pitch + mod * 0.35f);
        const float pitchMul = std::exp2 ((lfo1 * vib * 2.0f + bend * 2.0f) / 12.0f);
        const float pw = juce::jlimit (0.05f, 0.95f, pwBase + lfo1 * lfo1pwm * 0.4f);
        const float trem = 1.0f - lfo2amp * (0.5f - 0.5f * lfo2);

        float outL = 0.0f, outR = 0.0f;
        for (auto& v : voices)
        {
            if (! v.active)
                continue;

            v.freq = v.target + (v.freq - v.target) * glideCoef;
            const float envA = tickEnv (v.amp, ampC);
            if (v.amp.st == Env::Idle)
            {
                v.active = false;
                v.gate = false;
                continue;
            }
            const float envF = tickEnv (v.flt, fltC);
            const float base = v.freq * pitchMul;
            const float f1 = base * o1mul, f2 = base * o2mul;

            float l = 0.0f, r = 0.0f;
            for (int k = 0; k < uniN; ++k)
            {
                const float dt1 = juce::jlimit (1.0e-5f, 0.45f, f1 * uniMul[k] / fs);
                const float dt2 = juce::jlimit (1.0e-5f, 0.45f, f2 * uniMul[k] / fs);
                v.ph1[k] += dt1; if (v.ph1[k] >= 1.0f) v.ph1[k] -= 1.0f;
                v.ph2[k] += dt2; if (v.ph2[k] >= 1.0f) v.ph2[k] -= 1.0f;
                const float s = oscWave (v.ph1[k], dt1, o1w, pw) * o1l + oscWave (v.ph2[k], dt2, o2w, pw) * o2l;
                l += s * uniL[k];
                r += s * uniR[k];
            }
            v.subPh += f1 * 0.5f / fs;
            if (v.subPh >= 1.0f) v.subPh -= 1.0f;
            v.rng = v.rng * 1664525u + 1013904223u;
            const float nz = ((float) (v.rng >> 8) / 8388608.0f - 1.0f) * noiseL;
            const float cen = fastSin (v.subPh) * subL + nz;
            l = (l + cen) * 0.3f;
            r = (r + cen) * 0.3f;

            if ((i & 7) == 0 || v.a1 == 0.0f)
            {
                const float oct = keytrack * std::log2 (v.freq * (1.0f / 261.63f)) + fenvamt * envF * 5.0f
                                  + (lfo1 * lfo1cut + lfo2 * lfo2cut) * 3.0f;
                const float fc = juce::jlimit (20.0f, 0.46f * fs, cutoff * std::exp2 (oct));
                const float g = std::tan (kPi * fc / fs);
                v.k = kBase;
                v.a1 = 1.0f / (1.0f + g * (g + v.k));
                v.a2 = g * v.a1;
                v.a3 = g * v.a2;
                v.b1 = 1.0f / (1.0f + g * (g + 1.5f));
                v.b2 = g * v.b1;
                v.b3 = g * v.b2;
            }

            float in[2] = { l, r }, out[2];
            for (int c = 0; c < 2; ++c)
            {
                float x = in[c];
                if (fdrive > 0.001f) x = fastTanh (x * dg) * dinv;
                auto& s1 = v.f[c][0];
                const float v3 = x - s1.ic2;
                const float v1 = v.a1 * s1.ic1 + v.a2 * v3;
                const float v2 = s1.ic2 + v.a2 * s1.ic1 + v.a3 * v3;
                s1.ic1 = 2.0f * v1 - s1.ic1;
                s1.ic2 = 2.0f * v2 - s1.ic2;
                switch (ftype)
                {
                    case 0:  out[c] = v2; break;
                    case 1:
                    {
                        auto& s2 = v.f[c][1];
                        const float w3 = v2 - s2.ic2;
                        const float w1 = v.b1 * s2.ic1 + v.b2 * w3;
                        const float w2 = s2.ic2 + v.b2 * s2.ic1 + v.b3 * w3;
                        s2.ic1 = 2.0f * w1 - s2.ic1;
                        s2.ic2 = 2.0f * w2 - s2.ic2;
                        out[c] = w2;
                        break;
                    }
                    case 2:  out[c] = v1 * v.k; break;
                    default: out[c] = x - v.k * v1 - v2; break;
                }
            }

            const float gain = envA * (0.3f + 0.7f * v.vel) * trem;
            outL += out[0] * gain;
            outR += out[1] * gain;
        }
        L[i] = outL;
        R[i] = outR;
    }

    renderFx (buffer, n);

    // Lit keys, funk energy and flash for the interface
    uint64_t lo = 0, hi = 0;
    for (auto& v : voices)
        if (v.active && v.gate && v.note >= 0)
            (v.note < 64 ? lo : hi) |= 1ull << (v.note & 63);
    activeLo.store (lo);
    activeHi.store (hi);

    const float dtBlock = (float) n / fs;
    funkEnergy = funkEnergy * std::exp (-dtBlock / 1.6f) + funkAcc;
    funkAcc = 0.0f;
    funk.store (juce::jmin (1.0f, funkEnergy * 0.3f));
    hitLocal *= std::exp (-dtBlock / 0.18f);
    hit.store (hitLocal);
}

//==============================================================================
void GFunkProcessor::renderFx (juce::AudioBuffer<float>& buffer, int n)
{
    const float fs = (float) sr;
    float* l = buffer.getWritePointer (0);
    float* r = buffer.getWritePointer (1);

    // Tape: wobble (wow/flutter/drift) then asymmetric saturation
    const float wob = P[P_wobble], drive = P[P_drive];
    const float dk = 1.0f + drive * 9.0f, bias = 0.12f * drive, biasOut = std::tanh (bias);
    const float comp = 1.0f / std::sqrt (dk), blend = juce::jmin (1.0f, drive * 4.0f);
    const int wmask = (int) wobbleBuf[0].size() - 1;
    auto saturate = [&] (float x)
    {
        const float s = (std::tanh (x * dk + bias) - biasOut) * comp;
        return x + (s - x) * blend;
    };

    // Phaser
    const float phMix = P[P_phaser];
    // Delay
    static const float dlyBeats[] = { 0.25f, 1.0f / 3.0f, 0.5f, 0.75f, 1.0f };
    const float dlyTarget = juce::jlimit (64.0f, (float) (dlyBuf[0].size() - 4),
                                          (float) (60.0 / bpm * sr * dlyBeats[juce::jlimit (0, 4, (int) P[P_dlytime])]));
    const float dlyFb = P[P_dlyfb], dlyMix = P[P_dlymix];
    const int dmask = (int) dlyBuf[0].size() - 1;
    const float toneT = P[P_tone] * 0.7f;
    const float toneC = 1.0f - std::exp (-kTwoPi * 1200.0f / fs);

    for (int i = 0; i < n; ++i)
    {
        wobPhase += 0.7f / fs;      if (wobPhase >= 1.0f)     wobPhase -= 1.0f;
        flutterPhase += 6.3f / fs;  if (flutterPhase >= 1.0f) flutterPhase -= 1.0f;
        noiseState += 0.0005f * (rnd.nextFloat() * 2.0f - 1.0f - noiseState * 4.0f);

        float* ch[2] = { l, r };
        for (int c = 0; c < 2; ++c)
        {
            auto& buf = wobbleBuf[(size_t) c];
            buf[(size_t) wobbleWrite] = ch[c][i];
            const float offs = c == 0 ? 0.0f : 0.25f;
            const float mod = fastSin (std::fmod (wobPhase + offs, 1.0f)) * 40.0f
                              + fastSin (std::fmod (flutterPhase + offs, 1.0f)) * 5.0f + noiseState * 600.0f;
            const float delay = 64.0f + wob * mod * (fs / 44100.0f);
            const float rp = (float) wobbleWrite - delay;
            const int i0 = (int) std::floor (rp);
            const float fr = rp - (float) i0;
            const float a = buf[(size_t) (i0 & wmask)], b = buf[(size_t) ((i0 + 1) & wmask)];
            ch[c][i] = saturate (a + (b - a) * fr);
        }
        wobbleWrite = (wobbleWrite + 1) & wmask;

        if (phMix > 0.001f)
        {
            phPhase += 0.35f / fs;
            if (phPhase >= 1.0f) phPhase -= 1.0f;
            for (int c = 0; c < 2; ++c)
            {
                const float lfo = 0.5f + 0.5f * fastSin (std::fmod (phPhase + (c ? 0.25f : 0.0f), 1.0f));
                const float t = std::tan (kPi * juce::jmin (0.45f * fs, 250.0f * std::exp2 (lfo * 3.5f)) / fs);
                const float a = (t - 1.0f) / (t + 1.0f);
                const float in = ch[c][i];
                float y = in + apFb[c] * 0.45f * phMix;
                for (int s = 0; s < 4; ++s)
                {
                    const float o = a * y + apZ[c][s];
                    apZ[c][s] = y - a * o;
                    y = o;
                }
                apFb[c] = y;
                ch[c][i] = in * (1.0f - 0.5f * phMix) + y * 0.5f * phMix;
            }
        }
    }

    // Chorus
    chorus.setMix (P[P_chorus] * 0.6f);
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus.process (ctx);
    }

    // Ping-pong delay with filtered feedback
    for (int i = 0; i < n; ++i)
    {
        dlyCur += (dlyTarget - dlyCur) * 0.0004f;
        float rd[2];
        for (int c = 0; c < 2; ++c)
        {
            const float rp = (float) dlyWrite - dlyCur;
            const int i0 = (int) std::floor (rp);
            const float fr = rp - (float) i0;
            const auto& buf = dlyBuf[(size_t) c];
            const float a = buf[(size_t) (i0 & dmask)], b = buf[(size_t) ((i0 + 1) & dmask)];
            rd[c] = a + (b - a) * fr;
        }
        dlyLp[0] += 0.4f * (rd[1] - dlyLp[0]);
        dlyLp[1] += 0.4f * (rd[0] - dlyLp[1]);
        const float mono = 0.5f * (l[i] + r[i]);
        dlyBuf[0][(size_t) dlyWrite] = mono + dlyLp[0] * dlyFb;
        dlyBuf[1][(size_t) dlyWrite] = dlyLp[1] * dlyFb;
        dlyWrite = (dlyWrite + 1) & dmask;
        l[i] += rd[0] * dlyMix;
        r[i] += rd[1] * dlyMix;
    }

    // Reverb
    const float rv = P[P_revmix];
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.3f + 0.65f * P[P_revsize];
    rp.damping = 0.45f;
    rp.wetLevel = rv * 0.4f;
    rp.dryLevel = 1.0f - rv * 0.25f;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    reverb.processStereo (l, r, n);

    // Tone tilt, master volume, soft limiter, meters and scope feed
    const float vol = P[P_volume] * 2.0f;
    float peak = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        float* ch[2] = { l, r };
        for (int c = 0; c < 2; ++c)
        {
            float x = ch[c][i];
            toneLp[c] += toneC * (x - toneLp[c]);
            const float lo = toneLp[c], hi = x - lo;
            x = lo * (1.0f - toneT) + hi * (1.0f + toneT);
            x = fastTanh (x * vol * 1.2f) / 1.2f;
            ch[c][i] = x;
        }
        peak = juce::jmax (peak, std::fabs (l[i]), std::fabs (r[i]));

        const float mono = 0.5f * (l[i] + r[i]);
        const int fw = fftWrite.load (std::memory_order_relaxed);
        fftRing[(size_t) (fw & (fftSize - 1))] = mono;
        fftWrite.store (fw + 1, std::memory_order_release);

        scopeAcc += l[i];
        if (++scopeCount == 4)
        {
            const int w = scopeWrite.load (std::memory_order_relaxed);
            scopeRing[(size_t) (w & (scopeSize - 1))] = scopeAcc * 0.25f;
            scopeWrite.store (w + 1, std::memory_order_release);
            scopeAcc = 0.0f;
            scopeCount = 0;
        }
    }
    outputLevel.store (juce::jmax (peak, outputLevel.load() * 0.9f));
}

void GFunkProcessor::copyScope (float* dest) const
{
    const int w = scopeWrite.load (std::memory_order_acquire);
    for (int i = 0; i < 1024; ++i)
        dest[i] = scopeRing[(size_t) ((w + 1024 + i) & (scopeSize - 1))];
}

void GFunkProcessor::copyFft (float* dest) const
{
    const int w = fftWrite.load (std::memory_order_acquire);
    for (int i = 0; i < fftSize; ++i)
        dest[i] = fftRing[(size_t) ((w + i) & (fftSize - 1))];
}

//==============================================================================
const juce::String GFunkProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, numPresets) ? juce::String (presetInfo[index].name) : juce::String();
}

void GFunkProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, numPresets))
        return;

    currentProgram = index;
    for (int i = 0; i < NumParams; ++i)
        if (auto* param = apvts.getParameter (params[i].id))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (presetValues[index][i]));
            param->endChangeGesture();
        }
    hit.store (1.0f);
}

void GFunkProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    state.setProperty ("theme", blingTheme.load(), nullptr);
    state.setProperty ("bling", blingLevel.load(), nullptr);
    state.setProperty ("skin", skin.load(), nullptr);
    state.setProperty ("scrollInvert", scrollInvert.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void GFunkProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, numPresets - 1, (int) tree.getProperty ("program", 0));
            blingTheme.store ((int) tree.getProperty ("theme", 0));
            blingLevel.store ((int) tree.getProperty ("bling", 1));
            skin.store (juce::jlimit (0, numSkins - 1, (int) tree.getProperty ("skin", 0)));
            scrollInvert.store ((int) tree.getProperty ("scrollInvert", 1));
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* GFunkProcessor::createEditor() { return new GFunkEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GFunkProcessor(); }
