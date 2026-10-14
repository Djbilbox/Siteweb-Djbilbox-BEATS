#pragma once
#include "PluginProcessor.h"

class ShLNF : public juce::LookAndFeel_V4
{
public:
    ShLNF();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (15.f, juce::Font::plain); }
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
};

class ShPanel : public juce::Component
{
public:
    explicit ShPanel (ShProcessor&);
    ~ShPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob { juce::Slider s; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; juce::String label; bool lin = false; };
    struct Combo { juce::ComboBox c; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> a; };

    void addKnob (Knob&, int paramIndex, const juce::String& label, juce::Slider::SliderStyle = juce::Slider::RotaryHorizontalVerticalDrag);
    void addCombo (Combo&, int paramIndex, const juce::StringArray& items);
    void refreshPresetLabel();
    void showPresetMenu();
    void showExpansion();
    void stepPreset (int d);
    void savePreset();
    void loadUserPreset (const juce::File&);
    static juce::File userPresetDir();

    ShProcessor& proc;
    ShLNF lnf;
    Knob rate, amount, pitch, glide, filter, volume, reverb, chorus, dist, delay, dtime, feedback, tone, body, bright, width, texture, fenvK, resoK;
    Knob env[4], rsl[5];
    Knob bend, modw;
    Combo cWave, cLfo, cDest, cFilt, cMono;
    juce::TextButton prev { "<" }, next { ">" }, folder { "Presets" }, save { "Save" }, expansion;
    juce::Label presetLabel;
    juce::MidiKeyboardComponent keys;
    juce::Image camel, logo, cassette;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<Knob*> all;
};

// Editeur redimensionnable : le panneau reste en 1000x746 et est mis a l'echelle.
class ShEditor : public juce::AudioProcessorEditor
{
public:
    explicit ShEditor (ShProcessor& p) : AudioProcessorEditor (&p), panel (p)
    {
        addAndMakeVisible (panel);
        setResizable (true, true);
        setResizeLimits (500, 373, 2000, 1492);
        getConstrainer()->setFixedAspectRatio (1000.0 / 746.0);
        setSize (1000, 746);
    }
    void resized() override
    {
        panel.setBounds (0, 0, 1000, 746);
        panel.setTransform (juce::AffineTransform::scale ((float) getWidth() / 1000.f));
    }
private:
    ShPanel panel;
};
