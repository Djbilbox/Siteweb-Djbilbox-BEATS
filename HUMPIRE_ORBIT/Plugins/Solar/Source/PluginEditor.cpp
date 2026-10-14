#include "PluginEditor.h"

using namespace humpire;

SolarEditor::SolarEditor (SolarAudioProcessor& p)
    : HumpireEditorBase (p, 960, 600),
      proc (p),
      topBar (p.apvts, p.presets, "SOLAR - Autotune", p.inLevel, p.outLevel),
      sunView (p), curveView (p),
      waveform (p.waveform, theme::violet), xpBar (p.outLevel),
      modeAttach (*p.apvts.getParameter (solar::id::mode), [this] (float v) { applyMode (v > 0.5f); }, &p.undoManager),
      simpleAttach (*p.apvts.getParameter (solar::id::simple), [this] (float v) { applySimple (v > 0.5f); }, nullptr)
{
    // --- zones principales ---
    content.addAndMakeVisible (topBar);
    topBar.setBounds (0, 0, 960, 56);
    content.addAndMakeVisible (sunView);
    content.addAndMakeVisible (curveView);
    sunView.setBounds (16, 68, 600, 400);
    curveView.setBounds (16, 68, 600, 400);
    content.addAndMakeVisible (waveform);
    waveform.setBounds (16, 480, 600, 52);
    content.addAndMakeVisible (xpBar);
    xpBar.setBounds (632, 548, 312, 36);

    // --- mode / simple ---
    for (auto* b : { &autoBtn, &graphBtn, &snapBtn, &resetBtn })
        content.addAndMakeVisible (*b);
    content.addAndMakeVisible (simpleBtn);
    autoBtn.setBounds (632, 68, 92, 28);
    graphBtn.setBounds (728, 68, 92, 28);
    simpleBtn.setBounds (856, 68, 88, 28);
    autoBtn.onClick  = [this] { modeAttach.setValueAsCompleteGesture (0.0f); };
    graphBtn.onClick = [this] { modeAttach.setValueAsCompleteGesture (1.0f); };
    simpleBtn.onClick = [this] { simpleAttach.setValueAsCompleteGesture (simpleBtn.getToggleState() ? 1.0f : 0.0f); };
    snapBtn.onClick  = [this] { proc.snapGraphToScale(); curveView.repaint(); };
    resetBtn.onClick = [this] { proc.resetGraph(); curveView.repaint(); };
    snapBtn.setBounds (632, 436, 150, 28);
    resetBtn.setBounds (790, 436, 154, 28);

    // --- tonalite / gamme ---
    content.addAndMakeVisible (keyBox);
    content.addAndMakeVisible (scaleBox);
    keyBox.addItemList ({ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 1);
    for (int i = 0; i < solar::NumScales; ++i) scaleBox.addItem (solar::kScales[i].name, i + 1);
    keyAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, solar::id::key, keyBox);
    scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, solar::id::scale, scaleBox);
    keyBox.setBounds (632, 104, 86, 28);
    scaleBox.setBounds (724, 104, 220, 28);

    // --- knobs (3 rangees de 4) ---
    struct Slot { OrbitKnob* k; const char* id; int col, row; };
    const Slot slots[] = {
        { &retune, solar::id::retune, 0, 0 }, { &humanize, solar::id::humanize, 1, 0 }, { &amount, solar::id::amount, 2, 0 }, { &flex, solar::id::flex, 3, 0 },
        { &vibDepth, solar::id::vibDepth, 0, 1 }, { &vibRate, solar::id::vibRate, 1, 1 }, { &vibDelay, solar::id::vibDelay, 2, 1 }, { &portamento, solar::id::portamento, 3, 1 },
        { &mix, solar::id::mix, 0, 2 }, { &inGain, solar::id::inGain, 1, 2 }, { &outGain, solar::id::outGain, 2, 2 }, { &refA, solar::id::refA, 3, 2 },
    };
    for (auto& s : slots)
    {
        content.addAndMakeVisible (*s.k);
        s.k->attach (p.apvts, s.id);
        s.k->setBounds (632 + s.col * 78, 144 + s.row * 96, 78, 92);
    }

    // --- clavier de notes (active / desactive chaque classe de hauteur) ---
    for (int i = 0; i < 12; ++i)
    {
        auto* b = noteButtons.add (new juce::ToggleButton (solar::noteName (i)));
        const bool black = juce::String (solar::noteName (i)).contains ("#");
        b->setColour (juce::ToggleButton::tickColourId, black ? theme::violet : theme::amber);
        content.addAndMakeVisible (b);
        b->setBounds (16 + i * 50, 542, 48, 42);
        noteAttach.add (new juce::AudioProcessorValueTreeState::ButtonAttachment (p.apvts, solar::noteId (i), *b));
    }

    modeAttach.sendInitialUpdate();
    simpleAttach.sendInitialUpdate();
    proc.loadGraphFromState();
}

void SolarEditor::applyMode (bool graph)
{
    sunView.setVisible (! graph);
    curveView.setVisible (graph);
    autoBtn.setToggleState (! graph, juce::dontSendNotification);
    graphBtn.setToggleState (graph, juce::dontSendNotification);
    snapBtn.setVisible (graph);
    resetBtn.setVisible (graph);
}

void SolarEditor::applySimple (bool simple)
{
    simpleBtn.setToggleState (simple, juce::dontSendNotification);
    for (auto* k : { &flex, &vibDepth, &vibRate, &vibDelay, &portamento, &mix, &inGain, &outGain, &refA })
        k->setVisible (! simple);
    // En mode simple : 3 controles (Retune, Humanize, Amount) + tonalite / gamme
}
