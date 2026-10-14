#include "PluginEditor.h"

#if HUMPIRE_GLITCH_HAS_BODY
 #include "GlitchAssetsData.h"
#endif

namespace
{
constexpr int baseW = 760, baseH = 560;
const juce::Rectangle<int> bezelRect { 100, 64, 560, 232 }, lcdRect { 124, 84, 512, 192 };
constexpr int knobY = 316, knobSize = 64, knobSlotW = 86, knobAreaX = 120, knobAreaW = 520;
const juce::Colour plastic { 0xffcbc4b2 }, ink { 0xff3a362e };
}

GlitchEditor::GlitchEditor (GlitchAudioProcessor& p)
    : humpire::ScaledEditor (p, baseW, baseH), proc (p),
      topBar (p.apvts, p.presets, p.undo, p.inPeak, p.outPeak, "GLITCH"),
      lcd (&p.scope),
      btnA      ("A",      humpire::ConsoleButton::Shape::round, juce::Colour (0xffb8325a)),
      btnB      ("B",      humpire::ConsoleButton::Shape::round, juce::Colour (0xff6a4cc2)),
      btnStart  ("START",  humpire::ConsoleButton::Shape::pill,  juce::Colour (0xff55555c)),
      btnSelect ("SELECT", humpire::ConsoleButton::Shape::pill,  juce::Colour (0xff55555c))
{
    namespace id = glitch::id;

    content.paintFn = [this] (juce::Graphics& g) { paintBody (g); };

    // --- fond cache (PNG fourni ou rendu procedural 2x)
   #if HUMPIRE_GLITCH_HAS_BODY
    bodyImg = juce::ImageFileFormat::loadFrom (GlitchAssetsData::glitch_body_2x_png,
                                               (size_t) GlitchAssetsData::glitch_body_2x_pngSize);
   #endif
    if (! bodyImg.isValid())
    {
        bodyImg = juce::Image (juce::Image::ARGB, baseW * 2, baseH * 2, true);
        juce::Graphics ig (bodyImg);
        ig.addTransform (juce::AffineTransform::scale (2.0f));
        drawBodyProcedural (ig);
    }

    // --- composants
    content.addAndMakeVisible (topBar);
    content.addAndMakeVisible (lcd);
    lcd.setBounds (lcdRect);
    topBar.setBounds (0, 0, baseW, humpire::TopBar::height);

    const struct { const char* label; const char* pid; bool adv; } defs[6] = {
        { "BITS",  id::bits,  false }, { "RATE",  id::rate,  false }, { "SIZE", id::tone, true },
        { "DRIVE", id::drive, true  }, { "MIX",   id::mix,   false }, { "OUT",  id::out,  true } };

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];
        k.label = defs[i].label;  k.paramId = defs[i].pid;  k.advancedOnly = defs[i].adv;
        k.knob.setName (k.label);
        k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, k.paramId, k.knob);
        content.addAndMakeVisible (k.knob);
    }

    content.addAndMakeVisible (dpad);
    dpad.setBounds (25, 405, 110, 110);
    dpad.onStep = [this] (int dx, int dy)
    {
        if (dy != 0) stepParam (glitch::id::bits, dy < 0 ? 1.0f : -1.0f, false);
        if (dx != 0) stepParam (glitch::id::rate, dx > 0 ? 1.26f : 1.0f / 1.26f, true);
    };

    for (auto* b : { &btnA, &btnB, &btnStart, &btnSelect }) content.addAndMakeVisible (*b);
    btnA.setBounds (662, 412, 56, 56);   btnA.setGlyph ("A");
    btnB.setBounds (602, 444, 56, 56);   btnB.setGlyph ("B");
    btnSelect.setBounds (300, 468, 80, 26);
    btnStart.setBounds  (392, 468, 80, 26);
    btnA.onClick = [this] { cycleDither(); };
    for (auto* b : { &btnB, &btnStart, &btnSelect }) b->setClickingTogglesState (true);
    btnB.setShowLed (true);  btnStart.setShowLed (true);  btnSelect.setShowLed (true);
    attB      = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id::speaker, btnB);
    attStart  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id::clean,   btnStart);
    attSelect = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id::simple,  btnSelect);

    lastSimple = ! readBool (id::simple);   // force le 1er layout + sync du theme
    lastClean  = ! readBool (id::clean);
    timerCallback();
    startTimerHz (30);
}

