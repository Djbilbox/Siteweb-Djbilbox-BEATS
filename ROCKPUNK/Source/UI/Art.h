#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <vector>

#include "BinaryData.h"

namespace rk
{

//==============================================================================
// The artwork and where everything sits on it.
//
// Assets/faceplate.jpg, sprites.png and layout.json are produced by
// Tools/make_ui_assets.py from the master mockup. Every coordinate in
// layout.json is in mockup pixels (2400 x 1792, pixel i spanning [i, i + 1)),
// and the editor draws and hit-tests in that same unit: the panel scales it
// to its own 1000 x 746 design size, and the window scales the panel.
//==============================================================================
struct Art
{
    static constexpr float imageW  = 2400.0f;
    static constexpr float imageH  = 1792.0f;
    static constexpr float designW = 1000.0f;
    static constexpr float designH = 746.0f;
    static constexpr float kx = designW / imageW;
    static constexpr float ky = designH / imageH;

    struct Sprite
    {
        juce::Image image;
        float ax = 0.0f, ay = 0.0f;       // anchor, in sprite pixels
    };

    struct Knob  { juce::String id, param; float x = 0, y = 0; };
    struct Fader { juce::String id, param, group; float cx = 0, slotTop = 0, slotBottom = 0, minY = 0, maxY = 0; };
    struct Wheel { juce::String id, param; float x = 0, minY = 0, maxY = 0, x0 = 0, x1 = 0; };
    struct Combo { juce::String id, param; juce::Rectangle<float> box, face; };
    struct Black { float x0 = 0, x1 = 0; };

    juce::Image plate;
    std::map<juce::String, Sprite> sprites;
    std::vector<Knob> knobs;
    std::vector<Fader> faders;
    std::vector<Wheel> wheels;
    std::vector<Combo> combos;

    juce::Rectangle<float> grillA, grillB, cassette;
    juce::Rectangle<float> barFolder, barField, barPrev, barNext, barSave, barText;

    struct Keys
    {
        int lowNote = 36;
        float left = 0, right = 0, top = 0, blackBottom = 0, whiteBottom = 0;
        std::vector<float> separators;
        std::vector<Black> black;
    } keys;

    float knobCapRadius = 46.0f;
    bool valid = false;

    Art()
    {
        plate = juce::ImageFileFormat::loadFrom (BinaryData::faceplate_jpg, (size_t) BinaryData::faceplate_jpgSize);

        const auto parsed = juce::JSON::parse (juce::String::createStringFromData (BinaryData::layout_json,
                                                                                   BinaryData::layout_jsonSize));
        const auto atlas = juce::ImageFileFormat::loadFrom (BinaryData::sprites_png, (size_t) BinaryData::sprites_pngSize);
        if (! plate.isValid() || ! atlas.isValid() || ! parsed.isObject())
            return;

        if (auto* s = parsed["sprites"].getDynamicObject())
            for (const auto& p : s->getProperties())
            {
                Sprite sp;
                const auto& v = p.value;
                sp.image = atlas.getClippedImage ({ (int) v["x"], (int) v["y"], (int) v["w"], (int) v["h"] }).createCopy();
                sp.ax = num (v["ax"]);
                sp.ay = num (v["ay"]);
                sprites[p.name.toString()] = sp;
            }

        if (auto* o = parsed["knobs"].getDynamicObject())
            for (const auto& p : o->getProperties())
                knobs.push_back ({ p.name.toString(), p.value["param"].toString(), num (p.value["x"]), num (p.value["y"]) });

        if (auto* o = parsed["faders"].getDynamicObject())
            for (const auto& p : o->getProperties())
            {
                const auto& v = p.value;
                faders.push_back ({ p.name.toString(), v["param"].toString(), v["group"].toString(), num (v["cx"]),
                                    num (v["slotTop"]), num (v["slotBottom"]), num (v["thumbMinY"]), num (v["thumbMaxY"]) });
            }

        if (auto* o = parsed["wheels"].getDynamicObject())
            for (const auto& p : o->getProperties())
            {
                const auto& v = p.value;
                wheels.push_back ({ p.name.toString(), v["param"].toString(), num (v["x"]), num (v["minY"]), num (v["maxY"]),
                                    num (v["x0"]), num (v["x1"]) });
            }

        if (auto* a = parsed["combos"].getArray())
            for (const auto& v : *a)
            {
                Combo c;
                c.id = v["id"].toString();
                c.param = v["param"].toString();
                c.box = juce::Rectangle<float>::leftTopRightBottom (num (v["x0"]), num (v["y0"]), num (v["x1"]), num (v["y1"]));
                c.face = juce::Rectangle<float>::leftTopRightBottom (num (v["x0"]), num (v["faceY0"]), num (v["x1"]), num (v["faceY1"]));
                combos.push_back (c);
            }

        grillA   = rect (parsed["grills"]["a"]);
        grillB   = rect (parsed["grills"]["b"]);
        cassette = rect (parsed["cassette"]);

        const auto& bar = parsed["presetBar"];
        barFolder = rect (bar["folder"]);
        barField  = rect (bar["field"]);
        barPrev   = rect (bar["prev"]);
        barNext   = rect (bar["next"]);
        barSave   = rect (bar["save"]);
        barText   = rect (bar["text"]);

        const auto& k = parsed["keyboard"];
        keys.lowNote     = (int) k["lowNote"];
        keys.left        = num (k["left"]);
        keys.right       = num (k["right"]);
        keys.top         = num (k["top"]);
        keys.blackBottom = num (k["blackBottom"]);
        keys.whiteBottom = num (k["whiteBottom"]);
        if (auto* a = k["separators"].getArray())
            for (const auto& v : *a)
                keys.separators.push_back (num (v));
        if (auto* a = k["black"].getArray())
            for (const auto& v : *a)
                keys.black.push_back ({ num (v["x0"]), num (v["x1"]) + 1.0f });

        knobCapRadius = num (parsed["knobCapRadius"]);
        valid = sprites.count ("knobCap") > 0 && keys.separators.size() == 21 && keys.black.size() == 15;
    }

    const Sprite& sprite (const juce::String& name) const
    {
        static const Sprite none;
        const auto it = sprites.find (name);
        return it != sprites.end() ? it->second : none;
    }

    // Draws a sprite with its anchor at (x, y) of the current coordinate system
    // (mockup pixels), optionally turned about the anchor.
    void draw (juce::Graphics& g, const Sprite& s, float x, float y, float rotation = 0.0f) const
    {
        if (! s.image.isValid())
            return;

        auto t = juce::AffineTransform::translation (-s.ax, -s.ay);
        if (std::abs (rotation) > 1.0e-6f)
            t = t.rotated (rotation);

        // an image is composited with the opacity of the current colour
        g.setOpacity (1.0f);
        g.drawImageTransformed (s.image, t.translated (x, y), false);
    }

    static float num (const juce::var& v) { return (float) (double) v; }

    static juce::Rectangle<float> rect (const juce::var& v)
    {
        // x1 / y1 are the last pixel of the zone: the zone covers up to x1 + 1
        return juce::Rectangle<float>::leftTopRightBottom (num (v["x0"]), num (v["y0"]),
                                                           num (v["x1"]) + 1.0f, num (v["y1"]) + 1.0f);
    }
};

} // namespace rk
