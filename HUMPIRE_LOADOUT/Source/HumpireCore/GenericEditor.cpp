#include "GenericEditor.h"

namespace humpire
{
using namespace juce;

GenericEditor::GenericEditor (HumpireProcessor& p, UiSpec s)
    : ScaledEditor (p, W, H), proc (p), spec (std::move (s)),
      topBar (p.apvts, p.presets, p.undo, p.inPeak, p.outPeak, spec.title)
{
    content.paintFn = [this] (Graphics& g) { paintBody (g); };

    bodyImg = Image (Image::ARGB, W * 2, H * 2, true);
    {
        Graphics ig (bodyImg);
        ig.addTransform (AffineTransform::scale (2.0f));
        drawBodyCached (ig);
    }

    content.addAndMakeVisible (topBar);
    topBar.setBounds (0, 0, W, TopBar::height);

    simpleBtn.setClickingTogglesState (true);  cleanBtn.setClickingTogglesState (true);
    simpleBtn.setBounds (668, 9, 44, 22);      cleanBtn.setBounds (716, 9, 44, 22);
    content.addAndMakeVisible (simpleBtn);     content.addAndMakeVisible (cleanBtn);
    simpleAtt = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "simple", simpleBtn);
    cleanAtt  = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "clean",  cleanBtn);

    for (auto& k : spec.knobs)
    {
        auto s2 = std::make_unique<KnobSlot>();
        s2->label = k.label;  s2->advanced = k.advanced;  s2->knob.setName (k.label);
        s2->attach = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (p.apvts, k.id, s2->knob);
        content.addAndMakeVisible (s2->knob);
        knobs.push_back (std::move (s2));
    }

    const int nb = (int) spec.buttons.size();
    for (int i = 0; i < nb; ++i)
    {
        const auto& b = spec.buttons[(size_t) i];
        BtnSlot slot;
        slot.paramId = b.id;
        slot.btn = std::make_unique<ConsoleButton> (b.label, ConsoleButton::Shape::pill, spec.body1.brighter (0.25f));
        slot.btn->setGlyph (b.label);
        slot.btn->setShowLed (! b.momentary);
        auto* prm = p.apvts.getParameter (b.id);
        slot.isChoice = dynamic_cast<AudioParameterChoice*> (prm) != nullptr;
        const int bw2 = jmin (130, 700 / jmax (1, nb) - 8);
        const int x0 = (W - nb * (bw2 + 8) + 8) / 2;
        slot.btn->setBounds (x0 + i * (bw2 + 8), spec.buttonY, bw2, 34);

        if (slot.isChoice)
        {
            auto* sb = slot.btn.get();
            slot.btn->onClick = [prm, sb, this]
            {
                auto* c = static_cast<AudioParameterChoice*> (prm);
                const int next = (c->getIndex() + 1) % c->choices.size();
                prm->beginChangeGesture();  prm->setValueNotifyingHost (c->convertTo0to1 ((float) next));  prm->endChangeGesture();
                sb->setGlyph (c->choices[next]);
            };
            slot.btn->setGlyph (static_cast<AudioParameterChoice*> (prm)->getCurrentChoiceName());
        }
        else if (b.momentary)
        {
            auto* sb = slot.btn.get();
            slot.btn->onStateChange = [prm, sb]
            {
                const float v = sb->isDown() ? 1.0f : 0.0f;
                if (std::abs (prm->getValue() - v) < 0.01f) return;
                prm->beginChangeGesture();  prm->setValueNotifyingHost (v);  prm->endChangeGesture();
            };
        }
        else
        {
            slot.btn->setClickingTogglesState (true);
            slot.attach = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, b.id, *slot.btn);
        }
        content.addAndMakeVisible (*slot.btn);
        btns.push_back (std::move (slot));
    }

    lastSimple = ! proc.getBool ("simple");  lastClean = ! proc.getBool ("clean");
    layoutKnobs();
    startTimerHz (30);
}

GenericEditor::~GenericEditor() { stopTimer(); }