GlitchEditor::~GlitchEditor() { stopTimer(); }

bool GlitchEditor::readBool (const char* pid) const { return proc.apvts.getRawParameterValue (pid)->load() > 0.5f; }

void GlitchEditor::timerCallback()
{
    namespace id = glitch::id;
    const bool simple = readBool (id::simple), clean = readBool (id::clean), byp = readBool (id::bypass);

    if (clean != lastClean) { lastClean = clean; humpire::Theme::setClean (clean); content.repaint(); }
    if (simple != lastSimple) { lastSimple = simple; layoutKnobs(); content.repaint(); }

    auto val = [this] (const char* pid) { return proc.apvts.getRawParameterValue (pid)->load(); };
    const float r = val (id::rate);
    const juce::String rateStr = r >= 1000.0f ? juce::String (r / 1000.0f, 1) + "K" : juce::String ((int) r) + "H";
    static const char* dith[] = { "OFF", "RPDF", "TPDF" };

    lcd.setDimmed (byp);
    if (byp) lcd.setTexts ("** BYPASS **", "");
    else lcd.setTexts ("BITS " + juce::String (val (id::bits), 1) + "  RATE " + rateStr,
                       juce::String ("DITH ") + dith[juce::jlimit (0, 2, juce::roundToInt (val (id::dither)))]
                         + "  SPK " + (val (id::speaker) > 0.5f ? "ON" : "OFF")
                         + "  MIX " + juce::String (juce::roundToInt (val (id::mix) * 100.0f)));
    content.repaint (10, 80, 60, 40);   // LED d'alimentation
}

void GlitchEditor::layoutKnobs()
{
    const bool simple = readBool (glitch::id::simple);
    std::vector<KnobSlot*> shown;
    for (auto& k : knobs)
    {
        const bool vis = ! (simple && k.advancedOnly);
        k.knob.setVisible (vis);
        if (vis) shown.push_back (&k);
    }
    const int slotW = juce::jmin (knobSlotW + (simple ? 40 : 0), knobAreaW / juce::jmax (1, (int) shown.size()));
    const int startX = knobAreaX + (knobAreaW - slotW * (int) shown.size()) / 2;
    for (size_t i = 0; i < shown.size(); ++i)
        shown[i]->knob.setBounds (startX + (int) i * slotW + (slotW - knobSize) / 2, knobY, knobSize, knobSize);
}

void GlitchEditor::stepParam (const char* pid, float delta, bool multiplicative)
{
    auto* p = proc.apvts.getParameter (pid);
    if (p == nullptr) return;
    const float cur = p->convertFrom0to1 (p->getValue());
    const float next = multiplicative ? cur * delta : std::round (cur) + delta;
    p->beginChangeGesture();
    p->setValueNotifyingHost (p->convertTo0to1 (next));
    p->endChangeGesture();
}

void GlitchEditor::cycleDither()
{
    auto* p = proc.apvts.getParameter (glitch::id::dither);
    const int cur = juce::roundToInt (p->convertFrom0to1 (p->getValue()));
    p->beginChangeGesture();
    p->setValueNotifyingHost (p->convertTo0to1 ((float) ((cur + 1) % 3)));
    p->endChangeGesture();
}

