#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Neon;

namespace
{
constexpr int kKeyLow = 36, kKeyHigh = 96;   // 5 octaves
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }

const juce::Colour pink { 0xffff2fb4 }, cyan { 0xff22e6ff }, violet { 0xff8a4dff }, sunTop { 0xffffd23f }, sunBottom { 0xffff2f8a };
const juce::Colour ink { 0xff0b0618 }, glass { 0xcc120a26 }, text { 0xffe9dcff }, dim { 0xff9a88c4 };

juce::Colour neon (float t) { return pink.interpolatedWith (cyan, juce::jlimit (0.f, 1.f, t)); }

// Texte lumineux : halo flou (plusieurs passes decalees) puis texte net.
void glowText (juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r, juce::Font f, juce::Colour col, juce::Justification j, float spread = 2.5f)
{
    g.setFont (f);
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (col.withAlpha (0.10f));
        const float d = spread * (float) i;
        for (auto o : { juce::Point<float> (-d, 0), { d, 0 }, { 0, -d }, { 0, d } })
            g.drawText (s, r.translated (o.x, o.y), j, false);
    }
    g.setColour (col);
    g.drawText (s, r, j, false);
}

// Panneau de verre : fond translucide, contour degrade neon, titre a gauche.
void glassPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title, juce::Colour accent)
{
    g.setColour (accent.withAlpha (0.10f)); g.fillRoundedRectangle (r.expanded (4.f), 14.f);
    g.setColour (glass); g.fillRoundedRectangle (r, 12.f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.06f), 0, r.getY(), juce::Colours::transparentWhite, 0, r.getY() + 40.f, false));
    g.fillRoundedRectangle (r.reduced (1.f), 11.f);
    g.setGradientFill (juce::ColourGradient (accent, r.getX(), r.getY(), accent.interpolatedWith (violet, 0.7f).withAlpha (0.5f), r.getRight(), r.getBottom(), false));
    g.drawRoundedRectangle (r, 12.f, 1.4f);
    const auto tr = juce::Rectangle<float> (r.getX() + 14.f, r.getY() + 6.f, r.getWidth() - 28.f, 18.f);
    glowText (g, title, tr, juce::FontOptions (13.f, juce::Font::bold), accent, juce::Justification::centredLeft, 1.2f);
    g.setColour (accent.withAlpha (0.35f));
    const float tw = juce::GlyphArrangement::getStringWidth (juce::FontOptions (13.f, juce::Font::bold), title);
    g.drawLine (tr.getX() + tw + 10.f, tr.getCentreY(), r.getRight() - 14.f, tr.getCentreY(), 1.f);
}

void caption (juce::Graphics& g, const juce::String& s, juce::Rectangle<int> r, juce::Justification j = juce::Justification::centredLeft)
{
    g.setColour (dim); g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawFittedText (s, r, j, 1);
}
}

//==============================================================================
NeonLNF::NeonLNF()
{
    setColour (juce::ComboBox::textColourId, text);
    setColour (juce::PopupMenu::backgroundColourId, ink);
    setColour (juce::PopupMenu::textColourId, text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, pink.withAlpha (0.35f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::textColourOffId, text);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::HyperlinkButton::textColourId, cyan.withAlpha (0.85f));
    knobImage = juce::ImageFileFormat::loadFrom (BinaryData::knob_png, (size_t) BinaryData::knob_pngSize)
                    .rescaled (192, 192, juce::Graphics::highResamplingQuality);
}

void NeonLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.f);
    const float r = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
    const auto c = area.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const bool hot = s.isMouseOverOrDragging();

    juce::Path track; track.addCentredArc (c.x, c.y, r, r, 0, a0, a1, true);
    g.setColour (juce::Colours::white.withAlpha (0.07f)); g.strokePath (track, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path val; val.addCentredArc (c.x, c.y, r, r, 0, a0, ang, true);
        juce::ColourGradient grad (pink, c.x - r, c.y + r, cyan, c.x + r, c.y - r, false);
        for (float wdt : { 10.f, 6.f })   // halo
        {
            g.setGradientFill (grad); g.setOpacity (wdt > 8.f ? 0.12f : 0.25f);
            g.strokePath (val, juce::PathStrokeType (wdt, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setOpacity (1.f); g.setGradientFill (grad);
        g.strokePath (val, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // potard photo-realiste (image Flow) tourne a l'angle de la valeur ; repere a 12 h dans l'image
    const float kr = r * 0.8f;
    g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillEllipse (c.x - kr + 1.f, c.y - kr + 3.f, kr * 2, kr * 2);
    if (knobImage.isValid())
    {
        const float sc = kr * 2.f / (float) knobImage.getWidth();
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.setOpacity (1.f);
        g.drawImageTransformed (knobImage, juce::AffineTransform::translation (-0.5f * (float) knobImage.getWidth(), -0.5f * (float) knobImage.getHeight())
                                               .scaled (sc).rotated (ang).translated (c.x, c.y));
    }
    if (hot) { g.setColour (cyan.withAlpha (0.6f)); g.drawEllipse (c.x - kr, c.y - kr, kr * 2, kr * 2, 1.f); }
}

void NeonLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const float cx = (float) x + (float) w * 0.5f;
    if (s.getProperties()["wheel"])
    {
        const auto body = juce::Rectangle<float> ((float) x + 4.f, (float) y, (float) w - 8.f, (float) h);
        g.setColour (juce::Colours::black.withAlpha (0.7f)); g.fillRoundedRectangle (body, 7.f);
        g.setColour (violet.withAlpha (0.5f)); g.drawRoundedRectangle (body, 7.f, 1.f);
        const float th = 26.f;
        const auto cap = juce::Rectangle<float> (body.getX() + 3.f, juce::jlimit (body.getY(), body.getBottom() - th, pos - th * 0.5f), body.getWidth() - 6.f, th);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a3a70), 0, cap.getY(), juce::Colour (0xff1a1030), 0, cap.getBottom(), false));
        g.fillRoundedRectangle (cap, 5.f);
        g.setColour (cyan.withAlpha (0.9f)); g.fillRect (cap.getX() + 4.f, cap.getCentreY() - 1.f, cap.getWidth() - 8.f, 2.f);
        return;
    }
    const auto track = juce::Rectangle<float> (cx - 2.f, (float) y, 4.f, (float) h);
    g.setColour (juce::Colours::black.withAlpha (0.6f)); g.fillRoundedRectangle (track, 2.f);
    const auto fill = track.withTop (pos);
    g.setGradientFill (juce::ColourGradient (cyan, 0, (float) y, pink, 0, (float) (y + h), false));
    g.setOpacity (0.25f); g.fillRoundedRectangle (fill.expanded (3.f, 0), 3.f);
    g.setOpacity (1.f); g.fillRoundedRectangle (fill, 2.f);
    const auto cap = juce::Rectangle<float> (cx - 10.f, pos - 5.f, 20.f, 10.f);
    g.setColour (juce::Colour (0xff2a1e48)); g.fillRoundedRectangle (cap, 3.f);
    g.setColour (s.isMouseOverOrDragging() ? cyan : text.withAlpha (0.8f)); g.drawRoundedRectangle (cap, 3.f, 1.f);
    g.fillRect (cap.getX() + 4.f, cap.getCentreY() - 0.5f, cap.getWidth() - 8.f, 1.f);
}

void NeonLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& b)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.f);
    g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (r, 6.f);
    g.setGradientFill (juce::ColourGradient (pink, r.getX(), 0, cyan, r.getRight(), 0, false));
    g.setOpacity (b.isMouseOver (true) ? 1.f : 0.7f);
    g.drawRoundedRectangle (r, 6.f, 1.2f);
    g.setOpacity (1.f);
    juce::Path t; const float cx = (float) w - 13.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 4, cy - 2, cx + 4, cy - 2, cx, cy + 3);
    g.setColour (cyan); g.fillPath (t);
}

void NeonLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (8, 0, b.getWidth() - 26, b.getHeight());
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void NeonLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? pink.withAlpha (0.35f) : over ? violet.withAlpha (0.3f) : juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour ((over ? cyan : violet).withAlpha (0.8f)); g.drawRoundedRectangle (r, 6.f, 1.f);
}

void NeonLNF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    const auto r = b.getLocalBounds().toFloat().reduced (2.f);
    const bool on = b.getToggleState();
    const auto pill = r.withSizeKeepingCentre (r.getWidth(), juce::jmin (r.getHeight(), 18.f));
    g.setColour (on ? pink.withAlpha (0.35f) : juce::Colours::black.withAlpha (0.6f)); g.fillRoundedRectangle (pill, pill.getHeight() * 0.5f);
    g.setColour ((on || over) ? pink : violet.withAlpha (0.6f)); g.drawRoundedRectangle (pill, pill.getHeight() * 0.5f, 1.f);
    const float d = pill.getHeight() - 4.f;
    const float kx = on ? pill.getRight() - d - 2.f : pill.getX() + 2.f;
    if (on) { g.setColour (pink.withAlpha (0.4f)); g.fillEllipse (kx - 3.f, pill.getY() - 1.f, d + 6.f, d + 6.f); }
    g.setColour (on ? juce::Colours::white : dim); g.fillEllipse (kx, pill.getY() + 2.f, d, d);
}