void GenericEditor::layoutKnobs()
{
    const bool simple = proc.getBool ("simple");
    std::vector<KnobSlot*> shown;
    for (auto& k : knobs) { const bool vis = ! (simple && k->advanced); k->knob.setVisible (vis); if (vis) shown.push_back (k.get()); }
    if (shown.empty()) return;
    const int slotW = jmin (100, 740 / (int) shown.size()), size = jmin (74, slotW - 8);
    const int x0 = (W - slotW * (int) shown.size()) / 2;
    for (size_t i = 0; i < shown.size(); ++i)
        shown[i]->knob.setBounds (x0 + (int) i * slotW + (slotW - size) / 2, spec.knobY, size, size);
}

void GenericEditor::timerCallback()
{
    const bool simple = proc.getBool ("simple"), clean = proc.getBool ("clean");
    if (clean != lastClean)   { lastClean = clean;   Theme::setClean (clean); content.repaint(); }
    if (simple != lastSimple) { lastSimple = simple; layoutKnobs(); content.repaint(); }
    for (auto& b : btns)
        if (b.isChoice)
            if (auto* c = dynamic_cast<AudioParameterChoice*> (proc.apvts.getParameter (b.paramId)))
                b.btn->setGlyph (c->getCurrentChoiceName());
    tick();
    content.repaint (spec.display.expanded (6));
}

void GenericEditor::paintBody (Graphics& g)
{
    const bool clean = Theme::isClean();
    if (clean) drawBodyClean (g);
    else
    {
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImage (bodyImg, 0, 0, W, H, 0, 0, bodyImg.getWidth(), bodyImg.getHeight());
    }

    // ecran
    const auto d = spec.display.toFloat();
    {
        Graphics::ScopedSaveState s (g);
        Path clip;  clip.addRoundedRectangle (d, spec.style == BodyStyle::arcade ? 26.0f : 8.0f);
        g.reduceClipRegion (clip);
        g.setColour (Colour (0xff050507));  g.fillRect (d);
        paintDisplay (g, d, (float) ((Time::getMillisecondCounterHiRes() - t0) * 0.001));
        if (! clean)
        {
            // reflet de vitre + vignette
            g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.10f), d.getX(), d.getY(),
                                               Colours::transparentWhite, d.getX(), d.getCentreY(), false));
            g.fillRect (d);
            g.setGradientFill (ColourGradient (Colours::transparentBlack, d.getCentreX(), d.getCentreY(),
                                               Colours::black.withAlpha (spec.style == BodyStyle::arcade ? 0.55f : 0.25f), d.getX(), d.getY(), true));
            g.fillRect (d);
            if (spec.style == BodyStyle::arcade)
            {
                g.setColour (Colours::black.withAlpha (0.18f));
                for (float y = d.getY(); y < d.getBottom(); y += 3.0f) g.fillRect (d.getX(), y, d.getWidth(), 1.0f);   // scanlines CRT
            }
        }
    }

    // labels des knobs
    g.setFont (Theme::uiFont (10.0f, true));
    g.setColour (clean ? col::textDim : spec.accent.withMultipliedBrightness (0.95f));
    for (auto& k : knobs)
        if (k->knob.isVisible())
            g.drawText (k->label, k->knob.getX() - 14, k->knob.getBottom() + 3, k->knob.getWidth() + 28, 14, Justification::centred);
}

void GenericEditor::drawBodyClean (Graphics& g)
{
    g.fillAll (col::black);
    g.setColour (col::panel);  g.fillRoundedRectangle (Rectangle<float> (6, 46, W - 12, H - 52), 20.0f);
    g.setColour (Colours::white.withAlpha (0.08f));  g.drawRoundedRectangle (spec.display.toFloat(), 8.0f, 1.0f);
}

