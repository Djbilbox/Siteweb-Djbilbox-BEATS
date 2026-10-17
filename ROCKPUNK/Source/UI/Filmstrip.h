#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rk
{

//==============================================================================
// Filmstrip - a sprite sheet laid out as a grid of frames (rotation frames for
// knobs/switches, or a single full frame for crops). Frames are drawn into any
// destination rectangle, keeping the original aspect ratio by default.
//==============================================================================
class Filmstrip
{
public:
    Filmstrip() = default;

    void setStrip (const juce::Image& strip, int frameCount, int columns)
    {
        sheet = strip;
        frames = juce::jmax (1, frameCount);
        cols = juce::jmax (1, columns);
        frameW = sheet.getWidth() / cols;
        frameH = sheet.getHeight() / ((frames + cols - 1) / cols);
    }

    bool isValid() const noexcept { return sheet.isValid() && frames > 0; }
    int numFrames() const noexcept { return frames; }
    int getFrameWidth() const noexcept { return frameW; }
    int getFrameHeight() const noexcept { return frameH; }

    juce::Rectangle<int> frameRect (int i) const noexcept
    {
        i = juce::jlimit (0, frames - 1, i);
        return { (i % cols) * frameW, (i / cols) * frameH, frameW, frameH };
    }

    void drawFrame (juce::Graphics& g, int i, const juce::Rectangle<int>& dest,
                    bool centreKeepAspect = true) const
    {
        if (! sheet.isValid())
            return;

        const auto src = frameRect (i);

        juce::Rectangle<float> d;
        if (centreKeepAspect)
        {
            const float aspect = (float) src.getWidth() / (float) src.getHeight();
            auto fit = dest.toFloat();
            float w = fit.getWidth();
            float h = fit.getHeight();
            if (w / h > aspect) w = h * aspect;
            else                h = w / aspect;
            d = juce::Rectangle<float> (fit.getCentreX() - w * 0.5f,
                                        fit.getCentreY() - h * 0.5f, w, h);
        }
        else
        {
            d = dest.toFloat();
        }

        g.drawImage (sheet,
                     (int) d.getX(), (int) d.getY(), (int) d.getWidth(), (int) d.getHeight(),
                     src.getX(), src.getY(), src.getWidth(), src.getHeight());
    }

private:
    juce::Image sheet;
    int frames = 0;
    int cols = 1;
    int frameW = 1;
    int frameH = 1;
};

} // namespace rk