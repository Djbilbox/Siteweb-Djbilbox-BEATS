#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace wz
{

//==============================================================================
// WARZONE808 tactical UI palette. Mat-black chassis, tactical green (left /
// LFO / bass side) and alert red (right / FX-velocity side). Every colour in
// the interface is defined here so visual tuning happens in one place.
//==============================================================================
struct Palette
{
    static juce::Colour bg()          { return juce::Colour (0xFF0A0C0A); }
    static juce::Colour panel()       { return juce::Colour (0xFF131711); }
    static juce::Colour panelLight()  { return juce::Colour (0xFF1D2418); }
    static juce::Colour chrome()      { return juce::Colour (0xFF3A3F38); }
    static juce::Colour chromeHi()    { return juce::Colour (0xFF5C6355); }
    static juce::Colour text()        { return juce::Colour (0xFFEAF2E4); }
    static juce::Colour textDim()     { return juce::Colour (0xFF87907E); }
    static juce::Colour ink()         { return juce::Colour (0xFF0A0C0A); }
    static juce::Colour off()         { return juce::Colour (0xFF1E231C); }

    // tactical green (soft zone / left / LFO / bass)
    static juce::Colour green()       { return juce::Colour (0xFF3DDC74); }
    static juce::Colour greenDeep()   { return juce::Colour (0xFF1E7A3C); }
    // alert red (extreme zone / right / FX / velocity)
    static juce::Colour red()         { return juce::Colour (0xFFE8322A); }
    static juce::Colour redDeep()     { return juce::Colour (0xFF7A1512); }

    // "note energy" neons used while a note is triggering
    static juce::Colour neonGreen()   { return juce::Colour (0xFF4CFF8F); }
    static juce::Colour neonRed()     { return juce::Colour (0xFFFF453A); }

    static juce::Colour whiteKey()    { return juce::Colour (0xFFDFE6D6); }
};

} // namespace wz
