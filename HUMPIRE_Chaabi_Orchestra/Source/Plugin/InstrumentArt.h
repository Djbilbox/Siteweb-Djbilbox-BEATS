#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "HumpireCore/HumpireTheme.h"
#include "Instruments.h"

// Vector silhouettes (juce::Path) of the 8 instruments, drawn in a 100 x 100 box.
namespace humpire
{
struct InstrumentArt
{
    // Returns { body (filled), details (strokes) }
    static std::pair<juce::Path, juce::Path> make (int idx)
    {
        juce::Path body, detail;
        auto line = [&] (float x1, float y1, float x2, float y2) { detail.startNewSubPath (x1, y1); detail.lineTo (x2, y2); };

        switch (idx)
        {
            case 0: // Mandole: pear-shaped body, long neck, tuning pegs
                body.startNewSubPath (50, 40);
                body.cubicTo (78, 40, 90, 62, 76, 82);
                body.cubicTo (68, 96, 32, 96, 24, 82);
                body.cubicTo (10, 62, 22, 40, 50, 40);
                body.closeSubPath();
                body.addRoundedRectangle (46.5f, 4, 7, 38, 2);
                body.addRoundedRectangle (42, 0, 16, 10, 3);
                detail.addEllipse (42, 58, 16, 16);
                for (int i = 0; i < 4; ++i) line (47.f + (float) i * 2.0f, 6, 46.f + (float) i * 2.6f, 84);
                line (36, 84, 64, 84);
                break;

            case 1: // Banjo: round head, neck
                body.addEllipse (14, 44, 72, 52);
                body.addRoundedRectangle (46.5f, 2, 7, 46, 2);
                body.addRoundedRectangle (43, 0, 14, 9, 3);
                detail.addEllipse (22, 50, 56, 40);
                detail.addEllipse (34, 60, 32, 20);
                for (int i = 0; i < 12; ++i)
                {
                    const float a = (float) i * juce::MathConstants<float>::twoPi / 12.0f;
                    detail.addEllipse (50 + 33 * std::cos (a) - 1.4f, 70 + 24 * std::sin (a) - 1.4f, 2.8f, 2.8f);
                }
                line (50, 8, 50, 74);
                break;

            case 2: // Violin: figure-8 silhouette, f-holes
                body.startNewSubPath (50, 34);
                body.cubicTo (72, 30, 82, 44, 72, 56);
                body.cubicTo (66, 62, 66, 64, 72, 70);
                body.cubicTo (86, 84, 70, 98, 50, 96);
                body.cubicTo (30, 98, 14, 84, 28, 70);
                body.cubicTo (34, 64, 34, 62, 28, 56);
                body.cubicTo (18, 44, 28, 30, 50, 34);
                body.closeSubPath();
                body.addRoundedRectangle (47, 6, 6, 30, 2);
                body.addEllipse (44, 0, 12, 10);
                detail.startNewSubPath (38, 58); detail.cubicTo (34, 66, 40, 74, 38, 82);
                detail.startNewSubPath (62, 58); detail.cubicTo (66, 66, 60, 74, 62, 82);
                line (40, 88, 60, 88);
                line (50, 8, 50, 88);
                break;

            case 3: // Ney: knotted reed + finger holes
                body.addRoundedRectangle (44, 2, 12, 96, 5);
                for (int i = 0; i < 6; ++i) detail.addEllipse (47, 30.f + (float) i * 11.f, 6, 6);
                for (float y : { 18.f, 54.f, 80.f }) line (44, y, 56, y);
                detail.addEllipse (46, 6, 8, 5);
                break;

            case 4: // Qanun: trapezoid with strings
                body.startNewSubPath (10, 78);
                body.lineTo (28, 22);
                body.lineTo (92, 22);
                body.lineTo (92, 78);
                body.closeSubPath();
                for (int i = 0; i < 14; ++i) line (34.f + (float) i * 3.9f, 25, 22.f + (float) i * 5.4f, 76);
                detail.addEllipse (62, 40, 14, 14);
                detail.addEllipse (62, 58, 10, 10);
                break;

            case 5: // Derbouka: goblet (wide head, thin neck, flared foot)
                body.startNewSubPath (10, 16);
                body.lineTo (90, 16);
                body.cubicTo (90, 34, 60, 36, 59, 50);
                body.lineTo (59, 62);
                body.cubicTo (72, 70, 82, 82, 82, 96);
                body.lineTo (18, 96);
                body.cubicTo (18, 82, 28, 70, 41, 62);
                body.lineTo (41, 50);
                body.cubicTo (40, 36, 10, 34, 10, 16);
                body.closeSubPath();
                detail.addEllipse (10, 11, 80, 10);
                for (float x = 18; x < 84; x += 9) line (x, 22, x + 2, 30);
                line (28, 88, 72, 88);
                line (41, 58, 59, 58);
                break;

            case 6: // Tar: round frame + jingles
                body.addEllipse (6, 6, 88, 88);
                detail.addEllipse (14, 14, 72, 72);
                for (int i = 0; i < 4; ++i)
                {
                    const float a = (float) i * juce::MathConstants<float>::halfPi + 0.4f;
                    detail.addEllipse (50 + 40 * std::cos (a) - 6, 50 + 40 * std::sin (a) - 6, 12, 12);
                    line (50 + 40 * std::cos (a) - 6, 50 + 40 * std::sin (a), 50 + 40 * std::cos (a) + 6, 50 + 40 * std::sin (a));
                }
                detail.addEllipse (30, 30, 40, 40);
                break;

            default: // Bendir: frame + snare cord
                body.addEllipse (6, 6, 88, 88);
                detail.addEllipse (13, 13, 74, 74);
                for (float y : { 42.f, 50.f, 58.f }) line (16, y, 84, y);
                for (int i = 0; i < 8; ++i)
                {
                    const float a = (float) i * juce::MathConstants<float>::twoPi / 8.0f;
                    detail.addEllipse (50 + 44 * std::cos (a) - 1.6f, 50 + 44 * std::sin (a) - 1.6f, 3.2f, 3.2f);
                }
                break;
        }
        return { body, detail };
    }

    static void draw (juce::Graphics& g, int idx, juce::Rectangle<float> area, juce::Colour c, bool glow = false)
    {
        auto [body, detail] = make (idx);
        const auto t = juce::RectanglePlacement (juce::RectanglePlacement::centred)
                           .getTransformToFit (juce::Rectangle<float> (0, 0, 100, 100), area.reduced (area.getWidth() * 0.04f));
        if (glow)
        {
            g.setColour (c.withAlpha (0.18f));
            g.strokePath (body, juce::PathStrokeType (9.0f), t);
        }
        g.setGradientFill (juce::ColourGradient (c.brighter (0.25f), area.getX(), area.getY(),
                                                 c.darker (0.55f), area.getRight(), area.getBottom(), false));
        g.fillPath (body, t);
        g.setColour (theme::bg.withAlpha (0.55f));
        g.strokePath (detail, juce::PathStrokeType (1.4f), t);
        g.setColour (theme::gold.withAlpha (0.85f));
        g.strokePath (body, juce::PathStrokeType (1.2f), t);
    }
};
} // namespace humpire
