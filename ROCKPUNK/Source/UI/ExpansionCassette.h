#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

#include "../PluginProcessor.h"
#include "Palette.h"
#include "RockpunkLookAndFeel.h"

namespace rk
{

//==============================================================================
// Expansion cassette - preset cartridge manager styled as a tape cassette.
// The cassette artwork is a direct crop of the background image; the buttons
// stay transparent so the artwork shows through (RockpunkLookAndFeel handles
// their hover/press neon). Cartridges are .rpk files (plain XML of the APVTS
// state) stored in the project's Presets folder.
//==============================================================================
class ExpansionCassette final : public juce::Component
{
public:
    ExpansionCassette (PluginProcessor& p, RockpunkLookAndFeel& lfRef)
        : proc (p),
          artwork (lfRef)
    {
        // Pas de titre dessine : l'image affiche deja "EXPANSION" en lettrage.
        cartLabel.setColour (juce::Label::textColourId, Palette::textDim());
        cartLabel.setFont (juce::Font (10.0f));
        cartLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (cartLabel);

        initButton.setButtonText ("INIT");
        loadButton.setButtonText ("LOAD");
        saveButton.setButtonText ("SAVE");
        prevButton.setButtonText ("<");
        nextButton.setButtonText (">");

        for (auto* b : { &initButton, &loadButton, &saveButton, &prevButton, &nextButton })
        {
            b->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            b->setColour (juce::TextButton::textColourOffId, Palette::text());
            b->setColour (juce::TextButton::textColourOnId, Palette::ink());
            b->setColour (juce::TextButton::buttonOnColourId, Palette::red());
            b->setLookAndFeel (&artwork);
            addAndMakeVisible (b);
        }

        initButton.onClick = [this] { doInit(); };
        loadButton.onClick = [this] { doLoad(); };
        saveButton.onClick = [this] { doSave(); };
        prevButton.onClick = [this] { proc.stepPreset (-1); updateCartridgeLabel(); };
        nextButton.onClick = [this] { proc.stepPreset (1);  updateCartridgeLabel(); };

        refreshList();
    }

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();

        // Le preset peut avoir ete change depuis le bandeau du haut.
        if (proc.getCurrentPresetName() != cartLabel.getText())
            updateCartridgeLabel();
    }

    void resized() override
    {
        const auto r = getLocalBounds();
        // Le nom de la cartouche va sur l'etiquette blanche de la cassette
        // dessinee, pas au-dessus d'elle.
        cartLabel.setBounds (r.getWidth() / 4, (int) (r.getHeight() * 0.34f),
                             r.getWidth() / 2, 16);

        const int by = r.getHeight() - 30;
        const int bw = (r.getWidth() - 16) / 5;
        initButton.setBounds (4, by, bw, 24);
        loadButton.setBounds (8 + bw, by, bw, 24);
        saveButton.setBounds (12 + 2 * bw, by, bw, 24);
        prevButton.setBounds (16 + 3 * bw, by, bw, 24);
        nextButton.setBounds (20 + 4 * bw, by, bw, 24);
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();

        if (artwork.hasSprite ("cassette"))
        {
            artwork.drawCropSprite (g, "cassette", getLocalBounds());
        }
        else
        {
            g.setColour (Palette::panel());
            g.fillRoundedRectangle (r, 6.0f);
        }

        if (glow > 0.02f)
        {
            g.setColour (Palette::neonViolet().withAlpha (0.20f * glow));
            g.drawRoundedRectangle (r.expanded (2.0f), 6.0f, 2.0f);
        }

        g.setColour (Palette::chrome());
        g.drawRoundedRectangle (r, 6.0f, 1.0f);
    }

    void refreshList()
    {
        proc.refreshPresetList();
        updateCartridgeLabel();
    }

private:
    void updateCartridgeLabel()
    {
        cartLabel.setText (proc.getPresetFiles().isEmpty() ? "no cartridge"
                                                           : proc.getCurrentPresetName(),
                           juce::dontSendNotification);
    }

    void doInit()
    {
        for (auto* p : proc.getParameters())
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->getDefaultValue());
            p->endChangeGesture();
        }
    }

    void doLoad()
    {
        juce::FileChooser fc ("Charger une cartouche ROCKPUNK", proc.getPresetDirectory(), "*.rpk");
        if (fc.browseForFileToOpen())
        {
            proc.loadPreset (fc.getResult());
            updateCartridgeLabel();
        }
    }

    void doSave()
    {
        auto dir = proc.getPresetDirectory();
        dir.createDirectory();

        juce::FileChooser fc ("Enregistrer la cartouche ROCKPUNK",
                              dir.getChildFile ("Ma cartouche.rpk"), "*.rpk");
        if (fc.browseForFileToSave (true))
        {
            proc.savePreset (fc.getResult());
            updateCartridgeLabel();
        }
    }

    PluginProcessor& proc;
    RockpunkLookAndFeel& artwork;
    juce::Label cartLabel;
    juce::TextButton initButton, loadButton, saveButton, prevButton, nextButton;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace rk