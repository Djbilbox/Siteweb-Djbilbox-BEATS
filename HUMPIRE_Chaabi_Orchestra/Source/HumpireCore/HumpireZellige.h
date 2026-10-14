#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

// Motif zellige : etoile a 8 branches + losange de jonction, en filigrane.
namespace humpire
{
inline juce::Path makeEightPointStar (float cx, float cy, float rOut, float rIn)
{
    juce::Path p;
    for (int i = 0; i < 16; ++i)
    {
        const float a = juce::MathConstants<float>::pi * (float) i / 8.0f - juce::MathConstants<float>::halfPi;
        const float r = (i % 2 == 0) ? rOut : rIn;
        const float x = cx + r * std::cos (a), y = cy + r * std::sin (a);
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    p.closeSubPath();
    return p;
}

inline juce::Image makeZelligeImage (int w, int h, float cell, juce::Colour line)
{
    juce::Image img (juce::Image::ARGB, juce::jmax (1, w), juce::jmax (1, h), true);
    juce::Graphics g (img);
    g.setColour (line);

    juce::Path diamond;
    diamond.addRectangle (-cell * 0.09f, -cell * 0.09f, cell * 0.18f, cell * 0.18f);

    for (float y = 0; y < (float) h + cell; y += cell)
        for (float x = 0; x < (float) w + cell; x += cell)
        {
            g.strokePath (makeEightPointStar (x + cell * 0.5f, y + cell * 0.5f, cell * 0.46f, cell * 0.30f),
                          juce::PathStrokeType (1.0f));
            g.strokePath (diamond, juce::PathStrokeType (1.0f),
                          juce::AffineTransform::rotation (juce::MathConstants<float>::pi * 0.25f).translated (x, y));
        }
    return img;
}
} // namespace humpire