// ============================================================================ rendu
void GlitchEditor::paintBody (juce::Graphics& g)
{
    using namespace juce;
    const bool clean = humpire::Theme::isClean();
    if (clean) drawBodyClean (g);
    else
    {
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImage (bodyImg, 0, 0, baseW, baseH, 0, 0, bodyImg.getWidth(), bodyImg.getHeight());
    }

    // --- elements dynamiques : labels, LED d'alimentation
    const auto labelCol = clean ? humpire::col::textDim : ink;
    g.setFont (humpire::Theme::uiFont (10.0f, true));
    g.setColour (labelCol);
    for (auto& k : knobs)
        if (k.knob.isVisible())
            g.drawText (k.label, k.knob.getBounds().getX() - 10, k.knob.getBottom() + 4, k.knob.getWidth() + 20, 14, Justification::centred);

    g.drawText ("DITHER",  654, 392, 72, 14, Justification::centred);
    g.drawText ("SPEAKER", 594, 502, 72, 14, Justification::centred);
    g.drawText ("SIMPLE",  300, 498, 80, 14, Justification::centred);
    g.drawText ("CLEAN",   392, 498, 80, 14, Justification::centred);
    g.drawText ("BITS / RATE", 20, 516, 120, 14, Justification::centred);

    const bool powered = ! readBool (glitch::id::bypass);
    const auto led = Rectangle<float> (10, 10).withCentre ({ 52.0f, 104.0f });
    if (powered) { g.setColour (humpire::col::amber.withAlpha (0.35f)); g.fillEllipse (led.expanded (5.0f)); }
    g.setColour (powered ? humpire::col::amber : Colour (0xff3a2a1a));  g.fillEllipse (led);
    g.setColour (Colours::black.withAlpha (0.7f));                      g.drawEllipse (led, 1.2f);
    g.setColour (labelCol);  g.setFont (humpire::Theme::uiFont (8.5f, true));
    g.drawText ("POWER", 24, 114, 56, 12, Justification::centred);
}

