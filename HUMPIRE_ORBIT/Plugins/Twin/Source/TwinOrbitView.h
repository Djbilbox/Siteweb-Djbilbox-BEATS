#pragma once
#include <cmath>
#include "Core/OrbitVisualizer.h"
#include "Core/HumpireLookAndFeel.h"
#include "PluginProcessor.h"

/** Scene HUMPIRE TWIN : systeme binaire en perspective.
    - La voix principale (lead) = planete ambre au premier plan, au centre.
    - Chaque doublure = planete jumelle violette posee sur l'anneau orbital arriere :
      position gauche/droite = panoramique reel (Width), eloignement = retard courant (glisse avec le detune),
      petite orbite epicyclique = humanisation (amplitude Humanize, vitesse Drift),
      teinte = detune courant (cyan si plus aigu, magenta si plus grave).
    - Une traine lumineuse relie le lead a chaque jumelle ; des photons la parcourent en un temps
      proportionnel au retard (la lumiere du lead "arrive" chez sa jumelle apres Delay ms).
    - Une lueur binaire douce baigne le barycentre ; tout pulse avec le niveau de sortie. */
class TwinOrbitView : public OrbitVisualizer
{
public:
    explicit TwinOrbitView (TwinAudioProcessor& p) : proc (p) {}

protected:
    void drawScene (juce::Graphics& g, juce::Rectangle<float> area, float t) override
    {
        using namespace humpire;
        auto& ap = proc.apvts;
        const float level = juce::jlimit (0.0f, 1.0f, proc.outLevel.load() * 1.5f);
        smoothLevel += (level - smoothLevel) * 0.25f;
        const bool bypass = ap.getRawParameterValue (twin::id::bypass)->load() > 0.5f;
        const int numVoices = juce::jlimit (1, twin::kMaxVoices, (int) std::lround (ap.getRawParameterValue (twin::id::voices)->load()));
        const float hum   = ap.getRawParameterValue (twin::id::humanize)->load() * 0.01f;
        const float drift = ap.getRawParameterValue (twin::id::drift)->load();
        const float width = ap.getRawParameterValue (twin::id::width)->load();
        const float dim = bypass ? 0.35f : 1.0f;

        const juce::Point<float> c (area.getCentreX(), area.getCentreY() + area.getHeight() * 0.07f);
        const float rx = area.getWidth() * 0.40f, ry = rx * 0.34f;
        const juce::Point<float> lead (c.x, c.y + ry * 0.38f);

        // --- positions des jumelles ---
        juce::Point<float> pos[twin::kMaxVoices];
        float env[twin::kMaxVoices], cents[twin::kMaxVoices], dly[twin::kMaxVoices], lvl[twin::kMaxVoices];
        juce::Point<float> centroid = lead;
        float wsum = 1.0f;
        for (int k = 0; k < twin::kMaxVoices; ++k)
        {
            const auto& u = proc.voiceUi[(size_t) k];
            env[k] = juce::jlimit (0.0f, 1.0f, u.env.load (std::memory_order_relaxed));
            cents[k] = u.cents.load (std::memory_order_relaxed);
            dly[k] = u.delayMs.load (std::memory_order_relaxed);
            lvl[k] = juce::jlimit (0.0f, 1.0f, u.level.load (std::memory_order_relaxed) * 2.0f);
            const float pan = juce::jlimit (-1.0f, 1.0f, u.pan.load (std::memory_order_relaxed));
            const float a = juce::MathConstants<float>::pi * (1.5f + 0.5f * pan);
            const float s = 0.58f + 0.42f * juce::jlimit (0.0f, 1.0f, dly[k] / 48.0f);
            const float w = juce::MathConstants<float>::twoPi * drift * 0.8f;
            const float epi = 2.0f + hum * 10.0f;
            pos[k] = { c.x + rx * s * std::cos (a) + epi * std::cos (t * w + (float) k * 1.7f),
                       c.y + ry * s * std::sin (a) - 6.0f + epi * 0.6f * std::sin (t * w * 1.3f + (float) k) };
            if (env[k] > 0.01f) { centroid += pos[k] * env[k]; wsum += env[k]; }
        }
        centroid /= wsum;

        // --- lueur binaire au barycentre ---
        {
            const float gr = rx * (0.55f + 0.25f * smoothLevel);
            juce::ColourGradient glow (theme::violet.withAlpha (0.22f * dim), centroid, theme::violet.withAlpha (0.0f), centroid.translated (gr, 0), true);
            glow.addColour (0.25, theme::amber.withAlpha (0.08f * dim));
            g.setGradientFill (glow);
            g.fillEllipse (centroid.x - gr, centroid.y - gr * 0.6f, gr * 2.0f, gr * 1.2f);
        }

        // --- anneaux orbitaux (moitie arriere) + axe miroir ---
        drawHalfRing (g, c, rx, ry, true, theme::violet.withAlpha (0.35f * dim), 1.2f);
        drawHalfRing (g, c, rx * 0.72f, ry * 0.72f, true, theme::violet.withAlpha (0.14f * dim), 0.8f);
        {
            juce::Path axis;
            axis.startNewSubPath (c.x, area.getY() + 30.0f);
            axis.lineTo (c.x, area.getBottom() - 30.0f);
            const float dashes[] = { 3.0f, 6.0f };
            juce::Path dashed;
            juce::PathStrokeType (0.8f).createDashedStroke (dashed, axis, dashes, 2);
            g.setColour (theme::edge.withAlpha (0.8f));
            g.fillPath (dashed);
        }
        g.setFont (theme::pixel (8.0f));
        g.setColour (theme::textLo.withAlpha (dim));
        g.drawText ("L", juce::Rectangle<float> (c.x - rx - 24.0f, c.y - 8.0f, 16.0f, 16.0f), juce::Justification::centred, false);
        g.drawText ("R", juce::Rectangle<float> (c.x + rx + 8.0f, c.y - 8.0f, 16.0f, 16.0f), juce::Justification::centred, false);

        // --- traines lumineuses lead -> jumelles, photons voyageant a la vitesse du retard ---
        for (int k = 0; k < twin::kMaxVoices; ++k)
        {
            if (env[k] < 0.01f) continue;
            const auto p = pos[k];
            const juce::Point<float> ctrl ((lead.x + p.x) * 0.5f, juce::jmin (lead.y, p.y) - 42.0f);
            juce::Path trail;
            trail.startNewSubPath (lead);
            trail.quadraticTo (ctrl, p);
            const float alpha = env[k] * dim * (0.20f + 0.55f * juce::jmax (smoothLevel, lvl[k]));
            g.setGradientFill (juce::ColourGradient (theme::amber.withAlpha (alpha), lead, twinColour (cents[k]).withAlpha (alpha), p, false));
            g.strokePath (trail, juce::PathStrokeType (1.0f + 2.0f * lvl[k], juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            const float travel = 0.25f + juce::jmax (0.0f, dly[k]) * 0.03f;
            for (int j = 0; j < 3; ++j)
            {
                float u = t / travel + (float) j / 3.0f + (float) k * 0.21f;
                u -= std::floor (u);
                const float iu = 1.0f - u;
                const juce::Point<float> q (iu * iu * lead.x + 2.0f * iu * u * ctrl.x + u * u * p.x,
                                            iu * iu * lead.y + 2.0f * iu * u * ctrl.y + u * u * p.y);
                const float fa = std::sin (juce::MathConstants<float>::pi * u) * env[k] * dim;
                g.setColour (juce::Colours::white.interpolatedWith (theme::amber, iu).withAlpha (0.85f * fa));
                g.fillEllipse (q.x - 1.6f, q.y - 1.6f, 3.2f, 3.2f);
            }
        }

        // --- planetes jumelles ---
        for (int k = 0; k < twin::kMaxVoices; ++k)
        {
            if (env[k] < 0.01f) continue;
            const auto p = pos[k];
            const auto col = twinColour (cents[k]).withMultipliedAlpha (dim);
            const float r = (6.5f + 3.5f * lvl[k] + 2.0f * smoothLevel) * (0.6f + 0.4f * env[k]);
            drawOrbitRing (g, p, r * 2.1f, r * 0.7f, -0.25f, col.withAlpha (0.35f * env[k] * dim), 0.8f);
            drawGlowOrb (g, p, r, col, theme::violet.withMultipliedAlpha (env[k] * dim));

            g.setFont (theme::pixel (6.0f));
            g.setColour (theme::textHi.withAlpha (0.85f * env[k] * dim));
            g.drawText ("T" + juce::String (k + 1), juce::Rectangle<float> (p.x - 20.0f, p.y - r - 16.0f, 40.0f, 10.0f), juce::Justification::centred, false);
            g.setFont (theme::text (10.0f));
            g.setColour (theme::textLo.withAlpha (env[k] * dim));
            g.drawText ((cents[k] >= 0.0f ? "+" : "") + juce::String (cents[k], 1) + " ct  " + juce::String (dly[k], 1) + " ms",
                        juce::Rectangle<float> (p.x - 60.0f, p.y + r + 5.0f, 120.0f, 12.0f), juce::Justification::centred, false);
        }

        // --- moitie avant de l'anneau (passe devant les jumelles, derriere le lead) ---
        drawHalfRing (g, c, rx, ry, false, theme::amber.withAlpha (0.30f * dim), 1.4f);
        drawHalfRing (g, c, rx * 0.72f, ry * 0.72f, false, theme::amber.withAlpha (0.10f * dim), 0.8f);

        // --- planete lead (voix principale) ---
        {
            const float r = 17.0f + 9.0f * smoothLevel + 0.8f * std::sin (t * 2.2f);
            g.setColour (theme::amber.withAlpha (0.10f * dim));
            g.fillEllipse (lead.x - r * 2.6f, lead.y - r * 0.9f, r * 5.2f, r * 1.8f);
            drawGlowOrb (g, lead, r, theme::amber.withMultipliedAlpha (dim), theme::ember.withMultipliedAlpha (dim));
            g.setFont (theme::pixel (6.0f));
            g.setColour (juce::Colour (0xff1A0F05));
            g.drawText ("LEAD", juce::Rectangle<float> (lead.x - r, lead.y - 4.0f, r * 2.0f, 9.0f), juce::Justification::centred, false);
        }

        // --- HUD ---
        g.setFont (theme::pixel (7.0f));
        g.setColour (theme::amber.withAlpha (0.9f));
        g.drawText (bypass ? "BYPASS" : "BINARY LOCK  " + juce::String (numVoices) + (numVoices > 1 ? " TWINS" : " TWIN"),
                    juce::Rectangle<float> (area.getX() + 16.0f, area.getY() + 14.0f, 260.0f, 12.0f), juce::Justification::centredLeft, false);
        g.setColour (theme::textLo);
        g.drawText ("WIDTH " + juce::String (juce::roundToInt (width)) + "%  HUMAN " + juce::String (juce::roundToInt (hum * 100.0f)) + "%",
                    juce::Rectangle<float> (area.getRight() - 276.0f, area.getY() + 14.0f, 260.0f, 12.0f), juce::Justification::centredRight, false);
        g.setFont (theme::pixel (6.0f));
        g.drawText ("LATENCE 0 MS", juce::Rectangle<float> (area.getX() + 16.0f, area.getBottom() - 24.0f, 200.0f, 10.0f), juce::Justification::centredLeft, false);
    }

private:
    /** Teinte d'une jumelle selon son detune courant. */
    static juce::Colour twinColour (float centsNow)
    {
        const float a = juce::jlimit (0.0f, 1.0f, std::abs (centsNow) / 40.0f);
        const auto base = humpire::theme::violet.brighter (0.35f);
        return centsNow >= 0.0f ? base.interpolatedWith (humpire::theme::cyan, a * 0.8f)
                                : base.interpolatedWith (juce::Colour (0xffE05CFF), a * 0.8f);
    }

    /** Demi-ellipse (arriere = moitie haute, avant = moitie basse). */
    static void drawHalfRing (juce::Graphics& g, juce::Point<float> c, float rx, float ry, bool back, juce::Colour col, float thickness)
    {
        juce::Path p;
        constexpr int steps = 48;
        for (int i = 0; i <= steps; ++i)
        {
            const float a = juce::MathConstants<float>::pi * ((back ? 1.0f : 0.0f) + (float) i / (float) steps);
            const juce::Point<float> q (c.x + rx * std::cos (a), c.y + ry * std::sin (a));
            if (i == 0) p.startNewSubPath (q); else p.lineTo (q);
        }
        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (thickness));
    }

    TwinAudioProcessor& proc;
    float smoothLevel = 0.0f;
};
