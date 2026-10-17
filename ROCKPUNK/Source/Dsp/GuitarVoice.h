#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "../Params.h"
#include "GuitarString.h"

namespace rk
{

class GuitarSound final : public juce::SynthesiserSound
{
public:
    GuitarSound() = default;

    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
// One MIDI note = one guitar string picked by BOTH layers (A and B).
// Each layer has its own string voicing (tone/body/sustain/pick) and the final
// output is the layer-mix crossfade. Rendering is mono; a note-based equal-power
// pan gives the fretboard spread (scaled later by the output width control).
//==============================================================================
class GuitarVoice final : public juce::SynthesiserVoice
{
public:
    GuitarVoice() = default;

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<GuitarSound*> (s) != nullptr;
    }

    // ---- per-block layer configuration (set by the processor) ----
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        stringA.setSampleRate (sr);
        stringB.setSampleRate (sr);
        resonatorA.setSampleRate (sr);
        resonatorB.setSampleRate (sr);
        releaseCoeff = std::exp (-1.0f / (0.12f * (float) sr));
        env.setSampleRate (sr);
    }

    void setLayerA (bool on, float damping, float loopGain, float blend,
                    float body, float level, float pickBrightness) noexcept
    {
        layerAOn    = on;
        layerADamp  = damping;
        layerALoop  = loopGain;
        layerABlend = blend;
        layerABody  = body;
        layerALevel = level;
        pickA       = pickBrightness;
        tuneResonator (resonatorA, body);
    }

    void setLayerB (bool on, float damping, float loopGain, float blend,
                    float body, float level, float pickBrightness) noexcept
    {
        layerBOn    = on;
        layerBDamp  = damping;
        layerBLoop  = loopGain;
        layerBBlend = blend;
        layerBBody  = body;
        layerBLevel = level;
        pickB       = pickBrightness;
        tuneResonator (resonatorB, body);
    }

    void setLayerMix (float mix) noexcept { layerMix = mix; } // 0 = all A, 1 = all B

    // L'enveloppe que la facade annonce sous ATTACK / DECAY / SUSTAIN /
    // RELEASE. Une corde pincee a sa propre extinction ; cette enveloppe la
    // sculpte par-dessus, ce qui permet aussi bien un palm mute sec qu'une nappe
    // qui monte.
    void setEnvelope (float attackMs, float decayMs, float sustain01, float releaseMs) noexcept
    {
        juce::ADSR::Parameters p;
        p.attack  = attackMs  * 0.001f;
        p.decay   = decayMs   * 0.001f;
        p.sustain = sustain01;
        p.release = releaseMs * 0.001f;
        env.setParameters (p);
    }

    // GLIDE : temps mis pour rejoindre la hauteur de la nouvelle note.
    void setGlideMs (float ms) noexcept { glideMs = juce::jmax (0.0f, ms); }

    // Pour le banc de test : une enveloppe inerte rend la voix muette sans rien
    // signaler ailleurs.
    bool isEnvelopeActive() const noexcept { return env.isActive(); }

    // Decalage de hauteur en demi-tons : vibrato du LFO, molette PITCH BEND et
    // profondeur de la molette MODULATION s'y additionnent.
    void setPitchOffset (float semitones) noexcept { pitchOffset = semitones; }

    // VELOCITY : a 0 toutes les notes attaquent pareil, a 100 la nuance du jeu
    // passe entierement.
    void setVelocityAmount (float amount01) noexcept
    {
        velAmount = juce::jlimit (0.0f, 1.0f, amount01);
    }

    // [WAVEFORM] : durete du mediator, appliquee par-dessus les reglages PICK
    // des deux cordes. C'est bien la forme de l'excitation qui change.
    void setPickShape (int shape) noexcept
    {
        static const float factors[] = { 0.45f, 0.75f, 1.0f, 1.45f };
        pickFactor = factors[juce::jlimit (0, 3, shape)];
    }

