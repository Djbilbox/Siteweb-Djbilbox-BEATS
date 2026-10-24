#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>

using namespace GFunkSpec;
using juce::Colour;
using juce::Graphics;
using juce::Rectangle;

namespace
{
const Colour cyan { 0xff00f0ff }, magenta { 0xffff2bd6 };

const char* const bgData[numSkins] = { BinaryData::bg_0_jpg, BinaryData::bg_1_jpg, BinaryData::bg_2_jpg,
                                       BinaryData::bg_3_jpg, BinaryData::bg_4_jpg, BinaryData::bg_5_jpg };
const int bgSize[numSkins] = { BinaryData::bg_0_jpgSize, BinaryData::bg_1_jpgSize, BinaryData::bg_2_jpgSize,
                               BinaryData::bg_3_jpgSize, BinaryData::bg_4_jpgSize, BinaryData::bg_5_jpgSize };
const char* const knobData[numSkins] = { BinaryData::knob_0_png, BinaryData::knob_1_png, BinaryData::knob_2_png,
                                         BinaryData::knob_3_png, BinaryData::knob_4_png, BinaryData::knob_5_png };
const int knobSize[numSkins] = { BinaryData::knob_0_pngSize, BinaryData::knob_1_pngSize, BinaryData::knob_2_pngSize,
                                 BinaryData::knob_3_pngSize, BinaryData::knob_4_pngSize, BinaryData::knob_5_pngSize };

juce::Font monoFont (float size, bool bold = true)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Font sansFont (float size)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), size, juce::Font::bold));
}

double nowSeconds() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }
Rectangle<int> toRect (const Rect& r) { return { r.x, r.y, r.w, r.h }; }

juce::Image loadImage (const void* data, int size) { return juce::ImageCache::getFromMemory (data, size); }

// glossy dark button face shared by the header buttons and the tabs
void drawGlossFace (Graphics& g, Rectangle<float> r, float radius, bool pressed, bool on, Colour accent)
{
    g.setGradientFill (juce::ColourGradient (pressed ? Colour (0xff15161a) : (on ? Colour (0xff2c3038) : Colour (0xff3a3e48)), 0, r.getY(),
                                             Colour (0xff0b0c0f), 0, r.getBottom(), false));
    g.fillRoundedRectangle (r, radius);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (pressed ? 0.04f : 0.16f), 0, r.getY(),
                                             juce::Colours::transparentWhite, 0, r.getCentreY(), false));
    g.fillRoundedRectangle (r.reduced (1.5f, 1.2f).withHeight (r.getHeight() * 0.5f), radius * 0.8f);
    g.setColour (on ? accent.withAlpha (0.9f) : juce::Colours::white.withAlpha (0.28f));
    g.drawRoundedRectangle (r, radius, on ? 1.6f : 1.1f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawRoundedRectangle (r.expanded (0.8f), radius + 0.8f, 1.0f);
}
} // namespace

namespace ui
{
Colour themeColour (int theme, float t, float time)
{
    t -= std::floor (t);
    const float s = 0.5f + 0.5f * std::sin (t * juce::MathConstants<float>::twoPi);
    switch (theme)
    {
        case 0:  return Colour (0xffffb92e).interpolatedWith (Colour (0xffffec9a), s);
        case 1:  return Colour (0xffff2bd6).interpolatedWith (Colour (0xffff7ac8), s);
        case 2:  return cyan.interpolatedWith (magenta, s);
        default: return Colour::fromHSV (std::fmod (t + time * 0.08f, 1.0f), 0.82f, 1.0f, 1.0f);
    }
}
} // namespace ui

//==============================================================================
ArtLookAndFeel::ArtLookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId, Colour (0xff07090d));
    setColour (juce::PopupMenu::textColourId, Colour (0xffd9f7ff));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, magenta.darker (0.3f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void ArtLookAndFeel::setSkin (int index, const juce::Image& knobImage)
{
    skin = juce::jlimit (0, numSkins - 1, index);
    knob = knobImage;
}

juce::Font ArtLookAndFeel::getComboBoxFont (juce::ComboBox&) { return monoFont (11.0f); }
juce::Font ArtLookAndFeel::getPopupMenuFont() { return monoFont (13.0f); }

void ArtLookAndFeel::drawLabel (Graphics& g, juce::Label& l)
{
    if (l.isBeingEdited())
        return;
    const auto text = l.getText();
    const auto c = l.findColour (juce::Label::textColourId);
    const bool light = c.getPerceivedBrightness() > 0.55f;
    g.setFont (l.getFont());
    auto area = l.getLocalBounds();
    g.setColour ((light ? juce::Colours::black : juce::Colours::white).withAlpha (light ? 0.55f : 0.40f));
    g.drawText (text, area.translated (0, 1), l.getJustificationType(), false);
    g.setColour (c);
    g.drawText (text, area, l.getJustificationType(), false);
}

void ArtLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float startAngle,
                                       float endAngle, juce::Slider& s)
{
    const auto& sk = ui::skinDef (skin);
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const auto c = b.getCentre();
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    if (knob.isValid())
        g.drawImage (knob, b.withSizeKeepingCentre (64.0f, 64.0f));

    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
    const float zero = startAngle + 0.5f * (endAngle - startAngle);
    const float from = bipolar ? zero : startAngle;
    const bool active = s.isMouseOverOrDragging();
    const int theme = proc != nullptr ? proc->blingTheme.load() : 0;
    const auto accent = ui::themeColour (theme, pos * 0.5f, 0.0f);
    const auto tickCol = ui::col (sk.tick);

    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
        const float r0 = 29.5f, r1 = (i % 5 == 0) ? 33.0f : 31.5f;
        g.setColour (tickCol.withAlpha (i % 5 == 0 ? 0.95f : 0.60f));
        g.drawLine (c.x + std::sin (a) * r0, c.y - std::cos (a) * r0, c.x + std::sin (a) * r1, c.y - std::cos (a) * r1, i % 5 == 0 ? 1.6f : 1.0f);
    }

    const float R = 27.0f;
    juce::Path arc;
    arc.addCentredArc (c.x, c.y, R, R, 0.0f, juce::jmin (from, angle), juce::jmax (from, angle), true);
    g.setColour (juce::Colours::black.withAlpha (0.30f));
    g.strokePath (arc, juce::PathStrokeType (4.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (active ? accent.brighter (0.3f) : ui::col (sk.ring));
    g.strokePath (arc, juce::PathStrokeType (2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float ax = std::sin (angle), ay = -std::cos (angle);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawLine (c.x + ax * 6.0f + 0.6f, c.y + ay * 6.0f + 0.9f, c.x + ax * 17.5f + 0.6f, c.y + ay * 17.5f + 0.9f, 3.6f);
    g.setColour (ui::col (sk.pointer));
    g.drawLine (c.x + ax * 6.0f, c.y + ay * 6.0f, c.x + ax * 17.5f, c.y + ay * 17.5f, 2.8f);
}

void ArtLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                       juce::Slider::SliderStyle, juce::Slider& s)
{
    auto r = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    juce::Path p;
    p.addRoundedRectangle (r, 6.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.8f), 8, { 2, 3 }).drawForPath (g, p);

    juce::ColourGradient body (Colour (0xff17191e), r.getX(), 0, Colour (0xff17191e), r.getRight(), 0, false);
    body.addColour (0.35, Colour (0xffd0d4dc));
    body.addColour (0.5, Colour (0xfff6f8fb));
    body.addColour (0.7, Colour (0xff7e8591));
    g.setGradientFill (body);
    g.fillPath (p);

    g.saveState();
    g.reduceClipRegion (r.toNearestInt());
    const float offset = std::fmod ((pos - r.getY()) * 0.9f, 9.0f);
    for (float yy = r.getY() - 9.0f + offset; yy < r.getBottom() + 9.0f; yy += 9.0f)
    {
        const float depth = 1.0f - std::abs ((yy - r.getCentreY()) / (r.getHeight() * 0.6f));
        g.setColour (juce::Colours::black.withAlpha (0.45f * juce::jlimit (0.0f, 1.0f, depth + 0.3f)));
        g.drawHorizontalLine ((int) yy, r.getX() + 1, r.getRight() - 1);
        g.setColour (juce::Colours::white.withAlpha (0.20f));
        g.drawHorizontalLine ((int) yy + 1, r.getX() + 1, r.getRight() - 1);
    }
    g.restoreState();

    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.75f), 0, r.getY(), juce::Colours::transparentBlack, 0, r.getY() + 30, false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.75f), 0, r.getBottom(), juce::Colours::transparentBlack, 0, r.getBottom() - 30, false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawRoundedRectangle (r, 6.0f, 1.2f);

    if (s.getMinimum() < 0.0)
    {
        g.setColour (Colour (0xffff3d9a).withAlpha (0.9f));
        g.fillRect (r.getX() + 3, r.getCentreY() - 1.0f, r.getWidth() - 6, 2.0f);
    }
}

