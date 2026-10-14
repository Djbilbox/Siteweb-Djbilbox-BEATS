#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Theme cosmique HUMPIRE : palette + polices (Space Grotesk / Inter / Press Start 2P,
// avec repli automatique sur la police systeme si elles ne sont pas installees).
namespace humpire::theme
{
    inline const juce::Colour bg        { 0xff050508 };   // noir profond
    inline const juce::Colour panel     { 0xff0c0c15 };
    inline const juce::Colour panelHi   { 0xff161625 };
    inline const juce::Colour edge      { 0xff26263f };
    inline const juce::Colour amber     { 0xffF5B544 };   // etoile
    inline const juce::Colour ember     { 0xffFF7A3D };   // braise
    inline const juce::Colour violet    { 0xff7C5CFF };   // nebuleuse
    inline const juce::Colour cyan      { 0xff4FD8FF };
    inline const juce::Colour good      { 0xff59E0A0 };
    inline const juce::Colour danger    { 0xffFF4D5E };
    inline const juce::Colour textHi    { 0xffF2EEE6 };
    inline const juce::Colour textLo    { 0xff8A88A0 };

    /** Renvoie `wanted` si la police est installee, sinon la police sans-serif par defaut du systeme
        (evite tout comportement indefini avec un nom de police absent). */
    inline juce::String resolveTypeface (const juce::String& wanted)
    {
        static const juce::StringArray installed = juce::Font::findAllTypefaceNames();
        return installed.contains (wanted) ? wanted : juce::Font::getDefaultSansSerifFontName();
    }

    inline juce::Font title (float h) { return juce::Font (juce::FontOptions (resolveTypeface ("Space Grotesk"), h, juce::Font::bold)); }
    inline juce::Font text  (float h) { return juce::Font (juce::FontOptions (resolveTypeface ("Inter"), h, juce::Font::plain)); }
    inline juce::Font pixel (float h) { return juce::Font (juce::FontOptions (resolveTypeface ("Press Start 2P"), h, juce::Font::plain)); }

    /** Degrade or -> ambre -> braise. */
    inline juce::ColourGradient starGradient (juce::Point<float> a, juce::Point<float> b)
    {
        return juce::ColourGradient (amber, a, ember, b, false);
    }
}
