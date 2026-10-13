#pragma once
#include "PluginProcessor.h"

class VoodooLNF : public juce::LookAndFeel_V4
{
public:
    VoodooLNF();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (15.f, juce::Font::bold); }
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
};

class VoodooEditor : public juce::AudioProcessorEditor
{
public:
    explicit VoodooEditor (VoodooProcessor&);
    ~VoodooEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob { juce::Slider s; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; juce::String label; };
    struct Combo { juce::ComboBox c; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> a; };

    void addKnob (Knob&, int paramIndex, const juce::String& label, bool purple, juce::Slider::SliderStyle = juce::Slider::RotaryHorizontalVerticalDrag);
    void addCombo (Combo&, int paramIndex, const juce::StringArray& items);
    void refreshPresetLabel();
    void showPresetMenu();
    void stepPreset (int d);
    void savePreset();
    void showExpansion();

    VoodooProcessor& proc;
    VoodooLNF lnf;
    Knob rate, amount, pitch, glide, filter, volume, reverb, chorus, dist, delay, dtime, feedback;
    Knob env[4], timb[5];
    Knob bend, modw;
    Combo cWave, cLfo, cDest, cFilt, cMono;
    juce::TextButton prev { "<" }, next { ">" }, folder { "Presets" }, save { "Save" }, expansion { "EXPANSION" };
    juce::Label presetLabel;
    juce::MidiKeyboardComponent keys;
    juce::Image camel, logo;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<Knob*> all;
};