void ArtLookAndFeel::drawComboBox (Graphics& g, int w, int h, bool isDown, int, int, int, int, juce::ComboBox& box)
{
    const auto& sk = ui::skinDef (skin);
    auto r = Rectangle<float> (0.5f, 0.5f, (float) w - 1.0f, (float) h - 1.0f);
    g.setGradientFill (juce::ColourGradient (ui::col (sk.comboTop), 0, 0, ui::col (sk.comboBot), 0, (float) h, false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.10f), 0, 0, juce::Colours::transparentWhite, 0, (float) h * 0.5f, false));
    g.fillRoundedRectangle (r.reduced (1.2f).withHeight ((float) h * 0.5f), 5.0f);
    auto line = ui::col (sk.comboLine);
    g.setColour (line.withMultipliedAlpha (isDown || box.isMouseOver() ? 1.0f : 0.75f));
    g.drawRoundedRectangle (r, 6.0f, isDown || box.isMouseOver() ? 1.6f : 1.2f);
    juce::Path tri;
    const float cx = (float) w - 11.0f, cy = (float) h * 0.5f;
    tri.addTriangle (cx - 4.0f, cy - 2.5f, cx + 4.0f, cy - 2.5f, cx, cy + 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.92f));
    g.fillPath (tri);
}

void ArtLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (5, 1, box.getWidth() - 20, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

//==============================================================================
void GlyphButton::paintButton (Graphics& g, bool over, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (1.5f);
    const auto accent = ui::themeColour (theme, 0.1f, 0.0f);
    drawGlossFace (g, r.translated (0, down ? 1.0f : 0.0f), 9.0f, down, over, accent);

    auto c = r.getCentre().translated (0, down ? 1.0f : 0.0f);
    const Colour ink = lit ? Colour (0xffff3d9a) : juce::Colours::white.withAlpha (over ? 1.0f : 0.90f);
    g.setColour (ink);
    juce::Path q;
    switch (kind)
    {
        case Prev: q.addTriangle (c.x + 6, c.y - 8, c.x + 6, c.y + 8, c.x - 7, c.y); break;
        case Next: q.addTriangle (c.x - 6, c.y - 8, c.x - 6, c.y + 8, c.x + 7, c.y); break;
        case Star: q.addStar (c, 5, 4.5f, 10.0f, -0.3f); break;
        case Dice:
        {
            auto sq = Rectangle<float> (17, 17).withCentre (c);
            g.drawRoundedRectangle (sq, 3.5f, 1.8f);
            for (auto d : { juce::Point<float> (-4.5f, -4.5f), { 4.5f, -4.5f }, { 0, 0 }, { -4.5f, 4.5f }, { 4.5f, 4.5f } })
                g.fillEllipse (Rectangle<float> (3.0f, 3.0f).withCentre (c + d));
            return;
        }
        case Skin:
        {
            const Colour cols[3] = { Colour (0xffff3d9a), Colour (0xffffc83a), Colour (0xff00e5ff) };
            for (int i = 0; i < 3; ++i)
            {
                g.setColour (cols[i]);
                g.fillEllipse (Rectangle<float> (10.0f, 10.0f).withCentre (c + juce::Point<float> ((float) (i - 1) * 8.0f, i == 1 ? 5.0f : -3.0f)));
            }
            g.setColour (juce::Colours::white.withAlpha (0.55f));
            g.drawEllipse (Rectangle<float> (28.0f, 24.0f).withCentre (c), 1.4f);
            return;
        }
    }
    if (lit && kind == Star)
    {
        g.setColour (Colour (0xffff3d9a).withAlpha (0.30f));
        g.fillEllipse (Rectangle<float> (30, 30).withCentre (c));
        g.setColour (ink);
    }
    g.fillPath (q);
}

void TabButton::paintButton (Graphics& g, bool over, bool)
{
    auto r = getLocalBounds().toFloat().reduced (1.5f, 2.0f);
    const bool on = getToggleState();
    const auto accent = ui::themeColour (theme, 0.1f, time);
    drawGlossFace (g, r, 6.0f, false, on, accent);
    if (on)
    {
        g.setColour (accent);
        g.fillRoundedRectangle (r.getX() + 10, r.getBottom() - 4.2f, r.getWidth() - 20, 2.4f, 1.2f);
        g.setColour (accent.withAlpha (0.28f));
        g.fillRoundedRectangle (r.getX() + 6, r.getBottom() - 9.0f, r.getWidth() - 12, 8.0f, 4.0f);
    }
    g.setFont (monoFont (13.0f));
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawText (getButtonText().toUpperCase(), r.translated (0, 1.0f).toNearestInt(), juce::Justification::centred);
    g.setColour (on ? juce::Colours::white : juce::Colours::white.withAlpha (over ? 0.92f : 0.68f));
    g.drawText (getButtonText().toUpperCase(), r.toNearestInt(), juce::Justification::centred);
}

//==============================================================================
KeyboardView::KeyboardView (GFunkProcessor& p)
    : juce::MidiKeyboardComponent (p.keyState, juce::MidiKeyboardComponent::horizontalKeyboard), proc (p)
{
    setScrollButtonsVisible (false);
    setAvailableRange (36, 96);
    setLowestVisibleKey (36);
    setMidiChannel (1);
    setOctaveForMiddleC (4);
    setKeyWidth ((float) keyboardRect.w / 36.0f);
}

void KeyboardView::drawWhiteNote (int note, Graphics& g, Rectangle<float> a, bool isDown, bool isOver, Colour, Colour)
{
    const auto& sk = ui::skinDef (proc.skin.load());
    const bool lit = isDown || proc.isNoteLit (note);
    const float time = (float) nowSeconds();
    const auto accent = ui::themeColour (proc.blingTheme.load(), (float) note * 0.037f, time);

    const auto top = ui::col (sk.keyWTop), bot = ui::col (sk.keyWBot);
    juce::ColourGradient grad (lit ? accent.brighter (0.55f) : (isOver ? top.brighter (0.1f) : top), a.getX(), a.getY(),
                               lit ? accent.darker (0.2f) : bot, a.getX(), a.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (a.withTop (a.getY() - 6.0f).reduced (0.8f, 0.0f), 4.0f);
    g.setColour (juce::Colours::black.withAlpha (0.30f));
    g.drawVerticalLine ((int) a.getRight() - 1, a.getY(), a.getBottom());
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawVerticalLine ((int) a.getX() + 1, a.getY(), a.getBottom() - 4);
    if (note % 12 == 0)
    {
        g.setColour (bot.getPerceivedBrightness() > 0.35f ? Colour (0xff4a4d57) : Colour (0xff9fb0c8));
        g.setFont (monoFont (11.0f));
        g.drawText ("C" + juce::String (note / 12 - 1), a.withTrimmedBottom (8).toNearestInt(), juce::Justification::centredBottom);
    }
}

void KeyboardView::drawBlackNote (int note, Graphics& g, Rectangle<float> a, bool isDown, bool, Colour)
{
    const auto& sk = ui::skinDef (proc.skin.load());
    const bool lit = isDown || proc.isNoteLit (note);
    const auto accent = ui::themeColour (proc.blingTheme.load(), (float) note * 0.037f, (float) nowSeconds());
    juce::ColourGradient grad (ui::col (sk.keyBTop), a.getX(), a.getY(), lit ? accent : ui::col (sk.keyBBot), a.getX(), a.getBottom(), false);
    grad.addColour (0.82, lit ? accent.darker (0.2f) : ui::col (sk.keyBBot).brighter (0.05f));
    g.setGradientFill (grad);
    g.fillRoundedRectangle (a.withTop (a.getY() - 6.0f), 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.drawRoundedRectangle (a.reduced (1.5f, 0.0f).withTrimmedTop (-4.0f).withTrimmedBottom (a.getHeight() * 0.28f), 2.5f, 1.0f);
    g.setColour (lit ? accent.brighter (0.3f) : juce::Colours::white.withAlpha (0.14f));
    g.fillRect (a.getX() + 2.0f, a.getBottom() - 3.0f, a.getWidth() - 4.0f, 2.0f);
}

//==============================================================================
LedScreen::LedScreen (GFunkEditor& e, GFunkProcessor& p) : editor (e), proc (p)
{
    mask = loadImage (BinaryData::led_mask_png, BinaryData::led_mask_pngSize);
    startTimerHz (30);
}

void LedScreen::timerCallback()
{
    time += 1.0f / 30.0f;
    ++tick;
    const bool live = tab == 0 || tab == 2;
    const bool rainbow = proc.blingTheme.load() == 3;
    if (live || (tab == 3 && tick % 3 == 0) || (rainbow && tick % 6 == 0) || tick % 60 == 0)
        repaint();
}

std::vector<int> LedScreen::currentList() const
{
    std::vector<int> v;
    if (browseCat < numCategories)
        for (int i = 0; i < presetsPerCategory; ++i) v.push_back (browseCat * presetsPerCategory + i);
    else
        v.assign (editor.getFavourites().begin(), editor.getFavourites().end());
    return v;
}

void LedScreen::revealPreset (int index)
{
    browseCat = presetInfo[index].category;
    const auto list = currentList();
    const auto it = std::find (list.begin(), list.end(), index);
    const int pos = it == list.end() ? 0 : (int) (it - list.begin());
    scroll = juce::jlimit (0, juce::jmax (0, (int) list.size() - visibleRows()), pos - visibleRows() / 2);
}

void LedScreen::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (Colour (0xff04090d), b.getCentreX(), b.getCentreY(), Colour (0xff010203), 0, 0, true));
    g.fillAll();

    switch (tab)
    {
        case 0:  paintScope (g); break;
        case 1:  paintBrowser (g); break;
        case 2:  paintSpectrum (g); break;
        default: paintLights (g); break;
    }

    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    if (mask.isValid())
        g.drawImage (mask, b);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), 0, 0, juce::Colours::transparentWhite, 0, b.getHeight() * 0.45f, false));
    g.fillRect (b.withHeight (b.getHeight() * 0.45f));
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, b.getCentreX(), b.getCentreY(), juce::Colours::black.withAlpha (0.55f), 0, 0, true));
    g.fillAll();
}