void NeonLNF::drawBubble (juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>& body)
{
    g.setColour (ink.withAlpha (0.95f)); g.fillRoundedRectangle (body, 5.f);
    g.setColour (cyan); g.drawRoundedRectangle (body, 5.f, 1.f);
}

//==============================================================================
NeonPanel::NeonPanel (NeonAudioProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    hero = juce::ImageFileFormat::loadFrom (BinaryData::hero_jpg, (size_t) BinaryData::hero_jpgSize);

    addKnob (detune, P::detune, "DETUNE"); addKnob (glide, P::glide, "GLIDE");
    addKnob (semi2, P::osc2Semi, "SEMI"); addKnob (fine2, P::osc2Fine, "FINE"); addKnob (mix2, P::osc2Mix, "MIX");
    addKnob (cutoff, P::cutoff, "CUTOFF"); addKnob (res, P::res, "RESONANCE"); addKnob (fenv, P::fenv, "ENV AMT"); addKnob (drive, P::drive, "DRIVE");
    const char* adsr[4] = { "A", "D", "S", "R" };
    const int ampIdx[4] = { P::ampAtk, P::ampDec, P::ampSus, P::ampRel };
    for (int i = 0; i < 4; ++i) addKnob (ampEnv[i], ampIdx[i], adsr[i], juce::Slider::LinearVertical);
    addKnob (velGain, P::velBank, "VEL > LEVEL"); addKnob (velEnv, P::velAdsr, "VEL > ENV");
    addKnob (rate, P::lfoRate, "RATE"); addKnob (amount, P::lfoAmount, "AMOUNT");
    for (int i = 0; i < 4; ++i) addKnob (lfoEnv[i], P::lfoAtk + i, adsr[i], juce::Slider::LinearVertical);
    addKnob (dist, P::dist, "DISTORT"); addKnob (chorus, P::chorus, "CHORUS"); addKnob (reverb, P::reverb, "REVERB");
    addKnob (dmix, P::delayMix, "DELAY"); addKnob (dtime, P::delayTime, "TIME"); addKnob (dfb, P::delayFb, "FEEDBACK");
    addKnob (volume, P::volume, "VOLUME");
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical); addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    labelled.erase (std::remove_if (labelled.begin(), labelled.end(), [this] (Knob* k) { return k == &bend || k == &modw; }), labelled.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };

    addCombo (cWave, P::wave, waveNames); addCombo (cMode, P::vmode, modeNames); addCombo (cWave2, P::osc2Wave, wave2Names);
    addCombo (cFilt, P::ftype, filterNames); addCombo (cLfo, P::lfoSrc, lfoNames); addCombo (cDest, P::lfoDst, destNames);
    addAndMakeVisible (pitchSwitch);
    pitchAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, ids[P::lfoPitch], pitchSwitch);

    for (auto* b : { &prev, &next, &folder, &save }) addAndMakeVisible (*b);
    prev.onClick = [this] { stepPreset (-1); };
    next.onClick = [this] { stepPreset (1); };
    folder.onClick = [this] { showPresetMenu(); };
    save.onClick = [this] { savePreset(); };
    more.setFont (juce::FontOptions (11.f, juce::Font::bold), false, juce::Justification::centredRight);
    addAndMakeVisible (more);

    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (juce::FontOptions (18.f, juce::Font::bold));
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffc9fbff));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);
    bankLabel.setJustificationType (juce::Justification::centred);
    bankLabel.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    bankLabel.setColour (juce::Label::textColourId, pink.withAlpha (0.9f));
    bankLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (bankLabel);

    keys.setAvailableRange (kKeyLow, kKeyHigh);
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, pink.withAlpha (0.65f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, cyan.withAlpha (0.25f));
    keys.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xfff3eeff));
    keys.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff160d2a));
    keys.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff6c5a96));
    keys.setColour (juce::MidiKeyboardComponent::shadowColourId, violet.withAlpha (0.4f));
    keys.setScrollButtonsVisible (false);
    keys.setLowestVisibleKey (kKeyLow);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (W, H);
    startTimerHz (30);
}

NeonPanel::~NeonPanel() { stopTimer(); setLookAndFeel (nullptr); }

void NeonPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
{
    k.label = label;
    k.s.setSliderStyle (style);
    k.s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    k.s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    k.s.setPopupDisplayEnabled (true, true, this);
    k.s.setColour (juce::Slider::textBoxTextColourId, text);
    k.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, ids[idx], k.s);
    addAndMakeVisible (k.s);
    labelled.push_back (&k);
}

void NeonPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void NeonPanel::refreshPresetLabel()
{
    presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification);
    const auto& list = presets();
    const int i = proc.presetIndex;
    bankLabel.setText (i >= 0 && i < (int) list.size() ? juce::String (list[(size_t) i].bank).toUpperCase() + "   " + juce::String (i + 1) + " / " + juce::String ((int) list.size())
                                                       : juce::String ("USER PRESET"), juce::dontSendNotification);
}

void NeonPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File NeonPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("NEON SYNTH 80s").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void NeonPanel::showPresetMenu()
{
    juce::PopupMenu menu, sub; juce::String cur;
    const auto& list = presets();
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (cur != list[i].bank) { if (cur.isNotEmpty()) menu.addSubMenu (cur, sub); sub = juce::PopupMenu(); cur = list[i].bank; }
        sub.addItem ((int) i + 1, list[i].name, true, (int) i == proc.presetIndex);
    }
    menu.addSubMenu (cur, sub);
    menu.addSeparator();
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.neonpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .neonpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.neonpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void NeonPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.neonpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("neonpreset"));
        proc.presetIndex = -1; proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void NeonPanel::loadUserPreset (const juce::File& f)
{
    if (auto xml = juce::XmlDocument::parse (f))
        if (xml->hasTagName (proc.apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            for (int i = 0; i < COUNT; ++i)
                if (auto* prm = proc.apvts.getParameter (ids[i]))
                    if (auto child = vt.getChildWithProperty ("id", ids[i]); child.isValid() && i != P::bend && i != P::modw)
                        prm->setValueNotifyingHost (prm->convertTo0to1 ((float) child.getProperty ("value", prm->convertFrom0to1 (prm->getValue()))));
            proc.presetIndex = -1; proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
        }
}

//==============================================================================
void NeonPanel::drawScope (juce::Graphics& g) const
{
    const auto r = scopeArea.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.7f)); g.fillRoundedRectangle (r, 8.f);
    g.setColour (violet.withAlpha (0.18f));
    for (int i = 1; i < 6; ++i) g.drawVerticalLine ((int) (r.getX() + r.getWidth() * (float) i / 6.f), r.getY() + 3.f, r.getBottom() - 3.f);
    g.drawHorizontalLine ((int) r.getCentreY(), r.getX() + 3.f, r.getRight() - 3.f);

    // fenetre de 512 echantillons, calee sur un passage par zero montant pour une trace stable
    const int N = NeonAudioProcessor::scopeSize, view = 512;
    const int w = proc.scopeWrite.load (std::memory_order_acquire);
    int start = (w - view - 256 + N) & (N - 1);
    for (int k = 0; k < 256; ++k)
    {
        const int a = (start + k) & (N - 1), b = (a + 1) & (N - 1);
        if (proc.scope[(size_t) a] <= 0.f && proc.scope[(size_t) b] > 0.f) { start = a; break; }
    }
    float peak = 0.02f;
    for (int k = 0; k < view; ++k) peak = juce::jmax (peak, std::abs (proc.scope[(size_t) ((start + k) & (N - 1))]));
    const float gain = 0.42f * r.getHeight() / juce::jmax (0.25f, peak);
    juce::Path trace;
    for (int k = 0; k < view; k += 2)
    {
        const float x = r.getX() + 4.f + (r.getWidth() - 8.f) * (float) k / (float) view;
        const float y = r.getCentreY() - juce::jlimit (-0.47f * r.getHeight(), 0.47f * r.getHeight(), proc.scope[(size_t) ((start + k) & (N - 1))] * gain);
        if (k == 0) trace.startNewSubPath (x, y); else trace.lineTo (x, y);
    }
    g.setColour (cyan.withAlpha (0.25f)); g.strokePath (trace, juce::PathStrokeType (4.f));
    g.setColour (cyan); g.strokePath (trace, juce::PathStrokeType (1.4f));
    g.setColour (violet.withAlpha (0.7f)); g.drawRoundedRectangle (r, 8.f, 1.f);
}

