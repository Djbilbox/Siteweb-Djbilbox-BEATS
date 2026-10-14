#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <vector>

#include "Filmstrip.h"
#include "Palette.h"

namespace wz
{

//==============================================================================
// WarzoneLookAndFeel
// -------------------
// Renders the real UI elements straight from the master background image: the
// knobs and rotary switches are cropped out of the mockup and pre-rotated into
// filmstrips (rotation sprites), the level sliders / VU meters / cassette /
// piano use direct crops of their region. No generic rectangles are drawn over
// the artwork any more.
//
// Two ways to use the sprites:
//   1) standard JUCE widgets (Slider, Button, ToggleButton) simply get this
//      look-and-feel installed - drawRotarySlider / drawLinearSlider /
//      drawButtonBackground pick the sprite via the "sprite" component property.
//   2) the custom WARZONE808 components call drawKnobSprite()/drawSwitchSprite()/
//      drawSliderSprite()/drawCropSprite() directly from their paint().
//==============================================================================
class WarzoneLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    struct SpriteSpec
    {
        juce::String key;
        juce::String kind;   // "knob" | "switch" | "slider" | "crop"
        float size = 0.0f;   // square crop size in design px (0 = auto)
        int frames = 0;      // filmstrip frames (0 = auto)
        juce::String box;    // layout box key (default = key)
        float cx = -1.0f;    // crop centre override in design px (-1 = box centre)
        float cy = -1.0f;
    };

    //--------------------------------------------------------------------------
    // Sprite building. Call once with the loaded background image and the layout
    // boxes (design coordinates). designW/designH map design px -> image px.
    //--------------------------------------------------------------------------
    void buildSprites (const juce::Image& background,
                       const std::map<juce::String, juce::Rectangle<float>>& boxes,
                       float designW, float designH,
                       const std::vector<SpriteSpec>& specs)
    {
        sprites.clear();
        for (const auto& spec : specs)
        {
            const juce::String boxKey = spec.box.isEmpty() ? spec.key : spec.box;
            auto boxIt = boxes.find (boxKey);
            if (boxIt == boxes.end())
                continue;

            const auto& box = boxIt->second;
            const juce::Point<float> centre (spec.cx >= 0.0f ? spec.cx : box.getCentreX(),
                                             spec.cy >= 0.0f ? spec.cy : box.getCentreY());
            Sprite s;

            if (spec.kind == "knob")
            {
                const float sizePx = spec.size > 0.0f ? spec.size
                                                      : juce::jmin (box.getWidth(), box.getHeight()) * 0.78f;
                const int frames = spec.frames > 0 ? spec.frames : 16;
                // realistic pot sweep: 135 degrees (down-left) -> 405 degrees (down-right), 270 deg total
                const float startA = juce::MathConstants<float>::pi * 0.75f;
                const float endA   = juce::MathConstants<float>::pi * 2.25f;
                const auto strip = buildRotaryStrip (background, centre, designW, designH, sizePx, frames, startA, endA);
                s.strip.setStrip (strip, frames, frames);
            }
            else if (spec.kind == "switch")
            {
                const float sizePx = spec.size > 0.0f ? spec.size
                                                      : juce::jmax (26.0f, juce::jmin (box.getWidth(), box.getHeight()) * 0.9f);
                const int frames = spec.frames > 0 ? spec.frames : 2;
                // toggle states: 0 degrees (up) vs 180 degrees (down)
                const auto strip = buildRotaryStrip (background, centre, designW, designH, sizePx, frames,
                                                     0.0f, juce::MathConstants<float>::pi);
                s.strip.setStrip (strip, frames, frames);
            }
            else
            {
                s.strip.setStrip (buildCrop (background, box, designW, designH), 1, 1);
                s.horizontal = box.getWidth() >= box.getHeight();
            }

            sprites[spec.key] = s;
        }
    }

    bool hasSprite (const juce::String& key) const noexcept
    {
        const auto it = sprites.find (key);
        return it != sprites.end() && it->second.strip.isValid();
    }

