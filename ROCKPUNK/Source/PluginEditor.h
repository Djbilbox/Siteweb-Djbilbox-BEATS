#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "UI/AnimatedKnob.h"
#include "UI/CyclingSwitch.h"
#include "UI/ExpansionCassette.h"
#include "UI/LevelSlider.h"
#include "UI/Palette.h"
#include "UI/PresetBar.h"
#include "UI/RockpunkLookAndFeel.h"
#include "UI/SamplePicker.h"
#include "UI/VirtualPiano.h"
#include "UI/VuMeter.h"

namespace rk
{

class PluginProcessor;

class PluginEditor final : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    static constexpr float designW = 1120.0f;
    static constexpr float designH = 836.0f;

    void loadLayout();
    void buildSprites();
    void place (const juce::String& id, juce::Component& c);
    void applyGlow (float g);

    PluginProcessor& proc;
    juce::AudioProcessorValueTreeState& apvts;

    // sprite source of truth: crops + filmstrips built from the artwork
    RockpunkLookAndFeel lf;

    juce::Image backgroundImage;
    std::map<juce::String, juce::Rectangle<float>> positions;
    std::map<juce::String, juce::var> spriteOverrides;

    // addAndMakeVisible() ne prend pas possession du pointeur qu'on lui passe :
    // sans ce proprietaire, chaque composant cree ici fuyait a la fermeture de
    // la fenetre. Tout ce qui est alloue par l'editeur y est depose.
    juce::OwnedArray<juce::Component> ownedComponents;

    template <typename T>
    T* own (T* c)
    {
        ownedComponents.add (c);
        return c;
    }

    // knobs (artwork: LFO row, centre GLIDE/FILTER, right VOL..FB)
    AnimatedKnob* knobLfoRate = nullptr;
    AnimatedKnob* knobLfoAmount = nullptr;
    AnimatedKnob* knobLfoPitch = nullptr;
    AnimatedKnob* knobGlide = nullptr;
    AnimatedKnob* knobFilter = nullptr;
    AnimatedKnob* knobVol = nullptr;
    AnimatedKnob* knobReverb = nullptr;
    AnimatedKnob* knobChorus = nullptr;
    AnimatedKnob* knobDist = nullptr;
    AnimatedKnob* knobTime = nullptr;
    AnimatedKnob* knobFb = nullptr;

    // vertical sliders (ADSR row + right faders) and wheels
    LevelSlider* slAttack = nullptr;
    LevelSlider* slDecay = nullptr;
    LevelSlider* slSustain = nullptr;
    LevelSlider* slRelease = nullptr;
    LevelSlider* slFA = nullptr;
    LevelSlider* slFS = nullptr;
    LevelSlider* slFD = nullptr;
    LevelSlider* slFR = nullptr;
    LevelSlider* slFVel = nullptr;
    LevelSlider* wheelPitch = nullptr;
    LevelSlider* wheelMod = nullptr;

    // speaker grills double as the layer on/off switches
    CyclingSwitch* grillA = nullptr;
    CyclingSwitch* grillB = nullptr;

    // dropdown combos (crop sprites, discrete states)
    CyclingSwitch* swWf = nullptr;
    CyclingSwitch* swLfoSrc = nullptr;
    CyclingSwitch* swDest = nullptr;
    CyclingSwitch* swFilterType = nullptr;
    CyclingSwitch* swMonoPoly = nullptr;

    VuMeter* vuL = nullptr;
    VuMeter* vuR = nullptr;

    SamplePicker* samplePicker = nullptr;
    PresetBar* presetBar = nullptr;

    ExpansionCassette* cassette = nullptr;
    VirtualPiano* piano = nullptr;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<AnimatedKnob*> glowKnobs;
    std::vector<LevelSlider*> glowSliders;
    std::vector<CyclingSwitch*> glowSwitches;
    std::vector<VuMeter*> glowVus;

    std::set<int> lastNotes;
    float glow = 0.0f;
    float flash = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

} // namespace rk