void NeonPanel::renderStatic (juce::Graphics& g)
{
    constexpr int heroH = 340, dy = 252;
    // console sous le decor : noir violace + reflet assombri du decor
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff120a22), 0, (float) heroH, juce::Colour (0xff06030c), 0, (float) H, false));
    g.fillAll();
    if (hero.isValid())
    {
        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (0, heroH, W, 260);
            g.setOpacity (0.22f);
            g.drawImageTransformed (hero, juce::AffineTransform::scale ((float) W / (float) hero.getWidth(), -(float) heroH / (float) hero.getHeight())
                                              .translated (0.f, (float) heroH * 2.f));
        }
        g.setGradientFill (juce::ColourGradient (juce::Colour (0x00120a22), 0, (float) heroH, juce::Colour (0xff0a0614), 0, (float) heroH + 260.f, false));
        g.fillRect (0, heroH, W, 260);
        g.setOpacity (1.f);
        g.drawImage (hero, juce::Rectangle<float> (0.f, 0.f, (float) W, (float) heroH), juce::RectanglePlacement::fillDestination);
    }
    // voile sombre en haut (lisibilite de l'en-tete) et fondu vers la console
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xd8070410), 0, 0, juce::Colour (0x00070410), 0, 120.f, false));
    g.fillRect (0, 0, W, 120);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0x00120a22), 0, (float) heroH - 70.f, juce::Colour (0xff120a22), 0, (float) heroH, false));
    g.fillRect (0, heroH - 70, W, 70);
    g.setColour (pink.withAlpha (0.18f)); g.fillRect (0, heroH - 2, W, 5);
    g.setColour (pink.withAlpha (0.8f)); g.drawHorizontalLine (heroH, 0.f, (float) W);

    // titre
    const auto title = juce::Rectangle<float> (22.f, 10.f, 330.f, 46.f);
    g.setFont (juce::FontOptions (40.f, juce::Font::bold | juce::Font::italic));
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (pink.withAlpha (0.14f));
        g.drawText ("NEON SYNTH", title.translated (0, (float) i * 1.5f).expanded ((float) i), juce::Justification::centredLeft, false);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffeafcff), 0, title.getY() + 6.f, juce::Colour (0xffff8ad6), 0, title.getBottom() - 4.f, false));
    g.drawText ("NEON SYNTH", title, juce::Justification::centredLeft, false);
    glowText (g, "80s", { 284.f, 16.f, 80.f, 40.f }, juce::FontOptions (30.f, juce::Font::bold | juce::Font::italic), sunTop, juce::Justification::centredLeft, 1.5f);
    g.setColour (text.withAlpha (0.75f)); g.setFont (juce::FontOptions (11.f, juce::Font::bold));
    g.drawText (juce::String (juce::CharPointer_UTF8 ("HUMPIRE  \xc2\xb7  DJBILBOX BEATS  \xc2\xb7  V2.0")), juce::Rectangle<float> (24.f, 58.f, 330.f, 16.f), juce::Justification::centredLeft, false);

    // afficheur de preset (verre fume sur le decor)
    const auto lcd = juce::Rectangle<float> (370.f, 16.f, 400.f, 58.f);
    g.setColour (juce::Colours::black.withAlpha (0.65f)); g.fillRoundedRectangle (lcd, 10.f);
    g.setColour (cyan.withAlpha (0.05f));
    for (float yy = lcd.getY() + 3.f; yy < lcd.getBottom() - 2.f; yy += 3.f) g.drawHorizontalLine ((int) yy, lcd.getX() + 4.f, lcd.getRight() - 4.f);
    g.setGradientFill (juce::ColourGradient (pink, lcd.getX(), 0, cyan, lcd.getRight(), 0, false));
    g.drawRoundedRectangle (lcd, 10.f, 1.3f);

    // panneaux de la console
    auto P = [] (float x, float y, float w, float h) { return juce::Rectangle<float> (x, y + (float) dy, w, h); };
    glassPanel (g, P (20.f, 100.f, 250.f, 180.f), "OSC 1", cyan);
    glassPanel (g, P (280.f, 100.f, 330.f, 180.f), "OSC 2", cyan);
    glassPanel (g, P (620.f, 100.f, 460.f, 180.f), "FILTER", pink);
    glassPanel (g, P (20.f, 290.f, 330.f, 180.f), "AMP ENVELOPE", pink);
    glassPanel (g, P (360.f, 290.f, 400.f, 180.f), "LFO", violet.brighter (0.4f));
    glassPanel (g, P (770.f, 290.f, 310.f, 180.f), "EFFECTS", cyan);

    auto cap = [&] (const juce::String& t, int x, int y, int w, int h, juce::Justification j = juce::Justification::centredLeft)
    { caption (g, t, { x, y + dy, w, h }, j); };
    cap ("WAVE", 34, 132, 50, 26);  cap ("VOICES", 34, 166, 50, 26);
    cap ("WAVE", 294, 132, 50, 26);
    cap ("TYPE", 634, 132, 50, 26);
    cap ("SHAPE", 374, 324, 50, 26); cap ("TARGET", 374, 358, 50, 26);
    cap ("LFO > PITCH", 424, 430, 90, 22);
    cap ("LFO ENVELOPE", 620, 314, 130, 14, juce::Justification::centred);
    cap ("VELOCITY", 240, 314, 100, 14, juce::Justification::centred);

    // socle du clavier
    const auto deck = juce::Rectangle<float> (12.f, 734.f, (float) W - 24.f, 118.f);
    g.setColour (juce::Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (deck, 10.f);
    g.setColour (violet.withAlpha (0.35f)); g.drawRoundedRectangle (deck, 10.f, 1.f);
    caption (g, "BEND", { 22, 738, 50, 14 }, juce::Justification::centred);
    caption (g, "MOD", { 78, 738, 50, 14 }, juce::Justification::centred);
}

