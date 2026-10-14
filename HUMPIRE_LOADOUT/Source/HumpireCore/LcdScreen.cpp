#include "LcdScreen.h"

namespace humpire
{
LcdScreen::LcdScreen (const ScopeBuffer* s) : scope (s)
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (30);
}

void LcdScreen::setTexts (const juce::String& t, const juce::String& b)
{
    if (t == topText && b == bottomText) return;
    topText = t;  bottomText = b;  repaint();
}

void LcdScreen::setDimmed (bool d) { if (d != dimmed) { dimmed = d; repaint(); } }

void LcdScreen::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto b = getLocalBounds().toFloat();
    constexpr float px = 4.0f;                                    // taille du "pixel" LCD

    // fond + ombre interne
    g.setColour (dimmed ? tone2.darker (0.5f) : tone0);
    g.fillRoundedRectangle (b, 6.0f);
    if (dimmed) { g.setColour (tone3.withAlpha (0.55f)); g.fillRoundedRectangle (b, 6.0f); return; }

    {
        Graphics::ScopedSaveState s (g);
        Path clip;  clip.addRoundedRectangle (b, 6.0f);  g.reduceClipRegion (clip);
        g.setGradientFill (ColourGradient (tone3.withAlpha (0.35f), 0, 0, Colours::transparentBlack, 0, 14.0f, false));
        g.fillRect (b);                                            // ombre haute (bezel)
    }

    // texte
    const float th = 12.0f;
    const auto topArea = Rectangle<float> (b.getX() + 12, b.getY() + 8, b.getWidth() - 24, th + 4);
    const auto botArea = Rectangle<float> (b.getX() + 12, b.getBottom() - 8 - th - 4, b.getWidth() - 24, th + 4);
    g.setFont (Theme::lcdFont (th));
    for (auto [area, str] : { std::pair { topArea, topText }, std::pair { botArea, bottomText } })
    {
        g.setColour (tone1.withAlpha (0.6f));  g.drawText (str, area.translated (1.5f, 1.5f), Justification::centredLeft, false); // ghost
        g.setColour (tone3);                   g.drawText (str, area, Justification::centredLeft, false);
    }

    // forme d'onde pixelisee (miroir autour du centre)
    const auto wf = Rectangle<float> (b.getX() + 12, topArea.getBottom() + 6,
                                      b.getWidth() - 24, botArea.getY() - topArea.getBottom() - 12);
    const float midY = std::floor ((wf.getCentreY()) / px) * px;
    const float colW = std::floor (wf.getWidth() / (float) ScopeBuffer::size / px) * px;   // multiple de px
    const float cw = jmax (px, colW);
    const int   nCols = jmin (ScopeBuffer::size, (int) (wf.getWidth() / cw));
    const float maxHalf = std::floor (wf.getHeight() * 0.5f / px) * px;

    g.setColour (tone1);
    for (float xx = wf.getX(); xx < wf.getRight(); xx += px * 2)  g.fillRect (xx, midY, px, px * 0.5f); // ligne centrale pointillee

    if (scope != nullptr)
        for (int i = 0; i < nCols; ++i)
        {
            const float v = jlimit (0.0f, 1.0f, scope->get (ScopeBuffer::size - nCols + i));
            const float h = std::round (v * maxHalf / px) * px;
            const float x = wf.getX() + (float) i * cw;
            if (h <= 0.0f) continue;
            g.setColour (tone2);  g.fillRect (x, midY - h, cw - 1.0f, h * 2.0f + px);       // halo
            g.setColour (tone3);  g.fillRect (x, midY - h + px, cw - 1.0f, juce::jmax (px, h * 2.0f - px)); // coeur
        }

    // grille de pixels + reflet de vitre
    g.setColour (tone3.withAlpha (0.07f));
    for (float xx = 0; xx < b.getWidth(); xx += px) g.fillRect (b.getX() + xx, b.getY(), 1.0f, b.getHeight());
    for (float yy = 0; yy < b.getHeight(); yy += px) g.fillRect (b.getX(), b.getY() + yy, b.getWidth(), 1.0f);
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.10f), 0, b.getY(),
                                       Colours::transparentWhite, 0, b.getCentreY(), false));
    g.fillRoundedRectangle (b.withHeight (b.getHeight() * 0.5f), 6.0f);
}
} // namespace humpire
