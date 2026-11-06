#pragma once
#include "PluginProcessor.h"
#include "NeonFX.h"

class DzLNF : public juce::LookAndFeel_V4
{
public:
    DzLNF();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (15.f, juce::Font::plain); }
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;

    juce::Image faderImg;   // photo-real fader cap (the knob caps are part of the faceplate)
};

class DzPanel : public juce::Component, private juce::Timer
{
public:
    explicit DzPanel (DzProcessor&);
    ~DzPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    // one animation step (the timer calls it at 60 Hz; the frame-capture tool calls it directly)
    void advance (float dt);

private:
    // glow: light of the value arc (0 = off); mod: live modulated position 0..1 (< 0 = none); orbit: LFO dot phase (< 0 = none)
    struct Knob { juce::Slider s; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; juce::String label; bool lin = false;
                  float glow = 0.f, mod = -1.f, orbit = -1.f;
                  float shownGlow = -1.f, shownLed = -1.f; bool shownMod = false; };   // what the last repaint showed
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
    void timerCallback() override;
    void paintStatic (juce::Graphics&);   // everything that does not move: drawn once into bgCache
    void paintLive (juce::Graphics&);     // light, neon and motion on top
    float param (int index) const;
    bool animationsOn() const;
    void setAnimations (bool);
    void invalidateChanges();             // repaints only the areas whose light or motion changed
    juce::Point<float> runnerPos() const; // the light running around the preset screen

    DzProcessor& proc;
    DzLNF lnf;
    Knob rate, amount, pitch, glide, filter, volume, reverb, chorus, dist, delay, dtime, feedback, tone, body, bright, width, texture, fenvK, resoK;
    Knob env[4], rsl[5];
    Knob bend, modw;
    Combo cWave, cLfo, cDest, cFilt, cMono;
    juce::TextButton prev { "<" }, next { ">" }, folder { "Presets" }, save { "Save" }, expansion;
    juce::Label presetLabel;
    hfx::NeonKeyboard keys;
    juce::Image camel, logo, cassette, faceImg;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<Knob*> all;

    hfx::LiveView view;
    hfx::NeonTitle title;
    hfx::Sparks sparks;
    hfx::Bolts bolts;
    hfx::HaloImage haloA, haloB;          // the medallion's light ring in its two colours, cross-faded
    juce::Random rnd { 2027 };
    juce::Image bgCache;
    juce::Image ledSprite[3];             // lit LED with its glow: green, amber, red (drawn as images: far cheaper)
    float bgScale = 0.f;
    float reelAngle = 0.f, reelSpeed = 0.f, presetFlash = 0.f, idleBolt = 2.f, idleSpark = 0.f, lfoIdlePhase = 0.f, echo = 0.f;
    std::array<bool, 128> keyWasDown {};
    juce::RectangleList<int> prevMoving;  // where moving things were drawn last frame (they must be erased there)
    float shownRail = -1.f, shownMeter = -1.f, shownLine = -1.f;
    bool keysLit = false;
    double lastTick = 0.0;
    int skip = 0;
    float idleDt = 0.f;                   // time of the skipped quiet frames, given to the next drawn one
};

// Editeur redimensionnable : le panneau reste en 1000x746 et est mis a l'echelle.
class DzEditor : public juce::AudioProcessorEditor
{
public:
    explicit DzEditor (DzProcessor& p) : AudioProcessorEditor (&p), panel (p)
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
    DzPanel panel;
};
