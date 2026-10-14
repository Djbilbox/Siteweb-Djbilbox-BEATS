#include "TopBar.h"
#include "HumpireTheme.h"

using namespace humpire;

HumpireTopBar::HumpireTopBar (juce::AudioProcessorValueTreeState& a, PresetManager& pm, const juce::String& productName,
                              const std::atomic<float>& inL, const std::atomic<float>& outL)
    : apvts (a), presets (pm), product (productName), inMeter (inL), outMeter (outL)
{
    for (auto* b : { &prevBtn, &nextBtn, &nameBtn, &saveBtn, &abBtn, &undoBtn, &redoBtn })
        addAndMakeVisible (*b);
    addAndMakeVisible (bypassBtn);
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    bypassBtn.setColour (juce::ToggleButton::tickColourId, theme::danger);
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "bypass", bypassBtn);

    prevBtn.onClick = [this] { presets.prev(); };
    nextBtn.onClick = [this] { presets.next(); };
    nameBtn.onClick = [this] { presets.showMenu (&nameBtn); };
    saveBtn.onClick = [this] { askSaveName(); };
    abBtn.onClick   = [this] { presets.toggleAB(); abBtn.setButtonText (presets.getABSlot() == 0 ? "A" : "B"); };
    undoBtn.onClick = [this] { if (apvts.undoManager != nullptr) apvts.undoManager->undo(); };
    redoBtn.onClick = [this] { if (apvts.undoManager != nullptr) apvts.undoManager->redo(); };

    abBtn.setTooltip ("Compare A/B");
    presets.onPresetChanged = [this] { refreshPresetName(); };
    presets.onOrbitComplete = [this] { showOrbitComplete(); };
    refreshPresetName();
    abBtn.setButtonText (presets.getABSlot() == 0 ? "A" : "B");
}

HumpireTopBar::~HumpireTopBar()
{
    presets.onPresetChanged = nullptr;
    presets.onOrbitComplete = nullptr;
}

void HumpireTopBar::refreshPresetName() { nameBtn.setButtonText (presets.getCurrentName()); }

void HumpireTopBar::showOrbitComplete()
{
    badgeStart = juce::Time::currentTimeMillis();
    startTimerHz (30);
}

void HumpireTopBar::timerCallback()
{
    if (juce::Time::currentTimeMillis() - badgeStart > 2600) stopTimer();
    repaint();
}

void HumpireTopBar::askSaveName()
{
    auto* w = new juce::AlertWindow ("Sauvegarder le preset", "Nom du preset :", juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("name", presets.getCurrentName());
    w->addButton ("Sauver", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Annuler", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<HumpireTopBar> self (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([self, w] (int r)
    {
        if (r == 1 && self != nullptr) self->presets.saveUserPreset (w->getTextEditorContents ("name"));
    }), true);
}

void HumpireTopBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (theme::panelHi, 0, 0, theme::panel, 0, r.getHeight(), false));
    g.fillRect (r);
    g.setColour (theme::edge);
    g.drawHorizontalLine (getHeight() - 1, 0.0f, r.getWidth());

    // Logo : etoile + "HUMPIRE" + nom du produit
    const juce::Point<float> star (26.0f, r.getCentreY());
    juce::Path p;
    p.addStar (star, 4, 3.0f, 10.0f, juce::MathConstants<float>::pi * 0.25f);
    g.setGradientFill (theme::starGradient (star.translated (-10, -10), star.translated (10, 10)));
    g.fillPath (p);
    g.setColour (theme::amber.withAlpha (0.18f));
    g.fillEllipse (star.x - 14, star.y - 14, 28, 28);

    g.setFont (theme::title (18.0f));
    g.setColour (theme::textHi);
    g.drawText ("HUMPIRE", 46, 6, 100, 22, juce::Justification::centredLeft, false);
    g.setFont (theme::pixel (7.0f));
    g.setColour (theme::amber);
    g.drawText (product.toUpperCase(), 46, 29, 150, 14, juce::Justification::centredLeft, false);

    g.setFont (theme::pixel (6.0f));
    g.setColour (theme::textLo);
    g.drawText ("IN",  inMeter.getX() - 26,  inMeter.getY() - 3,  22, 14, juce::Justification::centredLeft, false);
    g.drawText ("OUT", outMeter.getX() - 26, outMeter.getY() - 3, 24, 14, juce::Justification::centredLeft, false);
}

void HumpireTopBar::paintOverChildren (juce::Graphics& g)
{
    const auto age = juce::Time::currentTimeMillis() - badgeStart;
    if (badgeStart == 0 || age > 2600) return;
    const float a = age < 300 ? (float) age / 300.0f : age > 2100 ? 1.0f - (float) (age - 2100) / 500.0f : 1.0f;
    auto b = nameBtn.getBounds().toFloat();   // le badge recouvre le nom du preset
    g.setColour (theme::bg.withAlpha (0.92f * a));
    g.fillRoundedRectangle (b, 10.0f);
    g.setGradientFill (juce::ColourGradient (theme::amber.withAlpha (a), b.getX(), 0, theme::ember.withAlpha (a), b.getRight(), 0, false));
    g.drawRoundedRectangle (b, 10.0f, 1.5f);
    g.setFont (theme::pixel (7.0f));
    g.drawText ("ORBIT COMPLETE", b, juce::Justification::centred, false);
}

void HumpireTopBar::resized()
{
    const int h = getHeight(), y = 10, bh = h - 20;
    auto meters = getLocalBounds().reduced (12, 0).removeFromRight (150);
    inMeter.setBounds (meters.getX() + 26, h / 2 - 11, meters.getWidth() - 26, 8);
    outMeter.setBounds (meters.getX() + 26, h / 2 + 3, meters.getWidth() - 26, 8);

    int x = 200;
    auto place = [&] (juce::Component& c, int w, int gapAfter) { c.setBounds (x, y, w, bh); x += w + gapAfter; };
    place (prevBtn, 28, 4);
    place (nameBtn, 210, 4);
    place (nextBtn, 28, 8);
    place (saveBtn, 48, 12);
    place (abBtn, 32, 8);
    place (undoBtn, 48, 4);
    place (redoBtn, 48, 12);
    place (bypassBtn, 70, 0);
}
