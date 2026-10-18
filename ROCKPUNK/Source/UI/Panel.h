#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "../PluginProcessor.h"
#include "Art.h"
#include "Palette.h"

#ifndef ROCKPUNK_VERSION_STRING
 #define ROCKPUNK_VERSION_STRING "1.1.0"
#endif

namespace rk
{

class Panel;

//==============================================================================
// Binds one parameter to a control: keeps the normalised value current whoever
// moves it (host automation, a preset, the user) and wraps edits in gestures so
// that hosts can record them.
//==============================================================================
class ParamBinder
{
public:
    ParamBinder (juce::AudioProcessorValueTreeState& state, const juce::String& id, std::function<void()> changed)
        : param (*state.getParameter (id)),
          attachment (param, [this, changed] (float v)
                      {
                          norm = param.convertTo0to1 (v);
                          if (changed)
                              changed();
                      })
    {
        norm = param.getValue();
        attachment.sendInitialUpdate();
    }

    float get01() const noexcept { return norm; }
    bool getBool() const noexcept { return norm > 0.5f; }
    int index (int numChoices) const noexcept
    {
        return juce::jlimit (0, juce::jmax (0, numChoices - 1), juce::roundToInt (norm * (float) (numChoices - 1)));
    }
    float defaultNorm() const { return param.getDefaultValue(); }
    juce::String text() const { return param.getText (norm, 0); }

    void beginGesture()                  { attachment.beginGesture(); }
    void endGesture()                    { attachment.endGesture(); }
    void setPartOfGesture (float n01)    { attachment.setValueAsPartOfGesture (param.convertFrom0to1 (juce::jlimit (0.0f, 1.0f, n01))); }
    void setComplete (float n01)         { attachment.setValueAsCompleteGesture (param.convertFrom0to1 (juce::jlimit (0.0f, 1.0f, n01))); }

    juce::RangedAudioParameter& param;

private:
    float norm = 0.0f;
    juce::ParameterAttachment attachment;
};

//==============================================================================
// A thing on the faceplate that draws itself and answers the mouse. Everything
// is expressed in mockup pixels, so the zone that reacts is the zone that is
// drawn, by construction.
//==============================================================================
class Control
{
public:
    explicit Control (Panel& p) : panel (p) {}
    virtual ~Control() = default;

    virtual bool hit (juce::Point<float> img) const = 0;
    virtual juce::Rectangle<float> footprint() const = 0;
    virtual void paint (juce::Graphics&, const Art&) {}

    virtual void mouseDown (juce::Point<float> /*img*/, const juce::MouseEvent&) {}
    virtual void mouseDrag (const juce::MouseEvent&) {}
    virtual void mouseUp (const juce::MouseEvent&) {}
    virtual void mouseWheel (float /*deltaY*/) {}
    virtual void mouseDouble() {}

    virtual juce::String tooltip() const { return {}; }
    virtual juce::String bubbleText() const { return {}; }
    virtual juce::Rectangle<float> bubbleAnchor() const { return footprint(); }
    virtual juce::MouseCursor::StandardCursorType cursor() const { return juce::MouseCursor::PointingHandCursor; }

    void changed();

    bool hovered = false;
    bool dragging = false;

protected:
    Panel& panel;
};

//==============================================================================
class Panel final : public juce::Component,
                    public juce::TooltipClient,
                    private juce::Timer
{
public:
    Panel (PluginProcessor&, const Art&);
    ~Panel() override;

    PluginProcessor& proc;
    const Art& art;
    juce::AudioProcessorValueTreeState& apvts;

    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    juce::String getTooltip() override;

    void repaintImage (juce::Rectangle<float> imageRect);
    juce::Point<float> toImage (juce::Point<float> local) const noexcept { return { local.x / Art::kx, local.y / Art::ky }; }
    juce::Rectangle<int> imageToScreen (juce::Rectangle<float> r);

    // actions shared by several controls
    void showPresetMenu (juce::Rectangle<float> anchor);
    void showCassetteMenu (juce::Rectangle<float> anchor);
    void savePresetDialog();
    void openPresetDialog();
    void setParam (const juce::String& id, float denormalised);
    void releaseAllNotes();
    void styleMenu (juce::PopupMenu& m) { m.setLookAndFeel (&getLookAndFeel()); }

    // keyboard (the panel keeps the pointer so that the editor can let go of held keys)
    Control* keyboard = nullptr;

private:
    void timerCallback() override;
    Control* controlAt (juce::Point<float> img) const;
    void setHover (Control*);
    void updateBubble();

    std::vector<std::unique_ptr<Control>> controls;
    Control* hot = nullptr;
    Control* active = nullptr;

    juce::Rectangle<float> bubbleRect;       // image pixels, empty when hidden
    juce::String bubbleString;

    std::set<int> lastNotes;
    juce::String lastPresetName;
    std::unique_ptr<juce::FileChooser> chooser;

    friend class Control;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Panel)
};

inline void Control::changed() { panel.repaintImage (footprint()); }

//==============================================================================
// Rotary knob: the knurled cap of the mockup, turned. 270 degrees, 7:30 to 4:30.
//==============================================================================
class KnobControl final : public Control
{
public:
    KnobControl (Panel& p, const Art::Knob& k, juce::String tip, std::function<void()> userChange = {})
        : Control (p), geo (k), tipText (std::move (tip)), onUserChange (std::move (userChange)),
          binder (p.apvts, k.param, [this] { changed(); })
    {
    }

    bool hit (juce::Point<float> p) const override { return p.getDistanceFrom ({ geo.x, geo.y }) <= 62.0f; }
    juce::Rectangle<float> footprint() const override { return { geo.x - 74.0f, geo.y - 74.0f, 148.0f, 148.0f }; }
    juce::MouseCursor::StandardCursorType cursor() const override { return juce::MouseCursor::UpDownResizeCursor; }
    juce::String tooltip() const override { return tipText; }
    juce::String bubbleText() const override { return binder.text(); }
    juce::Rectangle<float> bubbleAnchor() const override { return { geo.x - 40.0f, geo.y - 104.0f, 80.0f, 10.0f }; }

