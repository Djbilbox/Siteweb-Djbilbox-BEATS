#include "LedMeter.h"

namespace humpire
{
LedMeter::LedMeter (std::atomic<float>& s, Orientation o, int n, float fl)
    : source (s), orient (o), nSeg (n), floorDb (fl)
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (60);
}

void LedMeter::timerCallback()
{
    const float p = source.exchange (0.0f, std::memory_order_relaxed);
    level = juce::jmax (p, level * 0.90f);                  // chute ~ 300 ms
    if (level > hold) { hold = level; holdCount = 45; }     // peak hold 0.75 s
    else if (holdCount > 0) --holdCount;
    else hold *= 0.94f;
    repaint();
}

void LedMeter::paint (juce::Graphics& g)
{
    auto toNorm = [this] (float lin)
    {
        return juce::jlimit (0.0f, 1.0f, juce::jmap (juce::Decibels::gainToDecibels (lin, floorDb), floorDb, 0.0f, 0.0f, 1.0f));
    };
    const float lit = toNorm (level) * (float) nSeg, holdSeg = toNorm (hold) * (float) nSeg;
    const bool  horiz = orient == Orientation::horizontal;
    const auto  b = getLocalBounds().toFloat();
    const float gap = 1.5f;
    const float segLen = ((horiz ? b.getWidth() : b.getHeight()) - gap * (float) (nSeg - 1)) / (float) nSeg;

    for (int i = 0; i < nSeg; ++i)
    {
        const float f = (float) i / (float) nSeg;
        const auto on = f < 0.6f ? col::amber : (f < 0.85f ? col::orange : col::red);
        const bool isLit  = (float) i < lit;
        const bool isHold = (int) holdSeg - 1 == i && holdSeg > 0.5f;

        juce::Rectangle<float> r = horiz
            ? juce::Rectangle<float> (b.getX() + (float) i * (segLen + gap), b.getY(), segLen, b.getHeight())
            : juce::Rectangle<float> (b.getX(), b.getBottom() - (float) (i + 1) * segLen - (float) i * gap, b.getWidth(), segLen);

        g.setColour (isLit || isHold ? on : on.withAlpha (0.10f).interpolatedWith (col::black, 0.5f));
        g.fillRoundedRectangle (r, 1.0f);
        if (isLit && ! Theme::isClean())
        {
            g.setColour (on.withAlpha (0.25f));
            g.fillRoundedRectangle (r.expanded (1.0f), 2.0f);   // halo LED
        }
    }
}
} // namespace humpire
