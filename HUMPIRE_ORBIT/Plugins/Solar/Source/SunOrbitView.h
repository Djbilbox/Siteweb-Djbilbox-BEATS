#pragma once
#include "Core/OrbitVisualizer.h"
#include "Core/HumpireLookAndFeel.h"
#include "PluginProcessor.h"

/** Soleil au centre ; chaque note autorisee de la gamme = une orbite (rayon = hauteur) ;
    la courbe de pitch (voix brute en violet, voix corrigee en ambre) tourne autour du soleil. */
class SunOrbitView : public OrbitVisualizer
{
public:
    explicit SunOrbitView (SolarAudioProcessor& p) : proc (p) {}

protected:
    void drawScene (juce::Graphics& g, juce::Rectangle<float> area, float t) override
    {
        using namespace humpire;
        const auto c = area.getCentre();
        const float R = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - 8.0f;
        const float sunR = R * 0.17f;
        const float level = juce::jlimit (0.0f, 1.0f, proc.outLevel.load() * 1.5f);
        smoothLevel += (level - smoothLevel) * 0.25f;

        // --- orbites = notes autorisees ---
        const int key = (int) *proc.apvts.getRawParameterValue (solar::id::key);
        const int scale = (int) *proc.apvts.getRawParameterValue (solar::id::scale);
        unsigned mask = 0;
        for (int i = 0; i < 12; ++i) if (*proc.apvts.getRawParameterValue (solar::noteId (i)) > 0.5f) mask |= 1u << i;
        const bool graphMode = *proc.apvts.getRawParameterValue (solar::id::mode) > 0.5f;
        const auto deg = solar::buildDegrees (key, scale, mask);
        const float tgt = proc.uiTargetMidi.load();

        for (int oct = 1; oct <= 6; ++oct)
            for (int i = 0; i < deg.n; ++i)
            {
                const float m = (float) (12 * oct + key) + deg.d[i];
                if (m < kLow || m > kHigh) continue;
                const bool tonic = i == 0;
                const bool active = tgt > 0.0f && std::abs (tgt - m) < 0.02f;
                const float r = radiusFor (m, R, sunR);
                g.setColour (active ? theme::amber.withAlpha (0.85f) : tonic ? theme::violet.withAlpha (0.45f) : theme::violet.withAlpha (0.16f));
                g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, active ? 1.8f : tonic ? 1.2f : 0.8f);
            }