    void paint (juce::Graphics& g, const Art& art) override
    {
        const float angle = (binder.get01() - 0.5f) * juce::degreesToRadians (270.0f);
        art.draw (g, art.sprite ("knobCap"), geo.x, geo.y, angle);

        if (hovered || dragging)
        {
            g.setColour (Palette::pinkHot().withAlpha (dragging ? 0.55f : 0.32f));
            g.drawEllipse (geo.x - 63.0f, geo.y - 63.0f, 126.0f, 126.0f, 5.0f);
        }
    }

    void mouseDown (juce::Point<float>, const juce::MouseEvent&) override
    {
        startNorm = binder.get01();
        binder.beginGesture();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const float pixels = (float) e.getDistanceFromDragStartY();       // design pixels, up is negative
        const float full = e.mods.isShiftDown() ? 1100.0f : 200.0f;
        binder.setPartOfGesture (startNorm - pixels / full);
        if (onUserChange)
            onUserChange();
    }

    void mouseUp (const juce::MouseEvent&) override { binder.endGesture(); }

    void mouseWheel (float d) override
    {
        binder.beginGesture();
        binder.setPartOfGesture (binder.get01() + d * 0.15f);
        binder.endGesture();
        if (onUserChange)
            onUserChange();
    }

    void mouseDouble() override { binder.setComplete (binder.defaultNorm()); }

    const Art::Knob geo;

private:
    juce::String tipText;
    std::function<void()> onUserChange;
    ParamBinder binder;
    float startNorm = 0.0f;
};

//==============================================================================
// Vertical fader: thumb and value-dependent rail drawn from the mockup's own
// pixels (see Tools/make_ui_assets.py).
//==============================================================================
class FaderControl final : public Control
{
public:
    FaderControl (Panel& p, const Art::Fader& f, juce::String tip)
        : Control (p), geo (f), tipText (std::move (tip)),
          binder (p.apvts, f.param, [this] { changed(); })
    {
    }

    float thumbY() const noexcept { return geo.maxY - binder.get01() * (geo.maxY - geo.minY); }
    float halfThumb() const noexcept { return 29.0f; }

    bool hit (juce::Point<float> p) const override
    {
        return std::abs (p.x - geo.cx) <= 33.0f && p.y >= geo.slotTop - 8.0f && p.y <= geo.slotBottom + 8.0f;
    }

    juce::Rectangle<float> footprint() const override
    {
        return { geo.cx - 44.0f, geo.slotTop - 18.0f, 120.0f, geo.slotBottom - geo.slotTop + 60.0f };
    }

    juce::MouseCursor::StandardCursorType cursor() const override { return juce::MouseCursor::UpDownResizeCursor; }
    juce::String tooltip() const override { return tipText; }
    juce::String bubbleText() const override { return binder.text(); }
    juce::Rectangle<float> bubbleAnchor() const override { return { geo.cx - 40.0f, thumbY() - 84.0f, 80.0f, 10.0f }; }

    void paint (juce::Graphics& g, const Art& art) override
    {
        const auto& row = art.sprite ("railRow_" + geo.group);
        const auto& cap = art.sprite ("railCap_" + geo.group);
        const float y = thumbY();

        // pink outline from the thumb down to the end of the rail
        if (row.image.isValid() && cap.image.isValid())
        {
            const int left = (int) std::floor (geo.cx - 14.0f);
            const int capTop = (int) std::lround (geo.slotBottom + 1.0f) - cap.image.getHeight();
            const int top = (int) std::lround (y);
            if (top < capTop)
                g.drawImage (row.image, left, top, row.image.getWidth(), capTop - top,
                             0, 0, row.image.getWidth(), row.image.getHeight());
            g.drawImageAt (cap.image, left, capTop);
        }

        art.draw (g, art.sprite ("thumb_" + geo.group), geo.cx, y);

        if (hovered || dragging)
        {
            g.setColour (Palette::pinkHot().withAlpha (dragging ? 0.28f : 0.14f));
            g.fillRoundedRectangle (geo.cx - 32.0f, y - halfThumb(), 63.0f, 2.0f * halfThumb(), 7.0f);
        }
    }

    void mouseDown (juce::Point<float> img, const juce::MouseEvent&) override
    {
        binder.beginGesture();
        if (std::abs (img.y - thumbY()) > halfThumb())
            binder.setPartOfGesture ((geo.maxY - img.y) / (geo.maxY - geo.minY));     // jump to the click
        startNorm = binder.get01();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const float travel = (geo.maxY - geo.minY) * Art::ky;                   // design pixels
        const float scale = e.mods.isShiftDown() ? 0.2f : 1.0f;
        binder.setPartOfGesture (startNorm - scale * (float) e.getDistanceFromDragStartY() / travel);
    }

    void mouseUp (const juce::MouseEvent&) override { binder.endGesture(); }

    void mouseWheel (float d) override
    {
        binder.beginGesture();
        binder.setPartOfGesture (binder.get01() + d * 0.1f);
        binder.endGesture();
    }

    void mouseDouble() override { binder.setComplete (binder.defaultNorm()); }

    const Art::Fader geo;

private:
    juce::String tipText;
    ParamBinder binder;
    float startNorm = 0.0f;
};

//==============================================================================
// Pitch and modulation wheels. The ridges stay where they are; the block that
// the mockup shows at the middle slides with the value.
//==============================================================================
class WheelControl final : public Control
{
public:
    WheelControl (Panel& p, const Art::Wheel& w, juce::String tip, bool springBack)
        : Control (p), geo (w), tipText (std::move (tip)), spring (springBack),
          binder (p.apvts, w.param, [this] { changed(); })
    {
    }

    float blockY() const noexcept { return geo.maxY - binder.get01() * (geo.maxY - geo.minY); }

