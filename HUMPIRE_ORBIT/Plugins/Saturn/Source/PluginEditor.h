#pragma once
#include "Core/HumpireEditorBase.h"
#include "Core/OrbitKnob.h"
#include "Core/TopBar.h"
#include "Core/WaveformDisplay.h"
#include "Core/XpBar.h"
#include "PluginProcessor.h"
#include "SaturnOrbitView.h"
#include "SaturnReadout.h"

/** Editor of HUMPIRE SATURN (960 x 600, resizable 50-200 %, locked aspect ratio). */
class SaturnEditor : public HumpireEditorBase
{
public:
    explicit SaturnEditor (SaturnAudioProcessor&);
    ~SaturnEditor() override = default;

private:
    void applyMode (bool semitones);
    void applySimple (bool simple);

    juce::TooltipWindow tooltipWindow { this, 600 };    // the tooltips below only show up with one of these

    HumpireTopBar topBar;
    SaturnOrbitView orbit;
    WaveformDisplay waveform;
    SaturnReadout readout;
    XpBar xpBar;

    juce::TextButton degreesBtn { "DEGREES" }, semitonesBtn { "SEMITONES" }, simpleBtn { "SIMPLE" };
    juce::ComboBox keyBox, scaleBox;

    juce::OwnedArray<juce::ToggleButton> voiceBtns;
    juce::OwnedArray<OrbitKnob> semiKnobs, degKnobs, levelKnobs, panKnobs, detuneKnobs;
    OrbitKnob glideKnob { "Glide" }, mixKnob { "Mix" }, inGainKnob { "Input", humpire::theme::ember },
              outGainKnob { "Output", humpire::theme::ember };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttach, scaleAttach;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> voiceAttach;
    juce::ParameterAttachment modeAttach, simpleAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnEditor)
};
