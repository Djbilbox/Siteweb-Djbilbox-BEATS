#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

#include "../PluginProcessor.h"
#include "Palette.h"
#include "WarzoneLookAndFeel.h"

namespace wz
{

//==============================================================================
// Expansion cassette - preset cartridge manager styled as a tape cassette.
// The cassette artwork is a direct crop of the background image; the buttons
// stay transparent so the artwork shows through (WarzoneLookAndFeel handles
// their hover/press neon). Cartridges are .rpk files (plain XML of the APVTS
// state) stored in the project's Presets folder.
//==============================================================================
class ExpansionCassette final : public juce::Component
{
public:
    ExpansionCassette (PluginProcessor& p, WarzoneLookAndFeel& lfRef)
        : proc (p),
          artwork (lfRef)
    {
        title.setColour (juce::Label::textColourId, Palette::text());
        title.setFont (juce::Font (12.0f).boldened());
        title.setText ("EXPANSION CASSETTE", juce::dontSendNotification);
        addAndMakeVisible (title);

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
        prevButton.onClick = [this] { step (-1); };
        nextButton.onClick = [this] { step (1); };

        refreshList();
    }

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void resized() override
    {
        const auto r = getLocalBounds();
        title.setBounds (4, 4, r.getWidth() - 8, 18);
        cartLabel.setBounds (4, 24, r.getWidth() - 8, 16);

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
            g.setColour (Palette::neonGreen().withAlpha (0.20f * glow));
            g.drawRoundedRectangle (r.expanded (2.0f), 6.0f, 2.0f);
        }

        g.setColour (Palette::chrome());
        g.drawRoundedRectangle (r, 6.0f, 1.0f);
    }

    void refreshList()
    {
        presets.clear();
        auto dir = proc.getPresetDirectory();
        if (dir.exists())
        {
            for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.rpk"))
                presets.add (f);
        }
        currentIndex = -1;
        updateCartridgeLabel();
    }

private:
    void updateCartridgeLabel()
    {
        if (currentIndex >= 0 && currentIndex < presets.size())
            cartLabel.setText (presets[(size_t) currentIndex].getFileNameWithoutExtension(),
                               juce::dontSendNotification);
        else
            cartLabel.setText ("no cartridge", juce::dontSendNotification);
    }

    void step (int dir)
    {
        if (presets.isEmpty())
            return;
        currentIndex += dir;
        if (currentIndex < 0)
            currentIndex = presets.size() - 1;
        if (currentIndex >= presets.size())
            currentIndex = 0;
        doLoadCurrent();
    }

    void doLoadCurrent()
    {
        if (currentIndex >= 0 && currentIndex < presets.size())
            loadPreset (presets[(size_t) currentIndex]);
    }

    void loadPreset (const juce::File& f)
    {
        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
        if (xml != nullptr)
            proc.apvts.replaceState (juce::ValueTree::fromXml (*xml));
        updateCartridgeLabel();
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
        juce::FileChooser fc ("Load WARZONE808 cartridge", proc.getPresetDirectory(), "*.rpk");
        if (fc.browseForFileToOpen())
            loadPreset (fc.getResult());
    }

    void doSave()
    {
        auto dir = proc.getPresetDirectory();
        dir.createDirectory();

        juce::FileChooser fc ("Save WARZONE808 cartridge",
                              dir.getChildFile ("WARZONE808_GUITAR_01.rpk"), "*.rpk");
        if (fc.browseForFileToSave (true))
        {
            auto f = fc.getResult();
            if (! f.hasFileExtension (".rpk"))
                f = f.withFileExtension (".rpk");
            f.replaceWithText (proc.apvts.state.toXmlString());
            refreshList();
        }
    }

    PluginProcessor& proc;
    WarzoneLookAndFeel& artwork;
    juce::Label title, cartLabel;
    juce::TextButton initButton, loadButton, saveButton, prevButton, nextButton;
    juce::Array<juce::File> presets;
    int currentIndex = -1;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace wz