    //--------------------------------------------------------------------------
    // Sprite renderers used by the custom WARZONE808 components.
    //--------------------------------------------------------------------------
    void drawKnobSprite (juce::Graphics& g, const juce::String& key, float value01,
                         const juce::Rectangle<int>& dest) const
    {
        auto* s = findSprite (key);
        if (s == nullptr || ! s->strip.isValid())
            return;

        const int n = s->strip.numFrames();
        const int frame = juce::jlimit (0, n - 1, juce::roundToInt (value01 * (float) (n - 1)));
        s->strip.drawFrame (g, frame, dest, true);
    }

    void drawSwitchSprite (juce::Graphics& g, const juce::String& key, float value01,
                           const juce::Rectangle<int>& dest) const
    {
        drawKnobSprite (g, key, value01, dest);
    }

    void drawSliderSprite (juce::Graphics& g, const juce::String& key, float value01,
                           const juce::Rectangle<int>& dest) const
    {
        auto* s = findSprite (key);
        if (s == nullptr || ! s->strip.isValid())
            return;

        s->strip.drawFrame (g, 0, dest, false);

        const auto r = dest.toFloat();
        juce::Rectangle<float> fill;
        if (s->horizontal)
            fill = juce::Rectangle<float> (r.getX(), r.getY(), r.getWidth() * value01, r.getHeight());
        else
            fill = juce::Rectangle<float> (r.getX(), r.getBottom() - r.getHeight() * value01,
                                           r.getWidth(), r.getHeight() * value01);

        juce::ColourGradient grad (Palette::neonGreen().withAlpha (0.42f), r.getX(), r.getY(),
                                   Palette::neonRed().withAlpha (0.42f), r.getRight(), r.getY(), false);
        g.setGradientFill (grad);
        g.fillRect (fill);
    }

    void drawCropSprite (juce::Graphics& g, const juce::String& key,
                         const juce::Rectangle<int>& dest) const
    {
        auto* s = findSprite (key);
        if (s == nullptr || ! s->strip.isValid())
            return;

        s->strip.drawFrame (g, 0, dest, false);
    }

    //--------------------------------------------------------------------------
    // Standard JUCE LookAndFeel entry points - sprite-driven.
    //--------------------------------------------------------------------------
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const juce::String key = slider.getProperties().getWithDefault ("sprite", juce::var()).toString();
        if (key.isNotEmpty())
            drawKnobSprite (g, key, sliderPos, juce::Rectangle<int> (x, y, w, h));
        else
            juce::LookAndFeel_V4::drawRotarySlider (g, x, y, w, h, sliderPos,
                                                    rotaryStartAngle, rotaryEndAngle, slider);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                           float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        const juce::String key = slider.getProperties().getWithDefault ("sprite", juce::var()).toString();
        if (key.isNotEmpty())
            drawSliderSprite (g, key, sliderPos, juce::Rectangle<int> (x, y, w, h));
        else
            juce::LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos,
                                                    minSliderPos, maxSliderPos, style, slider);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused (backgroundColour);

        // transparent base: the cassette artwork shows through the buttons
        if (isButtonDown)
        {
            g.setColour (Palette::neonRed().withAlpha (0.40f));
            g.drawRoundedRectangle (button.getLocalBounds().toFloat().reduced (1.0f), 4.0f, 1.5f);
        }
        else if (isMouseOverButton)
        {
            g.setColour (Palette::neonGreen().withAlpha (0.30f));
            g.drawRoundedRectangle (button.getLocalBounds().toFloat().reduced (1.0f), 4.0f, 1.2f);
        }
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool isMouseOverButton, bool isButtonDown) override
    {
        const juce::String key = button.getProperties().getWithDefault ("sprite", juce::var()).toString();
        if (key.isNotEmpty())
            drawSwitchSprite (g, key, button.getToggleState() ? 1.0f : 0.0f,
                              button.getLocalBounds());
        else
            juce::LookAndFeel_V4::drawToggleButton (g, button, isMouseOverButton, isButtonDown);
    }