void NeonPanel::paint (juce::Graphics& g)
{
    // fond fixe (decor, panneaux, legendes) rendu une fois en 2x, puis couche animee par-dessus
    if (! bgCache.isValid())
    {
        bgCache = juce::Image (juce::Image::ARGB, W * 2, H * 2, true);
        juce::Graphics cg (bgCache);
        cg.addTransform (juce::AffineTransform::scale (2.f));
        renderStatic (cg);
    }
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (bgCache, juce::Rectangle<float> (0.f, 0.f, (float) W, (float) H));
    drawAnimated (g);
}

void NeonPanel::timerCallback()
{
    constexpr float dt = 1.f / 30.f;
    t += dt;
    const float pl = proc.peakL.exchange (0.f), pr = proc.peakR.exchange (0.f);
    auto follow = [] (float cur, float in) { return in > cur ? in : cur * 0.86f; };   // attaque instantanee, retombee ~0.25 s
    meterL = follow (meterL, pl); meterR = follow (meterR, pr);
    activity = juce::jlimit (0.f, 1.f, juce::jmax (meterL, meterR) * 2.2f);
    lfoPh = std::fmod (lfoPh + proc.apvts.getRawParameterValue (ids[P::lfoRate])->load() * dt, 1.f);
    notesHeld = false;
    for (int n = 0; n < 128 && ! notesHeld; ++n) notesHeld = proc.keyboardState.isNoteOnForChannels (0xffff, n);
    repaint();
    if (presetLabel.getText() != proc.currentPresetName()) refreshPresetLabel();   // changement par l'hote (programmes)
}

void NeonPanel::drawAnimated (juce::Graphics& g)
{
    // etoiles qui scintillent dans le ciel du decor
    juce::Random rnd (7);
    for (int i = 0; i < 60; ++i)
    {
        const float x = rnd.nextFloat() * (float) W, y = 6.f + rnd.nextFloat() * 150.f, sz = 1.f + rnd.nextFloat() * 1.6f;
        const float sp = 1.5f + rnd.nextFloat() * 3.f, ph = rnd.nextFloat() * 6.28f;
        const float a = 0.1f + 0.75f * juce::jmax (0.f, std::sin (t * sp + ph));
        g.setColour ((i % 4 == 0 ? cyan : juce::Colours::white).withAlpha (a * 0.8f));
        g.fillEllipse (x - sz * 0.5f, y - sz * 0.5f, sz, sz);
        if (a > 0.7f) { g.setColour (juce::Colours::white.withAlpha ((a - 0.7f) * 0.6f)); g.fillEllipse (x - sz * 1.6f, y - sz * 1.6f, sz * 3.2f, sz * 3.2f); }
    }

    // reflets qui descendent sur le soleil
    {
        const float scx = 269.f, scy = 97.f, sr = 112.f;
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path disc; disc.addEllipse (scx - sr, scy - sr, sr * 2.f, sr * 2.f);
        g.reduceClipRegion (disc);
        for (int i = 0; i < 3; ++i)
        {
            const float yy = scy - sr + std::fmod (t * 18.f + (float) i * sr * 0.66f, sr * 2.f);
            g.setColour (juce::Colour (0xffffe9a8).withAlpha (0.10f)); g.fillRect (scx - sr, yy, sr * 2.f, 3.f);
        }
    }

    // feux arriere : halo qui s'allume avec le son, reflet sur la route
    {
        const float flick = 0.03f * std::sin (t * 23.f) + 0.02f * std::sin (t * 7.f);
        const float a = juce::jlimit (0.f, 1.f, 0.12f + 0.6f * activity + flick);
        const juce::Colour red (0xffff2340);
        g.setGradientFill (juce::ColourGradient (red.withAlpha (a), 775.f, 224.f, red.withAlpha (0.f), 775.f + 190.f, 224.f, true));
        g.fillEllipse (585.f, 190.f, 380.f, 70.f);
        g.setGradientFill (juce::ColourGradient (red.withAlpha (a * 0.45f), 775.f, 312.f, red.withAlpha (0.f), 775.f + 150.f, 312.f, true));
        g.fillEllipse (625.f, 296.f, 300.f, 34.f);
    }

    // ligne d'horizon de la console : pulse avec le niveau
    {
        const float a = 0.45f + 0.55f * activity;
        g.setColour (pink.withAlpha (0.15f * a)); g.fillRect (0, 336, W, 9);
        g.setColour (pink.withAlpha (a)); g.drawHorizontalLine (340, 0.f, (float) W);
    }

    // balayage lumineux dans l'afficheur de preset
    {
        const auto lcd = juce::Rectangle<float> (372.f, 18.f, 396.f, 54.f);
        const float x = lcd.getX() + std::fmod (t * 90.f, lcd.getWidth() + 120.f) - 60.f;
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (lcd.toNearestInt());
        juce::ColourGradient sweep (cyan.withAlpha (0.f), x - 50.f, 0, cyan.withAlpha (0.f), x + 50.f, 0, false);
        sweep.addColour (0.5, cyan.withAlpha (0.10f));
        g.setGradientFill (sweep);
        g.fillRect (x - 50.f, lcd.getY(), 100.f, lcd.getHeight());
    }
}

