#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Theme HUMPIRE : anthracite, or, terre cuite.
namespace humpire::theme
{
inline const juce::Colour bg         { 0xff141416 };
inline const juce::Colour panel      { 0xff1c1c20 };
inline const juce::Colour panelHi    { 0xff2a2a30 };
inline const juce::Colour gold       { 0xfff5b544 };
inline const juce::Colour terracotta { 0xffc1440e };
inline const juce::Colour text       { 0xffeee6d6 };
inline const juce::Colour textDim    { 0xff8d8678 };
inline const juce::Colour ok         { 0xff6fbf73 };

inline juce::Font font (float height, bool bold = false)
{
    return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain));
}
} // namespace humpire::theme
