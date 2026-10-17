#pragma once

#include <juce_core/juce_core.h>

#include <cmath>
#include <vector>

namespace rk
{

//==============================================================================
// Karplus-Strong plucked string. One instance per string (per active note).
// The excitation is a short burst of filtered noise (the "pick"); the loop is a
// one-sample delay with a two-tap average (harmonics) and a one-pole damping
// filter. Cheap enough to run 8 polyphonic voices x 2 layers.
//==============================================================================
class GuitarString
{
public:
    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr;
        const size_t maxLen = (size_t) ((sr * 0.12) + 4.0); // covers down to ~8 Hz
        buffer.resize (maxLen, 0.0f);
    }

    void reset() noexcept
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        length = 2;
        writePos = 0;
        lastLow = 0.0f;
        lastNoise = 0.0f;
    }

    void setFrequency (float freqHz) noexcept
    {
        const float len = (float) sampleRate / std::max (20.0f, freqHz);
        length = juce::jlimit (2, (int) buffer.size() - 1, (int) (len + 0.5f));
        if (writePos >= length)
            writePos %= length;
    }

    // Pluck the string. velocity 0..1, brightness 0..1 controls the harmonic
    // content of the excitation (higher = sharper pick / more overtones).
    void pluck (float velocity, float brightness) noexcept
    {
        const float vel    = std::min (1.0f, std::max (0.0f, velocity));
        const float bright = std::min (1.0f, std::max (0.05f, brightness));

        for (int i = 0; i < length; ++i)
        {
            float noise = rng.nextFloat() * 2.0f - 1.0f;
            noise = lastNoise + bright * (noise - lastNoise);
            lastNoise = noise;
            buffer[(size_t) i] = noise * vel;
        }
        writePos = 0;
    }

    // One sample of the loop. damping 0..1 (1 = brightest, 0 = dullest),
    // loopGain must stay < 1 (higher = longer sustain).
    float process (float damping, float loopGain) noexcept
    {
        const float y0 = buffer[(size_t) writePos];
        int rp = writePos + 1;
        if (rp >= length)
            rp = 0;
        const float y1 = buffer[(size_t) rp];

        const float out = (y0 + y1) * 0.5f;
        const float damped = lastLow + damping * (out - lastLow);
        lastLow = damped;

        buffer[(size_t) writePos] = damped * loopGain;

        writePos = rp;
        return damped;
    }

private:
    double sampleRate = 48000.0;
    std::vector<float> buffer;
    juce::Random rng;
    int length = 2;
    int writePos = 0;
    float lastLow = 0.0f;
    float lastNoise = 0.0f;
};

//==============================================================================
// Simple resonant bandpass modelling the guitar body / air resonance.
// Adds the woody low-end "thump" of the instrument.
//==============================================================================
class BodyResonator
{
public:
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void reset() noexcept
    {
        x1 = x2 = y1 = y2 = 0.0f;
    }

    // freqHz ~ 80..400, q ~ 2..8
    void setParams (float freqHz, float q) noexcept
    {
        const float w0 = 6.28318530718f
            * std::min (0.45f * (float) sampleRate, std::max (20.0f, freqHz))
            / (float) sampleRate;
        const float alpha = std::sin (w0) / (2.0f * std::max (0.5f, q));
        b0 = alpha;
        b2 = -alpha;
        a1 = -2.0f * std::cos (w0);
        a2 = 1.0f - alpha;
        a0 = 1.0f + alpha;
    }

    float process (float x) noexcept
    {
        const float y = (b0 * x + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }

private:
    double sampleRate = 48000.0;
    float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
    float b0 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f, a0 = 1.0f;
};

} // namespace rk