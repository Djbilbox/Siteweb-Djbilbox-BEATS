#pragma once
#include "Core/HumpireEditorBase.h"
#include "Core/OrbitKnob.h"
#include "Core/TopBar.h"
#include "Core/WaveformDisplay.h"
#include "Core/XpBar.h"
#include "PluginProcessor.h"
#include "SunOrbitView.h"
#include "PitchCurveEditor.h"

/** Editeur HUMPIRE SOLAR (960 x 600, redimensionnable 50-200 %). */
class SolarEditor : public HumpireEditorBase
{
public:
    explicit SolarEditor (SolarAudioProcessor&);
    ~SolarEditor() override = default;

private:
    void applyMode (bool graph);
    void applySimple (bool simple);

    SolarAudioProcessor& proc;

    HumpireTopBar topBar;
    SunOrbitView sunView;
    PitchCurveEditor curveView;
    WaveformDisplay waveform;
    XpBar xpBar;

    juce::TextButton autoBtn { "AUTO" }, graphBtn { "GRAPH" }, snapBtn { "SNAP GAMME" }, resetBtn { "RESET" };
    juce::ToggleButton simpleBtn { "SIMPLE" };
    juce::ComboBox keyBox, scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttach, scaleAttach;

    OrbitKnob retune { "Retune" }, humanize { "Humanize" }, amount { "Amount" }, flex { "Flex-Tune" },
              vibDepth { "Vib Depth", humpire::theme::violet }, vibRate { "Vib Rate", humpire::theme::violet },
              vibDelay { "Vib Delay", humpire::theme::violet }, portamento { "Portamento", humpire::theme::cyan },
              mix { "Mix", humpire::theme::ember }, inGain { "Input", humpire::theme::ember },
              outGain { "Output", humpire::theme::ember }, refA { "Ref A", humpire::theme::textLo };

    juce::OwnedArray<juce::ToggleButton> noteButtons;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> noteAttach;

    juce::ParameterAttachment modeAttach, simpleAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolarEditor)
};
