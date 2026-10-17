#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <utility>
#include <vector>

#include "Palette.h"
#include "RockpunkLookAndFeel.h"

namespace rk
{

//==============================================================================
// Cycling switch for int / bool parameters (e.g. drive type, layer on/off).
// The graphic comes straight from the artwork: RockpunkLookAndFeel crops the
// real switch and rotates it into a filmstrip of N states. The state name is
// overlaid, and the whole thing glows neon while a note is triggering.
// No Attachment: it drives the raw AudioProcessorParameter directly and is
// re-synced from the host every UI tick (timerCallback in the editor).
//==============================================================================
class CyclingSwitch final : public juce::Component
{
public:
    CyclingSwitch (const juce::String& labelText, juce::AudioProcessorParameter& p,
                   std::vector<juce::String> names, RockpunkLookAndFeel& lfRef,
                   const juce::String& spriteKey)
        : label (labelText),
          param (p),
          stateNames (std::move (names)),
          artwork (lfRef),
          key (spriteKey)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void syncFromParam() noexcept { value01 = param.getValue(); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        const bool lit = value01 > 0.5f;
        const float glowLevel = juce::jmax (glow, lit ? 0.35f : 0.0f);

        // On ne repeint plus le controle.
        //
        // Une grille de haut-parleur et une liste deroulante ne sont pas
        // circulaires : les redecouper puis les redessiner par-dessus leur
        // propre dessin ne pouvait rien apporter, et le recadrage laissait ses
        // bords visibles sur la facade. Seuls les potards gagnent a etre
        // redessines, parce qu'un disque tourne retombe exactement dans son
        // logement.
        //
        // Assombrissement reserve aux grilles de haut-parleur, les seules dont
        // un etat signifie vraiment "coupe". Sur un selecteur, POLY ou DOUX ne
        // sont pas des extinctions, et le voile tombait sur leur libelle.
        if (! showStateText && ! lit)
        {
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.fillRoundedRectangle (r, 4.0f);
        }

        if (! showStateText)
            return;

        const int idx = stateNames.size() > 1
            ? juce::jlimit (0, (int) stateNames.size() - 1,
                            (int) (value01 * (float) (stateNames.size() - 1) + 0.5f))
            : 0;
        const juce::String txt = idx < (int) stateNames.size() ? stateNames[(size_t) idx] : label;

        // Texte seul, sans cadre ni fond : l'encadrer revenait a dessiner une
        // boite magenta autour de "HORS SERVICE", posee sur l'artwork.
        g.setColour (lit ? Palette::neonRose() : Palette::text());
        g.setFont (juce::Font (juce::jlimit (8.0f, 11.0f, r.getHeight() * 0.45f)).boldened());
        g.drawText (txt, r, juce::Justification::centred);
    }

    // Les grilles de haut-parleur n'affichent pas leur etat en toutes lettres :
    // un "ON" minuscule pose au milieu d'une grille de 260 px se lit comme une
    // salissure, pas comme une information.
    void setShowStateText (bool shouldShow) noexcept { showStateText = shouldShow; }

    void mouseDown (const juce::MouseEvent&) override
    {
        const int n = (int) stateNames.size();
        if (n < 2)
            return;

        float next = value01 + 1.0f / (float) (n - 1);
        if (next > 1.0f)
            next = 0.0f;

        param.beginChangeGesture();
        param.setValueNotifyingHost (next);
        param.endChangeGesture();
        value01 = next;
        repaint();
    }

private:
    void drawFallback (juce::Graphics& g, const juce::Rectangle<float>& r, bool lit, float glowLevel) const
    {
        g.setColour (lit ? Palette::red() : Palette::off());
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (glowLevel > 0.02f ? Palette::neonViolet() : Palette::chrome());
        g.drawRoundedRectangle (r, 4.0f, 1.2f);
    }

    juce::String label;
    bool showStateText = true;
    juce::AudioProcessorParameter& param;
    std::vector<juce::String> stateNames;
    RockpunkLookAndFeel& artwork;
    juce::String key;
    float value01 = 0.0f;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace rk