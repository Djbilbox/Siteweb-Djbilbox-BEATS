#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

// HumpireCore : palette commune, polices, mode Clean/Skeuo, utilitaires atomiques.
namespace humpire
{
namespace col
{
    inline const juce::Colour black   { 0xff0b0b0f };
    inline const juce::Colour panel   { 0xff15151c };
    inline const juce::Colour amber   { 0xfff5b544 };
    inline const juce::Colour orange  { 0xffff7a3d };
    inline const juce::Colour violet  { 0xff8a5cff };   // etats actifs
    inline const juce::Colour red     { 0xffff3b30 };
    inline const juce::Colour text    { 0xffe8e6df };
    inline const juce::Colour textDim { 0xff8d8b86 };
}

struct Theme
{
    /** true = UI moderne plate ("Clean"), false = skeuomorphique. */
    static bool isClean() noexcept;
    static void setClean (bool shouldBeClean) noexcept;

    /** Police LCD retro (Press Start 2P si embarquee, sinon monospace). */
    static juce::Font lcdFont (float height);
    /** Police UI (sans systeme). */
    static juce::Font uiFont (float height, bool bold = false);
};

/** Max atomique pour float (thread audio -> meters UI). */
inline void atomicMax (std::atomic<float>& a, float v) noexcept
{
    float cur = a.load (std::memory_order_relaxed);
    while (v > cur && ! a.compare_exchange_weak (cur, v, std::memory_order_relaxed)) {}
}
} // namespace humpire