    bool hit (juce::Point<float> p) const override
    {
        return p.x >= geo.x0 && p.x <= geo.x1 && p.y >= geo.minY - 50.0f && p.y <= geo.maxY + 50.0f;
    }

    juce::Rectangle<float> footprint() const override
    {
        return { geo.x0 - 4.0f, geo.minY - 56.0f, geo.x1 - geo.x0 + 8.0f, geo.maxY - geo.minY + 112.0f };
    }

    juce::MouseCursor::StandardCursorType cursor() const override { return juce::MouseCursor::UpDownResizeCursor; }
    juce::String tooltip() const override { return tipText; }
    juce::String bubbleText() const override { return binder.text(); }
    juce::Rectangle<float> bubbleAnchor() const override { return { geo.x - 40.0f, blockY() - 80.0f, 80.0f, 10.0f }; }

    void paint (juce::Graphics& g, const Art& art) override
    {
        const auto& block = art.sprite ("block_" + geo.id);
        const float y = blockY();
        art.draw (g, block, geo.x, y);

        if (hovered || dragging)
        {
            g.setColour (Palette::pinkHot().withAlpha (dragging ? 0.26f : 0.13f));
            g.fillRoundedRectangle (geo.x - 27.5f, y - 25.5f, 55.0f, 51.0f, 7.0f);
        }
    }

    void mouseDown (juce::Point<float> img, const juce::MouseEvent&) override
    {
        binder.beginGesture();
        if (std::abs (img.y - blockY()) > 26.0f)
            binder.setPartOfGesture ((geo.maxY - img.y) / (geo.maxY - geo.minY));
        startNorm = binder.get01();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const float travel = (geo.maxY - geo.minY) * Art::ky;
        binder.setPartOfGesture (startNorm - (float) e.getDistanceFromDragStartY() / travel);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (spring)
            binder.setPartOfGesture (binder.defaultNorm());       // a real pitch wheel springs back
        binder.endGesture();
    }

    void mouseWheel (float d) override
    {
        if (spring)
            return;
        binder.beginGesture();
        binder.setPartOfGesture (binder.get01() + d * 0.1f);
        binder.endGesture();
    }

    void mouseDouble() override { binder.setComplete (binder.defaultNorm()); }

    const Art::Wheel geo;

private:
    juce::String tipText;
    bool spring = false;
    ParamBinder binder;
    float startNorm = 0.0f;
};

//==============================================================================
// Drop-down: the box is on the faceplate, the text in it is drawn here.
//==============================================================================
class DropBoxControl final : public Control
{
public:
    DropBoxControl (Panel& p, const Art::Combo& c, juce::StringArray itemNames, juce::String captionText, juce::String tip)
        : Control (p), geo (c), items (std::move (itemNames)), caption (std::move (captionText)), tipText (std::move (tip)),
          binder (p.apvts, c.param, [this] { changed(); })
    {
    }

    bool hit (juce::Point<float> p) const override { return geo.box.expanded (6.0f, 4.0f).contains (p); }
    juce::Rectangle<float> footprint() const override { return geo.box.expanded (8.0f, 4.0f).withBottom (geo.box.getBottom() + 52.0f); }
    juce::String tooltip() const override { return tipText; }

    void paint (juce::Graphics& g, const Art&) override
    {
        const auto face = geo.face.reduced (4.0f, 3.0f);
        if (hovered)
        {
            g.setColour (juce::Colours::white.withAlpha (0.07f));
            g.fillRoundedRectangle (face, 12.0f);
        }

        // value, in the condensed capitals the faceplate is printed in
        const auto font = juce::Font (juce::FontOptions (36.0f, juce::Font::bold)).withHorizontalScale (0.84f);
        g.setFont (font);
        g.setColour (hovered ? juce::Colours::white : Palette::label());
        g.drawText (items[binder.index (items.size())].toUpperCase(),
                    juce::Rectangle<float> (geo.face.getX() + 14.0f, geo.face.getY(), geo.face.getWidth() - 76.0f, geo.face.getHeight()),
                    juce::Justification::centredLeft, false);

        // what the box controls, under it
        g.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)).withHorizontalScale (0.9f));
        g.setColour (Palette::labelDim());
        g.drawText (caption, juce::Rectangle<float> (geo.box.getX(), geo.box.getBottom() + 4.0f, geo.box.getWidth(), 34.0f),
                    juce::Justification::centred, false);
    }

    void mouseDown (juce::Point<float>, const juce::MouseEvent&) override
    {
        juce::PopupMenu menu;
        const int current = binder.index (items.size());
        for (int i = 0; i < items.size(); ++i)
            menu.addItem (i + 1, items[i], true, i == current);

        juce::Component::SafePointer<Panel> alive (&panel);
        panel.styleMenu (menu);
        menu.showMenuAsync (juce::PopupMenu::Options()
                                .withTargetScreenArea (panel.imageToScreen (geo.box))
                                .withMinimumWidth (juce::roundToInt (geo.box.getWidth() * Art::kx * panel.getApproximateScaleFactorForComponent (&panel)))
                                .withStandardItemHeight (26),
                            [alive, this] (int result)
                            {
                                if (alive != nullptr && result > 0 && items.size() > 1)
                                    binder.setComplete ((float) (result - 1) / (float) (items.size() - 1));
                            });
    }

    const Art::Combo geo;

private:
    juce::StringArray items;
    juce::String caption, tipText;
    ParamBinder binder;
};

//==============================================================================
// Speaker grill: switches a string layer on and off.
//==============================================================================
class GrillControl final : public Control
{
public:
    GrillControl (Panel& p, juce::Rectangle<float> area, const juce::String& param, juce::String tip)
        : Control (p), zone (area), tipText (std::move (tip)), binder (p.apvts, param, [this] { changed(); })
    {
    }

    bool hit (juce::Point<float> p) const override { return zone.contains (p); }
    juce::Rectangle<float> footprint() const override { return zone.expanded (14.0f); }
    juce::String tooltip() const override { return tipText; }

