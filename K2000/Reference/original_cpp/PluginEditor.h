#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/K2000LookAndFeel.h"
#include "UI/PitchModWheels.h"
#include "UI/NeonKeyboard.h"
#include "UI/CassetteSlot.h"

class K2000AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit K2000AudioProcessorEditor(K2000AudioProcessor&);
    ~K2000AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    K2000AudioProcessor& processor;
    K2000UI::K2000LookAndFeel customLookAndFeel;

    // UI Modules
    K2000UI::CassetteSlot cassetteSlot;
    K2000UI::PitchModWheels wheels;
    K2000UI::NeonKeyboard keyboard;

    // Header Controls
    juce::TextButton prevPatchBtn { "<" };
    juce::TextButton nextPatchBtn { ">" };
    juce::TextButton browsePatchBtn { "📁" };
    juce::TextButton savePatchBtn { "💾" };
    juce::Label patchDisplay;

    // Knobs
    juce::Slider lfoRateSlider;
    juce::Slider lfoAmountSlider;
    juce::ToggleButton lfoPitchToggle;

    juce::Slider volSlider;
    juce::Slider revSlider;
    juce::Slider choSlider;
    juce::Slider distMasterSlider;

    juce::Slider glideSlider;
    juce::Slider filterSlider;
    juce::Slider filterDriveSlider;
    juce::Slider delayMixSlider;
    juce::Slider delayTimeSlider;
    juce::Slider delayFbSlider;

    // Faders
    juce::Slider lfoAttackSlider;
    juce::Slider lfoDecaySlider;
    juce::Slider lfoSustainSlider;
    juce::Slider lfoReleaseSlider;

    juce::Slider ampAttackSlider;
    juce::Slider ampDecaySlider;
    juce::Slider ampSustainSlider;
    juce::Slider velBankSlider;
    juce::Slider velAdsrSlider;

    // Dropdowns
    juce::ComboBox waveCombo;
    juce::ComboBox lfoSourceCombo;
    juce::ComboBox lfoDestCombo;
    juce::ComboBox filterTypeCombo;
    juce::ComboBox voiceModeCombo;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;

    void setupKnob(juce::Slider& slider, const juce::String& paramId, const juce::String& labelText);
    void setupFader(juce::Slider& slider, const juce::String& paramId, const juce::String& labelText);
    void setupCombo(juce::ComboBox& combo, const juce::String& paramId, const juce::StringArray& items);

    void updatePatchDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(K2000AudioProcessorEditor)
};
