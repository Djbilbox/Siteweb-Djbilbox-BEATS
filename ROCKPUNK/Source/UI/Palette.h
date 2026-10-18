#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace rk
{

//==============================================================================
// ROCKPUNK colours. Sampled from the artwork, so that what the editor draws
// matches what is printed on the faceplate.
//==============================================================================
struct Palette
{
    static juce::Colour pink()        { return juce::Colour (0xFFEB588E); }   // preset text, arrows
    static juce::Colour pinkHot()     { return juce::Colour (0xFFFF4F9A); }   // highlights and glows
    static juce::Colour label()       { return juce::Colour (0xFFD2D2D2); }   // printed labels
    static juce::Colour labelDim()    { return juce::Colour (0xFF8E8E92); }
    static juce::Colour ink()         { return juce::Colour (0xFF0B0A0A); }

    static juce::Colour popupBack()   { return juce::Colour (0xFF18131A); }
    static juce::Colour popupText()   { return juce::Colour (0xFFE9DEE4); }
};

} // namespace rk
