#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../PluginProcessor.h"
#include "Palette.h"

namespace rk
{

//==============================================================================
// Le bandeau de presets du haut.
//
// L'image de fond dessine un navigateur complet - dossier, fleches, nom,
// disquette - mais rien n'etait branche derriere : le plugin affichait
// eternellement "ROCKPUNK - Init Patch" et paraissait n'avoir qu'un seul son.
// Ce composant est transparent et pose ses zones cliquables exactement sur les
// elements dessines ; seul le nom du preset est ecrit, par-dessus le champ.
//==============================================================================
class PresetBar final : public juce::Component
{
public:
    explicit PresetBar (PluginProcessor& p) : proc (p)
    {
        setInterceptsMouseClicks (true, false);
    }

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }

    void timerTick() noexcept
    {
        glow += (glowTarget - glow) * 0.18f;

        const auto name = proc.getCurrentPresetName();
        if (name != shownName)
        {
            shownName = name;
            repaint();
        }
    }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        const auto field = fieldArea();

        // Le nom "ROCKPUNK - Init Patch" est grave dans l'image : sans ce
        // masque, le nom reel du preset s'ecrivait par-dessus et les deux
        // textes se chevauchaient. On repeint donc l'interieur du champ, sans
        // deborder sur son liseré rose.
        g.setColour (juce::Colour (0xff1a0d14));
        g.fillRoundedRectangle (field.reduced (2.0f, 3.0f), 3.0f);

        g.setFont (juce::Font (15.0f).boldened());
        g.setColour (Palette::neonRose().withAlpha (0.55f + 0.45f * glow));
        g.drawFittedText (proc.getCurrentPresetName().toUpperCase(),
                          field.reduced (26, 0).toNearestInt(),
                          juce::Justification::centred, 1);

        // Retour visuel du survol, cale sur la zone reellement cliquable.
        if (hot != Zone::none)
        {
            g.setColour (Palette::neonViolet().withAlpha (0.22f));
            g.fillRoundedRectangle (zoneArea (hot), 4.0f);
        }
    }

    //==========================================================================
    void mouseMove (const juce::MouseEvent& e) override
    {
        const auto z = zoneAt (e.position);
        if (z != hot)
        {
            hot = z;
            repaint();
        }
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hot = Zone::none;
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        switch (zoneAt (e.position))
        {
            case Zone::folder:  proc.getPresetDirectory().revealToUser(); break;
            case Zone::prev:    proc.stepPreset (-1); break;
            case Zone::next:    proc.stepPreset (1);  break;
            case Zone::name:    showList(); break;
            case Zone::save:    save(); break;
            case Zone::none:    break;
        }
    }

private:
    enum class Zone { none, folder, prev, name, next, save };

    // Fractions de la largeur du bandeau : elles suivent le dessin de l'image,
    // donc un redimensionnement de la fenetre les emmene avec lui.
    juce::Rectangle<float> zoneArea (Zone z) const
    {
        const auto r = getLocalBounds().toFloat();
        const float w = r.getWidth();
        const float top = r.getY() + r.getHeight() * 0.15f;
        const float h = r.getHeight() * 0.70f;

        switch (z)
        {
            case Zone::folder: return { r.getX() + w * 0.030f, top, w * 0.095f, h };
            case Zone::prev:   return { r.getX() + w * 0.155f, top, w * 0.060f, h };
            case Zone::name:   return { r.getX() + w * 0.215f, top, w * 0.560f, h };
            case Zone::next:   return { r.getX() + w * 0.775f, top, w * 0.060f, h };
            case Zone::save:   return { r.getX() + w * 0.870f, top, w * 0.105f, h };
            case Zone::none:   break;
        }
        return {};
    }

    juce::Rectangle<float> fieldArea() const { return zoneArea (Zone::name); }

    Zone zoneAt (juce::Point<float> p) const
    {
        for (auto z : { Zone::folder, Zone::prev, Zone::name, Zone::next, Zone::save })
            if (zoneArea (z).contains (p))
                return z;
        return Zone::none;
    }

    void showList()
    {
        proc.refreshPresetList();
        const auto& files = proc.getPresetFiles();

        juce::PopupMenu menu;
        if (files.isEmpty())
        {
            menu.addItem (1, "aucun preset", false, false);
        }
        else
        {
            for (int i = 0; i < files.size(); ++i)
                menu.addItem (i + 1, files[i].getFileNameWithoutExtension(),
                              true, i == proc.getCurrentPresetIndex());
        }

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                            [this] (int result)
                            {
                                if (result > 0)
                                    proc.loadPreset (result - 1);
                            });
    }

    void save()
    {
        auto dir = proc.getPresetDirectory();
        juce::FileChooser fc ("Enregistrer le preset ROCKPUNK",
                              dir.getChildFile ("Mon preset.rpk"), "*.rpk");

        if (fc.browseForFileToSave (true))
            proc.savePreset (fc.getResult());
    }

    PluginProcessor& proc;
    juce::String shownName;
    Zone hot = Zone::none;
    float glow = 0.0f, glowTarget = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};

} // namespace rk
