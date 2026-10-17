#pragma once

#include <cmath>

namespace rk
{

//==============================================================================
// Exponential ADSR envelope.
// Time parameters are in milliseconds. Segments are exponential; attack reaches
// ~99% in "attackMs", decay settles to sustain in "decayMs", release falls by ~60 dB
// in "releaseMs".
//==============================================================================
class Adsr
{
public:
    enum class State { idle, attack, decay, sustain, release };

    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    void reset() noexcept
    {
        state = State::idle;
        value = 0.0f;
    }

    void noteOn() noexcept
    {
        if (state == State::idle)
            value = 0.0f;
        state = State::attack;
    }

    void noteOff() noexcept
    {
        if (state != State::idle)
            state = State::release;
    }

    bool isIdle() const noexcept { return state == State::idle; }
    bool isActive() const noexcept { return state != State::idle && value > 0.0005f; }
    float getValue() const noexcept { return value; }

    void updateTimes (float attackMs, float decayMs, float sustainLevel, float releaseMs) noexcept
    {
        sustain = std::min (1.0f, std::max (0.0f, sustainLevel));

        const float srMs = (float) sampleRate * 0.001f;

        attackCoef  = attackMs  <= 1.0f ? 1.0f : (1.0f - std::exp (-4.6f / (attackMs  * srMs)));
        decayCoef   = decayMs   <= 1.0f ? 0.0f : std::exp (-4.6f / (decayMs   * srMs));
        releaseCoef = releaseMs <= 1.0f ? 0.0f : std::exp (-9.2f / (releaseMs * srMs));
    }

    float process() noexcept
    {
        switch (state)
        {
            case State::attack:
                value += (1.0f - value) * attackCoef;
                if (value >= 0.995f) { value = 1.0f; state = State::decay; }
                break;

            case State::decay:
                value = sustain + (value - sustain) * decayCoef;
                if (std::fabs (value - sustain) < 0.0005f) { value = sustain; state = State::sustain; }
                break;

            case State::sustain:
                value = sustain;
                break;

            case State::release:
                value *= releaseCoef;
                if (value <= 0.0005f) { value = 0.0f; state = State::idle; }
                break;

            case State::idle:
                value = 0.0f;
                break;
        }
        return value;
    }

private:
    State  state = State::idle;
    double sampleRate = 48000.0;
    float  value = 0.0f;
    float  sustain = 0.8f;
    float  attackCoef = 1.0f;
    float  decayCoef = 0.999f;
    float  releaseCoef = 0.999f;
};

} // namespace rk