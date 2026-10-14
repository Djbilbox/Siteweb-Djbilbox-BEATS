#pragma once
#include "Core/OrbitVisualizer.h"
#include "Core/HumpireLookAndFeel.h"
#include "PluginProcessor.h"

/** Mode Graphique : on dessine a la souris la fonction de transfert hauteur d'entree -> hauteur de sortie
    (classe de hauteur, 1 octave, 8 points / demi-ton). Escalier = gamme ; pentes douces = quarts de ton / glissandi.
    Clic gauche + glisser = dessiner, clic droit + glisser = effacer (retour a la diagonale). */
class PitchCurveEditor : public OrbitVisualizer
{
public:
    explicit PitchCurveEditor (SolarAudioProcessor& p) : proc (p) {}

    void mouseDown (const juce::MouseEvent& e) override { lastIdx = -1; paintAt (e); }
    void mouseDrag (const juce::MouseEvent& e) override { paintAt (e); }
    void mouseUp (const juce::MouseEvent&) override { lastIdx = -1; proc.commitGraphToState(); }

protected:
    void drawScene (juce::Graphics& g, juce::Rectangle<float> area, float t) override
    {
        using namespace humpire;
        plot = area.reduced (34.0f, 26.0f);
        const float sunR = plot.getHeight() * 0.05f;
        drawGlowOrb (g, plot.getCentre(), sunR, juce::Colour (0xffFFE9B0), theme::ember);   // soleil en filigrane

        // grille : demi-tons (x) et sorties (y)
        g.setFont (theme::text (10.0f));
        for (int s = 0; s <= 12; ++s)
        {
            const float x = plot.getX() + plot.getWidth() * (float) s / 12.0f;
            g.setColour (s % 12 == 0 ? theme::edge.brighter (0.3f) : theme::edge.withAlpha (0.6f));
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
            if (s < 12)
            {
                g.setColour (theme::textLo);
                g.drawText (solar::noteName (s), (int) x - 14, (int) plot.getBottom() + 4, 28, 14, juce::Justification::centred, false);
            }
        }
        for (int s = (int) kYMin; s <= (int) kYMax; ++s)
        {
            const float y = yFor ((float) s);
            g.setColour (theme::edge.withAlpha (s % 12 == 0 ? 0.9f : 0.35f));
            g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        }
        g.setColour (theme::textLo);
        g.drawText ("ENTREE (classe de hauteur)", plot.getX(), plot.getBottom() + 16.0f, plot.getWidth(), 12.0f, juce::Justification::centred, false);

        // diagonale = pas de correction
        g.setColour (theme::violet.withAlpha (0.55f));
        g.drawLine (plot.getX(), yFor (0.0f), plot.getRight(), yFor (12.0f), 1.0f);

        // courbe dessinee
        juce::Path curve;
        for (int i = 0; i < solar::kGraphPoints; ++i)
        {
            const float pc = (float) i / 8.0f;
            const float out = pc + proc.graphDelta[(size_t) i].load (std::memory_order_relaxed);
            const juce::Point<float> pt (xFor (pc), yFor (out));
            if (i == 0) curve.startNewSubPath (pt); else curve.lineTo (pt);
        }
        for (int k = 3; k >= 1; --k)
        {
            g.setColour (theme::amber.withAlpha (0.07f * (float) (4 - k)));
            g.strokePath (curve, juce::PathStrokeType (2.0f + (float) k * 3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour (theme::amber);
        g.strokePath (curve, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // point vivant : hauteur chantee -> hauteur corrigee
        const float dm = proc.uiDetMidi.load();
        if (dm > 0.0f)
        {
            float pc = std::fmod (dm, 12.0f); if (pc < 0.0f) pc += 12.0f;
            drawGlowOrb (g, { xFor (pc), yFor (pc + proc.uiOffset.load()) }, 4.5f, theme::cyan, theme::violet);
        }
        g.setColour (theme::textLo);
        g.setFont (theme::pixel (7.0f));
        g.drawText ("GRAPH  -  DESSINE LA COURBE DE PITCH", getLocalBounds().toFloat().removeFromTop (22.0f).reduced (10, 0), juce::Justification::centredLeft, false);
        juce::ignoreUnused (t);
    }

private:
    static constexpr float kYMin = -3.0f, kYMax = 15.0f;
    float xFor (float pc) const { return plot.getX() + plot.getWidth() * pc / 12.0f; }
    float yFor (float out) const { return plot.getBottom() - plot.getHeight() * (out - kYMin) / (kYMax - kYMin); }

    void paintAt (const juce::MouseEvent& e)
    {
        if (plot.getWidth() <= 1.0f) return;
        const bool erase = e.mods.isPopupMenu();
        const float pc = juce::jlimit (0.0f, 11.99f, (e.position.x - plot.getX()) / plot.getWidth() * 12.0f);
        const int idx = juce::jlimit (0, solar::kGraphPoints - 1, (int) std::lround (pc * 8.0f));
        const float out = kYMin + (plot.getBottom() - e.position.y) / plot.getHeight() * (kYMax - kYMin);

        auto setPoint = [&] (int i, float outVal)
        {
            const float pci = (float) i / 8.0f;
            proc.graphDelta[(size_t) i].store (erase ? 0.0f : juce::jlimit (-6.0f, 6.0f, outVal - pci), std::memory_order_relaxed);
        };
        if (lastIdx >= 0 && std::abs (idx - lastIdx) > 1)      // remplit les trous lors d'un geste rapide
        {
            const int step = idx > lastIdx ? 1 : -1;
            for (int i = lastIdx; i != idx; i += step)
                setPoint (i, lastOut + (out - lastOut) * (float) (i - lastIdx) / (float) (idx - lastIdx));
        }
        setPoint (idx, out);
        lastIdx = idx; lastOut = out;
        repaint();
    }

    SolarAudioProcessor& proc;
    juce::Rectangle<float> plot;
    int lastIdx = -1;
    float lastOut = 0.0f;
};