    void paint (juce::Graphics& g, const Art&) override
    {
        const bool on = binder.getBool();
        const auto grill = zone.reduced (6.0f);

        if (! on)
        {
            g.setColour (juce::Colours::black.withAlpha (0.62f));
            g.fillRoundedRectangle (grill, 14.0f);
        }
        else if (hovered)
        {
            g.setColour (Palette::pinkHot().withAlpha (0.10f));
            g.fillRoundedRectangle (grill, 14.0f);
        }

        // pilot light
        const juce::Point<float> led (zone.getCentreX(), zone.getY() + 36.0f);
        if (on)
        {
            g.setColour (Palette::pinkHot().withAlpha (0.28f));
            g.fillEllipse (led.x - 17.0f, led.y - 17.0f, 34.0f, 34.0f);
            g.setColour (Palette::pinkHot());
        }
        else
        {
            g.setColour (juce::Colour (0xFF3A2430));
        }
        g.fillEllipse (led.x - 8.0f, led.y - 8.0f, 16.0f, 16.0f);

        if (hovered)
        {
            g.setColour (Palette::pinkHot().withAlpha (0.45f));
            g.drawRoundedRectangle (zone.reduced (2.0f), 16.0f, 4.0f);
        }
    }

    void mouseDown (juce::Point<float>, const juce::MouseEvent&) override
    {
        binder.setComplete (binder.getBool() ? 0.0f : 1.0f);
    }

private:
    juce::Rectangle<float> zone;
    juce::String tipText;
    ParamBinder binder;
};

//==============================================================================
// A clickable zone with a hover light: the cassette, the folder, the arrows...
//==============================================================================
class ZoneControl : public Control
{
public:
    ZoneControl (Panel& p, juce::Rectangle<float> area, float corner, juce::String tip, std::function<void()> action)
        : Control (p), zone (area), radius (corner), tipText (std::move (tip)), onClick (std::move (action))
    {
    }

    bool hit (juce::Point<float> p) const override { return zone.contains (p); }
    juce::Rectangle<float> footprint() const override { return zone.expanded (10.0f); }
    juce::String tooltip() const override { return tipText; }

    void paint (juce::Graphics& g, const Art&) override
    {
        if (dragging)
        {
            g.setColour (Palette::pinkHot().withAlpha (0.30f));
            g.fillRoundedRectangle (zone, radius);
        }
        else if (hovered)
        {
            g.setColour (Palette::pinkHot().withAlpha (0.16f));
            g.fillRoundedRectangle (zone, radius);
            g.setColour (Palette::pinkHot().withAlpha (0.55f));
            g.drawRoundedRectangle (zone.reduced (2.0f), radius, 3.0f);
        }
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (onClick && zone.contains (panel.toImage (e.position)))
            onClick();
    }

protected:
    juce::Rectangle<float> zone;
    float radius;
    juce::String tipText;
    std::function<void()> onClick;
};

//==============================================================================
// The preset name, in the display of the faceplate.
//==============================================================================
class PresetNameControl final : public ZoneControl
{
public:
    PresetNameControl (Panel& p, juce::Rectangle<float> area, std::function<void()> action)
        : ZoneControl (p, area, 12.0f, "Click to choose a preset", std::move (action))
    {
    }

    void paint (juce::Graphics& g, const Art&) override
    {
        if (hovered)
        {
            g.setColour (Palette::pinkHot().withAlpha (0.10f));
            g.fillRoundedRectangle (zone, radius);
        }

        g.setFont (juce::Font (juce::FontOptions (46.0f)));
        g.setColour (Palette::pink());
        g.drawFittedText (panel.proc.getCurrentPresetName(), zone.reduced (6.0f, 0.0f).toNearestInt(),
                          juce::Justification::centred, 1, 0.7f);
    }
};

//==============================================================================
// The keyboard drawn in the mockup: its key boundaries were measured on the
// picture, so a key that lights is the key that is printed. 22 white keys,
// C2 to C5.
//==============================================================================
class KeyboardControl final : public Control
{
public:
    KeyboardControl (Panel& p, const Art& a) : Control (p), art (a)
    {
        const auto& k = art.keys;
        bounds.push_back (k.left);
        for (float s : k.separators)
            bounds.push_back (s);
        bounds.push_back (k.right);
    }

    int numWhite() const noexcept { return (int) bounds.size() - 1; }

    int whiteNote (int i) const noexcept
    {
        static const int offs[] = { 0, 2, 4, 5, 7, 9, 11 };
        return art.keys.lowNote + 12 * (i / 7) + offs[i % 7];
    }

    int blackNote (int j) const noexcept
    {
        static const int offs[] = { 1, 3, 6, 8, 10 };
        return art.keys.lowNote + 12 * (j / 5) + offs[j % 5];
    }

    juce::Rectangle<float> whiteRect (int i) const noexcept
    {
        return { bounds[(size_t) i], art.keys.top, bounds[(size_t) i + 1] - bounds[(size_t) i], art.keys.whiteBottom - art.keys.top };
    }

    juce::Rectangle<float> blackRect (int j) const noexcept
    {
        const auto& b = art.keys.black[(size_t) j];
        return { b.x0, art.keys.top, b.x1 - b.x0, art.keys.blackBottom - art.keys.top + 3.0f };
    }

    bool hit (juce::Point<float> p) const override
    {
        return p.x >= art.keys.left && p.x <= art.keys.right && p.y >= art.keys.top && p.y <= art.keys.whiteBottom;
    }

    juce::Rectangle<float> footprint() const override
    {
        return { art.keys.left - 6.0f, art.keys.top - 4.0f, art.keys.right - art.keys.left + 12.0f,
                 art.keys.whiteBottom - art.keys.top + 8.0f };
    }

    juce::MouseCursor::StandardCursorType cursor() const override { return juce::MouseCursor::PointingHandCursor; }
    juce::String tooltip() const override { return "On-screen keyboard (C2 to C5) - click and slide"; }