void NeonPanel::drawNeonFrames (juce::Graphics& g) const
{
    struct Pan { juce::Rectangle<float> r; juce::Colour c; float led; };
    const auto& pr = proc.apvts;
    auto prm = [&pr] (int i) { return pr.getRawParameterValue (ids[i])->load(); };
    const float held = notesHeld ? 1.f : 0.f;
    const float lfoLed = 0.5f + 0.5f * std::sin (lfoPh * juce::MathConstants<float>::twoPi);
    const bool fxOn = prm (P::dist) > 0.02f || prm (P::chorus) > 0.02f || prm (P::reverb) > 0.02f || prm (P::delayMix) > 0.02f;
    const Pan pans[6] = {
        { { 20.f, 352.f, 250.f, 180.f }, cyan, held },
        { { 280.f, 352.f, 330.f, 180.f }, cyan, prm (P::osc2Mix) > 0.01f ? held : 0.f },
        { { 620.f, 352.f, 460.f, 180.f }, pink, juce::jmax (held * 0.35f, activity) },
        { { 20.f, 542.f, 330.f, 180.f }, pink, activity },
        { { 360.f, 542.f, 400.f, 180.f }, violet.brighter (0.4f), lfoLed * juce::jlimit (0.2f, 1.f, prm (P::lfoAmount) * 1.5f) },
        { { 770.f, 542.f, 310.f, 180.f }, cyan, fxOn ? 0.45f + 0.55f * activity : 0.f },
    };
    for (int i = 0; i < 6; ++i)
    {
        const auto& p = pans[i];
        // halo du contour, plus fort quand le module travaille
        g.setColour (p.c.withAlpha (0.05f + 0.22f * p.led));
        g.drawRoundedRectangle (p.r.expanded (2.f), 13.f, 4.f);

        // lumiere qui fait le tour du panneau
        juce::Path frame; frame.addRoundedRectangle (p.r, 12.f);
        const float len = frame.getLength();
        const float head = std::fmod (t * 70.f + (float) i * len * 0.37f, len);
        for (int k = 0; k < 14; ++k)
        {
            const auto pt = frame.getPointAlongPath (std::fmod (head - (float) k * 3.f + len, len));
            const float a = (1.f - (float) k / 14.f) * (0.35f + 0.5f * p.led);
            g.setColour (p.c.withAlpha (a * 0.35f)); g.fillEllipse (pt.x - 4.f, pt.y - 4.f, 8.f, 8.f);
            g.setColour (juce::Colours::white.withAlpha (a * 0.7f)); g.fillEllipse (pt.x - 1.2f, pt.y - 1.2f, 2.4f, 2.4f);
        }

        // LED d'activite en haut a droite
        const juce::Point<float> c (p.r.getRight() - 20.f, p.r.getY() + 15.f);
        g.setColour (juce::Colours::black.withAlpha (0.8f)); g.fillEllipse (c.x - 5.f, c.y - 5.f, 10.f, 10.f);
        if (p.led > 0.02f)
        {
            g.setColour (p.c.withAlpha (0.35f * p.led)); g.fillEllipse (c.x - 9.f, c.y - 9.f, 18.f, 18.f);
            g.setColour (p.c.interpolatedWith (juce::Colours::white, 0.3f).withAlpha (0.25f + 0.75f * p.led)); g.fillEllipse (c.x - 3.5f, c.y - 3.5f, 7.f, 7.f);
        }
        else { g.setColour (p.c.withAlpha (0.18f)); g.fillEllipse (c.x - 3.5f, c.y - 3.5f, 7.f, 7.f); }
    }
}