void LedScreen::paintScope (Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (6.0f);
    const auto accent = ui::themeColour (proc.blingTheme.load(), 0.0f, time);

    g.setColour (cyan.withAlpha (0.20f));
    for (int i = 1; i < 12; ++i)
        g.drawVerticalLine ((int) (b.getX() + b.getWidth() * (float) i / 12.0f), b.getY(), b.getBottom());
    for (int i = 1; i < 8; ++i)
        g.drawHorizontalLine ((int) (b.getY() + b.getHeight() * (float) i / 8.0f), b.getX(), b.getRight());
    g.setColour (cyan.withAlpha (0.45f));
    g.drawHorizontalLine ((int) b.getCentreY(), b.getX(), b.getRight());
    g.drawVerticalLine ((int) b.getCentreX(), b.getY(), b.getBottom());

    float s[1024];
    proc.copyScope (s);
    int trig = 0;
    for (int i = 1; i < 480; ++i)
        if (s[i - 1] <= 0.0f && s[i] > 0.0f) { trig = i; break; }

    juce::Path wave;
    const float midY = b.getCentreY(), amp = b.getHeight() * 0.44f;
    constexpr int pts = 512;
    for (int i = 0; i < pts; ++i)
    {
        const float px = b.getX() + b.getWidth() * (float) i / (pts - 1);
        const float py = midY - juce::jlimit (-1.0f, 1.0f, s[trig + i] * 2.4f) * amp;
        if (i == 0) wave.startNewSubPath (px, py); else wave.lineTo (px, py);
    }
    g.setColour (magenta.withAlpha (0.45f));
    g.strokePath (ghost, juce::PathStrokeType (2.0f));
    ghost = wave;
    for (float w : { 11.0f, 7.0f, 4.0f })
    {
        g.setColour (cyan.withAlpha (w > 8 ? 0.12f : (w > 5 ? 0.22f : 0.4f)));
        g.strokePath (wave, juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (Colour (0xffe8ffff));
    g.strokePath (wave, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setFont (monoFont (13.0f));
    g.setColour (accent);
    g.drawText ("GFUNK 2027  /  SCOPE", b.toNearestInt().reduced (4, 2), juce::Justification::topLeft);
    g.setColour (cyan);
    g.drawText (juce::String (presetInfo[proc.getCurrentProgram()].name).toUpperCase(), b.toNearestInt().reduced (4, 2), juce::Justification::bottomLeft);
}

void LedScreen::paintBrowser (Graphics& g)
{
    const int rows = visibleRows();
    const int cur = proc.getCurrentProgram();
    const int theme = proc.blingTheme.load();
    const auto accent = ui::themeColour (theme, 0.3f, time);
    const auto list = currentList();
    const int n = (int) list.size();
    scroll = juce::jlimit (0, juce::jmax (0, n - rows), scroll);

    g.setFont (monoFont (15.0f));
    for (int i = 0; i <= numCategories; ++i)
    {
        const auto row = Rectangle<int> (4, 5 + i * rowH, catW, rowH - 1);
        const bool sel = i == browseCat;
        if (sel)
        {
            g.setColour (accent.withAlpha (0.95f));
            g.fillRect (row);
        }
        g.setColour (sel ? Colour (0xff021014) : (i == numCategories ? magenta : Colour (0xffcaf8ff)));
        g.drawText (i == numCategories ? juce::String ("* FAVORITES") : juce::String (categoryNames[i]), row.reduced (6, 0), juce::Justification::centredLeft);
    }

    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawVerticalLine (catW + 10, 5, (float) getHeight() - 5);

    const int x0 = catW + 18;
    for (int r = 0; r < rows; ++r)
    {
        const int li = scroll + r;
        if (li >= n) break;
        const int idx = list[(size_t) li];
        const auto row = Rectangle<int> (x0, 5 + r * rowH, getWidth() - x0 - barW - 12, rowH - 1);
        const bool isCur = idx == cur;
        if (isCur)
        {
            g.setColour (accent);
            g.fillRect (row);
        }
        else if (r == hoverRow)
        {
            g.setColour (cyan.withAlpha (0.24f));
            g.fillRect (row);
        }
        g.setColour (isCur ? Colour (0xff021014) : Colour (0xff86f7ff));
        g.drawText (juce::String (idx + 1).paddedLeft ('0', 4) + "  " + presetInfo[idx].name, row.reduced (6, 0), juce::Justification::centredLeft);
        if (editor.isFavourite (idx))
        {
            g.setColour (isCur ? Colour (0xff021014) : magenta);
            g.drawText ("*", row.reduced (6, 0), juce::Justification::centredRight);
        }
    }

    // thick scrollbar with arrows
    auto track = Rectangle<int> (getWidth() - barW - 5, 5, barW, getHeight() - 10);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.fillRoundedRectangle (track.toFloat(), 5.0f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawRoundedRectangle (track.toFloat(), 5.0f, 1.0f);
    if (n > rows)
    {
        const float th = (float) track.getHeight();
        const float bh = juce::jmax (44.0f, th * (float) rows / (float) n);
        const float by = (float) track.getY() + (th - bh) * (float) scroll / (float) (n - rows);
        auto thumb = Rectangle<float> ((float) track.getX() + 1.5f, by, (float) barW - 3.0f, bh);
        g.setGradientFill (juce::ColourGradient (accent.brighter (0.3f), thumb.getX(), 0, accent.darker (0.3f), thumb.getRight(), 0, false));
        g.fillRoundedRectangle (thumb, 4.5f);
        g.setColour (juce::Colours::white.withAlpha (draggingBar ? 0.9f : 0.5f));
        g.drawRoundedRectangle (thumb, 4.5f, 1.2f);
        g.setColour (Colour (0xff021014).withAlpha (0.55f));
        for (int k = -1; k <= 1; ++k)
            g.fillRect (thumb.getX() + 3.0f, thumb.getCentreY() + (float) k * 4.0f - 0.7f, thumb.getWidth() - 6.0f, 1.4f);
    }
    if (list.empty())
    {
        g.setColour (magenta);
        g.drawText ("RIGHT-CLICK A PRESET TO ADD IT HERE", getLocalBounds().withLeft (x0), juce::Justification::centred);
    }
}

void LedScreen::paintSpectrum (Graphics& g)
{
    proc.copyFft (fftData.data());
    for (int i = 0; i < 2048; ++i)
    {
        const float w = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * (float) i / 2047.0f);
        fftData[(size_t) i] *= w;
    }
    std::fill (fftData.begin() + 2048, fftData.end(), 0.0f);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    const float sr = (float) juce::jmax (8000.0, proc.getSampleRate());
    const float binHz = sr / 2048.0f;
    constexpr int nb = 64;
    auto b = getLocalBounds().toFloat().reduced (10.0f, 12.0f).withTrimmedBottom (14.0f);
    const float bw = b.getWidth() / nb;
    const int segs = 22;
    const float segH = b.getHeight() / segs;
    const auto accent = ui::themeColour (proc.blingTheme.load(), 0.2f, time);

    for (int i = 0; i < nb; ++i)
    {
        const float f0 = 30.0f * std::pow (16000.0f / 30.0f, (float) i / nb);
        const float f1 = 30.0f * std::pow (16000.0f / 30.0f, (float) (i + 1) / nb);
        float m = 0.0f;
        for (int k = juce::jmax (1, (int) (f0 / binHz)); k <= juce::jmin (1023, juce::jmax ((int) (f1 / binHz), (int) (f0 / binHz))); ++k)
            m = juce::jmax (m, fftData[(size_t) k]);
        const float db = 20.0f * std::log10 (m / 512.0f + 1.0e-6f);
        const float v = juce::jlimit (0.0f, 1.0f, (db + 70.0f) / 70.0f);
        bars[i] = juce::jmax (v, bars[i] * 0.86f);
        peaks[i] = juce::jmax (bars[i], peaks[i] - 0.012f);

        const int lit = (int) (bars[i] * segs);
        for (int sIdx = 0; sIdx < segs; ++sIdx)
        {
            const float t = (float) sIdx / segs;
            const auto col = t < 0.6f ? cyan.interpolatedWith (accent, t / 0.6f) : accent.interpolatedWith (magenta, (t - 0.6f) / 0.4f);
            auto r = Rectangle<float> (b.getX() + bw * (float) i + 1.0f, b.getBottom() - segH * (float) (sIdx + 1) + 1.0f, bw - 2.0f, segH - 2.0f);
            g.setColour (sIdx < lit ? col : col.withAlpha (0.08f));
            g.fillRect (r);
        }
        const int pk = juce::jlimit (0, segs - 1, (int) (peaks[i] * segs));
        g.setColour (juce::Colours::white);
        g.fillRect (Rectangle<float> (b.getX() + bw * (float) i + 1.0f, b.getBottom() - segH * (float) (pk + 1) + 1.0f, bw - 2.0f, segH - 2.0f));
    }
    g.setFont (monoFont (11.0f));
    g.setColour (Colour (0xffcfe9f2));
    g.drawText ("30", getLocalBounds().reduced (10, 2), juce::Justification::bottomLeft);
    g.drawText ("SPECTRUM", getLocalBounds().reduced (10, 2), juce::Justification::centredBottom);
    g.drawText ("16k", getLocalBounds().reduced (10, 2), juce::Justification::bottomRight);
}

void LedScreen::paintLights (Graphics& g)
{
    const int theme = proc.blingTheme.load(), level = proc.blingLevel.load(), skin = proc.skin.load();
    const auto accent = ui::themeColour (theme, 0.2f, time);
    g.setFont (monoFont (17.0f));
    g.setColour (accent);
    g.drawText ("LIGHT SHOW  &  SKINS", 14, 6, 400, 24, juce::Justification::centredLeft);

    auto button = [&] (Rectangle<int> r, const juce::String& text, bool on, Colour col)
    {
        g.setColour (on ? col : Colour (0xff0c141a));
        g.fillRect (r);
        g.setColour (on ? Colour (0xff020406) : col);
        g.drawRect (r, 1);
        g.setFont (monoFont (12.5f));
        g.drawText (text, r, juce::Justification::centred);
    };
    auto label = [&] (const juce::String& t, int y)
    {
        g.setFont (monoFont (13.0f));
        g.setColour (Colour (0xffcfe9f2));
        g.drawText (t, 14, y, 100, 22, juce::Justification::centredLeft);
    };

    const char* tn[] = { "GOLD", "CANDY", "NEON", "ALL" };
    label ("COLOURS", 40);
    for (int i = 0; i < 4; ++i)
    {
        themeRects[i] = { 120 + i * 96, 40, 90, 22 };
        button (themeRects[i], tn[i], i == theme, ui::themeColour (i, 0.2f, time));
    }
    const char* ln[] = { "OFF", "NORMAL", "MAX" };
    label ("BLING", 72);
    for (int i = 0; i < 3; ++i)
    {
        blingRects[i] = { 120 + i * 96, 72, 90, 22 };
        button (blingRects[i], ln[i], i == level, accent);
    }
    label ("SKIN", 104);
    for (int i = 0; i < numSkins; ++i)
    {
        skinRects[i] = { 120 + (i % 3) * 180, 104 + (i / 3) * 26, 174, 22 };
        button (skinRects[i], ui::skinDef (i).name, i == skin, ui::themeColour (theme, (float) i * 0.17f, time));
    }
    label ("SCROLL", 160);
    const char* sn[] = { "NORMAL", "INVERTED" };
    for (int i = 0; i < 2; ++i)
    {
        scrollRects[i] = { 120 + i * 96, 160, 90, 22 };
        button (scrollRects[i], sn[i], (i == 1) == (proc.scrollInvert.load() != 0), accent);
    }
    label ("FUNK", 192);
    const float funk = proc.funk.load();
    const int n = 28;
    for (int i = 0; i < n; ++i)
    {
        const float t = (float) i / n;
        const auto col = cyan.interpolatedWith (magenta, t);
        g.setColour (t < funk ? col : col.withAlpha (0.10f));
        g.fillRect (Rectangle<float> (120.0f + (float) i * 17.0f, 194.0f, 14.0f, 18.0f));
    }
    g.setColour (Colour (0xff9fe9f5));
    g.setFont (monoFont (11.5f));
    g.drawText ("PLAY FUNKY: THE MORE YOU PLAY, THE MORE EVERYTHING LIGHTS UP.", 14, 214, getWidth() - 28, 18, juce::Justification::centredLeft);
}

void LedScreen::setScrollFromBar (int y)
{
    const int n = (int) currentList().size(), rows = visibleRows();
    if (n <= rows) return;
    const float th = (float) getHeight() - 10.0f;
    const float bh = juce::jmax (44.0f, th * (float) rows / (float) n);
    const float t = juce::jlimit (0.0f, 1.0f, ((float) y - 5.0f - bh * 0.5f) / (th - bh));
    scroll = juce::jlimit (0, n - rows, (int) std::round (t * (float) (n - rows)));
    repaint();
}

void LedScreen::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();
    repaint();
    if (tab == 3)
    {
        for (int i = 0; i < 4; ++i) if (themeRects[i].contains (p)) proc.blingTheme.store (i);
        for (int i = 0; i < 3; ++i) if (blingRects[i].contains (p)) proc.blingLevel.store (i);
        for (int i = 0; i < numSkins; ++i) if (skinRects[i].contains (p)) editor.setSkin (i);
        for (int i = 0; i < 2; ++i) if (scrollRects[i].contains (p)) proc.scrollInvert.store (i);
        return;
    }
    if (tab != 1)
        return;

    if (p.x >= getWidth() - barW - 8)
    {
        draggingBar = true;
        setScrollFromBar (p.y);
        return;
    }
    if (p.x < catW + 10)
    {
        const int row = (p.y - 5) / rowH;
        if (row >= 0 && row <= numCategories)
        {
            browseCat = row;
            scroll = 0;
        }
        return;
    }
    const int row = (p.y - 5) / rowH;
    const auto list = currentList();
    const int li = scroll + row;
    if (row >= 0 && row < visibleRows() && li < (int) list.size())
    {
        if (e.mods.isPopupMenu())
            editor.toggleFavourite (list[(size_t) li]);
        else
            editor.loadPreset (list[(size_t) li]);
    }
}

void LedScreen::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingBar)
        setScrollFromBar (e.y);
}