    int noteAt (juce::Point<float> p) const noexcept
    {
        if (! hit (p))
            return -1;

        for (int j = 0; j < (int) art.keys.black.size(); ++j)
            if (blackRect (j).contains (p))
                return blackNote (j);

        for (int i = 0; i < numWhite(); ++i)
            if (p.x >= bounds[(size_t) i] && p.x < bounds[(size_t) i + 1])
                return whiteNote (i);

        return -1;
    }

    void setNotesDown (const std::set<int>& notes)
    {
        if (notes != external)
        {
            external = notes;
            changed();
        }
    }

    void paint (juce::Graphics& g, const Art&) override
    {
        auto isDown = [this] (int note) { return external.count (note) > 0 || note == mouseNote; };

        for (int i = 0; i < numWhite(); ++i)
        {
            const int note = whiteNote (i);
            const bool down = isDown (note);
            const bool over = note == hoverNote && ! down;
            if (! down && ! over)
                continue;

            const auto r = whiteRect (i).reduced (4.0f, 0.0f);
            juce::Graphics::ScopedSaveState state (g);
            for (int j = 0; j < (int) art.keys.black.size(); ++j)
                if (blackRect (j).intersects (r))
                    g.excludeClipRegion (blackRect (j).getSmallestIntegerContainer());

            if (down)
            {
                g.setGradientFill (juce::ColourGradient (Palette::pinkHot().withAlpha (0.22f), 0.0f, r.getY(),
                                                         Palette::pinkHot().withAlpha (0.62f), 0.0f, r.getBottom(), false));
                g.fillRoundedRectangle (r, 8.0f);
            }
            else
            {
                g.setColour (juce::Colours::white.withAlpha (0.07f));
                g.fillRoundedRectangle (r, 8.0f);
            }
        }

        for (int j = 0; j < (int) art.keys.black.size(); ++j)
        {
            const int note = blackNote (j);
            const bool down = isDown (note);
            const bool over = note == hoverNote && ! down;
            if (! down && ! over)
                continue;

            const auto r = blackRect (j).reduced (6.0f, 3.0f).withTrimmedBottom (4.0f);
            g.setColour (down ? Palette::pinkHot().withAlpha (0.60f) : juce::Colours::white.withAlpha (0.10f));
            g.fillRoundedRectangle (r, 5.0f);
        }
    }

    void mouseMoveTo (juce::Point<float> img)
    {
        const int n = noteAt (img);
        if (n != hoverNote)
        {
            hoverNote = n;
            changed();
        }
    }

    void mouseDown (juce::Point<float> img, const juce::MouseEvent&) override { press (noteAt (img), img); }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const auto img = panel.toImage (e.position);
        const int n = noteAt (img);
        if (n != mouseNote && n >= 0)
            press (n, img);
    }

    void mouseUp (const juce::MouseEvent&) override { release(); }

    void release()
    {
        if (mouseNote >= 0)
        {
            panel.proc.getKeyboardState().noteOff (1, mouseNote, 0.0f);
            mouseNote = -1;
            changed();
        }
    }

    void clearHover()
    {
        if (hoverNote >= 0)
        {
            hoverNote = -1;
            changed();
        }
    }

private:
    void press (int note, juce::Point<float> img)
    {
        if (note < 0)
            return;

        release();
        const float velocity = juce::jlimit (0.3f, 1.0f, juce::jmap (img.y, art.keys.top, art.keys.whiteBottom, 0.4f, 1.0f));
        mouseNote = note;
        panel.proc.getKeyboardState().noteOn (1, note, velocity);
        changed();
    }

    const Art& art;
    std::vector<float> bounds;
    std::set<int> external;
    int mouseNote = -1;
    int hoverNote = -1;
};

