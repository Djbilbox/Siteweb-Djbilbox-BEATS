#pragma once

#include "PluginProcessor.h"
#include <set>

class GFunkEditor;

namespace ui
{
// Light-show palette: 0 gold, 1 candy, 2 neon, 3 all colours
juce::Colour themeColour (int theme, float t, float time);
inline const GFunkSpec::SkinDef& skinDef (int i) { return GFunkSpec::skins[juce::jlimit (0, GFunkSpec::numSkins - 1, i)]; }
inline juce::Colour col (unsigned argb) { return juce::Colour ((juce::uint32) argb); }
}

//==============================================================================
class ArtLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ArtLookAndFeel();
    void setSkin (int index, const juce::Image& knobImage);

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float startAngle,
                           float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos,
                           float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool isDown, int bx, int by, int bw, int bh,
                       juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawLabel (juce::Graphics&, juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    juce::Image knob;
    GFunkProcessor* proc = nullptr;
    int skin = 0;
};

//==============================================================================
class GlyphButton : public juce::Button
{
public:
    enum Kind { Prev, Next, Dice, Star, Skin };
    explicit GlyphButton (Kind k) : juce::Button ("glyph"), kind (k) {}
    void paintButton (juce::Graphics&, bool over, bool down) override;
    bool lit = false;
    int theme = 0;

private:
    Kind kind;
};

class TabButton : public juce::Button
{
public:
    explicit TabButton (const juce::String& t) : juce::Button (t) { setClickingTogglesState (true); setRadioGroupId (77); }
    void paintButton (juce::Graphics&, bool over, bool down) override;
    int theme = 0;
    float time = 0.0f;
};

//==============================================================================
class LedScreen : public juce::Component, private juce::Timer
{
public:
    LedScreen (GFunkEditor&, GFunkProcessor&);
    ~LedScreen() override { stopTimer(); }

