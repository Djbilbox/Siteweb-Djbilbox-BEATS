#pragma once
#include "HumpireTheme.h"
#include "HumpireLookAndFeel.h"

namespace humpire
{
/** Base d'editeur redimensionnable 50-200 % a ratio verrouille.
    Tous les composants sont poses dans `content` (taille de reference baseW x baseH)
    qui est mis a l'echelle vectoriellement -> rendu net a toutes les tailles. */
class ScaledEditor : public juce::AudioProcessorEditor
{
public:
    struct Content : juce::Component
    {
        std::function<void (juce::Graphics&)> paintFn;
        void paint (juce::Graphics& g) override { if (paintFn) paintFn (g); }
    };

    ScaledEditor (juce::AudioProcessor& p, int baseW, int baseH)
        : juce::AudioProcessorEditor (p), bw (baseW), bh (baseH)
    {
        setLookAndFeel (&laf);
        addAndMakeVisible (content);
        content.setBounds (0, 0, bw, bh);

        setResizable (true, true);
        setResizeLimits (bw / 2, bh / 2, bw * 2, bh * 2);
        getConstrainer()->setFixedAspectRatio ((double) bw / (double) bh);
        setSize (bw, bh);
    }

    ~ScaledEditor() override { setLookAndFeel (nullptr); }

    void resized() override
    {
        content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) bw));
    }

    void paint (juce::Graphics& g) override { g.fillAll (col::black); }

protected:
    Content content;
    const int bw, bh;

private:
    HumpireLookAndFeel laf;
};
} // namespace humpire