void NeonPanel::drawMeter (juce::Graphics& g) const
{
    // vu-metre LED stereo, pose sur le decor en bas a droite
    const auto box = juce::Rectangle<float> (858.f, 290.f, 226.f, 38.f);
    g.setColour (juce::Colours::black.withAlpha (0.6f)); g.fillRoundedRectangle (box, 8.f);
    g.setColour (violet.withAlpha (0.5f)); g.drawRoundedRectangle (box, 8.f, 1.f);
    g.setColour (dim); g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    g.drawText ("OUT", juce::Rectangle<float> (box.getX() + 6.f, box.getY(), 28.f, box.getHeight()), juce::Justification::centredLeft, false);
    constexpr int segs = 24;
    const float x0 = box.getX() + 34.f, sw = (box.getRight() - 8.f - x0) / (float) segs;
    for (int ch = 0; ch < 2; ++ch)
    {
        const float lvl = ch == 0 ? meterL : meterR;
        const float db = juce::Decibels::gainToDecibels (lvl, -60.f);
        const int lit = juce::roundToInt (juce::jmap (db, -48.f, 0.f, 0.f, (float) segs));
        const float y = box.getY() + 9.f + (float) ch * 12.f;
        for (int s = 0; s < segs; ++s)
        {
            const float f = (float) s / (float) (segs - 1);
            const juce::Colour c = f < 0.65f ? cyan.interpolatedWith (violet, f / 0.65f)
                                 : f < 0.88f ? pink : juce::Colour (0xffffd23f);
            const auto seg = juce::Rectangle<float> (x0 + (float) s * sw, y, sw - 2.f, 8.f);
            if (s < lit)
            {
                g.setColour (c.withAlpha (0.35f)); g.fillRoundedRectangle (seg.expanded (1.5f), 2.f);
                g.setColour (c); g.fillRoundedRectangle (seg, 1.5f);
            }
            else { g.setColour (c.withAlpha (0.12f)); g.fillRoundedRectangle (seg, 1.5f); }
        }
    }
}

void NeonPanel::paintOverChildren (juce::Graphics& g)
{
    drawNeonFrames (g);
    drawMeter (g);
    drawScope (g);
    g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    for (auto* k : labelled)
    {
        const auto r = k->s.getBounds();
        const bool lin = k->s.getSliderStyle() == juce::Slider::LinearVertical;
        g.setColour (k->s.isMouseOverOrDragging() ? cyan : text.withAlpha (0.85f));
        g.drawFittedText (k->label, r.getX() - 20, r.getBottom() + (lin ? 2 : -2), r.getWidth() + 40, 14, juce::Justification::centred, 1);
    }
}

void NeonPanel::resized()
{
    auto knob = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (cx - sz / 2, cy - sz / 2, sz, sz); };

    // en-tete
    prev.setBounds (378, 26, 34, 38); next.setBounds (728, 26, 34, 38);
    presetLabel.setBounds (416, 22, 308, 32); bankLabel.setBounds (416, 52, 308, 16);
    folder.setBounds (782, 18, 76, 26); save.setBounds (782, 48, 76, 26);
    scopeArea = { 870, 14, 210, 62 };
    more.setBounds (870, 78, 210, 16);

    // OSC 1
    cWave.c.setBounds (88, 384, 170, 26); cMode.c.setBounds (88, 418, 170, 26);
    knob (detune, 92, 484, 50); knob (glide, 196, 484, 50);
    // OSC 2
    cWave2.c.setBounds (348, 384, 248, 26);
    knob (semi2, 340, 474, 56); knob (fine2, 445, 474, 56); knob (mix2, 550, 474, 56);
    // FILTER
    cFilt.c.setBounds (688, 384, 160, 26);
    knob (cutoff, 740, 472, 84);
    knob (res, 872, 466, 56); knob (fenv, 952, 466, 56); knob (drive, 1032, 466, 56);
    // AMP
    for (int i = 0; i < 4; ++i) ampEnv[i].s.setBounds (40 + i * 48, 578, 30, 112);
    knob (velGain, 290, 612, 46); knob (velEnv, 290, 682, 46);
    // LFO
    cLfo.c.setBounds (428, 576, 140, 26); cDest.c.setBounds (428, 610, 140, 26);
    pitchSwitch.setBounds (376, 682, 44, 22);
    knob (rate, 592, 604, 50); knob (amount, 592, 680, 50);
    for (int i = 0; i < 4; ++i) lfoEnv[i].s.setBounds (636 + i * 30, 586, 26, 104);
    // EFFECTS
    knob (dist, 812, 604, 50); knob (chorus, 878, 604, 50); knob (reverb, 944, 604, 50);
    knob (dmix, 812, 680, 50); knob (dtime, 878, 680, 50); knob (dfb, 944, 680, 50);
    knob (volume, 1030, 642, 70);

    // clavier
    bend.s.setBounds (28, 754, 38, 92); modw.s.setBounds (84, 754, 38, 92);
    keys.setBounds (140, 740, W - 160, 106);
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));
}