    void setTab (int t) { tab = t; repaint(); }
    void revealPreset (int index);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { draggingBar = false; }
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { hoverRow = -1; repaint(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override;
    void paintScope (juce::Graphics&);
    void paintBrowser (juce::Graphics&);
    void paintSpectrum (juce::Graphics&);
    void paintLights (juce::Graphics&);
    std::vector<int> currentList() const;
    int visibleRows() const { return (getHeight() - 8) / rowH; }
    void setScrollFromBar (int y);

    static constexpr int rowH = 20, catW = 200, barW = 14;

    GFunkEditor& editor;
    GFunkProcessor& proc;
    int tab = 0, browseCat = 0, scroll = 0, hoverRow = -1, tick = 0;
    bool draggingBar = false;
    float scrollAcc = 0.0f, time = 0.0f;
    juce::Image mask;
    juce::Path ghost;
    juce::dsp::FFT fft { 11 };
    std::array<float, 4096> fftData {};
    float bars[64] {}, peaks[64] {};
    juce::Rectangle<int> themeRects[4], blingRects[3], skinRects[6], scrollRects[2];
};

//==============================================================================
class PresetLcd : public juce::Component
{
public:
    PresetLcd (GFunkEditor& e, GFunkProcessor& p);
    void paint (juce::Graphics&) override;
    void showReadout (const juce::String& text);

private:
    GFunkEditor& editor;
    GFunkProcessor& proc;
    juce::Image mask;
    juce::String readout;
    juce::uint32 readoutStamp = 0;
};

class MeterView : public juce::Component
{
public:
    explicit MeterView (GFunkProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;

private:
    GFunkProcessor& proc;
    float level = 0.0f, peak = 0.0f;
};

class KeyboardView : public juce::MidiKeyboardComponent
{
public:
    KeyboardView (GFunkProcessor& p);
    void drawWhiteNote (int note, juce::Graphics&, juce::Rectangle<float> area, bool isDown, bool isOver,
                        juce::Colour lineColour, juce::Colour textColour) override;
    void drawBlackNote (int note, juce::Graphics&, juce::Rectangle<float> area, bool isDown, bool isOver,
                        juce::Colour fill) override;
    juce::Rectangle<float> keyRect (int note) const { return getRectangleForKey (note); }

private:
    GFunkProcessor& proc;
};

//==============================================================================
class BackgroundView : public juce::Component
{
public:
    BackgroundView();
    void setImage (const juce::Image& i) { img = i; repaint(); }
    void paint (juce::Graphics&) override;

private:
    juce::Image img;
};

// The man, the woman and the Impala: a thick baked 3D cut-out with parallax that bounces on every hit.
class HeroView : public juce::Component, private juce::Timer
{
public:
    explicit HeroView (GFunkProcessor& p);
    ~HeroView() override { stopTimer(); }
    bool hitTest (int, int) override { return false; }
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    GFunkProcessor& proc;
    juce::Image cutout;
    float ox = 0.0f, oy = 0.0f, bob = 0.0f, bounce = 0.0f, time = 0.0f;
};

class BlingOverlay : public juce::Component
{
public:
    BlingOverlay (GFunkProcessor& p, KeyboardView& k);
    void paint (juce::Graphics&) override;
    void advance();
    void markDirty();
    bool isAnimating() const { return smoothFunk > 0.02f || smoothHit > 0.02f || ! sparkles.empty(); }

private:
    struct Sparkle { juce::Point<float> pos; float life, size, rot; juce::Colour col; };
    void addRing (juce::Rectangle<float> r, float radius, float inset, float spacing);

    GFunkProcessor& proc;
    KeyboardView& keys;
    juce::Image sprite;
    std::vector<juce::Point<float>> bulbs;
    std::vector<Sparkle> sparkles;
    juce::Random rnd;
    float time = 0.0f, lastHit = 0.0f, smoothHit = 0.0f, smoothFunk = 0.0f;
    double lastNow = 0.0;
};

class LogoView : public juce::Component
{
public:
    explicit LogoView (GFunkProcessor& p);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { if (onClick) onClick(); }
    std::function<void()> onClick;

private:
    GFunkProcessor& proc;
    juce::Image img;
};

class SplashView : public juce::Component, private juce::Timer
{
public:
    SplashView();
    ~SplashView() override { stopTimer(); }
    void show();
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { holdMs = 0; }

private:
    void timerCallback() override;
    juce::Image img;
    float alpha = 1.0f;
    int holdMs = 4200;
};

//==============================================================================
class GFunkEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GFunkEditor (GFunkProcessor&);
    ~GFunkEditor() override;

    void paint (juce::Graphics&) override {}
    void resized() override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

    void loadPreset (int index);
    void setSkin (int index);
    bool isFavourite (int index) const { return favourites.count (index) > 0; }
    void toggleFavourite (int index);
    const std::set<int>& getFavourites() const { return favourites; }
    void showReadout (const juce::String& text) { lcd.showReadout (text); }

private:
    struct Knob
    {
        juce::Slider slider;
        juce::ComboBox combo;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cAttach;
    };

    void timerCallback() override;
    void loadFavourites();
    void saveFavourites() const;
    void readoutFor (juce::Slider&);
    void applySkin();

    GFunkProcessor& proc;
    ArtLookAndFeel laf;
    juce::ComponentBoundsConstrainer constrainer;

    juce::Component content;                 // everything lives here so the whole UI can be scaled
    BackgroundView background;
    juce::OwnedArray<Knob> knobs;
    LedScreen screen;
    PresetLcd lcd;
    MeterView meter;
    GlyphButton prevBtn { GlyphButton::Prev }, nextBtn { GlyphButton::Next }, diceBtn { GlyphButton::Dice },
        starBtn { GlyphButton::Star }, skinBtn { GlyphButton::Skin };
    juce::OwnedArray<TabButton> tabs;
    KeyboardView keyboard;
    juce::Slider pitchWheel, modWheel;
    HeroView hero;
    BlingOverlay overlay;
    LogoView logo;
    SplashView splash;

    std::set<int> favourites;
    int lastProgram = -1, appliedSkin = -1;
    juce::uint64 lastLit = ~0ull;
    int frame = 0, lastTheme = -1;
    juce::Random rnd;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GFunkEditor)
};
