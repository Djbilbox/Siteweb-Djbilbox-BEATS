#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Palette.h"
#include "RockpunkLookAndFeel.h"

namespace rk
{

//==============================================================================
// Level fader (0..1), horizontal or vertical. The track is a direct crop of the
// artwork (RockpunkLookAndFeel::drawSliderSprite) with a translucent neon fill
// over the active portion. Same glow behaviour as the knobs while a note is
// triggering. In vertical mode no label text is drawn (the artwork carries its
// own labels) - it is also used for the pitch / mod wheels.
//==============================================================================
class LevelSlider final : public juce::Component
{
public:
    LevelSlider (const juce::String& labelText, RockpunkLookAndFeel& lfRef,
                 const juce::String& spriteKey, bool vertical = false)
        : levelLabel (labelText),
          artwork (lfRef),
          key (spriteKey),
          isVertical (vertical)
    {
        slider.setSliderStyle (isVertical ? juce::Slider::LinearVertical
                                          : juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (&sliderLnF);
        addAndMakeVisible (slider);
    }

    juce::Slider slider;

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void resized() override { slider.setBounds (getLocalBounds()); }

    // La boite est aussi large que le cabochon dessine, pour pouvoir le
    // decouper en entier dans l'image ; mais les cabochons voisins se touchent
    // sur la facade, et deux boites qui se chevauchent se voleraient la souris.
    // Seule la bande centrale, celle du rail, repond donc au clic.
    bool hitTest (int x, int y) override
    {
        if (! isVertical)
            return true;

        const int half = juce::jmax (6, getWidth() / 5);
        return std::abs (x - getWidth() / 2) <= half && y >= 0 && y < getHeight();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        const float v = (float) slider.getNormalisableRange().convertTo0to1 (slider.getValue());

        if (isVertical)
        {
            // On ne repeint plus rien du fader.
            //
            // Le rail et son cabochon sont dessines dans l'image de fond, a une
            // position fixe. Les redessiner obligeait a d'abord les effacer, et
            // aucun effacement automatique ne tient : le rail a des extremites
            // arrondies, un halo en bas et des graduations sur les cotes, donc
            // rien ne permet de reconstituer ce qui est cache sous le cabochon.
            // Chaque tentative laissait un rectangle sur la facade.
            //
            // Le repere de valeur est donc pose dans la marge, a cote du rail,
            // la ou il n'y a que du metal nu : l'artwork reste intact et la
            // valeur reste lisible.
            drawMargeMarker (g, r, v);
            return;
        }

        if (glow > 0.02f)
        {
            g.setColour (Palette::neonRose().withAlpha (0.30f * glow));
            g.fillRoundedRectangle (r.expanded (2.0f), 5.0f);
        }

        if (artwork.hasSprite (key))
            artwork.drawSliderSprite (g, key, v, getLocalBounds());
        else
            drawFallback (g, r, v);

        if (! isVertical)
        {
            g.setColour (glow > 0.02f ? Palette::neonViolet() : Palette::textDim());
            g.setFont (juce::Font (9.0f).boldened());
            g.drawText (levelLabel + "  " + juce::String ((int) (v * 100.0f)) + "%",
                        juce::Rectangle<float> (r.getX() + 4.0f, r.getY() + 1.0f,
                                                r.getWidth() - 8.0f, r.getHeight() - 2.0f),
                        juce::Justification::centred);
        }
    }

private:
    // Petit chevron lumineux colle au bord de la boite, a la hauteur de la
    // valeur. Il vise le rail sans jamais mordre dessus.
    void drawMargeMarker (juce::Graphics& g, const juce::Rectangle<float>& r, float v) const
    {
        const float size = juce::jlimit (5.0f, 9.0f, r.getWidth() * 0.16f);
        const float travel = juce::jmax (0.0f, r.getHeight() - 2.0f * size);
        const float y = r.getBottom() - size - travel * v;

        juce::Path chevron;
        chevron.addTriangle (r.getX(), y - size,
                             r.getX(), y + size,
                             r.getX() + size * 1.3f, y);

        // Un seul chevron, colle au rail. Une paire de part et d'autre finissait
        // par toucher le rail voisin - les cinq faders ne sont separes que de
        // 43 px - et la rangee devenait un semis de triangles dont on ne savait
        // plus lequel appartenait a qui.
        const float lift = 0.55f + 0.45f * glow;
        g.setColour (Palette::neonViolet().withAlpha (0.35f * lift));
        g.fillPath (chevron, juce::AffineTransform::translation (-1.0f, 0.0f));
        g.setColour (Palette::neonRose().withAlpha (lift));
        g.fillPath (chevron);
    }

    void drawFallback (juce::Graphics& g, const juce::Rectangle<float>& r, float v) const
    {
        g.setColour (Palette::off());
        g.fillRoundedRectangle (r, 4.0f);

        juce::ColourGradient fillGrad (Palette::red(), r.getX(), r.getY(),
                                       Palette::orange(), r.getRight(), r.getY(), false);
        g.setGradientFill (fillGrad);

        if (isVertical)
            g.fillRoundedRectangle (juce::Rectangle<float> (r.getX(), r.getBottom() - r.getHeight() * v,
                                                            r.getWidth(), r.getHeight() * v), 4.0f);
        else
            g.fillRoundedRectangle (juce::Rectangle<float> (r.getX(), r.getY(),
                                                            r.getWidth() * v, r.getHeight()), 4.0f);

        g.setColour (Palette::chrome());
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    }

    BlankSliderLookAndFeel sliderLnF;
    RockpunkLookAndFeel& artwork;
    juce::String levelLabel;
    juce::String key;
    bool isVertical = false;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace rk