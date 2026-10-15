#pragma once
#include <cmath>
#include "Core/OrbitVisualizer.h"
#include "TwinParams.h"

/** Animated scene of HUMPIRE TWIN: a golden lead star with one comet-like double per voice.
    Orbit radius = micro-delay, orbit speed = detune, colour = stereo position
    (cyan = left, violet = right, amber = centre). */
class TwinOrbitView : public OrbitVisualizer
{
public:
    explicit TwinOrbitView (const twin::UiState& s) : state (s) {}

protected:
    void drawScene (juce::Graphics& g, juce::Rectangle<float> area, float t) override
    {
        using namespace humpire::theme;
        const auto c = area.getCentre();
        const float unit = juce::jmin (area.getWidth(), area.getHeight());
        const float tilt = -0.30f;
        const float ca = std::cos (tilt), sa = std::sin (tilt);

        // Reference orbits for 5, 20 and 40 ms
        for (float ms : { 5.0f, 20.0f, 40.0f })
        {
            const float r = unit * radiusFor (ms);
            drawOrbitRing (g, c, r, r * 0.46f, tilt, edge, 1.0f);
        }

        // Lead voice
        drawGlowOrb (g, c, unit * 0.030f, amber, ember);

        // One double per active voice
        const int voices = juce::jlimit (0, twin::kMaxVoices, state.voices.load (std::memory_order_relaxed));
        for (int k = 0; k < voices; ++k)
        {
            const auto i = (size_t) k;
            const float ms  = state.delayMs[i].load (std::memory_order_relaxed);
            const float ct  = state.detuneCt[i].load (std::memory_order_relaxed);
            const float pan = juce::jlimit (-1.0f, 1.0f, state.pan[i].load (std::memory_order_relaxed));

            const float r = unit * radiusFor (ms);
            const float rx = r * (1.0f + 0.10f * pan), ry = r * 0.46f;
            const float dir = (k % 2 == 0) ? 1.0f : -1.0f;
            const float speed = dir * (0.55f + 0.035f * std::abs (ct));
            const float a0 = t * speed + 1.7f * (float) k;
            const juce::Colour col = amber.interpolatedWith (cyan, juce::jmax (0.0f, -pan))
                                          .interpolatedWith (violet, juce::jmax (0.0f, pan));

            auto pointAt = [&] (float a)
            {
                const float x = rx * std::cos (a), y = ry * std::sin (a);
                return juce::Point<float> (c.x + x * ca - y * sa, c.y + x * sa + y * ca);
            };

            drawOrbitRing (g, c, rx, ry, tilt, col.withAlpha (0.16f), 1.0f);

            const auto head = pointAt (a0);
            g.setColour (col.withAlpha (0.12f));
            g.drawLine (c.x, c.y, head.x, head.y, 1.0f);

            for (int j = 1; j <= 14; ++j)   // comet trail
            {
                const auto p = pointAt (a0 - dir * 0.065f * (float) j);
                g.setColour (col.withAlpha (0.40f * (1.0f - (float) j / 15.0f)));
                g.fillEllipse (p.x - 1.4f, p.y - 1.4f, 2.8f, 2.8f);
            }
            drawGlowOrb (g, head, unit * 0.022f, col, col.darker (0.5f));
        }

        g.setFont (pixel (8.0f));
        g.setColour (textLo);
        g.drawText ("DOUBLES x" + juce::String (voices), area.reduced (14.0f).removeFromBottom (16.0f),
                    juce::Justification::bottomLeft, false);
    }

private:
    /** Orbit radius (fraction of the scene) for a micro-delay of 5..40 ms. */
    static float radiusFor (float ms) noexcept
    {
        return 0.10f + 0.26f * juce::jlimit (0.0f, 1.0f, (ms - 5.0f) / 35.0f);
    }

    const twin::UiState& state;
};
