#pragma once
#include <array>
#include "Core/HumpireEditorBase.h"
#include "Core/OrbitKnob.h"
#include "Core/TopBar.h"
#include "Core/WaveformDisplay.h"
#include "Core/XpBar.h"
#include "PluginProcessor.h"
#include "TwinOrbitView.h"
#include "TwinWidgets.h"

/** Editeur HUMPIRE TWIN (960 x 600, redimensionnable 50-200 %), meme langage visuel que SOLAR. */
class TwinEditor : public HumpireEditorBase
{
public:
    explicit TwinEditor (TwinAudioProcessor&);
    ~TwinEditor() override = default;

private:
    void applyVoices (int n);
    void applySimple (bool simple);

    TwinAudioProcessor& proc;

    HumpireTopBar topBar;
    TwinOrbitView orbitView;
    WaveformDisplay waveform;
    CorrelationMeter corrMeter;
    TwinVoiceStrip voiceStrip;
    XpBar xpBar;

    juce::Label voicesLabel;
    std::array<juce::TextButton, twin::kMaxVoices> voiceBtns;
    juce::ToggleButton simpleBtn { "SIMPLE" };

    OrbitKnob detune { "Detune" }, delay { "Delay" }, spread { "Spread" },
              humanize { "Humanize", humpire::theme::violet }, drift { "Drift", humpire::theme::violet },
              width { "Width", humpire::theme::cyan }, lowCut { "Low Cut", humpire::theme::cyan }, highCut { "High Cut", humpire::theme::cyan },
              mix { "Mix", humpire::theme::ember }, inGain { "Input", humpire::theme::ember }, outGain { "Output", humpire::theme::ember };

    juce::ParameterAttachment voicesAttach, simpleAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwinEditor)
};
