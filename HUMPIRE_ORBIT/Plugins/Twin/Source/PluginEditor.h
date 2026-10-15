#pragma once
#include "Core/HumpireEditorBase.h"
#include "Core/OrbitKnob.h"
#include "Core/TopBar.h"
#include "Core/WaveformDisplay.h"
#include "Core/XpBar.h"
#include "PluginProcessor.h"
#include "TwinOrbitView.h"
#include "TwinWidgets.h"

/** Editor of HUMPIRE TWIN (960 x 600, resizable 50-200 %). */
class TwinEditor : public HumpireEditorBase
{
public:
    explicit TwinEditor (TwinAudioProcessor&);
    ~TwinEditor() override = default;

private:
    void applyDoubles (int voices);
    void applySimple (bool simple);

    TwinAudioProcessor& proc;

    HumpireTopBar topBar;
    TwinOrbitView orbit;
    TwinVoiceReadout readout;
    WaveformDisplay waveform;
    XpBar xpBar;

    juce::OwnedArray<juce::TextButton> doubleButtons;
    juce::TextButton reseedBtn { "RESEED" };
    juce::ToggleButton simpleBtn { "SIMPLE" };

    OrbitKnob detune { "Detune", humpire::theme::amber },
              delay { "Delay", humpire::theme::amber },
              width { "Width", humpire::theme::cyan },
              mix { "Mix", humpire::theme::ember },
              humanize { "Humanize", humpire::theme::violet },
              drift { "Drift", humpire::theme::violet },
              hpKnob { "High-Pass", humpire::theme::cyan },
              lpKnob { "Low-Pass", humpire::theme::cyan },
              outGain { "Output", humpire::theme::ember };

    juce::ParameterAttachment doublesAttach, simpleAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwinEditor)
};
