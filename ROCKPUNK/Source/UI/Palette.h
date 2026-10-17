#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace rk
{

//==============================================================================
// ROCKPUNK dark UI palette. Every colour in the interface is defined here so
// visual tuning happens in one place.
//==============================================================================
struct Palette
{
    static juce::Colour bg()          { return juce::Colour (0xFF0B0A0A); }
    static juce::Colour panel()       { return juce::Colour (0xFF161312); }
    static juce::Colour panelLight()  { return juce::Colour (0xFF201C1A); }
    static juce::Colour chrome()      { return juce::Colour (0xFF3A3A38); }
    static juce::Colour chromeHi()    { return juce::Colour (0xFF5A5550); }
    static juce::Colour red()         { return juce::Colour (0xFFE8302A); }
    static juce::Colour orange()      { return juce::Colour (0xFFFF7A1A); }
    static juce::Colour text()        { return juce::Colour (0xFFFFF2D0); }
    static juce::Colour textDim()     { return juce::Colour (0xFF8A857C); }
    static juce::Colour ink()         { return juce::Colour (0xFF0B0A0A); }
    static juce::Colour off()         { return juce::Colour (0xFF22201E); }

    // neon "note energy" colours used when a note is triggered
    static juce::Colour neonViolet()  { return juce::Colour (0xFFA45BFF); }
    static juce::Colour neonRose()    { return juce::Colour (0xFFFF4FD8); }

    static juce::Colour whiteKey()    { return juce::Colour (0xFFE6DCC8); }
};

} // namespace rk