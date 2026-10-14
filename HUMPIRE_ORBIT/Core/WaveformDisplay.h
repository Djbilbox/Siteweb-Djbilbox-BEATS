#pragma once
#include <array>
#include <atomic>
#include <juce_gui_basics/juce_gui_basics.h>
#include "HumpireTheme.h"

/** Historique lock-free d'enveloppes (pic) : ecrit par le thread audio, lu par l'UI. */
class WaveformFifo
{
public:
    static constexpr int size = 256;
    void push (float peak) noexcept
    {
        const int h = head.load (std::memory_order_relaxed);
        data[(size_t) h].store (peak, std::memory_order_relaxed);
        head.store ((h + 1) % size, std::memory_order_release);
    }
    /** Copie ordonnee (ancien -> recent). */
    void copyTo (float* dst) const noexcept
    {
        const int h = head.load (std::memory_order_acquire);
        for (int i = 0; i < size; ++i)
            dst[i] = data[(size_t) ((h + i) % size)].load (std::memory_order_relaxed);
    }
private:
    std::array<std::atomic<float>, size> data {};
    std::atomic<int> head { 0 };
};

/** Forme d'onde miroir cosmique (enveloppe defilante). */
class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay (const WaveformFifo& f, juce::Colour c = humpire::theme::violet) : fifo (f), colour (c) { startTimerHz (30); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (humpire::theme::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (humpire::theme::edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        const float mid = r.getCentreY(), amp = r.getHeight() * 0.46f;
        juce::Path p;
        p.startNewSubPath (r.getX(), mid);
        for (int i = 0; i < WaveformFifo::size; ++i)
        {
            const float x = r.getX() + r.getWidth() * (float) i / (float) (WaveformFifo::size - 1);
            p.lineTo (x, mid - amp * juce::jlimit (0.0f, 1.0f, buf[i] * 1.6f));
        }
        for (int i = WaveformFifo::size - 1; i >= 0; --i)
        {
            const float x = r.getX() + r.getWidth() * (float) i / (float) (WaveformFifo::size - 1);
            p.lineTo (x, mid + amp * juce::jlimit (0.0f, 1.0f, buf[i] * 1.6f));
        }
        p.closeSubPath();
        g.setGradientFill (juce::ColourGradient (colour.withAlpha (0.15f), r.getX(), 0, colour.interpolatedWith (humpire::theme::amber, 0.6f).withAlpha (0.85f), r.getRight(), 0, false));
        g.fillPath (p);
        g.setColour (colour.withAlpha (0.35f));
        g.drawHorizontalLine ((int) mid, r.getX() + 6, r.getRight() - 6);
    }

private:
    void timerCallback() override { fifo.copyTo (buf); repaint(); }
    const WaveformFifo& fifo;
    juce::Colour colour;
    float buf[WaveformFifo::size] {};
};