void GenericEditor::drawBodyCached (Graphics& g)
{
    g.fillAll (col::black);
    const auto body = Rectangle<float> (6, 46, W - 12, H - 52);
    const float cr = spec.style == BodyStyle::controller ? 70.0f : (spec.style == BodyStyle::gamer ? 10.0f : 28.0f);

    g.setColour (Colours::black.withAlpha (0.6f));  g.fillRoundedRectangle (body.translated (0, 4), cr);
    g.setGradientFill (ColourGradient (spec.body1.brighter (0.2f), 0, body.getY(), spec.body2, 0, body.getBottom(), false));
    g.fillRoundedRectangle (body, cr);

    Random rnd (777);
    {
        Graphics::ScopedSaveState s (g);
        Path clip;  clip.addRoundedRectangle (body, cr);  g.reduceClipRegion (clip);
        const bool brushed = spec.style == BodyStyle::rack || spec.style == BodyStyle::gamer;
        if (brushed)
            for (float y = body.getY(); y < body.getBottom(); y += 1.5f)
            { g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (0.015f + rnd.nextFloat() * 0.03f));
              g.fillRect (body.getX(), y, body.getWidth(), 1.0f); }
        else
            for (int i = 0; i < 7000; ++i)
            { g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (0.015f + rnd.nextFloat() * 0.03f));
              g.fillRect (body.getX() + rnd.nextFloat() * body.getWidth(), body.getY() + rnd.nextFloat() * body.getHeight(), 1.2f, 1.2f); }

        // decorations par style
        const auto ac = spec.accent;
        switch (spec.style)
        {
            case BodyStyle::rack:
                for (int i = 0; i < 14; ++i) { g.setColour (Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (40.0f + i * 14.0f, 508, 8, 26, 3); g.fillRoundedRectangle (W - 40.0f - (i + 1) * 14.0f, 508, 8, 26, 3); }
                break;
            case BodyStyle::speaker:
                for (float y = 500; y < H - 14; y += 9) for (float x = 60; x < W - 60; x += 9)
                { g.setColour (Colours::black.withAlpha (0.65f)); g.fillEllipse (x + ((int) ((y - 500) / 9) % 2) * 4.5f, y, 4.0f, 4.0f); }
                break;
            case BodyStyle::gamer:
                for (int i = 0; i < 9; ++i) { g.setColour (Colours::black.withAlpha (0.5f)); g.fillRect (20.0f, 100.0f + i * 22, 6.0f, 12.0f); g.fillRect (W - 26.0f, 100.0f + i * 22, 6.0f, 12.0f); }
                break;
            case BodyStyle::cartridge:
                for (int i = 0; i < 4; ++i) { g.setColour (Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (200.0f + i * 100.0f, 502, 80, 30, 4); }
                break;
            case BodyStyle::amp:
                for (float x = 20; x < W - 20; x += 6) { g.setColour (Colours::black.withAlpha (0.18f)); g.fillRect (x, 500.0f, 2.0f, 40.0f); }
                break;
            case BodyStyle::arcade: case BodyStyle::controller: break;
        }
        g.setColour (ac.withAlpha (0.8f));  g.fillRect (body.getX() + 20, body.getY() + 2, body.getWidth() - 40, 2.0f);   // liseré metal doré
    }
    g.setColour (Colours::white.withAlpha (0.35f));  g.drawRoundedRectangle (body.reduced (1.0f), cr, 1.5f);
    g.setColour (Colours::black.withAlpha (0.4f));   g.drawRoundedRectangle (body, cr, 1.0f);

    // marque + bezel
    g.setColour (spec.accent);  g.setFont (Theme::uiFont (13.0f, true));
    g.drawText ("HUMPIRE", 40, 44, 200, 18, Justification::centredLeft);
    g.setColour (spec.accent.withAlpha (0.7f));  g.setFont (Theme::uiFont (10.0f, true));
    g.drawText (spec.subtitle, 400, 46, 380, 16, Justification::centredRight);

    const auto bz = spec.display.toFloat().expanded (10.0f);
    g.setGradientFill (ColourGradient (Colour (0xff2c2c34), 0, bz.getY(), Colour (0xff0b0b0e), 0, bz.getBottom(), false));
    g.fillRoundedRectangle (bz, spec.style == BodyStyle::arcade ? 34.0f : 14.0f);
    g.setColour (spec.accent.withAlpha (0.5f));
    g.drawRoundedRectangle (bz.reduced (1.0f), spec.style == BodyStyle::arcade ? 34.0f : 14.0f, 1.2f);

    // vis
    for (auto c : { Point<float> (24, 70), Point<float> (W - 24.0f, 70), Point<float> (24, H - 26.0f), Point<float> (W - 24.0f, H - 26.0f) })
    {
        g.setGradientFill (ColourGradient (Colour (0xffb0aa9a), c.x - 3, c.y - 3, Colour (0xff4b473d), c.x + 4, c.y + 4, true));
        g.fillEllipse (c.x - 5, c.y - 5, 10, 10);
        g.setColour (Colours::black.withAlpha (0.6f));  g.drawLine (c.x - 3, c.y - 1, c.x + 3, c.y + 1, 1.4f);
    }
}
} // namespace humpire
