#include "TopBar.h"

namespace humpire
{
TopBar::TopBar (juce::AudioProcessorValueTreeState& s, PresetManager& p, juce::UndoManager& u,
                std::atomic<float>& inPeak, std::atomic<float>& outPeak, const juce::String& t)
    : apvts (s), presets (p), undo (u), title (t),
      bypassAttach (s, "bypass", bypassBtn),
      inMeter (inPeak, LedMeter::Orientation::horizontal, 16),
      outMeter (outPeak, LedMeter::Orientation::horizontal, 16)
{
    for (auto* b : { &prevBtn, &nextBtn, &nameBtn, &saveBtn, &abBtn, &undoBtn, &redoBtn, &bypassBtn })
        addAndMakeVisible (*b);
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    bypassBtn.setClickingTogglesState (true);
    bypassBtn.setColour (juce::TextButton::buttonOnColourId, col::red);
    abBtn.setClickingTogglesState (false);

    prevBtn.onClick = [this] { presets.prev(); };
    nextBtn.onClick = [this] { presets.next(); };
    nameBtn.onClick = [this] { showPresetMenu(); };
    saveBtn.onClick = [this] { promptSave(); };
    abBtn.onClick   = [this] { presets.toggleAB(); };
    undoBtn.onClick = [this] { undo.undo(); };
    redoBtn.onClick = [this] { undo.redo(); };

    presets.addChangeListener (this);
    refresh();
    startTimerHz (10);
}

TopBar::~TopBar() { presets.removeChangeListener (this); }

void TopBar::refresh()
{
    nameBtn.setButtonText (presets.getCurrentName());
    abBtn.setButtonText (presets.isB() ? "B" : "A");
    abBtn.setToggleState (presets.isB(), juce::dontSendNotification);
    abBtn.setColour (juce::TextButton::buttonOnColourId, col::violet);
}

void TopBar::timerCallback()
{
    undoBtn.setEnabled (undo.canUndo());
    redoBtn.setEnabled (undo.canRedo());
}

void TopBar::showPresetMenu()
{
    constexpr int base = 1000;
    auto menu = presets.buildMenu (base);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&nameBtn),
                        [this] (int r) { if (r >= base) presets.load (r - base); });
}

void TopBar::promptSave()
{
    auto* w = new juce::AlertWindow ("Sauvegarder le preset", "Nom du preset :", juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("n", presets.getCurrentName());
    w->addButton ("Sauver", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Annuler", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<TopBar> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, w] (int r)
    {
        if (r == 1 && safe != nullptr) safe->presets.saveUserPreset (w->getTextEditorContents ("n"));
    }), true);
}

void TopBar::paint (juce::Graphics& g)
{
    using namespace juce;
    auto b = getLocalBounds().toFloat();
    if (Theme::isClean()) g.fillAll (col::panel);
    else g.setGradientFill (ColourGradient (Colour (0xff2a2a33), 0, 0, col::black, 0, b.getBottom(), false)), g.fillRect (b);
    g.setColour (col::amber.withAlpha (0.55f));
    g.fillRect (b.removeFromBottom (1.0f));

    g.setColour (col::amber);        g.setFont (Theme::uiFont (15.0f, true));
    g.drawText ("HUMPIRE", 10, 4, 120, 18, Justification::centredLeft);
    g.setColour (col::textDim);      g.setFont (Theme::uiFont (9.5f, true));
    g.drawText (title, 10, 21, 120, 12, Justification::centredLeft);

    g.setColour (col::textDim);      g.setFont (Theme::uiFont (9.0f, true));
    g.drawText ("IN",  inLabel,  Justification::centredLeft);
    g.drawText ("OUT", outLabel, Justification::centredLeft);
}

void TopBar::resized()
{
    constexpr int y = 8, h = 24;
    prevBtn.setBounds (132, y, 22, h);
    nameBtn.setBounds (158, y, 140, h);
    nextBtn.setBounds (302, y, 22, h);
    saveBtn.setBounds (330, y, 40, h);
    abBtn.setBounds   (376, y, 30, h);
    undoBtn.setBounds (412, y, 44, h);
    redoBtn.setBounds (460, y, 44, h);
    bypassBtn.setBounds (510, y, 56, h);

    inLabel  = { 574, 10, 16, 10 };   inMeter.setBounds  (592, 10, 64, 8);
    outLabel = { 574, 23, 16, 10 };   outMeter.setBounds (592, 23, 64, 8);
    // zone libre 660..750 : reservee (ex. toggles Simple/Avance specifiques au plugin)
}
} // namespace humpire
