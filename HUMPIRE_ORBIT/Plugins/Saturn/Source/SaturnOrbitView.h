#pragma once
#include <array>
#include <cmath>
#include "Core/OrbitVisualizer.h"
#include "PluginProcessor.h"

/** Colour of voice v: V1 amber, V2 violet, V3 cyan, V4 green. */
inline juce::Colour saturnVoiceColour (int v)
{
    using namespace humpire;
    switch (v)
    {
        case 0:  return theme::amber;
        case 1:  return theme::violet.brighter (0.35f);
        case 2:  return theme::cyan;
        default: return theme::good;
    }
}

/** HUMPIRE SATURN scene: a ringed gas giant. Each voice owns one ring (V1 innermost); a bead orbits it,
    faster when the voice sits higher, and its size follows the voice level. The sung note is a lead orb
    on a tuning wheel around the planet, where the lit ticks are the notes of the current scale. */
class SaturnOrbitView : public OrbitVisualizer
{
public:
    explicit SaturnOrbitView (SaturnAudioProcessor& p) : proc (p)
    {
        for (int v = 0; v < saturn::kNumVoices; ++v)
            onFlag[(size_t) v] = proc.apvts.getRawParameterValue (saturn::voiceId (v, saturn::vOn));
        keyFlag = proc.apvts.getRawParameterValue (saturn::id::key);
        scaleFlag = proc.apvts.getRawParameterValue (saturn::id::scale);
    }

protected:
    void drawScene (juce::Graphics& g, juce::Rectangle<float> area, float t) override
    {
        using namespace humpire;
        using saturn::kNumVoices;
        const auto c = area.getCentre();
        const float R = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 12.0f;
        const float pr = R * 0.20f;                                  // planet radius
        const float twoPi = juce::MathConstants<float>::twoPi, pi = juce::MathConstants<float>::pi;
        const float dt = lastT < 0.0f ? 0.0f : juce::jlimit (0.0f, 0.1f, t - lastT);
        lastT = t;

        const float outLvl = juce::jlimit (0.0f, 1.0f, proc.outLevel.load (std::memory_order_relaxed) * 1.5f);
        smoothLevel += (outLvl - smoothLevel) * 0.2f;

        const float det = proc.uiDetMidi.load (std::memory_order_relaxed);
        const float tracked = proc.uiTrackedMidi.load (std::memory_order_relaxed);
        const bool voiced = det > 0.0f && tracked > 0.0f;

        // ring geometry and bead state
        const float tilt = -0.22f, flat = 0.36f;
        std::array<float, kNumVoices> rx {}, ry {}, shift {}, lvl {};
        std::array<bool, kNumVoices> on {};
        for (int v = 0; v < kNumVoices; ++v)
        {
            rx[(size_t) v] = pr * (1.90f + 0.55f * (float) v);
            ry[(size_t) v] = rx[(size_t) v] * flat;
            on[(size_t) v] = onFlag[(size_t) v]->load() > 0.5f;
            shift[(size_t) v] = proc.uiVoiceShift[(size_t) v].load (std::memory_order_relaxed);
            lvl[(size_t) v] = juce::jlimit (0.0f, 1.0f, proc.uiVoiceLevel[(size_t) v].load (std::memory_order_relaxed));
            const float omega = (0.55f + 0.12f * (float) v) * std::exp2 (shift[(size_t) v] / 12.0f) * (0.8f + 0.6f * smoothLevel);
            phase[(size_t) v] = std::fmod (phase[(size_t) v] + omega * dt, twoPi);
        }

        // scale membership of the 12 pitch classes (for the tuning wheel)
        const int key = juce::jlimit (0, 11, (int) keyFlag->load());
        const auto deg = saturn::buildDegrees ((int) scaleFlag->load());
        bool inScale[12] {};
        for (int i = 0; i < deg.n; ++i)
            inScale[(key + (int) std::lround (deg.d[i])) % 12] = true;

        // 1. back halves of the rings (behind the planet)
        for (int v = 0; v < kNumVoices; ++v)
            drawRing (g, c, rx[(size_t) v], ry[(size_t) v], tilt, pi, twoPi, saturnVoiceColour (v), on[(size_t) v]);

        // 2. beads on the back halves
        for (int v = 0; v < kNumVoices; ++v)
            drawBead (g, c, rx[(size_t) v], ry[(size_t) v], tilt, phase[(size_t) v], v,
                      on[(size_t) v], shift[(size_t) v], lvl[(size_t) v], false);

        // 3. planet: halo, body, bands
        {
            juce::ColourGradient halo (theme::amber.withAlpha (0.30f + 0.20f * smoothLevel), c,
                                       theme::ember.withAlpha (0.0f), c.translated (pr * 2.6f, 0.0f), true);
            g.setGradientFill (halo);
            g.fillEllipse (c.x - pr * 2.6f, c.y - pr * 2.6f, pr * 5.2f, pr * 5.2f);

            juce::ColourGradient body (theme::amber.brighter (0.25f), c.translated (-pr * 0.35f, -pr * 0.40f),
                                       theme::ember.darker (0.55f), c.translated (pr * 0.8f, pr * 0.9f), true);
            g.setGradientFill (body);
            g.fillEllipse (c.x - pr, c.y - pr, pr * 2.0f, pr * 2.0f);

            for (int k = -3; k <= 3; ++k)
            {
                const float dy = (float) k * pr * 0.24f;
                const float hc = std::sqrt (std::max (0.0f, pr * pr - dy * dy)) * 0.92f;
                g.setColour (theme::ember.withAlpha (k % 2 == 0 ? 0.22f : 0.12f));
                g.drawLine (c.x - hc, c.y + dy, c.x + hc, c.y + dy, 2.0f);
            }
        }

        // 4. note name on the planet
        if (voiced)
        {
            const int n = (int) std::lround (tracked);
            const float cents = 100.0f * (det - tracked);
            g.setColour (juce::Colour (0xff1A0F05));
            g.setFont (theme::title (pr * 0.52f));
            g.drawText (juce::String (saturn::noteName (n)) + juce::String (n / 12 - 1),
                        juce::Rectangle<float> (c.x - pr, c.y - pr * 0.62f, pr * 2.0f, pr * 0.62f),
                        juce::Justification::centred, false);
            g.setFont (theme::text (pr * 0.24f));
            g.drawText ((cents >= 0.0f ? "+" : "") + juce::String (cents, 0) + " ct",
                        juce::Rectangle<float> (c.x - pr, c.y + pr * 0.10f, pr * 2.0f, pr * 0.40f),
                        juce::Justification::centred, false);
        }
        else
        {
            g.setColour (juce::Colour (0xff1A0F05));
            g.setFont (theme::pixel (pr * 0.20f));
            g.drawText ("SATURN", juce::Rectangle<float> (c.x - pr, c.y - pr * 0.2f, pr * 2.0f, pr * 0.4f),
                        juce::Justification::centred, false);
        }

        // 5. tuning wheel: 12 ticks, scale members lit
        const float wheelR = pr * 1.40f;
        for (int pc = 0; pc < 12; ++pc)
        {
            const float a = -0.5f * pi + twoPi * (float) pc / 12.0f;
            const float len = inScale[pc] ? 4.0f : 2.0f;
            g.setColour (inScale[pc] ? theme::amber.withAlpha (0.85f) : theme::edge);
            g.drawLine (c.x + std::cos (a) * (wheelR - len), c.y + std::sin (a) * (wheelR - len),
                        c.x + std::cos (a) * (wheelR + len), c.y + std::sin (a) * (wheelR + len),
                        inScale[pc] ? 2.0f : 1.0f);
        }

        // 6. front halves of the rings and beads
        for (int v = 0; v < kNumVoices; ++v)
            drawRing (g, c, rx[(size_t) v], ry[(size_t) v], tilt, 0.0f, pi, saturnVoiceColour (v), on[(size_t) v]);
        for (int v = 0; v < kNumVoices; ++v)
            drawBead (g, c, rx[(size_t) v], ry[(size_t) v], tilt, phase[(size_t) v], v,
                      on[(size_t) v], shift[(size_t) v], lvl[(size_t) v], true);

        // 7. lead orb: the sung note on the tuning wheel
        if (voiced)
        {
            const float a = -0.5f * pi + twoPi * std::fmod (det, 12.0f) / 12.0f;
            drawGlowOrb (g, juce::Point<float> (c.x + std::cos (a) * wheelR, c.y + std::sin (a) * wheelR),
                         2.6f + 1.2f * smoothLevel, juce::Colour (0xffFFE9B0), theme::amber);
        }

        // 8. ring labels above each ring
        for (int v = 0; v < kNumVoices; ++v)
        {
            float x, y;
            ringPoint (c, rx[(size_t) v], ry[(size_t) v], tilt, -0.5f * pi, x, y);
            g.setFont (theme::pixel (7.0f));
            g.setColour (on[(size_t) v] ? saturnVoiceColour (v) : theme::textLo);
            g.drawText ("V" + juce::String (v + 1), juce::Rectangle<float> (x - 14.0f, y - 15.0f, 28.0f, 10.0f),
                        juce::Justification::centred, false);
        }
    }

private:
    /** Point of a tilted ring at parameter angle th (th in 0..2pi; sin(th) > 0 is the front half). */
    static void ringPoint (juce::Point<float> c, float rx, float ry, float tilt, float th, float& x, float& y) noexcept
    {
        const float px = rx * std::cos (th), py = ry * std::sin (th);
        const float ct = std::cos (tilt), st = std::sin (tilt);
        x = c.x + px * ct - py * st;
        y = c.y + px * st + py * ct;
    }