//==============================================================================
// Panel
//==============================================================================
inline Panel::Panel (PluginProcessor& p, const Art& artwork)
    : proc (p), art (artwork), apvts (p.apvts)
{
    setOpaque (false);
    setSize ((int) Art::designW, (int) Art::designH);

    auto tipFor = [] (const juce::String& id) -> juce::String
    {
        static const std::map<juce::String, juce::String> tips =
        {
            { "lfoRate",   "RATE - speed of the LFO" },
            { "lfoAmount", "AMOUNT - depth of the LFO on the DESTINATION chosen below" },
            { "lfoPitch",  "PITCH - vibrato depth, whatever the destination" },
            { "glide",     "GLIDE - time taken to slide from one note to the next" },
            { "filter",    "FILTER - cut-off frequency (turning it switches the filter on if FILTER TYPE is off)" },
            { "vol",       "VOLUME - master level" },
            { "reverb",    "REVERB - amount of reverb" },
            { "chorus",    "CHORUS - amount of chorus (stereo widening)" },
            { "dist",      "DISTORTION - amount of drive after the amp" },
            { "time",      "TIME - delay time (turning it switches the delay on)" },
            { "fb",        "FEEDBACK - number of delay repeats (turning it switches the delay on)" },
            { "fA",        "Filter envelope - ATTACK" },
            { "fS",        "Filter envelope - SUSTAIN" },
            { "fD",        "Filter envelope - DECAY" },
            { "fR",        "Filter envelope - RELEASE" },
            { "fVel",      "VELOCITY - how much the playing dynamics shape each note (0 = every note equal)" },
            { "attack",    "Volume envelope - ATTACK" },
            { "decay",     "Volume envelope - DECAY" },
            { "sustain",   "Volume envelope - SUSTAIN" },
            { "release",   "Volume envelope - RELEASE" },
            { "pitchWheel", "PITCH BEND - plus or minus two semitones, springs back to the middle" },
            { "modWheel",   "MODULATION - opens the vibrato (also follows MIDI CC1)" },
        };
        const auto it = tips.find (id);
        return it != tips.end() ? it->second : juce::String();
    };

    // ---- knobs ----
    for (const auto& k : art.knobs)
    {
        std::function<void()> touched;

        // TIME and FEEDBACK have nothing to act on while the delay is off, and the
        // faceplate has no switch for it: turning them switches it on.
        if (k.id == "time" || k.id == "fb")
            touched = [this]
            {
                if (apvts.getRawParameterValue ("delayOn")->load() < 0.5f)
                    setParam ("delayOn", 1.0f);
            };

        // Same for FILTER while the filter type is "Off".
        if (k.id == "filter")
            touched = [this]
            {
                if (apvts.getRawParameterValue ("fltType")->load() > 2.5f)
                    setParam ("fltType", 0.0f);
            };

        controls.push_back (std::make_unique<KnobControl> (*this, k, tipFor (k.id), touched));
    }

    for (const auto& f : art.faders)
        controls.push_back (std::make_unique<FaderControl> (*this, f, tipFor (f.id)));

    for (const auto& w : art.wheels)
        controls.push_back (std::make_unique<WheelControl> (*this, w, tipFor (w.id), w.id == "pitchWheel"));

    // ---- drop-downs ----
    struct ComboInfo { const char* id; juce::StringArray items; const char* caption; const char* tip; };
    const ComboInfo infos[] =
    {
        { "wf",         names::pick(),      "WAVEFORM",    "WAVEFORM - how hard the pick is: soft, round, bright or sharp" },
        { "lfoSrc",     names::lfoWave(),   "LFO SOURCE",  "LFO SOURCE - shape of the LFO" },
        { "dest",       names::lfoDest(),   "DESTINATION", "DESTINATION - what the LFO moves: pitch, filter, volume or drive" },
        { "filterType", names::filter(),    "FILTER TYPE", "FILTER TYPE - low-pass, band-pass, high-pass or off" },
        { "monoPoly",   names::voiceMode(), "MONO/POLY",   "MONO/POLY - polyphonic, or one note at a time (glide joins the notes)" },
    };
    for (const auto& c : art.combos)
        for (const auto& i : infos)
            if (c.id == i.id)
                controls.push_back (std::make_unique<DropBoxControl> (*this, c, i.items, i.caption, i.tip));

    // ---- speaker grills ----
    controls.push_back (std::make_unique<GrillControl> (*this, art.grillA, "laOn",
                                                        "STRING LAYER A - click to switch it on or off"));
    controls.push_back (std::make_unique<GrillControl> (*this, art.grillB, "lbOn",
                                                        "STRING LAYER B - click to switch it on or off"));

    // ---- preset display ----
    controls.push_back (std::make_unique<ZoneControl> (*this, art.barFolder, 14.0f, "Presets",
                                                       [this] { showPresetMenu (art.barFolder); }));
    controls.push_back (std::make_unique<ZoneControl> (*this, art.barPrev, 14.0f, "Previous preset",
                                                       [this] { proc.stepPreset (-1); repaintImage (art.barField); }));
    controls.push_back (std::make_unique<ZoneControl> (*this, art.barNext, 14.0f, "Next preset",
                                                       [this] { proc.stepPreset (1); repaintImage (art.barField); }));
    controls.push_back (std::make_unique<ZoneControl> (*this, art.barSave, 14.0f, "Save the current sound as a preset",
                                                       [this] { savePresetDialog(); }));
    controls.push_back (std::make_unique<PresetNameControl> (*this, art.barText, [this] { showPresetMenu (art.barField); }));

    // ---- cassette ----
    controls.push_back (std::make_unique<ZoneControl> (*this, art.cassette, 22.0f,
                                                       "EXPANSION - presets, init, your own samples, delay options",
                                                       [this] { showCassetteMenu (art.cassette); }));

    auto kb = std::make_unique<KeyboardControl> (*this, art);
    keyboard = kb.get();
    controls.push_back (std::move (kb));

    startTimerHz (30);
}

inline Panel::~Panel()
{
    stopTimer();
    releaseAllNotes();
}

inline void Panel::releaseAllNotes()
{
    if (auto* k = dynamic_cast<KeyboardControl*> (keyboard))
        k->release();
}

inline void Panel::repaintImage (juce::Rectangle<float> r)
{
    repaint (juce::Rectangle<float> (r.getX() * Art::kx, r.getY() * Art::ky, r.getWidth() * Art::kx, r.getHeight() * Art::ky)
                 .getSmallestIntegerContainer().expanded (2));
}

inline juce::Rectangle<int> Panel::imageToScreen (juce::Rectangle<float> r)
{
    // Corner by corner, so that the pop-up is anchored on the zone itself and not
    // on a zone rounded outwards in design pixels (up to one scaled pixel a side).
    const auto topLeft = localPointToGlobal (juce::Point<float> (r.getX() * Art::kx, r.getY() * Art::ky));
    const auto bottomRight = localPointToGlobal (juce::Point<float> (r.getRight() * Art::kx, r.getBottom() * Art::ky));
    return juce::Rectangle<int>::leftTopRightBottom (juce::roundToInt (topLeft.x), juce::roundToInt (topLeft.y),
                                                     juce::roundToInt (bottomRight.x), juce::roundToInt (bottomRight.y));
}

inline Control* Panel::controlAt (juce::Point<float> img) const
{
    for (auto it = controls.rbegin(); it != controls.rend(); ++it)
        if ((*it)->hit (img))
            return it->get();
    return nullptr;
}

inline void Panel::setHover (Control* c)
{
    if (c == hot)
        return;

    if (hot != nullptr)
    {
        hot->hovered = false;
        hot->changed();
        if (auto* k = dynamic_cast<KeyboardControl*> (hot))
            k->clearHover();
    }

    hot = c;

    if (hot != nullptr)
    {
        hot->hovered = true;
        hot->changed();
        setMouseCursor (juce::MouseCursor (hot->cursor()));
    }
    else
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }

    updateBubble();
}