void LedScreen::mouseMove (const juce::MouseEvent& e)
{
    const int row = tab == 1 && e.x > catW + 12 && e.x < getWidth() - barW - 8 ? (e.y - 5) / rowH : -1;
    if (row != hoverRow)
    {
        hoverRow = row;
        repaint();
    }
}

void LedScreen::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    if (tab != 1)
        return;
    const int n = (int) currentList().size();
    float dy = w.deltaY;
    if (w.isReversed)
        dy = -dy;
    const float dir = proc.scrollInvert.load() != 0 ? 1.0f : -1.0f;
    scrollAcc += dir * dy * 14.0f;
    const int step = (int) scrollAcc;
    if (step != 0)
    {
        scrollAcc -= (float) step;
        scroll = juce::jlimit (0, juce::jmax (0, n - visibleRows()), scroll + step);
    }
    repaint();
}

//==============================================================================
PresetLcd::PresetLcd (GFunkEditor& e, GFunkProcessor& p) : editor (e), proc (p)
{
    mask = loadImage (BinaryData::led_mask_png, BinaryData::led_mask_pngSize);
}

void PresetLcd::showReadout (const juce::String& text)
{
    readout = text;
    readoutStamp = juce::Time::getMillisecondCounter();
    repaint();
}

void PresetLcd::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (Colour (0xff03121a), b.getCentreX(), b.getCentreY(), Colour (0xff010305), 0, 0, true));
    g.fillAll();

    const int theme = proc.blingTheme.load();
    const float time = (float) nowSeconds();
    const auto accent = ui::themeColour (theme, 0.1f, time);
    const int cur = proc.getCurrentProgram();
    const bool showRead = juce::Time::getMillisecondCounter() - readoutStamp < 1500 && readout.isNotEmpty();

    if (showRead)
    {
        g.setFont (monoFont (25.0f));
        g.setColour (cyan);
        g.drawText (readout.toUpperCase(), getLocalBounds().reduced (14, 0), juce::Justification::centred);
    }
    else
    {
        g.setFont (monoFont (25.0f));
        g.setColour (accent);
        g.drawText (juce::String (presetInfo[cur].name).toUpperCase(), 14, 2, getWidth() - 28, 27, juce::Justification::centredLeft);
        g.setFont (monoFont (13.0f));
        g.setColour (cyan);
        g.drawText (juce::String (categoryNames[presetInfo[cur].category]), 14, 28, 300, 16, juce::Justification::centredLeft);
        g.setColour (Colour (0xffcaf8ff));
        g.drawText (juce::String (cur + 1).paddedLeft ('0', 4) + " / " + juce::String (numPresets), getWidth() - 160, 28, 146, 16, juce::Justification::centredRight);
        if (editor.isFavourite (cur))
        {
            g.setColour (magenta);
            g.drawText ("* FAV", getWidth() - 260, 28, 90, 16, juce::Justification::centredRight);
        }
    }

    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    if (mask.isValid())
        g.drawImage (mask, 0, 0, getWidth(), getHeight(), 0, 0, getWidth() * 2, getHeight() * 2);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.08f), 0, 0, juce::Colours::transparentWhite, 0, b.getHeight() * 0.5f, false));
    g.fillRect (b.withHeight (b.getHeight() * 0.5f));
}

