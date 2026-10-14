#include "HumpireTheme.h"

#if HUMPIRE_HAS_FONTS
 #include "HumpireFontsData.h"
#endif

namespace humpire
{
static std::atomic<bool> gClean { false };

bool Theme::isClean() noexcept          { return gClean.load (std::memory_order_relaxed); }
void Theme::setClean (bool c) noexcept  { gClean.store (c, std::memory_order_relaxed); }

juce::Font Theme::lcdFont (float h)
{
#if HUMPIRE_HAS_FONTS
    static const juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor (
        HumpireFontsData::PressStart2PRegular_ttf, (size_t) HumpireFontsData::PressStart2PRegular_ttfSize);
    if (tf != nullptr)
        return juce::Font (juce::FontOptions (tf).withHeight (h));
#endif
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h, juce::Font::bold));
}

juce::Font Theme::uiFont (float h, bool bold)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}
} // namespace humpire
