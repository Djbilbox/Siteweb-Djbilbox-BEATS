#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "HumpireCore/HumpireKnob.h"
#include "HumpireCore/HumpireLookAndFeel.h"
#include "HumpireCore/HumpireMeter.h"
#include "HumpireCore/HumpireZellige.h"
#include "PluginProcessor.h"

namespace humpire
{
using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

// Background: anthracite + zellige watermark
class Canvas : public juce::Component
{
public:
    Canvas();
    void paint (juce::Graphics&) override;
private:
    juce::Image zellige;
};

// Instrument card (left column)
class InstrumentCard : public juce::Component
{
public:
    InstrumentCard (ChaabiProcessor&, int index);
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
private:
    ChaabiProcessor& proc;
    int idx;
};

// Signature artwork of the selected instrument
class ArtView : public juce::Component
{
public:
    explicit ArtView (ChaabiProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
private:
    ChaabiProcessor& proc;
};

// Phrase pad: click = trigger (held = loop), handle = drag & drop MIDI into the DAW
class PhrasePad : public juce::Component
{
public:
    PhrasePad (ChaabiProcessor&, int instrument, int index);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
private:
    bool onHandle (juce::Point<int> pt) const { return pt.x > getWidth() - 34; }
    ChaabiProcessor& proc;
    int inst, idx;
    bool down = false, exported = false;
};

// 16-step sequencer (5 lanes)
class SequencerView : public juce::Component
{
public:
    explicit SequencerView (ChaabiProcessor& p) : proc (p) {}
    void setInstrument (int seqInst) { inst = seqInst; repaint(); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::Rectangle<float> cell (int lane, int step) const;
    ChaabiProcessor& proc;
    int inst = 0;
};

// Keyboard with coloured zones (phrases, keyswitches, range of each instrument)
class ZonedKeyboard : public juce::MidiKeyboardComponent
{
public:
    using juce::MidiKeyboardComponent::MidiKeyboardComponent;
    std::function<juce::Colour (int)> zoneColour;
protected:
    void drawWhiteNote (int note, juce::Graphics&, juce::Rectangle<float>, bool, bool, juce::Colour, juce::Colour) override;
    void drawBlackNote (int note, juce::Graphics&, juce::Rectangle<float>, bool, bool, juce::Colour) override;
};

class ChaabiEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ChaabiEditor (ChaabiProcessor&);
    ~ChaabiEditor() override;
    void resized() override;

private:
    static constexpr int kW = 1280, kH = 760;

    void timerCallback() override;
    void bindInstrumentKnobs();
    void layoutTabs();
    void rebuildPhrasePads();
    void showPresetMenu();
    void askPresetName();
    void chooseLibrary();
    HumpireKnob* addGlobalKnob (const juce::String& label, const juce::String& paramId);
    void paintTopBar();

    ChaabiProcessor& proc;
    HumpireLookAndFeel laf;
    Canvas content;

    // top bar
    juce::Label logo;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, presetBtn, saveBtn { "SAVE" }, abBtn { "A" }, undoBtn { "UNDO" }, redoBtn { "REDO" };
    juce::ToggleButton bypassBtn { "BYPASS" };
    HumpireMeter inMeter, outMeter;

    // instrument column + artwork
    std::array<std::unique_ptr<InstrumentCard>, kNumInstruments> cards;
    ArtView art;
    juce::Label titleLabel, taglineLabel, statusLabel;
    double loadProgress = 0.0;
    juce::ProgressBar loadBar;
    juce::TextButton libraryBtn { "SAMPLES FOLDER" };
    juce::ToggleButton multiBtn { "MULTI" }, activeBtn { "ACTIVE" };

    // knobs
    std::array<std::unique_ptr<HumpireKnob>, 10> instKnobs;
    std::array<std::unique_ptr<SliderAtt>, 10> instAtts;
    std::unique_ptr<ButtonAtt> activeAtt;
    std::vector<std::unique_ptr<HumpireKnob>> globalKnobs;
    std::vector<std::unique_ptr<SliderAtt>> globalAtts;
    std::unique_ptr<HumpireKnob> authKnob;
    std::unique_ptr<SliderAtt> authAtt;

    // bottom tabs
    juce::TextButton tabPlay { "PLAY" }, tabPhrases { "PHRASES" }, tabPatterns { "PATTERNS" };
    juce::Component playPanel, phrasePanel, patternPanel;
    juce::Label playInfo;
    std::vector<std::unique_ptr<PhrasePad>> pads;
    SequencerView seqView;
    juce::TextButton seqDerb { "DERBOUKA" }, seqTar { "TAR" };
    juce::ToggleButton seqRun { "SEQ ON" };
    juce::ComboBox seqRate;
    HumpireKnob seqSwingKnob { "Swing" };
    std::unique_ptr<ButtonAtt> seqRunAtt;
    std::unique_ptr<ComboAtt> seqRateAtt;
    std::unique_ptr<SliderAtt> seqSwingAtt;

    ZonedKeyboard keyboard;
    std::unique_ptr<ButtonAtt> bypassAtt, multiAtt;
    std::unique_ptr<juce::FileChooser> chooser;

    int shownSelected = -1;
    int seqInst = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChaabiEditor)
};
} // namespace humpire
