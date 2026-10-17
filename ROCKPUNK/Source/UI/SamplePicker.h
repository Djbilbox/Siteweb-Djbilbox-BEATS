#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Palette.h"
#include "RockpunkLookAndFeel.h"

namespace rk
{

class PluginProcessor;

//==============================================================================
// Sample selector strip: a dropdown of the Player sample-library guitar WAVs
// (bound to the "smpSel" parameter) plus a live readout of the loaded file and
// its auto-detected root note. Header-only, mirroring the other UI widgets.
//==============================================================================
class SamplePicker final : public juce::Component
{
public:
    SamplePicker (PluginProcessor& p, juce::AudioProcessorValueTreeState& st, RockpunkLookAndFeel& rlf)
        : proc (p), apvts (st)
    {
        title.setText ("SAMPLE", juce::dontSendNotification);
        title.setFont (juce::Font (10.0f).boldened());
        title.setColour (juce::Label::textColourId, Palette::textDim());
        addAndMakeVisible (title);

        info.setFont (juce::Font (10.0f));
        info.setColour (juce::Label::textColourId, Palette::text());
        // Un dossier vide sans moyen de l'ouvrir laisse l'utilisateur devant une
        // liste muette : la ligne d'etat est cliquable et mene au dossier.
        info.setInterceptsMouseClicks (true, false);
        info.addMouseListener (this, false);
        info.setTooltip (proc.getUserSampleDirectory().getFullPathName());
        addAndMakeVisible (info);

        combo.setLookAndFeel (&rlf);
        combo.setColour (juce::ComboBox::backgroundColourId, Palette::ink());
        combo.setColour (juce::ComboBox::textColourId, Palette::text());
        combo.setColour (juce::ComboBox::arrowColourId, Palette::orange());
        combo.setColour (juce::ComboBox::outlineColourId, Palette::chromeHi());
        combo.setTextWhenNoChoicesAvailable ("dossier vide");
        combo.setTextWhenNothingSelected ("dossier vide");
        combo.setTooltip ("Vos propres enregistrements, transposes sur tout le clavier.\n"
                          "Cliquez sur la ligne du dessous pour ouvrir le dossier.");

        // La banque arrive groupee par categorie (Electric / Acoustic / Player) :
        // on insere un titre a chaque changement. getNumItems() ignore les
        // titres, donc l'identifiant reste bien index + 1.
        const auto names = proc.getSampleFiles();
        juce::String currentCategory;
        for (int i = 0; i < names.size(); ++i)
        {
            const auto category = proc.getSampleCategory (i);
            if (category.isNotEmpty() && category != currentCategory)
            {
                currentCategory = category;
                combo.addSectionHeading (category.toUpperCase());
            }

            combo.addItem (displayName (names[i]), i + 1);
        }

        combo.onChange = [this] { select (combo.getSelectedId() - 1); };
        addAndMakeVisible (combo);

        syncFromParam();
    }

    void resized() override
    {
        title.setBounds (0, 0, 58, 14);
        combo.setBounds (62, 0, 172, 20);
        info.setBounds (0, 24, 240, 14);
    }

    void paint (juce::Graphics& g) override
    {
        // Un panneau plein masquait l'artwork sur toute sa surface. Seule la
        // liste a besoin d'un fond lisible : le reste laisse voir l'image.
        const auto box = combo.getBounds().toFloat().expanded (2.0f);
        g.setColour (Palette::ink().withAlpha (0.80f));
        g.fillRoundedRectangle (box, 4.0f);
        g.setColour (Palette::chrome().withAlpha (0.45f));
        g.drawRoundedRectangle (box, 4.0f, 1.0f);
    }

    // Keep the dropdown + readout in sync with the parameter (e.g. after loading
    // a preset). Called from the editor timer.
    void syncFromParam()
    {
        if (auto* raw = apvts.getRawParameterValue ("smpSel"))
        {
            const int sel = (int) raw->load();
            if (sel != lastSel)
            {
                lastSel = sel;
                if (combo.getSelectedId() != sel + 1)
                    combo.setSelectedId (sel + 1, juce::dontSendNotification);
            }
        }

        info.setText (describeLoaded(), juce::dontSendNotification);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        auto dir = proc.getUserSampleDirectory();
        dir.createDirectory();
        dir.revealToUser();
    }

private:
    void select (int index)
    {
        if (index < 0 || index >= proc.getNumSampleFiles())
            return;

        // Passer par la plage du parametre plutot qu'un diviseur ecrit en dur :
        // le nombre d'entrees de la banque n'est pas celui de la plage.
        if (auto* p = apvts.getParameter ("smpSel"))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 ((float) index));
    }

    static juce::String midiNoteName (float midi)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        const int m = juce::jlimit (0, 127, (int) std::round (midi));
        return juce::String (names[m % 12]) + juce::String ((m / 12) - 1);
    }

    // Les entrees venues de la bibliotheque Player s'appellent toutes
    // "<quelque chose> Guitar" : le suffixe n'apprend rien dans une liste de
    // guitares, il est retire a l'affichage seulement.
    static juce::String displayName (const juce::String& name)
    {
        const auto shortName = name.upToFirstOccurrenceOf (" Guitar", false, false).trim();
        return shortName.isEmpty() ? name : shortName;
    }

    juce::String describeLoaded() const
    {
        auto name = proc.getCurrentSampleName();
        if (name.isEmpty())
            return proc.getNumSampleFiles() == 0
                ? juce::String ("deposez vos .wav ici - cliquez pour ouvrir")
                : juce::String ("aucun sample");

        // juce::String(const char*) lit les octets en Latin-1 : un caractere
        // UTF-8 ecrit en dur ici ressortait en mojibake a l'ecran. ASCII only.
        return displayName (name) + "   " + midiNoteName (proc.getSampleRootNote());
    }

    PluginProcessor& proc;
    juce::AudioProcessorValueTreeState& apvts;

    juce::Label title, info;
    juce::ComboBox combo;
    int lastSel = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplePicker)
};

} // namespace rk