    // ---- SynthesiserVoice ----
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                    int /*currentPitchWheelPosition*/) override
    {
        voiceNote = midiNoteNumber;

        const float freq = midiToFreq ((float) midiNoteNumber);
        targetFreq = freq;

        // Sans glissando la hauteur est prise immediatement ; avec, on part de
        // la derniere note jouee par cette voix et on y glisse.
        if (glideMs <= 0.5f || currentFreq <= 0.0f)
            currentFreq = freq;

        stringA.setFrequency (currentFreq);
        stringB.setFrequency (currentFreq);

        const float raw = juce::jlimit (0.001f, 1.0f, velocity);
        const float vel = 1.0f - velAmount * (1.0f - raw);

        stringA.pluck (vel, juce::jlimit (0.02f, 1.0f, pickA * pickFactor));
        stringB.pluck (vel, juce::jlimit (0.02f, 1.0f, pickB * pickFactor));

        env.setSampleRate (sampleRate);
        env.noteOn();
        gain = 0.0f;
        releasing = false;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            releasing = true;
            env.noteOff();
        }
        else
        {
            clearCurrentNote();
            env.reset();
            gain = 0.0f;
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample,
                          int numSamples) override
    {
        if (! isVoiceActive())
            return;

        const float panPos = juce::jmap ((float) (voiceNote % 12), 0.0f, 11.0f, -1.0f, 1.0f);
        const float angle  = (panPos + 1.0f) * 0.5f * 3.14159265f * 0.5f;
        const float panL   = std::cos (angle);
        const float panR   = std::sin (angle);

        const bool  aOn   = layerAOn;
        const bool  bOn   = layerBOn;
        const float dampA = layerADamp;
        const float dampB = layerBDamp;
        const float gA    = layerALoop;
        const float gB    = layerBLoop;
        const float blA   = layerABlend;
        const float blB   = layerBBlend;
        const float bodyA = layerABody;
        const float bodyB = layerBBody;
        const float wA    = (1.0f - layerMix) * layerALevel;
        const float wB    = layerMix * layerBLevel;

        const int numCh = outputBuffer.getNumChannels() >= 2 ? 2 : 1;

        // GLIDE : coefficient d'approche exponentielle vers la note visee.
        const float glideCoeff = glideMs > 0.5f
            ? std::exp (-1.0f / (glideMs * 0.001f * (float) sampleRate))
            : 0.0f;

        // Le decalage de hauteur (vibrato, pitch bend, molette) est constant sur
        // le bloc : la corde n'est reaccordee qu'une fois s'il a bouge, une
        // reallocation de la boucle a chaque echantillon ne s'entendrait pas et
        // couterait cher.
        const bool pitchMoved = std::fabs (pitchOffset - appliedOffset) > 0.001f;
        if (pitchMoved)
            appliedOffset = pitchOffset;

        const float bendRatio = std::pow (2.0f, appliedOffset / 12.0f);
        float lastTuned = -1.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            if (glideCoeff > 0.0f && std::fabs (currentFreq - targetFreq) > 0.05f)
                currentFreq = targetFreq + (currentFreq - targetFreq) * glideCoeff;

            const float tuned = currentFreq * bendRatio;
            if (std::fabs (tuned - lastTuned) > 0.05f)
            {
                stringA.setFrequency (tuned);
                stringB.setFrequency (tuned);
                lastTuned = tuned;
            }

            // Deux instructions et non un produit : l'ordre d'evaluation des
            // operandes de * n'est pas defini, et les deux avancent un etat.
            const float adsr = env.getNextSample();
            const float envValue = gainEnv() * adsr;

            float out = 0.0f;

            if (aOn)
            {
                const float sa = stringA.process (dampA, gA);
                const float res = bodyA > 0.001f ? resonatorA.process (sa) : 0.0f;
                out += (sa * (1.0f - bodyA) + res * bodyA) * blA * wA;
            }

            if (bOn)
            {
                const float sb = stringB.process (dampB, gB);
                const float res = bodyB > 0.001f ? resonatorB.process (sb) : 0.0f;
                out += (sb * (1.0f - bodyB) + res * bodyB) * blB * wB;
            }

            out *= envValue;

            outputBuffer.addSample (0, startSample + i, out * panL);
            if (numCh >= 2)
                outputBuffer.addSample (1, startSample + i, out * panR);
        }
    }

private:
    // Le resonateur naissait avec des coefficients a zero et personne ne les
    // renseignait : process() rendait le silence, donc monter BODY ne faisait
    // qu'attenuer la corde au lieu d'ajouter le bois de la caisse.
    //
    // La resonance monte avec la quantite de corps (95 Hz pour une grande
    // caisse profonde, 230 Hz pour un petit corps claquant) et le facteur de
    // qualite avec elle, pour que le potard change de couleur et pas seulement
    // de dosage.
    static void tuneResonator (BodyResonator& r, float body) noexcept
    {
        const float amount = juce::jlimit (0.0f, 1.0f, body);
        r.setParams (95.0f + 135.0f * amount, 3.0f + 3.0f * amount);
    }

    // Rampe de 1,5 ms contre le clic d'attaque. L'extinction de la note
    // appartient desormais a l'enveloppe ADSR : garder en plus l'ancien fondu
    // fixe de 120 ms aurait coupe toute release plus longue.
    float gainEnv() noexcept
    {
        if (gain < 1.0f)
        {
            gain += 1.0f / (0.0015f * (float) sampleRate);
            gain = std::min (1.0f, gain);
        }

        if (releasing && ! env.isActive())
        {
            gain = 0.0f;
            clearCurrentNote();
        }

        return gain;
    }

    double sampleRate = 48000.0;
    float  releaseCoeff = 0.99f;
    float  gain = 0.0f;
    bool   releasing = false;
    int    voiceNote = -1;

    juce::ADSR env;
    float glideMs = 0.0f;
    float currentFreq = 0.0f, targetFreq = 0.0f;
    float pitchOffset = 0.0f, appliedOffset = 0.0f;
    float velAmount = 0.6f;
    float pickFactor = 1.0f;

    GuitarString stringA, stringB;
    BodyResonator resonatorA, resonatorB;

    bool  layerAOn = true, layerBOn = true;
    float layerADamp = 0.9f, layerBDamp = 0.9f;
    float layerALoop = 0.99f, layerBLoop = 0.99f;
    float layerABlend = 1.0f, layerBBlend = 1.0f;
    float layerABody = 0.0f, layerBBody = 0.0f;
    float layerALevel = 1.0f, layerBLevel = 1.0f;
    float layerMix = 0.5f;
    float pickA = 0.4f, pickB = 0.4f;
};

} // namespace rk