void MeterView::paint (Graphics& g)
{
    const float target = juce::jlimit (0.0f, 1.0f, proc.getOutputLevel() * 1.15f);
    level += (target - level) * 0.45f;
    peak = juce::jmax (level, peak - 0.012f);
    const int n = 30;
    const float w = (float) getWidth() / n;
    for (int i = 0; i < n; ++i)
    {
        const float t = (float) i / n;
        const auto col = t < 0.7f ? cyan.interpolatedWith (Colour (0xffb06bff), t / 0.7f) : Colour (0xffb06bff).interpolatedWith (magenta, (t - 0.7f) / 0.3f);
        const bool on = t < level || (int) (peak * n) == i;
        g.setColour (on ? col : col.withAlpha (0.10f));
        g.fillRoundedRectangle (w * (float) i + 1.0f, 4.0f, w - 2.5f, (float) getHeight() - 8.0f, 1.5f);
    }
}

//==============================================================================
BackgroundView::BackgroundView()
{
    setInterceptsMouseClicks (false, false);
    setBufferedToImage (true);
}

void BackgroundView::paint (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    if (img.isValid())
        g.drawImage (img, getLocalBounds().toFloat());
}

//==============================================================================
namespace
{
const Rectangle<int> heroBounds { heroRect.x - 40, heroRect.y - 22, windowW - (heroRect.x - 40), heroRect.h + 40 };
}

HeroView::HeroView (GFunkProcessor& p) : proc (p)
{
    cutout = loadImage (BinaryData::hero_cutout_png, BinaryData::hero_cutout_pngSize);
    setInterceptsMouseClicks (false, false);
    startTimerHz (20);
}

void HeroView::timerCallback()
{
    time += 1.0f / 20.0f;
    const auto mp = getLocalPoint (nullptr, juce::Desktop::getMousePosition().toFloat());
    float tx = 0.0f, ty = 0.0f;
    if (juce::Desktop::getInstance().getMainMouseSource().getScreenPosition() != juce::Point<float>())
    {
        const auto c = juce::Point<float> ((float) (heroRect.x - heroBounds.getX() + heroRect.w / 2), (float) (heroRect.y - heroBounds.getY() + heroRect.h / 2));
        tx = juce::jlimit (-1.0f, 1.0f, (mp.x - c.x) / 700.0f) * -9.0f;
        ty = juce::jlimit (-1.0f, 1.0f, (mp.y - c.y) / 500.0f) * -6.0f;
    }
    ox += (tx - ox) * 0.25f;
    oy += (ty - oy) * 0.25f;
    bob = std::sin (time * 1.4f) * 1.8f;
    const float hit = proc.hit.load();
    bounce += (hit - bounce) * 0.5f;
    repaint();
}

