#pragma once
#include "HumpireTheme.h"

namespace humpire
{
/** Pad XY lie a 2 parametres (stick de manette ou pad carre). Double-clic = reset.
    springBack : le stick revient au centre/defaut au relachement. */
class XYPad : public juce::Component
{
public:
    XYPad (juce::AudioProcessorValueTreeState& apvts, const juce::String& xId, const juce::String& yId,
           bool round = true, bool springBack = false);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    void setFromMouse (const juce::MouseEvent&);
    void resetToDefault();

    juce::RangedAudioParameter* px;
    juce::RangedAudioParameter* py;
    juce::ParameterAttachment ax, ay;
    float nx = 0.5f, ny = 0.5f;
    bool  isRound, spring;
};
} // namespace humpire
