#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <vector>

#include "Filmstrip.h"
#include "Palette.h"

namespace rk
{

//==============================================================================
// RockpunkLookAndFeel
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
//   2) the custom ROCKPUNK components call drawKnobSprite()/drawSwitchSprite()/
//      drawSliderSprite()/drawCropSprite() directly from their paint().
//==============================================================================
class RockpunkLookAndFeel final : public juce::LookAndFeel_V4
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
                const auto crop = buildCrop (background, box, designW, designH);
                s.strip.setStrip (crop, 1, 1);
                s.horizontal = box.getWidth() >= box.getHeight();

                if (spec.kind == "slider" && ! s.horizontal)
                    cutFaderParts (s, crop);
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
    // Sprite renderers used by the custom ROCKPUNK components.
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

        // Fader anime depuis l'image : rail vide repete sur toute la course,
        // puis le curseur decoupe dans l'artwork, pose a la valeur. Plus rien
        // n'est peint par-dessus et il n'y a qu'un seul curseur.
        if (s->thumbImage.isValid() && s->railTile.isValid())
        {
            // Une ligne etiree sur toute la hauteur : le rail vide.
            g.drawImage (s->railTile, dest.getX(), dest.getY(), dest.getWidth(), dest.getHeight(),
                         0, 0, s->railTile.getWidth(), s->railTile.getHeight());

            const int span = juce::jmax (3, (int) std::round (s->thumbImage.getHeight()
                                               * dest.getWidth() / (float) s->thumbImage.getWidth()));
            const int travel = juce::jmax (0, dest.getHeight() - span);
            const int top = dest.getBottom() - span - (int) std::round (travel * value01);

            g.drawImage (s->thumbImage, dest.getX(), top, dest.getWidth(), span,
                         0, 0, s->thumbImage.getWidth(), s->thumbImage.getHeight());
            return;
        }

        s->strip.drawFrame (g, 0, dest, false);

        // Repli quand le curseur n'a pas pu etre isole dans l'image : une
        // pastille lumineuse a la valeur, plutot qu'un aplat sur toute la course.
        const auto r = dest.toFloat();
        const float thickness = juce::jlimit (3.0f, 7.0f,
                                              (s->horizontal ? r.getWidth() : r.getHeight()) * 0.06f);

        juce::Rectangle<float> cap;
        if (s->horizontal)
            cap = juce::Rectangle<float> (r.getX() + (r.getWidth() - thickness) * value01, r.getY(),
                                          thickness, r.getHeight());
        else
            cap = juce::Rectangle<float> (r.getX(),
                                          r.getBottom() - thickness - (r.getHeight() - thickness) * value01,
                                          r.getWidth(), thickness);

        g.setColour (Palette::neonViolet().withAlpha (0.30f));
        g.fillRoundedRectangle (cap.expanded (1.5f), 3.0f);
        g.setColour (Palette::neonRose().withAlpha (0.95f));
        g.fillRoundedRectangle (cap, 2.0f);
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
            g.setColour (Palette::neonRose().withAlpha (0.40f));
            g.drawRoundedRectangle (button.getLocalBounds().toFloat().reduced (1.0f), 4.0f, 1.5f);
        }
        else if (isMouseOverButton)
        {
            g.setColour (Palette::neonViolet().withAlpha (0.30f));
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

        // Pieces d'un fader, decoupees dans l'image de fond.
        //
        // Le rail dessine porte deja son curseur, a une position fixe. Poser
        // une pastille par-dessus donnait deux curseurs sur le meme rail : celui
        // de l'image, immobile, et le mien. On decoupe donc le curseur, on
        // rebouche sa place avec une tranche de rail vide, et on le redessine a
        // la valeur : c'est l'image elle-meme qui bouge.
        juce::Image railTile;    // tranche de rail sans curseur, repetee
        juce::Image thumbImage;  // le curseur seul
    };

    // Repere le curseur dans le recadrage d'un fader vertical : c'est la bande
    // de lignes nettement plus claires que le rail, qui est presque noir.
    static void cutFaderParts (Sprite& s, const juce::Image& crop)
    {
        const int w = crop.getWidth(), h = crop.getHeight();
        if (w < 2 || h < 24)
            return;

        std::vector<float> rows ((size_t) h, 0.0f);
        for (int y = 0; y < h; ++y)
        {
            float sum = 0.0f;
            for (int x = 0; x < w; ++x)
                sum += crop.getPixelAt (x, y).getBrightness();
            rows[(size_t) y] = sum / (float) w;
        }

        auto sorted = rows;
        std::sort (sorted.begin(), sorted.end());
        const float median = sorted[sorted.size() / 2];
        const float peak   = sorted.back();
        if (peak <= median + 0.05f)
            return;                            // pas de curseur discernable

        // On part de la ligne la plus claire - le trait lumineux au milieu du
        // cabochon - puis on s'etend tant que les lignes restent au-dessus du
        // rail, qui est presque noir. Un seuil haut n'attrapait que ce trait et
        // laissait le corps gris du cabochon dans la tranche dite "propre",
        // qui se retrouvait alors repetee tout le long du rail.
        const float threshold = median + 0.30f * (peak - median);

        int seed = 0;
        for (int y = 1; y < h; ++y)
            if (rows[(size_t) y] > rows[(size_t) seed])
                seed = y;

        int top = seed, bottom = seed;
        while (top > 0 && rows[(size_t) (top - 1)] > threshold)
            --top;
        while (bottom < h - 1 && rows[(size_t) (bottom + 1)] > threshold)
            ++bottom;

        if (bottom - top < 3 || bottom - top > h * 2 / 3)
            return;

        top = juce::jmax (0, top - 1);
        bottom = juce::jmin (h - 1, bottom + 1);
        const int span = bottom - top + 1;

        // Le rail vide est UNE seule ligne, la plus sombre du recadrage, etiree
        // sur toute la course. Choisir une tranche de plusieurs lignes obligeait
        // a deviner ou le cabochon s'arrete : quand la tranche mordait dessus,
        // son grip se retrouvait repete tout du long et le fader devenait une
        // colonne rayee. Une seule ligne garde le profil horizontal exact du
        // rail et ne peut porter aucune texture verticale.
        int darkest = 0;
        for (int y = 1; y < h; ++y)
            if (rows[(size_t) y] < rows[(size_t) darkest])
                darkest = y;

        s.thumbImage = crop.getClippedImage ({ 0, top, w, span }).createCopy();
        s.railTile   = crop.getClippedImage ({ 0, darkest, w, 1 }).createCopy();
    }

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

} // namespace rk