void HeroView::paint (Graphics& g)
{
    const int theme = proc.blingTheme.load();
    const float t = (float) nowSeconds();
    const float f = proc.funk.load();
    const auto accent = ui::themeColour (theme, 0.15f + t * 0.05f, t);
    const float px = (float) (heroRect.x - heroBounds.getX()), py = (float) (heroRect.y - heroBounds.getY());

    // neon sign
    {
        const float flick = 0.82f + 0.18f * std::sin (t * 23.0f) * (f > 0.05f ? 1.0f : 0.15f);
        g.setFont (juce::Font (juce::FontOptions ("Impact", 40.0f, juce::Font::plain)));
        const juce::String sign = "G-FUNK";
        auto area = Rectangle<float> (px + 22.0f, py + 12.0f, 240.0f, 48.0f);
        for (float w : { 9.0f, 6.0f, 3.0f })
        {
            g.setColour (accent.withAlpha (0.05f * flick * (1.0f + proc.hit.load())));
            for (float dx = -w; dx <= w; dx += w) for (float dy = -w; dy <= w; dy += w)
                g.drawText (sign, area.translated (dx * 0.4f, dy * 0.4f), juce::Justification::centredLeft, false);
        }
        g.setColour (accent.withAlpha (juce::jlimit (0.0f, 1.0f, 0.55f + 0.45f * flick)));
        g.drawText (sign, area, juce::Justification::centredLeft, false);
        g.setColour (juce::Colours::white.withAlpha (0.65f * flick));
        g.drawText (sign, area.translated (-0.6f, -0.6f), juce::Justification::centredLeft, false);
        g.setFont (monoFont (11.5f));
        g.setColour (accent.withAlpha (0.85f));
        g.drawText ("WEST COAST EDITION  *  2027", Rectangle<float> (px + 24.0f, py + 56.0f, 300.0f, 16.0f), juce::Justification::centredLeft, false);
    }

    if (! cutout.isValid())
        return;

    // the baked 3D relief cut-out with parallax, bob and a bounce on every hit
    const float scale = 0.462f;
    const float w = (float) cutout.getWidth() * scale, h = (float) cutout.getHeight() * scale;
    const float cx = 8.0f + w * 0.5f + ox;       // starts just over the screen bezel, pops out of the panel
    const float bottom = py + (float) heroRect.h + 12.0f + oy + bob - bounce * 7.0f;
    auto dest = Rectangle<float> (w, h).withCentre ({ cx, bottom - h * 0.5f });

    // coloured glow behind the car
    g.setColour (accent.withAlpha (0.10f + 0.30f * bounce + 0.12f * f));
    g.fillEllipse (dest.reduced (w * 0.12f, h * 0.30f).translated (0, h * 0.22f));

    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.setOpacity (1.0f);
    g.drawImage (cutout, dest);
}

//==============================================================================
BlingOverlay::BlingOverlay (GFunkProcessor& p, KeyboardView& k) : proc (p), keys (k)
{
    setInterceptsMouseClicks (false, false);
    sprite = juce::Image (juce::Image::ARGB, 64, 64, true);
    {
        Graphics g (sprite);
        juce::ColourGradient rg (juce::Colours::white, 32, 32, juce::Colours::transparentWhite, 0, 32, true);
        rg.addColour (0.25, juce::Colours::white.withAlpha (0.55f));
        g.setGradientFill (rg);
        g.fillEllipse (0, 0, 64, 64);
    }
    addRing (toRect (headerRect).toFloat(), 12.0f, 2.6f, 34.0f);
    addRing (toRect (screenRect).toFloat(), 16.0f, 4.0f, 28.0f);
    addRing (toRect (keysRect).toFloat(), 14.0f, 3.0f, 34.0f);
}

void BlingOverlay::addRing (Rectangle<float> r, float radius, float inset, float spacing)
{
    juce::Path p;
    p.addRoundedRectangle (r.reduced (inset), radius);
    juce::PathFlatteningIterator it (p, {}, 0.5f);
    float acc = spacing;
    while (it.next())
    {
        juce::Point<float> a (it.x1, it.y1), b (it.x2, it.y2);
        const float len = a.getDistanceFrom (b);
        float d = 0.0f;
        while (acc <= len - d)
        {
            d += acc;
            bulbs.push_back (a + (b - a) * (d / len));
            acc = spacing;
        }
        acc -= (len - d);
    }
}

void BlingOverlay::advance()
{
    const double now = nowSeconds();
    const float dt = (float) juce::jlimit (0.0, 0.25, lastNow > 0.0 ? now - lastNow : 1.0 / 30.0);
    lastNow = now;
    time += dt;
    const float hit = proc.hit.load(), funk = proc.funk.load();
    smoothHit += (hit - smoothHit) * 0.6f;
    smoothFunk += (funk - smoothFunk) * 0.25f;
    const int level = proc.blingLevel.load();
    if (level > 0 && hit > lastHit + 0.1f && ! bulbs.empty())
    {
        const int n = 2 + (int) (hit * 8.0f * (level == 2 ? 1.7f : 1.0f));
        for (int i = 0; i < n && sparkles.size() < 90; ++i)
            sparkles.push_back ({ bulbs[(size_t) rnd.nextInt ((int) bulbs.size())], 0.0f, 5.0f + rnd.nextFloat() * 11.0f, rnd.nextFloat(),
                                  ui::themeColour (proc.blingTheme.load(), rnd.nextFloat(), time) });
    }
    lastHit = hit;
    for (auto& s : sparkles) s.life += dt * 2.1f;
    sparkles.erase (std::remove_if (sparkles.begin(), sparkles.end(), [] (const Sparkle& s) { return s.life >= 1.0f; }), sparkles.end());
}

void BlingOverlay::markDirty()
{
    if (proc.blingLevel.load() == 0)
    {
        repaint();
        return;
    }
    for (const auto& b : bulbs)
        repaint (juce::Rectangle<int> (30, 30).withCentre (b.toInt()));
    for (const auto& s : sparkles)
        repaint (juce::Rectangle<int> ((int) (s.size * 3.0f) + 8, (int) (s.size * 3.0f) + 8).withCentre (s.pos.toInt()));
    auto b = getLocalBounds();
    repaint (b.removeFromBottom (26));
    repaint (b.withWidth (14));
    repaint (b.withLeft (b.getRight() - 14));
    repaint (Rectangle<int> (-20, -30, 440, 130));
    if (smoothHit > 0.02f)
        for (int i = 0; i < numPanels; ++i)
            repaint (toRect (panelRects[i]).expanded (8));
    const auto kb = keys.getPosition();
    for (int note = 36; note <= 96; ++note)
        if (proc.isNoteLit (note))
        {
            auto r = keys.keyRect (note).translated ((float) kb.x, (float) kb.y);
            repaint (Rectangle<int> (120, 100).withCentre ({ (int) r.getCentreX(), (int) r.getY() - 8 }));
        }
}