        // --- soleil : couronne, rayons, noyau ---
        const float pulse = 1.0f + 0.12f * smoothLevel + 0.03f * std::sin (t * 2.0f);
        for (int k = 0; k < 18; ++k)
        {
            const float a = t * 0.08f + (float) k * juce::MathConstants<float>::twoPi / 18.0f;
            const float len = sunR * (1.5f + 0.5f * std::sin (t * 1.3f + (float) k * 1.7f)) * (1.0f + smoothLevel * 0.8f);
            juce::Path ray;
            ray.addTriangle (c.x + std::cos (a - 0.05f) * sunR, c.y + std::sin (a - 0.05f) * sunR,
                             c.x + std::cos (a + 0.05f) * sunR, c.y + std::sin (a + 0.05f) * sunR,
                             c.x + std::cos (a) * (sunR + len), c.y + std::sin (a) * (sunR + len));
            g.setColour (theme::amber.withAlpha (0.20f + 0.25f * smoothLevel));
            g.fillPath (ray);
        }
        juce::ColourGradient corona (theme::amber.withAlpha (0.55f), c, theme::ember.withAlpha (0.0f), c.translated (sunR * 3.2f * pulse, 0), true);
        g.setGradientFill (corona);
        g.fillEllipse (c.x - sunR * 3.2f * pulse, c.y - sunR * 3.2f * pulse, sunR * 6.4f * pulse, sunR * 6.4f * pulse);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffFFE9B0), c.translated (-sunR * 0.3f, -sunR * 0.3f),
                                                 theme::ember, c.translated (sunR, sunR), true));
        g.fillEllipse (c.x - sunR * pulse, c.y - sunR * pulse, sunR * 2.0f * pulse, sunR * 2.0f * pulse);

        // --- courbes de pitch en orbite ---
        float det[SolarAudioProcessor::historySize], out[SolarAudioProcessor::historySize];
        const int head = proc.historyHead.load (std::memory_order_acquire);
        for (int i = 0; i < SolarAudioProcessor::historySize; ++i)
        {
            const auto& f = proc.history[(size_t) ((head + i) % SolarAudioProcessor::historySize)];   // ancien -> recent
            det[i] = f.det.load (std::memory_order_relaxed);
            out[i] = f.out.load (std::memory_order_relaxed);
        }
        drawOrbitCurve (g, c, R, sunR, det, theme::violet.brighter (0.4f), 1.4f);
        drawOrbitCurve (g, c, R, sunR, out, theme::amber, 2.2f);

        // planetes en tete de courbe
        const auto head2 = pointFor (out[SolarAudioProcessor::historySize - 1], SolarAudioProcessor::historySize - 1, c, R, sunR);
        const auto head1 = pointFor (det[SolarAudioProcessor::historySize - 1], SolarAudioProcessor::historySize - 1, c, R, sunR);
        if (out[SolarAudioProcessor::historySize - 1] > 0.0f) drawGlowOrb (g, head2, 4.0f, theme::amber, theme::ember);
        if (det[SolarAudioProcessor::historySize - 1] > 0.0f) drawGlowOrb (g, head1, 2.6f, theme::violet.brighter (0.5f), theme::violet);

        // --- note au coeur du soleil ---
        const float dm = proc.uiDetMidi.load();
        g.setColour (juce::Colour (0xff1A0F05));
        if (dm > 0.0f && tgt > 0.0f)
        {
            const int n = (int) std::lround (graphMode ? dm + proc.uiOffset.load() : tgt);
            g.setFont (theme::title (sunR * 0.9f));
            g.drawText (juce::String (solar::noteName (n)) + juce::String (n / 12 - 1), juce::Rectangle<float> (c.x - sunR, c.y - sunR * 0.7f, sunR * 2.0f, sunR * 1.1f), juce::Justification::centred, false);
            g.setFont (theme::text (sunR * 0.38f));
            const int cents = (int) std::lround ((dm - (graphMode ? dm + proc.uiOffset.load() : tgt)) * 100.0f);
            g.drawText ((cents > 0 ? "+" : "") + juce::String (cents) + " ct", juce::Rectangle<float> (c.x - sunR, c.y + sunR * 0.25f, sunR * 2.0f, sunR * 0.5f), juce::Justification::centred, false);
        }
        else
        {
            g.setFont (theme::pixel (sunR * 0.28f));
            g.drawText ("SOLAR", juce::Rectangle<float> (c.x - sunR, c.y - sunR * 0.3f, sunR * 2.0f, sunR * 0.6f), juce::Justification::centred, false);
        }
    }

private:
    static constexpr float kLow = 36.0f, kHigh = 84.0f;     // C2 .. C6

    static float radiusFor (float midi, float R, float sunR)
    {
        const float f = juce::jlimit (0.0f, 1.0f, (midi - kLow) / (kHigh - kLow));
        return sunR * 1.9f + (R - sunR * 1.9f) * f;
    }

    static juce::Point<float> pointFor (float midi, int k, juce::Point<float> c, float R, float sunR)
    {
        const float a = -juce::MathConstants<float>::halfPi
                        + juce::MathConstants<float>::twoPi * (float) (k - (SolarAudioProcessor::historySize - 1)) / (float) SolarAudioProcessor::historySize;
        const float r = radiusFor (midi, R, sunR);
        return { c.x + r * std::cos (a), c.y + r * std::sin (a) };
    }

    static void drawOrbitCurve (juce::Graphics& g, juce::Point<float> c, float R, float sunR, const float* m, juce::Colour col, float w)
    {
        constexpr int N = SolarAudioProcessor::historySize, chunks = 6;
        for (int ch = 0; ch < chunks; ++ch)
        {
            const int a = ch * N / chunks, b = juce::jmin (N, (ch + 1) * N / chunks + 1);
            juce::Path p;
            bool open = false;
            for (int k = a; k < b; ++k)
            {
                if (m[k] <= 0.0f) { open = false; continue; }
                const auto pt = pointFor (m[k], k, c, R, sunR);
                if (! open) { p.startNewSubPath (pt); open = true; } else p.lineTo (pt);
            }
            const float alpha = 0.10f + 0.90f * (float) (ch + 1) / (float) chunks;
            g.setColour (col.withAlpha (alpha * 0.25f));
            g.strokePath (p, juce::PathStrokeType (w * 3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (col.withAlpha (alpha));
            g.strokePath (p, juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    SolarAudioProcessor& proc;
    float smoothLevel = 0.0f;
};
