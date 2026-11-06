#pragma once
#include "PluginProcessor.h"

// Look synthwave : verre sombre, contours neon magenta -> cyan, potards a halo.
class NeonLNF : public juce::LookAndFeel_V4
{
public:
    NeonLNF();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (13.5f, juce::Font::bold); }
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return juce::FontOptions (12.f, juce::Font::bold); }
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawBubble (juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>&) override;

private:
    juce::Image knobImage;   // photo du potard (Flow), tournee selon la valeur
};

class NeonPanel : public juce::Component, private juce::Timer
{
public:
    explicit NeonPanel (NeonAudioProcessor&);
    ~NeonPanel() override;
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

    static constexpr int W = 1100, H = 860;

private:
    struct Knob { juce::Slider s; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; juce::String label; };
    struct Combo { juce::ComboBox c; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> a; };

    void timerCallback() override;
    void addKnob (Knob&, int paramIndex, const juce::String& label, juce::Slider::SliderStyle = juce::Slider::RotaryHorizontalVerticalDrag);
    void addCombo (Combo&, int paramIndex, const juce::StringArray& items);
    void refreshPresetLabel();
    void showPresetMenu();
    void stepPreset (int d);
    void savePreset();
    void loadUserPreset (const juce::File&);
    static juce::File userPresetDir();
    void drawScope (juce::Graphics&) const;
    void renderStatic (juce::Graphics&);
    void drawAnimated (juce::Graphics&);
    void drawNeonFrames (juce::Graphics&) const;
    void drawMeter (juce::Graphics&) const;

    NeonAudioProcessor& proc;
    NeonLNF lnf;
    Knob detune, glide, semi2, fine2, mix2, cutoff, res, fenv, drive;
    Knob ampEnv[4], velGain, velEnv;
    Knob rate, amount, lfoEnv[4];
    Knob dist, chorus, reverb, dmix, dtime, dfb, volume;
    Knob bend, modw;
    Combo cWave, cMode, cWave2, cFilt, cLfo, cDest;
    juce::ToggleButton pitchSwitch;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pitchAttach;
    juce::TextButton prev { "<" }, next { ">" }, folder { "PRESETS" }, save { "SAVE" };
    juce::HyperlinkButton more { "More 80s sounds: K2000  >", juce::URL ("https://djbilboxbeats.gumroad.com/l/sirmb") };
    juce::Label presetLabel, bankLabel;
    juce::MidiKeyboardComponent keys;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<Knob*> labelled;
    juce::Image hero, bgCache;
    juce::Rectangle<int> scopeArea;
    float t = 0.f, meterL = 0.f, meterR = 0.f, activity = 0.f, lfoPh = 0.f;   // etat des animations (timer UI)
    bool notesHeld = false;
};

// Editeur redimensionnable : le panneau reste en W x H et est mis a l'echelle.
class NeonAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit NeonAudioProcessorEditor (NeonAudioProcessor& p) : AudioProcessorEditor (&p), panel (p)
    {
        addAndMakeVisible (panel);
        setResizable (true, true);
        setResizeLimits (NeonPanel::W / 2, NeonPanel::H / 2, NeonPanel::W * 2, NeonPanel::H * 2);
        getConstrainer()->setFixedAspectRatio ((double) NeonPanel::W / NeonPanel::H);
        setSize (NeonPanel::W, NeonPanel::H);
    }
    void resized() override
    {
        panel.setBounds (0, 0, NeonPanel::W, NeonPanel::H);
        panel.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) NeonPanel::W));
    }
private:
    NeonPanel panel;
};