void BlingOverlay::paint (Graphics& g)
{
    const int level = proc.blingLevel.load(), theme = proc.blingTheme.load();
    if (level == 0)
        return;

    const float boost = level == 2 ? 1.35f : 1.0f;
    const float f = smoothFunk, hit = smoothHit;
    const float e = juce::jlimit (0.0f, 1.0f, (0.12f + f) * boost);
    const auto base = ui::themeColour (theme, time * 0.2f, time);
    auto b = getLocalBounds().toFloat();

    const float ga = juce::jlimit (0.0f, 1.0f, 0.10f + 0.45f * e + 0.4f * hit);
    g.setGradientFill (juce::ColourGradient (base.withAlpha (ga), 0, b.getBottom(), base.withAlpha (0.0f), 0, b.getBottom() - 22.0f, false));
    g.fillRect (b.withTop (b.getBottom() - 22.0f));
    g.setGradientFill (juce::ColourGradient (base.withAlpha (ga * 0.6f), 0, 0, base.withAlpha (0.0f), 12.0f, 0, false));
    g.fillRect (b.withWidth (12.0f));
    g.setGradientFill (juce::ColourGradient (base.withAlpha (ga * 0.6f), b.getRight(), 0, base.withAlpha (0.0f), b.getRight() - 12.0f, 0, false));
    g.fillRect (b.withLeft (b.getRight() - 12.0f));

    if (hit > 0.02f)
    {
        for (int i = 0; i < numPanels; ++i)
        {
            const auto r = toRect (panelRects[i]).toFloat();
            g.setColour (ui::themeColour (theme, (float) i * 0.17f, time).withAlpha (juce::jlimit (0.0f, 0.8f, hit * 0.7f * boost)));
            g.drawRoundedRectangle (r.reduced (3.5f), 12.0f, 2.2f);
            g.setColour (ui::themeColour (theme, (float) i * 0.17f, time).withAlpha (hit * 0.18f * boost));
            g.drawRoundedRectangle (r.reduced (1.0f), 14.0f, 6.0f);
        }
    }

    {
        g.setColour (Colour (0xffffc93a).withAlpha (juce::jlimit (0.0f, 0.9f, 0.04f + 0.50f * hit + 0.2f * f)));
        g.drawImage (sprite, Rectangle<float> (-20, -30, 440, 130), juce::RectanglePlacement::stretchToFit, true);
    }

    const float speed = 2.2f + f * 10.0f * boost;
    for (size_t i = 0; i < bulbs.size(); ++i)
    {
        float wave = 0.5f + 0.5f * std::sin (time * speed - (float) i * 0.5f);
        wave = wave * wave * wave;
        const float br = juce::jmin (1.0f, 0.04f + 0.9f * wave * (0.25f + 0.75f * e) + hit * 0.6f);
        const auto col = ui::themeColour (theme, (float) i * 0.013f + time * 0.05f, time);
        if (br > 0.2f)
        {
            const float size = 8.0f + 18.0f * br;
            g.setColour (col.withAlpha (br * 0.9f));
            g.drawImage (sprite, Rectangle<float> (size, size).withCentre (bulbs[i]), juce::RectanglePlacement::stretchToFit, true);
        }
        g.setColour ((br > 0.2f ? juce::Colours::white : col).withAlpha (0.2f + 0.75f * br));
        g.fillEllipse (Rectangle<float> (3.0f, 3.0f).withCentre (bulbs[i]));
    }

    const auto kb = keys.getPosition();
    for (int note = 36; note <= 96; ++note)
    {
        if (! proc.isNoteLit (note)) continue;
        auto r = keys.keyRect (note).translated ((float) kb.x, (float) kb.y);
        const auto col = ui::themeColour (theme, (float) note * 0.037f, time);
        g.setColour (col.withAlpha (0.9f));
        g.drawImage (sprite, Rectangle<float> (110.0f, 90.0f).withCentre ({ r.getCentreX(), r.getY() - 8.0f }), juce::RectanglePlacement::stretchToFit, true);
    }

    for (auto& s : sparkles)
    {
        const float k = std::sin (s.life * juce::MathConstants<float>::pi);
        const float r = s.size * k;
        juce::Path star;
        star.addStar (s.pos, 4, r * 0.12f, r, s.rot * 3.14f + s.life * 0.6f);
        g.setColour (s.col.withAlpha (0.8f * k));
        g.drawImage (sprite, Rectangle<float> (r * 2.4f, r * 2.4f).withCentre (s.pos), juce::RectanglePlacement::stretchToFit, true);
        g.setColour (juce::Colours::white.withAlpha (0.95f * k));
        g.fillPath (star);
    }
}

//==============================================================================
LogoView::LogoView (GFunkProcessor& p) : proc (p)
{
    img = loadImage (BinaryData::logo_png, BinaryData::logo_pngSize);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void LogoView::paint (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImage (img, getLocalBounds().toFloat());
    const float hit = proc.hit.load();
    if (hit > 0.05f && proc.blingLevel.load() > 0)
    {
        g.setOpacity (juce::jlimit (0.0f, 0.7f, hit * 0.6f));
        g.drawImage (img, getLocalBounds().toFloat());
    }
}

SplashView::SplashView()
{
    img = loadImage (BinaryData::hero_jpg, BinaryData::hero_jpgSize);
    startTimerHz (30);
}

void SplashView::show()
{
    alpha = 1.0f;
    holdMs = 4200;
    setVisible (true);
    toFront (false);
}

void SplashView::timerCallback()
{
    if (! isVisible())
        return;
    if (holdMs > 0)
        holdMs -= 33;
    else
        alpha -= 0.07f;
    if (alpha <= 0.0f)
        setVisible (false);
    repaint();
}

void SplashView::paint (Graphics& g)
{
    g.setOpacity (juce::jlimit (0.0f, 1.0f, alpha));
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImage (img, getLocalBounds().toFloat());
    const float pulse = 0.55f + 0.45f * std::sin ((float) nowSeconds() * 4.0f);
    g.setColour (juce::Colours::white.withAlpha (alpha * pulse));
    g.setFont (monoFont (15.0f));
    g.drawText ("CLICK TO ENTER", getLocalBounds().withTrimmedBottom (26), juce::Justification::centredBottom);
}

//==============================================================================
GFunkEditor::GFunkEditor (GFunkProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), screen (*this, p), lcd (*this, p), meter (p), keyboard (p),
      hero (p), overlay (p, keyboard), logo (p)
{
    laf.proc = &proc;
    setLookAndFeel (&laf);
    setSize (windowW, windowH);
    constrainer.setFixedAspectRatio ((double) windowW / (double) windowH);
    constrainer.setSizeLimits (800, 406, 2400, 1218);
    setConstrainer (&constrainer);
    setResizable (true, true);
    loadFavourites();

    addAndMakeVisible (content);
    content.setBounds (0, 0, windowW, windowH);
    content.addAndMakeVisible (background);
    background.setBounds (0, 0, windowW, windowH);

    for (int i = 0; i < numUiItems; ++i)
    {
        const auto& it = uiItems[i];
        const auto& d = params[it.param];
        auto* k = knobs.add (new Knob());

        k->label.setText (d.label, juce::dontSendNotification);
        k->label.setJustificationType (juce::Justification::centred);
        k->label.setFont (sansFont (11.0f));
        k->label.setInterceptsMouseClicks (false, false);
        k->label.setBounds (it.lx, it.ly, it.lw, it.lh);
        content.addAndMakeVisible (k->label);

        if (it.kind == 0)
        {
            auto& s = k->slider;
            s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
            s.setMouseDragSensitivity (220);
            s.getProperties().set ("label", d.label);
            s.getProperties().set ("param", it.param);
            s.setBounds (it.x, it.y, it.w, it.h);
            s.setBufferedToImage (true);
            s.onValueChange = [this, &s] { readoutFor (s); };
            content.addAndMakeVisible (s);
            k->sAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, d.id, s);
            s.addMouseListener (this, false);
        }
        else
        {
            auto& c = k->combo;
            c.addItemList (juce::StringArray::fromTokens (d.choices, "|", ""), 1);
            c.setBounds (it.x, it.y, it.w, it.h);
            c.onChange = [this, k, &d] { lcd.showReadout (juce::String (d.label) + " " + k->combo.getText()); };
            content.addAndMakeVisible (c);
            k->cAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, d.id, c);
        }
    }

    content.addAndMakeVisible (screen);
    screen.setBounds (toRect (screenInnerRect));
    screen.setTab (1);

    static const char* tabNames[] = { "SCOPE", "BROWSER", "SPECTRUM", "LIGHTS" };
    for (int i = 0; i < 4; ++i)
    {
        auto* t = tabs.add (new TabButton (tabNames[i]));
        t->setBounds (tabsRect.x + i * tabsRect.w / 4, tabsRect.y, tabsRect.w / 4, tabsRect.h);
        t->onClick = [this, i] { screen.setTab (i); };
        content.addAndMakeVisible (t);
    }
    tabs[1]->setToggleState (true, juce::dontSendNotification);

    content.addAndMakeVisible (lcd);
    lcd.setBounds (toRect (lcdRect));
    content.addAndMakeVisible (meter);
    meter.setBounds (toRect (meterRect).reduced (4, 2));

    GlyphButton* btns[] = { &prevBtn, &nextBtn, &diceBtn, &starBtn, &skinBtn };
    for (int i = 0; i < 5; ++i)
    {
        btns[i]->setBounds (btnX[i], btnY, 40, btnH);
        content.addAndMakeVisible (btns[i]);
    }
    prevBtn.onClick = [this] { loadPreset (proc.getCurrentProgram() - 1); };
    nextBtn.onClick = [this] { loadPreset (proc.getCurrentProgram() + 1); };
    diceBtn.onClick = [this] { loadPreset (rnd.nextInt (numPresets)); };
    starBtn.onClick = [this] { toggleFavourite (proc.getCurrentProgram()); };
    skinBtn.onClick = [this] { setSkin ((proc.skin.load() + 1) % numSkins); };

    content.addAndMakeVisible (keyboard);
    keyboard.setBounds (toRect (keyboardRect));

    for (auto* w : { &pitchWheel, &modWheel })
    {
        w->setSliderStyle (juce::Slider::LinearVertical);
        w->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        w->setLookAndFeel (&laf);
        w->setMouseDragSensitivity (120);
        content.addAndMakeVisible (w);
    }
    pitchWheel.setRange (-1.0, 1.0, 0.0);
    pitchWheel.setValue (0.0, juce::dontSendNotification);
    pitchWheel.onValueChange = [this] { proc.uiPitchBend.store ((float) pitchWheel.getValue()); };
    pitchWheel.onDragEnd = [this] { pitchWheel.setValue (0.0, juce::sendNotificationSync); };
    modWheel.setRange (0.0, 1.0, 0.0);
    modWheel.onValueChange = [this] { proc.uiModWheel.store ((float) modWheel.getValue()); };
    pitchWheel.setBounds (wheelsRect.x + 4, wheelsRect.y + 6, 34, wheelsRect.h - 12);
    modWheel.setBounds (wheelsRect.x + 46, wheelsRect.y + 6, 34, wheelsRect.h - 12);

    content.addAndMakeVisible (hero);
    hero.setBounds (heroBounds);
    content.addAndMakeVisible (overlay);
    overlay.setBounds (0, 0, windowW, windowH);
    content.addAndMakeVisible (logo);
    logo.setBounds (22, 5, 272, 51);
    logo.onClick = [this] { splash.show(); };
    content.addAndMakeVisible (splash);
    splash.setBounds (0, 0, windowW, windowH);

    // Never keep keyboard focus: in a DAW the host must keep receiving the computer keyboard / MIDI
    // after a click on a preset button, otherwise notes stop reaching the synth until the window is moved.
    std::function<void (juce::Component&)> noFocus = [&noFocus] (juce::Component& c)
    {
        c.setWantsKeyboardFocus (false);
        c.setMouseClickGrabsKeyboardFocus (false);
        for (auto* child : c.getChildren())
            noFocus (*child);
    };
    noFocus (*this);
    keyboard.setWantsKeyboardFocus (false);

    applySkin();
    resized();
    startTimerHz (30);
}