inline void Panel::updateBubble()
{
    Control* c = active != nullptr ? active : hot;
    juce::String text;
    juce::Rectangle<float> rect;

    if (c != nullptr)
    {
        text = c->bubbleText();
        if (text.isNotEmpty())
        {
            const auto anchor = c->bubbleAnchor();
            const float w = juce::jmax (110.0f, 20.0f * (float) text.length() + 40.0f);
            rect = juce::Rectangle<float> (anchor.getCentreX() - w * 0.5f, anchor.getY() - 8.0f, w, 46.0f);
            rect.setPosition (juce::jlimit (6.0f, Art::imageW - w - 6.0f, rect.getX()), juce::jmax (6.0f, rect.getY()));
        }
    }

    if (text != bubbleString || rect != bubbleRect)
    {
        if (! bubbleRect.isEmpty())
            repaintImage (bubbleRect.expanded (12.0f));
        bubbleString = text;
        bubbleRect = rect;
        if (! bubbleRect.isEmpty())
            repaintImage (bubbleRect.expanded (12.0f));
    }
}

inline void Panel::paint (juce::Graphics& g)
{
    g.addTransform (juce::AffineTransform::scale (Art::kx, Art::ky));
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);

    // Each control paints in a graphics state of its own. Sprites are drawn with
    // the opacity of the current colour: without this, the translucent glow of a
    // hovered control left its alpha behind, and every sprite painted after it
    // (faders, wheel blocks, knobs) came out faded or invisible.
    for (auto& c : controls)
        if (g.getClipBounds().toFloat().intersects (c->footprint()))
        {
            juce::Graphics::ScopedSaveState state (g);
            g.setColour (juce::Colours::black);
            c->paint (g, art);
        }

    if (! bubbleRect.isEmpty())
    {
        g.setColour (juce::Colour (0xF0150F14));
        g.fillRoundedRectangle (bubbleRect, 12.0f);
        g.setColour (Palette::pinkHot().withAlpha (0.85f));
        g.drawRoundedRectangle (bubbleRect.reduced (1.5f), 12.0f, 3.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions (30.0f, juce::Font::bold)));
        g.drawText (bubbleString, bubbleRect, juce::Justification::centred, false);
    }
}

inline void Panel::mouseMove (const juce::MouseEvent& e)
{
    const auto img = toImage (e.position);
    setHover (controlAt (img));
    if (auto* k = dynamic_cast<KeyboardControl*> (hot))
        k->mouseMoveTo (img);
}

inline void Panel::mouseExit (const juce::MouseEvent&)
{
    if (active == nullptr)
        setHover (nullptr);
}

inline void Panel::mouseDown (const juce::MouseEvent& e)
{
    const auto img = toImage (e.position);
    auto* c = controlAt (img);
    setHover (c);
    active = c;
    if (active != nullptr)
    {
        active->dragging = true;
        active->mouseDown (img, e);
        active->changed();
        updateBubble();
    }
}

inline void Panel::mouseDrag (const juce::MouseEvent& e)
{
    if (active != nullptr)
    {
        active->mouseDrag (e);
        updateBubble();
    }
}

inline void Panel::mouseUp (const juce::MouseEvent& e)
{
    if (active != nullptr)
    {
        auto* c = active;
        active = nullptr;
        c->dragging = false;
        c->mouseUp (e);
        c->changed();
    }

    const auto img = toImage (e.position);
    setHover (nullptr);
    setHover (controlAt (img));
    updateBubble();
}

inline void Panel::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (auto* c = controlAt (toImage (e.position)))
        c->mouseDouble();
}

inline void Panel::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    if (auto* c = controlAt (toImage (e.position)))
    {
        c->mouseWheel (w.isReversed ? -w.deltaY : w.deltaY);
        updateBubble();
    }
}

inline juce::String Panel::getTooltip()
{
    if (active != nullptr)
        return {};
    return hot != nullptr ? hot->tooltip() : juce::String();
}

inline void Panel::timerCallback()
{
    if (auto* k = dynamic_cast<KeyboardControl*> (keyboard))
    {
        auto notes = proc.getActiveNotesSnapshot();
        if (notes != lastNotes)
        {
            lastNotes = notes;
            k->setNotesDown (notes);
        }
    }

    const auto name = proc.getCurrentPresetName();
    if (name != lastPresetName)
    {
        lastPresetName = name;
        repaintImage (art.barField);
    }

    updateBubble();
}

inline void Panel::setParam (const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (value));
        p->endChangeGesture();
    }
}

//==============================================================================
// Menus and dialogs
//==============================================================================
inline void Panel::showPresetMenu (juce::Rectangle<float> anchor)
{
    proc.refreshPresetList();
    const auto& files = proc.getPresetFiles();
    const int current = proc.getCurrentPresetIndex();

    // The factory presets are numbered by family, in the order of the library.
    static const struct { int last; const char* name; } families[] =
    {
        { 14, "Crunch rhythm" }, { 26, "Leads" }, { 36, "High gain" }, { 48, "Clean" },
        { 56, "Bare strings" }, { 64, "Nocturnal" }, { 72, "Textures" }, { 76, "Low end" },
    };
    constexpr int numFamilies = (int) (sizeof (families) / sizeof (families[0]));

    std::vector<juce::PopupMenu> groups ((size_t) numFamilies + 1);
    for (int i = 0; i < files.size(); ++i)
    {
        const auto name = files[i].getFileNameWithoutExtension();
        int group = numFamilies;       // user presets
        if (name.length() > 3 && juce::CharacterFunctions::isDigit (name[0]) && juce::CharacterFunctions::isDigit (name[1]) && name[2] == ' ')
        {
            const int number = name.substring (0, 2).getIntValue();
            for (int f = 0; f < numFamilies; ++f)
                if (number <= families[f].last) { group = f; break; }
        }
        groups[(size_t) group].addItem (i + 1, name, true, i == current);
    }

    juce::PopupMenu menu;
    for (int f = 0; f < numFamilies; ++f)
        if (groups[(size_t) f].getNumItems() > 0)
            menu.addSubMenu (families[f].name, groups[(size_t) f]);

    if (groups[(size_t) numFamilies].getNumItems() > 0)
        menu.addSubMenu ("My presets", groups[(size_t) numFamilies]);

    if (files.isEmpty())
        menu.addItem (99999, "No preset found", false);

    menu.addSeparator();
    menu.addItem (9001, "Open preset file...");
    menu.addItem (9002, "Save preset as...");

    juce::Component::SafePointer<Panel> alive (this);
    styleMenu (menu);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (imageToScreen (anchor)).withStandardItemHeight (26),
                        [alive] (int result)
                        {
                            if (alive == nullptr || result == 0)
                                return;
                            if (result == 9001)       alive->openPresetDialog();
                            else if (result == 9002)  alive->savePresetDialog();
                            else if (result > 0)      alive->proc.loadPreset (result - 1);
                            alive->repaintImage (alive->art.barField);
                        });
}

