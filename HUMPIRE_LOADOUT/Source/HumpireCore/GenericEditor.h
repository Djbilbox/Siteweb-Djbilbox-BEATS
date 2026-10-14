#pragma once
#include "ScaledEditor.h"
#include "TopBar.h"
#include "FilmstripKnob.h"
#include "ConsoleButton.h"
#include "HumpireProcessor.h"

namespace humpire
{
enum class BodyStyle { arcade, rack, gamer, speaker, amp, cartridge, controller };

struct KnobSpec   { const char* id; const char* label; bool advanced = false; };
/** bool => toggle ; choice => clic = valeur suivante ; momentary => actif tant que presse. */
struct ButtonSpec { const char* id; const char* label; bool momentary = false; };

struct UiSpec
{
    juce::String title;                  // sous-titre de la barre
    juce::String subtitle;               // texte grave sur le corps
    BodyStyle style = BodyStyle::rack;
    juce::Colour body1 { 0xff2a2a30 }, body2 { 0xff0f0f13 }, accent { 0xfff5b544 };
    std::vector<KnobSpec> knobs;
    std::vector<ButtonSpec> buttons;
    juce::Rectangle<int> display { 60, 58, 700, 236 };
    int knobY = 318, buttonY = 452;
};

/** Editeur "objet physique" generique : corps dessine en code (cache 2x), ecran d'affichage specifique
    (paintDisplay), rangee de knobs, rangee de boutons, mode Simple/Avance + Clean. */
class GenericEditor : public ScaledEditor, protected juce::Timer
{
public:
    static constexpr int W = 820, H = 560;

    GenericEditor (HumpireProcessor&, UiSpec);
    ~GenericEditor() override;

protected:
    /** Dessine le contenu de l'ecran (coordonnees de content). t = secondes. */
    virtual void paintDisplay (juce::Graphics&, juce::Rectangle<float> area, float t) = 0;
    /** Appele a 30 Hz (thread message). */
    virtual void tick() {}
    /** Pour ajouter des composants specifiques (pads, sliders...). Appeler apres la construction de la base. */
    void finishSetup() { layoutKnobs(); }

    HumpireProcessor& proc;
    UiSpec spec;
    juce::Rectangle<int> displayRect() const { return spec.display; }

private:
    struct KnobSlot
    {
        FilmstripKnob knob;
        juce::String label;
        bool advanced = false;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };
    struct BtnSlot
    {
        std::unique_ptr<ConsoleButton> btn;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attach;
        juce::String paramId;
        bool isChoice = false;
    };

    void timerCallback() final;
    void layoutKnobs();
    void paintBody (juce::Graphics&);
    void drawBodyCached (juce::Graphics&);
    void drawBodyClean (juce::Graphics&);

    TopBar topBar;
    std::vector<std::unique_ptr<KnobSlot>> knobs;
    std::vector<BtnSlot> btns;
    juce::TextButton simpleBtn { "SIMPLE" }, cleanBtn { "CLEAN" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> simpleAtt, cleanAtt;
    juce::Image bodyImg;
    bool lastSimple = false, lastClean = false;
    double t0 = juce::Time::getMillisecondCounterHiRes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor)
};
} // namespace humpire