GFunkEditor::~GFunkEditor()
{
    stopTimer();
    pitchWheel.setLookAndFeel (nullptr);
    modWheel.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void GFunkEditor::resized()
{
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) windowW));
}

void GFunkEditor::applySkin()
{
    const int s = juce::jlimit (0, numSkins - 1, proc.skin.load());
    if (s == appliedSkin)
        return;
    appliedSkin = s;
    background.setImage (loadImage (bgData[s], bgSize[s]));
    laf.setSkin (s, loadImage (knobData[s], knobSize[s]));

    const auto& sk = ui::skinDef (s);
    const auto comboText = (sk.comboLine & 0xff000000u) == 0xff000000u ? ui::col (sk.comboLine).brighter (0.5f) : Colour (0xfff2f4f8);
    for (auto* k : knobs)
    {
        k->label.setColour (juce::Label::textColourId, ui::col (sk.label));
        k->combo.setColour (juce::ComboBox::textColourId, comboText);
        k->combo.setColour (juce::ComboBox::arrowColourId, comboText);
        k->label.repaint();
        k->slider.repaint();
        k->combo.repaint();
    }
    keyboard.repaint();
}

void GFunkEditor::setSkin (int index)
{
    proc.skin.store (juce::jlimit (0, numSkins - 1, index));
    applySkin();
    lcd.showReadout ("SKIN  " + juce::String (ui::skinDef (proc.skin.load()).name));
}

void GFunkEditor::timerCallback()
{
    const int cur = proc.getCurrentProgram();
    if (cur != lastProgram)
    {
        lastProgram = cur;
        screen.revealPreset (cur);
        screen.repaint();
        lcd.repaint();
        starBtn.lit = isFavourite (cur);
        starBtn.repaint();
    }
    applySkin();

    ++frame;
    const int theme = proc.blingTheme.load();
    if (theme != lastTheme || frame % 15 == 0)
    {
        lastTheme = theme;
        for (auto* tb : tabs) { tb->theme = theme; tb->time = (float) nowSeconds(); tb->repaint(); }
        for (auto* gb : { &prevBtn, &nextBtn, &diceBtn, &starBtn, &skinBtn }) gb->theme = theme;
    }

    uint64_t lit = 0;
    for (int n = 36; n <= 96; ++n)
        if (proc.isNoteLit (n)) lit ^= (1ull << (n - 36));
    if (lit != lastLit)
    {
        lastLit = lit;
        keyboard.repaint();
    }
    if (proc.blingLevel.load() > 0 && lit != 0 && frame % 3 == 0)
        keyboard.repaint();

    const bool busy = juce::jmax (proc.funk.load(), proc.hit.load()) > 0.03f || lit != 0 || overlay.isAnimating();
    if (busy || frame % 30 == 0)
    {
        overlay.advance();
        overlay.markDirty();
    }
    if (proc.getOutputLevel() > 0.0005f || frame % 10 == 0)
        meter.repaint();
    if (frame % 15 == 0)
        lcd.repaint();
}

void GFunkEditor::loadPreset (int index)
{
    index = (index % numPresets + numPresets) % numPresets;
    proc.setCurrentProgram (index);
    screen.revealPreset (index);
    screen.repaint();
    lcd.repaint();
}

void GFunkEditor::toggleFavourite (int index)
{
    if (! favourites.erase (index))
        favourites.insert (index);
    saveFavourites();
    starBtn.lit = isFavourite (proc.getCurrentProgram());
    starBtn.repaint();
    screen.repaint();
}

static juce::File favouritesFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        #ifdef GFUNK_FREE
        .getChildFile ("HUMPIRE").getChildFile ("GFUNK2027Free").getChildFile ("favorites.txt");
#else
        .getChildFile ("HUMPIRE").getChildFile ("GFUNK2027").getChildFile ("favorites.txt");
#endif
}

void GFunkEditor::loadFavourites()
{
    favourites.clear();
    const auto f = favouritesFile();
    if (! f.existsAsFile()) return;
    for (const auto& tok : juce::StringArray::fromTokens (f.loadFileAsString(), ", \n", ""))
        if (tok.containsOnly ("0123456789") && tok.isNotEmpty())
            favourites.insert (juce::jlimit (0, numPresets - 1, tok.getIntValue()));
}

void GFunkEditor::saveFavourites() const
{
    juce::StringArray lines;
    for (int i : favourites) lines.add (juce::String (i));
    const auto f = favouritesFile();
    f.getParentDirectory().createDirectory();
    f.replaceWithText (lines.joinIntoString (","));
}

void GFunkEditor::readoutFor (juce::Slider& s)
{
    lcd.showReadout (s.getProperties()["label"].toString() + "  " + s.getTextFromValue (s.getValue()));
}

void GFunkEditor::mouseEnter (const juce::MouseEvent& e)
{
    if (auto* s = dynamic_cast<juce::Slider*> (e.eventComponent))
        if (s->getProperties().contains ("label"))
            readoutFor (*s);
}

void GFunkEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (auto* s = dynamic_cast<juce::Slider*> (e.eventComponent))
        if (s->getProperties().contains ("label"))
            readoutFor (*s);
}
