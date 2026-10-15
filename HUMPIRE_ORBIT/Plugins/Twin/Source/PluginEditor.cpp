#include "PluginEditor.h"

using namespace humpire;

TwinEditor::TwinEditor (TwinAudioProcessor& p)
    : HumpireEditorBase (p, 960, 600),
      proc (p),
      topBar (p.apvts, p.presets, "TWIN - Auto Double", p.inLevel, p.outLevel),
      orbit (p.ui),
      readout (p.ui),
      waveform (p.waveform, theme::violet),
      xpBar (p.outLevel),
      doublesAttach (*p.apvts.getParameter (twin::id::doubles),
                     [this] (float v) { applyDoubles (juce::roundToInt (v)); }, &p.undoManager),
      simpleAttach (*p.apvts.getParameter (twin::id::simple),
                    [this] (float v) { applySimple (v > 0.5f); }, nullptr)
{
    // --- top bar, scene, meters ---
    content.addAndMakeVisible (topBar);
    topBar.setBounds (0, 0, 960, 56);
    content.addAndMakeVisible (orbit);
    orbit.setBounds (16, 68, 600, 400);
    content.addAndMakeVisible (waveform);
    waveform.setBounds (16, 480, 600, 52);
    content.addAndMakeVisible (readout);
    readout.setBounds (632, 436, 312, 100);
    content.addAndMakeVisible (xpBar);
    xpBar.setBounds (632, 548, 312, 36);

    // --- doubles selector: 1 2 3 4 ---
    for (int i = 0; i < twin::kMaxVoices; ++i)
    {
        auto* b = doubleButtons.add (new juce::TextButton (juce::String (i + 1)));
        content.addAndMakeVisible (*b);
        b->setBounds (632 + i * 78, 68, 74, 28);
        b->onClick = [this, i] { doublesAttach.setValueAsCompleteGesture ((float) (i + 1)); };
    }

    // --- simple / advanced and reseed ---
    content.addAndMakeVisible (simpleBtn);
    simpleBtn.setBounds (632, 104, 150, 28);
    simpleBtn.onClick = [this] { simpleAttach.setValueAsCompleteGesture (simpleBtn.getToggleState() ? 1.0f : 0.0f); };

    content.addAndMakeVisible (reseedBtn);
    reseedBtn.setBounds (790, 104, 154, 28);
    reseedBtn.onClick = [this] { proc.requestReseed(); };

    // --- knobs: row 0 always visible (Simple mode), rows 1-2 in Advanced mode ---
    struct Slot { OrbitKnob* knob; const char* id; int col, row; };
    const Slot slots[] = {
        { &detune,   twin::id::detune,   0, 0 }, { &delay,    twin::id::delay,    1, 0 },
        { &width,    twin::id::width,    2, 0 }, { &mix,      twin::id::mix,      3, 0 },
        { &humanize, twin::id::humanize, 0, 1 }, { &drift,    twin::id::drift,    1, 1 },
        { &hpKnob,   twin::id::hpFreq,   2, 1 }, { &lpKnob,   twin::id::lpFreq,   3, 1 },
        { &outGain,  twin::id::outGain,  0, 2 },
    };
    for (const auto& s : slots)
    {
        content.addAndMakeVisible (*s.knob);
        s.knob->attach (p.apvts, s.id);
        s.knob->setBounds (632 + s.col * 78, 144 + s.row * 96, 78, 92);
    }

    doublesAttach.sendInitialUpdate();
    simpleAttach.sendInitialUpdate();
}

void TwinEditor::applyDoubles (int voices)
{
    for (int i = 0; i < doubleButtons.size(); ++i)
        doubleButtons[i]->setToggleState (i + 1 == voices, juce::dontSendNotification);
}

void TwinEditor::applySimple (bool simple)
{
    simpleBtn.setToggleState (simple, juce::dontSendNotification);
    for (auto* k : { &humanize, &drift, &hpKnob, &lpKnob, &outGain })
        k->setVisible (! simple);
}
