#pragma once
#include "HumpireTheme.h"
#include <array>

namespace humpire
{
/** Tampon lock-free audio -> UI : 64 colonnes d'amplitude (0..1), ecrit par le thread audio. */
struct ScopeBuffer
{
    static constexpr int size = 64;

    ScopeBuffer() { for (auto& c : col) c.store (0.0f); }

    void push (float v) noexcept
    {
        const int h = head.load (std::memory_order_relaxed);
        col[(size_t) h].store (v, std::memory_order_relaxed);
        head.store ((h + 1) % size, std::memory_order_release);
    }
    /** i = 0 : colonne la plus ancienne. */
    float get (int i) const noexcept
    {
        const int h = head.load (std::memory_order_acquire);
        return col[(size_t) ((h + i) % size)].load (std::memory_order_relaxed);
    }

    std::array<std::atomic<float>, (size_t) size> col;
    std::atomic<int> head { 0 };
};

/** Ecran LCD olive 4 teintes, forme d'onde pixelisee + 2 lignes de texte retro. */
class LcdScreen : public juce::Component, private juce::Timer
{
public:
    explicit LcdScreen (const ScopeBuffer* scopeSource);
    ~LcdScreen() override { stopTimer(); }

    void setTexts (const juce::String& top, const juce::String& bottom);
    void setDimmed (bool shouldDim);   // bypass : ecran eteint

    void paint (juce::Graphics&) override;

    // 4 teintes originales (olive) : claire -> sombre
    static inline const juce::Colour tone0 { 0xffc4cf86 }, tone1 { 0xff97a85a },
                                     tone2 { 0xff5d7336 }, tone3 { 0xff263519 };
private:
    void timerCallback() override { repaint(); }

    const ScopeBuffer* scope;
    juce::String topText, bottomText;
    bool dimmed = false;
};
} // namespace humpire