    static juce::Path arcPath (juce::Point<float> c, float rx, float ry, float tilt, float a0, float a1)
    {
        juce::Path p;
        constexpr int steps = 56;
        for (int i = 0; i <= steps; ++i)
        {
            float x = 0.0f, y = 0.0f;
            ringPoint (c, rx, ry, tilt, a0 + (a1 - a0) * (float) i / (float) steps, x, y);
            if (i == 0) p.startNewSubPath (x, y);
            else        p.lineTo (x, y);
        }
        return p;
    }

    /** Draws the half of a ring from a0 to a1 (front = 0..pi, back = pi..2pi). */
    static void drawRing (juce::Graphics& g, juce::Point<float> c, float rx, float ry, float tilt,
                          float a0, float a1, juce::Colour col, bool on)
    {
        const auto path = arcPath (c, rx, ry, tilt, a0, a1);
        if (on)
        {
            g.setColour (col.withAlpha (0.14f));
            g.strokePath (path, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour (col.withAlpha (on ? 0.85f : 0.14f));
        g.strokePath (path, juce::PathStrokeType (on ? 1.6f : 0.9f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    /** Bead of voice v on its ring, drawn only in the half given by `front`. */
    void drawBead (juce::Graphics& g, juce::Point<float> c, float rx, float ry, float tilt, float th, int v,
                   bool on, float shift, float level, bool front) const
    {
        if (! on) return;
        const bool isFront = std::sin (th) >= 0.0f;
        if (isFront != front) return;

        using namespace humpire;
        const auto col = saturnVoiceColour (v);
        float x = 0.0f, y = 0.0f;
        ringPoint (c, rx, ry, tilt, th, x, y);

        // short comet tail behind the bead
        juce::Path tail;
        constexpr int steps = 10;
        for (int i = 0; i <= steps; ++i)
        {
            float tx = 0.0f, ty = 0.0f;
            ringPoint (c, rx, ry, tilt, th - 0.7f * (float) i / (float) steps, tx, ty);
            if (i == 0) tail.startNewSubPath (tx, ty);
            else        tail.lineTo (tx, ty);
        }
        g.setColour (col.withAlpha (0.35f));
        g.strokePath (tail, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float r = 2.6f + 4.0f * std::sqrt (level);
        drawGlowOrb (g, juce::Point<float> (x, y), r, juce::Colours::white.interpolatedWith (col, 0.4f), col);

        g.setFont (theme::text (10.0f));
        g.setColour (col.withAlpha(0.95f));
        g.drawText ((shift >= 0.0f ? "+" : "") + juce::String (shift, 1),
                    juce::Rectangle<float> (x + 6.0f, y - 15.0f, 44.0f, 12.0f), juce::Justification::left, false);
    }

    SaturnAudioProcessor& proc;
    std::array<std::atomic<float>*, saturn::kNumVoices> onFlag {};
    std::atomic<float>* keyFlag = nullptr;
    std::atomic<float>* scaleFlag = nullptr;
    std::array<float, saturn::kNumVoices> phase {};
    float lastT = -1.0f;
    float smoothLevel = 0.0f;
};