void GlitchEditor::drawBodyClean (juce::Graphics& g)
{
    using namespace juce;
    g.fillAll (humpire::col::black);
    g.setColour (humpire::col::panel);
    g.fillRoundedRectangle (Rectangle<float> (6, 46, baseW - 12, baseH - 52), 24.0f);
    g.setColour (Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (bezelRect.toFloat(), 14.0f, 1.0f);
    g.drawRoundedRectangle (Rectangle<float> (6, 46, baseW - 12, baseH - 52), 24.0f, 1.0f);
}

/** Corps de la console : plastique texture, bezel, creux, vis, grille HP. Rendu une fois (cache 2x). */
void GlitchEditor::drawBodyProcedural (juce::Graphics& g)
{
    using namespace juce;
    g.fillAll (humpire::col::black);

    const auto body = Rectangle<float> (6, 46, baseW - 12, baseH - 52);
    // ombre portee
    g.setColour (Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (body.translated (0, 4), 40.0f);
    // plastique
    g.setGradientFill (ColourGradient (plastic.brighter (0.15f), 0, body.getY(), plastic.darker (0.28f), 0, body.getBottom(), false));
    g.fillRoundedRectangle (body, 40.0f);

    // grain du plastique (deterministe)
    Random rnd (4242);
    {
        Graphics::ScopedSaveState s (g);
        Path clip;  clip.addRoundedRectangle (body, 40.0f);  g.reduceClipRegion (clip);
        for (int i = 0; i < 9000; ++i)
        {
            g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (0.02f + rnd.nextFloat() * 0.04f));
            g.fillRect (body.getX() + rnd.nextFloat() * body.getWidth(), body.getY() + rnd.nextFloat() * body.getHeight(), 1.2f, 1.2f);
        }
    }
    // biseaux
    g.setColour (Colours::white.withAlpha (0.45f));  g.drawRoundedRectangle (body.reduced (1.0f), 40.0f, 1.5f);
    g.setColour (Colours::black.withAlpha (0.35f));  g.drawRoundedRectangle (body, 40.0f, 1.0f);

    // marque
    g.setColour (ink);  g.setFont (humpire::Theme::uiFont (13.0f, true));
    g.drawText ("HUMPIRE", 100, 44, 200, 18, Justification::centredLeft);
    g.setFont (humpire::Theme::uiFont (10.0f, true));
    g.setColour (Colour (0xff6b2f8c));
    g.drawText ("GLITCH  //  LO-FI SYSTEM", 360, 46, 300, 16, Justification::centredRight);

    // bezel de l'ecran
    const auto bz = bezelRect.toFloat();
    g.setColour (Colours::black.withAlpha (0.35f));  g.fillRoundedRectangle (bz.translated (0, 2), 22.0f);
    g.setGradientFill (ColourGradient (Colour (0xff3b3b44), 0, bz.getY(), Colour (0xff131318), 0, bz.getBottom(), false));
    g.fillRoundedRectangle (bz, 22.0f);
    g.setColour (Colours::white.withAlpha (0.18f));  g.drawRoundedRectangle (bz.reduced (1.0f), 22.0f, 1.0f);
    g.setColour (Colour (0xff8d8a96));  g.setFont (humpire::Theme::uiFont (8.5f, true));
    g.drawText ("4-TONE REFLECTIVE DISPLAY", bezelRect.getX() + 24, bezelRect.getY() + 3, 300, 14, Justification::centredLeft);
    g.setColour (Colours::black.withAlpha (0.8f));
    g.fillRoundedRectangle (lcdRect.toFloat().expanded (3.0f), 8.0f);          // joint autour de l'ecran

    // bande de controles (creux)
    const auto strip = Rectangle<float> (110, 306, 540, 100);
    g.setColour (Colours::black.withAlpha (0.12f));  g.fillRoundedRectangle (strip, 16.0f);
    g.setColour (Colours::white.withAlpha (0.25f));  g.drawRoundedRectangle (strip.translated (0, 1), 16.0f, 1.0f);
    g.setColour (Colours::black.withAlpha (0.2f));   g.drawRoundedRectangle (strip, 16.0f, 1.0f);

    // creux croix + boutons A/B
    auto well = [&g] (Rectangle<float> r, float cr, float angle)
    {
        Path p;  p.addRoundedRectangle (r, cr);
        p.applyTransform (AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY()));
        g.setColour (Colours::black.withAlpha (0.14f));  g.fillPath (p);
        g.setColour (Colours::white.withAlpha (0.3f));   g.strokePath (p, PathStrokeType (1.0f), AffineTransform::translation (0, 1));
        g.setColour (Colours::black.withAlpha (0.22f));  g.strokePath (p, PathStrokeType (1.0f));
    };
    well ({ 20, 400, 120, 120 }, 60.0f, 0.0f);
    well ({ 586, 414, 150, 94 }, 47.0f, -0.42f);

    // grille de haut-parleur
    for (int i = 0; i < 8; ++i)
    {
        const auto slot = Rectangle<float> (6, 28).withPosition (308.0f + (float) i * 18.0f, 522.0f);
        g.setColour (Colours::black.withAlpha (0.6f));   g.fillRoundedRectangle (slot, 3.0f);
        g.setColour (Colours::white.withAlpha (0.3f));   g.fillRect (slot.getX() + 0.5f, slot.getBottom(), 5.0f, 1.0f);
    }
    // vis
    for (auto c : { Point<float> (26, 70), Point<float> (734, 70), Point<float> (26, 534), Point<float> (734, 534) })
    {
        g.setGradientFill (ColourGradient (Colour (0xff9a9486), c.x - 3, c.y - 3, Colour (0xff4b473d), c.x + 4, c.y + 4, true));
        g.fillEllipse (c.x - 5, c.y - 5, 10, 10);
        g.setColour (Colours::black.withAlpha (0.6f));
        g.drawLine (c.x - 3, c.y - 1, c.x + 3, c.y + 1, 1.4f);
    }
}
