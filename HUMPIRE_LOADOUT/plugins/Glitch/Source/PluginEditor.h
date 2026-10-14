#pragma once
#include <HumpireCore/ScaledEditor.h>
#include <HumpireCore/TopBar.h>
#include <HumpireCore/LcdScreen.h>
#include <HumpireCore/FilmstripKnob.h>
#include <HumpireCore/ConsoleButton.h>
#include "PluginProcessor.h"

/** Console portable retro ORIGINALE : ecran LCD olive, croix, boutons A/B, pilules start/select, grille HP.
    Mapping des boutons :
      Croix haut/bas = Bits +/-1      Croix gauche/droite = Rate x/÷ 1.26 (tierce majeure environ)
      A = cycle Dither (Off/RPDF/TPDF)    B = Speaker ON/OFF
      SELECT = Simple/Avance              START = Clean/Skeuo                               */
class GlitchEditor : public humpire::ScaledEditor, private juce::Timer
{
public:
    explicit GlitchEditor (GlitchAudioProcessor&);
    ~GlitchEditor() override;

private:
    struct KnobSlot
    {
        humpire::FilmstripKnob knob;
        juce::String label;
        const char*  paramId = "";
        bool advancedOnly = false;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void timerCallback() override;
    void layoutKnobs();
    void paintBody (juce::Graphics&);
    void drawBodyProcedural (juce::Graphics&);   // rendu cache 2x
    void drawBodyClean (juce::Graphics&);
    void stepParam (const char* paramId, float delta, bool multiplicative);
    void cycleDither();
    bool readBool (const char* paramId) const;

    GlitchAudioProcessor& proc;
    humpire::TopBar topBar;
    humpire::LcdScreen lcd;
    std::array<KnobSlot, 6> knobs;
    humpire::DPad dpad;
    humpire::ConsoleButton btnA, btnB, btnStart, btnSelect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attB, attStart, attSelect;

    juce::Image bodyImg;
    bool lastSimple = true, lastClean = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlitchEditor)
};