inline void Panel::showCassetteMenu (juce::Rectangle<float> anchor)
{
    proc.rescanSamples();

    juce::PopupMenu menu;
    menu.addSectionHeader ("EXPANSION");
    menu.addItem (1, "Init patch");
    menu.addItem (2, "Open preset file...");
    menu.addItem (3, "Save preset as...");
    menu.addItem (4, "Show presets folder");
    menu.addSeparator();

    const bool delayOn = apvts.getRawParameterValue ("delayOn")->load() > 0.5f;
    const bool delaySync = apvts.getRawParameterValue ("delaySync")->load() > 0.5f;
    menu.addItem (10, "Delay on", true, delayOn);
    menu.addItem (11, "Delay synced to the host tempo", true, delaySync);
    menu.addSeparator();

    // Your own guitars: the sampled layer plays the files dropped in the Samples folder.
    const bool sampleOn = apvts.getRawParameterValue ("smpOn")->load() > 0.5f;
    const int selected = (int) apvts.getRawParameterValue ("smpSel")->load();
    const auto sampleNames = proc.getSampleFiles();

    juce::PopupMenu samples;
    samples.addItem (5000, "Off", true, ! sampleOn);
    samples.addSeparator();

    if (sampleNames.isEmpty())
    {
        samples.addItem (99998, "No file yet - drop .wav, .aif or .flac files in the Samples folder", false);
    }
    else
    {
        juce::PopupMenu category;
        juce::String categoryName;
        for (int i = 0; i < sampleNames.size(); ++i)
        {
            const auto c = proc.getSampleCategory (i);
            if (c != categoryName)
            {
                if (categoryName.isNotEmpty() || category.getNumItems() > 0)
                    samples.addSubMenu (categoryName, category);
                category = juce::PopupMenu();
                categoryName = c;
            }
            category.addItem (10000 + i, sampleNames[i], true, sampleOn && i == selected);
        }
        samples.addSubMenu (categoryName, category);
    }

    samples.addSeparator();
    samples.addItem (5001, "Open the Samples folder");
    menu.addSubMenu ("Your samples", samples);
    menu.addSeparator();
    menu.addItem (20, "About ROCKPUNK GUITAR");

    juce::Component::SafePointer<Panel> alive (this);
    styleMenu (menu);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (imageToScreen (anchor)).withStandardItemHeight (26),
                        [alive] (int result)
                        {
                            if (alive == nullptr || result <= 0)
                                return;

                            auto& p = *alive;
                            switch (result)
                            {
                                case 1:    p.proc.initPatch(); break;
                                case 2:    p.openPresetDialog(); break;
                                case 3:    p.savePresetDialog(); break;
                                case 4:    p.proc.getPresetDirectory().revealToUser(); break;
                                case 10:   p.setParam ("delayOn", p.apvts.getRawParameterValue ("delayOn")->load() > 0.5f ? 0.0f : 1.0f); break;
                                case 11:   p.setParam ("delaySync", p.apvts.getRawParameterValue ("delaySync")->load() > 0.5f ? 0.0f : 1.0f); break;
                                case 5000: p.setParam ("smpOn", 0.0f); break;
                                case 5001:
                                {
                                    auto dir = p.proc.getUserSampleDirectory();
                                    dir.createDirectory();
                                    dir.revealToUser();
                                    break;
                                }
                                case 20:
                                    juce::AlertWindow::showMessageBoxAsync (
                                        juce::MessageBoxIconType::NoIcon, "ROCKPUNK GUITAR",
                                        juce::String ("Version ") + ROCKPUNK_VERSION_STRING
                                            + "\n\nTwo modelled strings, the Urban Slayer amp, drive, delay, chorus and reverb."
                                              "\nNo recording is shipped: every note is calculated."
                                              "\n\nDJBILBOX BEATS - djbilboxbeats.com\nsupport@djbilboxbeats.com");
                                    break;
                                default:
                                    if (result >= 10000)
                                    {
                                        p.setParam ("smpSel", (float) (result - 10000));
                                        p.setParam ("smpOn", 1.0f);
                                    }
                                    break;
                            }
                            p.repaintImage (p.art.barField);
                        });
}

inline void Panel::savePresetDialog()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", proc.getPresetDirectory().getChildFile ("My preset.rpk"), "*.rpk");
    juce::Component::SafePointer<Panel> alive (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [alive] (const juce::FileChooser& fc)
                          {
                              if (alive == nullptr || fc.getResult() == juce::File())
                                  return;
                              alive->proc.savePreset (fc.getResult());
                              alive->repaintImage (alive->art.barField);
                          });
}

inline void Panel::openPresetDialog()
{
    chooser = std::make_unique<juce::FileChooser> ("Open preset", proc.getPresetDirectory(), "*.rpk");
    juce::Component::SafePointer<Panel> alive (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [alive] (const juce::FileChooser& fc)
                          {
                              if (alive == nullptr || ! fc.getResult().existsAsFile())
                                  return;
                              alive->proc.loadPreset (fc.getResult());
                              alive->repaintImage (alive->art.barField);
                          });
}

} // namespace rk
