#pragma once
#include <atomic>
#include <cmath>
#include "SolarParams.h"

/** Instantane de reglages lu par bloc dans le thread audio. */
struct CorrectionParams
{
    bool graphMode = false;
    int key = 0, scale = solar::Minor;
    unsigned noteMask = 0xFFF;
    float retuneMs = 25.0f, flexCents = 0.0f, humanize = 0.2f, amount = 1.0f, refA = 440.0f, portamentoMs = 0.0f;
    const std::atomic<float>* graph = nullptr;     // solar::kGraphPoints deltas (demi-tons) par classe de hauteur
};

/** Logique de correction a cadence de controle (~100 Hz) : f0 detecte -> decalage cible en demi-tons.
    Pipeline : note cible (gamme ou courbe) -> portamento -> zone morte Flex -> Amount -> lissage Retune
    (ralenti sur les notes tenues par Humanize). Boucle ouverte : analyse l'entree, jamais la sortie. */
class PitchCorrector
{
public:
    void reset() noexcept
    {
        offset = 0.0f; glided = 0.0f; lastTarget = -1000.0f;
        haveTarget = false; voiced = false; unvoicedFrames = 0; voicedTime = 0.0f; holdTime = 0.0f;
        detMidi = tgtMidi = 0.0f;
    }

    void update (float f0, double dt, const CorrectionParams& p) noexcept
    {
        if (f0 <= 0.0f)
        {
            if (++unvoicedFrames > 6)     // ~60 ms : vraie coupure -> retour doux a zero
            {
                voiced = false; voicedTime = 0.0f; holdTime = 0.0f; haveTarget = false;
                offset += (0.0f - offset) * (1.0f - std::exp ((float) (-dt / 0.08)));
            }
            return;
        }
        unvoicedFrames = 0;
        voiced = true;
        voicedTime += (float) dt;

        const float midi = 69.0f + 12.0f * std::log2 (f0 / p.refA);
        float target;

        if (p.graphMode && p.graph != nullptr)
        {
            float pc = std::fmod (midi, 12.0f); if (pc < 0.0f) pc += 12.0f;
            const float x = pc * 8.0f;
            const int i0 = (int) x, i1 = (i0 + 1) % solar::kGraphPoints;
            const float fr = x - (float) i0;
            const float a = p.graph[i0 % solar::kGraphPoints].load (std::memory_order_relaxed);
            const float b = p.graph[i1].load (std::memory_order_relaxed);
            target = midi + a + (b - a) * fr;
            glided = target; haveTarget = true;
            holdTime += (float) dt;
        }
        else
        {
            const auto deg = solar::buildDegrees (p.key, p.scale, p.noteMask);
            float lo, hi;
            solar::bracket (midi, p.key, deg, lo, hi);
            const float nearest = (midi - lo <= hi - midi) ? lo : hi;
            // hysteresis : on ne quitte la note courante qu'au-dela de 15 cents apres la frontiere
            const bool lastValid = std::abs (lastTarget - lo) < 1e-3f || std::abs (lastTarget - hi) < 1e-3f;
            target = nearest;
            if (lastValid && std::abs (midi - lastTarget) <= std::abs (midi - nearest) + 0.15f)
                target = lastTarget;

            if (std::abs (target - lastTarget) > 1e-3f) holdTime = 0.0f; else holdTime += (float) dt;
            lastTarget = target;

            if (! haveTarget || p.portamentoMs <= 0.5f) glided = target;
            else glided += (target - glided) * (1.0f - std::exp ((float) (-dt / (p.portamentoMs * 0.001 / 3.0))));
            haveTarget = true;
        }

        float raw = glided - midi;
        const float dev = std::abs (raw) * 100.0f;
        raw = (raw < 0.0f ? -1.0f : 1.0f) * std::max (0.0f, dev - p.flexCents) * 0.01f;   // zone morte Flex-Tune
        raw = juce_clamp (raw * p.amount, -6.0f, 6.0f);

        const double tau = (double) p.retuneMs * 0.001 * (1.0 + 4.0 * p.humanize * std::min (1.0f, holdTime / 0.6f));
        const float alpha = tau < 1e-4 ? 1.0f : 1.0f - std::exp ((float) (-dt / tau));
        offset += (raw - offset) * alpha;

        detMidi = midi;
        tgtMidi = glided;
    }

    float getOffsetSemitones() const noexcept { return offset; }
    bool  isVoiced() const noexcept           { return voiced; }
    float getVoicedTime() const noexcept      { return voicedTime; }
    float getDetectedMidi() const noexcept    { return detMidi; }
    float getTargetMidi() const noexcept      { return tgtMidi; }

private:
    static float juce_clamp (float v, float lo, float hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }

    float offset = 0.0f, glided = 0.0f, lastTarget = -1000.0f, voicedTime = 0.0f, holdTime = 0.0f;
    float detMidi = 0.0f, tgtMidi = 0.0f;
    bool haveTarget = false, voiced = false;
    int unvoicedFrames = 0;
};
