#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "UI/Art.h"
#include "UI/Panel.h"

namespace rk
{

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (p),
      proc (p),
      art (std::make_unique<Art>())
{
    setLookAndFeel (&lnf);

    panel = std::make_unique<Panel> (proc, *art);
    addAndMakeVisible (*panel);

    tooltips = std::make_unique<juce::TooltipWindow> (this, 700);

    // Resizable, at a fixed aspect ratio: the panel is scaled as a whole, so the
    // artwork cannot be distorted.
    setResizable (true, true);
    setResizeLimits (500, 373, 2000, 1492);
    if (auto* aspect = getConstrainer())
        aspect->setFixedAspectRatio ((double) Art::designW / (double) Art::designH);

    // The size chosen is part of the plug-in's state, like a knob position.
    const int saved = (int) proc.apvts.state.getProperty ("uiWidth", (int) Art::designW);
    const int width = juce::jlimit (500, 2000, saved);
    setSize (width, juce::roundToInt ((float) width * Art::designH / Art::designW));
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    panel->releaseAllNotes();       // a key still held when the window closes must not ring on
    setLookAndFeel (nullptr);
}

//==============================================================================
void PluginEditor::resized()
{
    const float scale = (float) getWidth() / Art::designW;
    panel->setBounds (0, 0, (int) Art::designW, (int) Art::designH);
    panel->setTransform (juce::AffineTransform::scale (scale));

    proc.apvts.state.setProperty ("uiWidth", getWidth(), nullptr);
    proc.apvts.state.setProperty ("uiHeight", getHeight(), nullptr);
}

void PluginEditor::rebuildPlate (int width, int height)
{
    if (art->plate.isValid())
        scaledPlate = art->plate.rescaled (width, height, juce::Graphics::highResamplingQuality);
}

void PluginEditor::paint (juce::Graphics& g)
{
    if (! art->valid)
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);
        g.drawFittedText ("ROCKPUNK GUITAR - artwork missing", getLocalBounds(), juce::Justification::centred, 1);
        return;
    }

    const float pixelScale = (float) g.getInternalContext().getPhysicalPixelScaleFactor();
    const int pw = juce::jmax (1, juce::roundToInt ((float) getWidth() * pixelScale));
    const int ph = juce::jmax (1, juce::roundToInt ((float) getHeight() * pixelScale));

    if (! scaledPlate.isValid())
    {
        rebuildPlate (pw, ph);                     // first paint: the picture is needed now
    }
    else if (scaledPlate.getWidth() != pw || scaledPlate.getHeight() != ph)
    {
        // While the window is being dragged, show the old copy stretched and
        // resample once the size settles.
        pendingW = pw;
        pendingH = ph;
        startTimer (120);
    }

    if (scaledPlate.isValid())
        g.drawImage (scaledPlate, getLocalBounds().toFloat());
}

void PluginEditor::timerCallback()
{
    stopTimer();
    rebuildPlate (pendingW, pendingH);
    repaint();
}

} // namespace rk
