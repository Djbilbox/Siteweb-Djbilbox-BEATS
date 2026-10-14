#pragma once
#include "HumpireTheme.h"

namespace humpire
{
/** Meter a segments LED. Source = pic lineaire ecrit par le thread audio (atomicMax).
    Le meter lit+remet a zero (exchange) : une seule instance par source. */
class LedMeter : public juce::Component, private juce::Timer
{
public:
    enum class Orientation { vertical, horizontal };

    LedMeter (std::atomic<float>& peakSource, Orientation o = Orientation::vertical,
              int numSegments = 14, float floorDb = -60.0f);
    ~LedMeter() override { stopTimer(); }

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    std::atomic<float>& source;
    Orientation orient;
    int   nSeg;
    float floorDb, level = 0.0f, hold = 0.0f;
    int   holdCount = 0;
};
} // namespace humpire
