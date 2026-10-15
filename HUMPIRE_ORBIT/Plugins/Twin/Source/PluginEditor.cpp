#include "PluginEditor.h"

using namespace humpire;

TwinEditor::TwinEditor (TwinAudioProcessor& p)
    : HumpireEditorBase (p, 960, 600),
      proc (p),
      topBar (p.apvts, p.presets, "TWIN - Auto-doubler", p.inLevel, p.outLevel),
      orbitView (p),
      waveform (p.waveform, theme::violet),
      corrMeter (p.correlation),
      voiceStrip (p),
      xpBar (p.outLevel),
      voicesAttach (*p.apvts.getParameter (twin::id::voices), [this] (float v) { applyVoices ((int) std::lround (v)); }, &p.undoManager),
      simpleAttach (*p.apvts.getParameter (twin::id::simple), [this] (float v) { applySimple (v > 0.5f); }, nullptr)
{
    // --- zones principales (coordonnees de base 960 x 600) ---
    content.addAndMakeVisible (topBar);
    topBar.setBounds (0, 0, 960, 56);
    content.addAndMakeVisible (orbitView);
    orbitView.setBounds (16, 68, 600, 400);
    content.addAndMakeVisible (waveform);
    waveform.setBounds (16, 480, 600, 52);
    content.addAndMakeVisible (corrMeter);
    corrMeter.setBounds (16, 542, 600, 42);
    content.addAndMakeVisible (voiceStrip);
    voiceStrip.setBounds (632, 412, 312, 124);
    content.addAndMakeVisible (xpBar);
    xpBar.setBounds (632, 548, 312, 36);

    // --- nombre de doublures (1..4) + mode simple ---
    voicesLabel.setText ("TWINS", juce::dontSendNotification);
    voicesLabel.setFont (theme::pixel (8.0f));
    voicesLabel.setColour (juce::Label::textColourId, theme::amber);
    voicesLabel.setJustificationType (juce::Justification::centredLeft);
    content.addAndMakeVisible (voicesLabel);
    voicesLabel.setBounds (632, 68, 66, 28);
    for (int i = 0; i < twin::kMaxVoices; ++i)
    {
        auto& b = voiceBtns[(size_t) i];
        b.setButtonText (juce::String (i + 1));
        b.setTooltip (juce::String (i + 1) + (i == 0 ? " doublure" : " doublures"));
        content.addAndMakeVisible (b);
        b.setBounds (700 + i * 38, 68, 34, 28);
        b.onClick = [this, i] { voicesAttach.setValueAsCompleteGesture ((float) (i + 1)); };
    }
    content.addAndMakeVisible (simpleBtn);
    simpleBtn.setBounds (856, 68, 88, 28);
    simpleBtn.onClick = [this] { simpleAttach.setValueAsCompleteGesture (simpleBtn.getToggleState() ? 1.0f : 0.0f); };

    // --- knobs ---
    const std::pair<OrbitKnob*, const char*> knobs[] = {
        { &detune, twin::id::detune }, { &delay, twin::id::delay }, { &spread, twin::id::spread }, { &humanize, twin::id::humanize },
        { &drift, twin::id::drift }, { &width, twin::id::width }, { &lowCut, twin::id::hpf }, { &highCut, twin::id::lpf },
        { &mix, twin::id::mix }, { &inGain, twin::id::inGain }, { &outGain, twin::id::outGain },
    };
    for (auto& k : knobs)
    {
        content.addAndMakeVisible (*k.first);
        k.first->attach (p.apvts, k.second);
    }

    voicesAttach.sendInitialUpdate();
    simpleAttach.sendInitialUpdate();
}

void TwinEditor::applyVoices (int n)
{
    for (int i = 0; i < twin::kMaxVoices; ++i)
        voiceBtns[(size_t) i].setToggleState (i + 1 == n, juce::dontSendNotification);
}

void TwinEditor::applySimple (bool simple)
{
    simpleBtn.setToggleState (simple, juce::dontSendNotification);
    auto place = [] (OrbitKnob& k, int col, int row) { k.setVisible (true); k.setBounds (632 + col * 78, 108 + row * 100, 78, 92); };

    if (simple)
    {
        // Mode simple : 4 controles essentiels (Detune, Delay, Width, Mix) + nombre de doublures
        for (auto* k : { &spread, &humanize, &drift, &lowCut, &highCut, &inGain, &outGain })
            k->setVisible (false);
        place (detune, 0, 0); place (delay, 1, 0); place (width, 2, 0); place (mix, 3, 0);
    }
    else
    {
        place (detune, 0, 0); place (delay, 1, 0);  place (spread, 2, 0);  place (humanize, 3, 0);
        place (drift, 0, 1);  place (width, 1, 1);  place (lowCut, 2, 1);  place (highCut, 3, 1);
        place (mix, 0, 2);    place (inGain, 1, 2); place (outGain, 2, 2);
    }
}
