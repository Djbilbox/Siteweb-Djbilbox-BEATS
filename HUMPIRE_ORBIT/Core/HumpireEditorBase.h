#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "HumpireLookAndFeel.h"

/** Base des editeurs HUMPIRE : conception en coordonnees fixes (baseW x baseH), redimensionnable
    50-200 % a ratio verrouille. Tout est vectoriel : un simple setTransform met a l'echelle. */
class HumpireEditorBase : public juce::AudioProcessorEditor
{
public:
    HumpireEditorBase (juce::AudioProcessor& p, int baseWidth, int baseHeight)
        : juce::AudioProcessorEditor (p), baseW (baseWidth), baseH (baseHeight)
    {
        setLookAndFeel (&laf);
        content.setSize (baseW, baseH);
        addAndMakeVisible (content);
        constrainer.setFixedAspectRatio ((double) baseW / (double) baseH);
        constrainer.setSizeLimits (baseW / 2, baseH / 2, baseW * 2, baseH * 2);
        setConstrainer (&constrainer);
        setResizable (true, true);
        setSize (baseW, baseH);
    }

    ~HumpireEditorBase() override { setLookAndFeel (nullptr); }

    void paint (juce::Graphics& g) override { g.fillAll (humpire::theme::bg); }

    void resized() override
    {
        content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) baseW));
        content.setBounds (0, 0, baseW, baseH);
    }

protected:
    /** Les composants sont ajoutes ici, en coordonnees de base. */
    juce::Component content;
    const int baseW, baseH;

private:
    HumpireLookAndFeel laf;
    juce::ComponentBoundsConstrainer constrainer;
};