private:
    struct Sprite
    {
        Filmstrip strip;
        bool horizontal = true;
    };

    const Sprite* findSprite (const juce::String& key) const noexcept
    {
        const auto it = sprites.find (key);
        return it != sprites.end() ? &it->second : nullptr;
    }

    //--------------------------------------------------------------------------
    // Crop a square region around the given centre (design px), then rotate it
    // into a filmstrip sweeping from startAngle to endAngle. A circular clip
    // keeps the corners transparent so the rotating knob floats over the
    // artwork; a subtle dark bezel ring seals the crop edge.
    //--------------------------------------------------------------------------
    juce::Image buildRotaryStrip (const juce::Image& bg, const juce::Point<float>& centreDesign,
                                  float designW, float designH, float sizePx, int frames,
                                  float startAngle, float endAngle) const
    {
        const float sx = (float) bg.getWidth() / designW;
        const float sy = (float) bg.getHeight() / designH;
        const juce::Point<float> centre (centreDesign.x * sx, centreDesign.y * sy);

        const int size = juce::jmax (8, juce::roundToInt (sizePx * juce::jmin (sx, sy)));
        const auto src = juce::Rectangle<int> (juce::roundToInt (centre.x) - size / 2,
                                               juce::roundToInt (centre.y) - size / 2,
                                               size, size).getIntersection (bg.getBounds());

        const int fw = juce::jmax (1, src.getWidth());
        const int fh = juce::jmax (1, src.getHeight());
        const float bezel = juce::jmax (1.0f, (float) fw * 0.02f);

        juce::Image strip (juce::Image::ARGB, fw * frames, fh, true);
        {
            juce::Graphics g (strip);
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);

            for (int i = 0; i < frames; ++i)
            {
                const float t = frames > 1 ? (float) i / (float) (frames - 1) : 0.0f;
                const float angle = startAngle + (endAngle - startAngle) * t;
                const float cx = (float) i * fw + fw * 0.5f;
                const float cy = fh * 0.5f;

                g.saveState();
                juce::Path clip;
                clip.addEllipse ((float) i * fw, 0.0f, (float) fw, (float) fh);
                g.reduceClipRegion (clip);
                g.addTransform (juce::AffineTransform::rotation (angle, cx, cy));
                g.drawImage (bg, i * fw, 0, fw, fh, src.getX(), src.getY(), fw, fh);
                g.restoreState();

                g.setColour (juce::Colours::black.withAlpha (0.35f));
                g.drawEllipse ((float) i * fw + bezel * 0.5f, bezel * 0.5f,
                               (float) fw - bezel, (float) fh - bezel, bezel);
            }
        }

        return strip;
    }

    //--------------------------------------------------------------------------
    // Straight crop of a layout box (no rotation) - for sliders, VU meters,
    // the cassette panel and the piano keyboard strip.
    //--------------------------------------------------------------------------
    juce::Image buildCrop (const juce::Image& bg, const juce::Rectangle<float>& box,
                           float designW, float designH) const
    {
        const float sx = (float) bg.getWidth() / designW;
        const float sy = (float) bg.getHeight() / designH;
        const auto src = juce::Rectangle<int> (juce::roundToInt (box.getX() * sx),
                                               juce::roundToInt (box.getY() * sy),
                                               juce::roundToInt (box.getWidth() * sx),
                                               juce::roundToInt (box.getHeight() * sy))
                             .getIntersection (bg.getBounds());

        if (src.isEmpty())
            return {};

        juce::Image crop (juce::Image::ARGB, src.getWidth(), src.getHeight(), true);
        {
            juce::Graphics g (crop);
            g.drawImage (bg, 0, 0, src.getWidth(), src.getHeight(),
                         src.getX(), src.getY(), src.getWidth(), src.getHeight());
        }
        return crop;
    }

    std::map<juce::String, Sprite> sprites;
};

} // namespace wz
