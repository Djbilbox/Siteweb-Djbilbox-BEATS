#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Fk;

namespace
{
const juce::Colour amber { 0xffffd36a }, ember { 0xffb44bff }, dark { 0xff0f0a14 };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }

// Section header: text centred on (cx, cy) in mock-up units, thin rules running out to +/-half on both sides of the text.
void header (juce::Graphics& g, const juce::String& text, int cx, int cy, int half, juce::Colour col, float size = 13.f, bool rules = true)
{
    const juce::Font font (juce::FontOptions (size, juce::Font::bold));
    juce::GlyphArrangement ga; ga.addLineOfText (font, text, 0.f, 0.f);
    const float tw = ga.getBoundingBox (0, -1, true).getWidth();
    const float x = (float) cx * K, y = (float) cy * K, h = (float) half * K;
    g.setFont (font);
    g.setColour (col);
    g.drawText (text, juce::Rectangle<float> (x - tw * 0.5f - 6.f, y - 10.f, tw + 12.f, 20.f), juce::Justification::centred, false);
    if (rules && h > tw * 0.5f + 14.f)
    {
        g.setColour (col.withAlpha (0.45f));
        g.drawLine (x - h, y, x - tw * 0.5f - 8.f, y, 1.f);
        g.drawLine (x + tw * 0.5f + 8.f, y, x + h, y, 1.f);
    }
}

// Small caption centred on (cx, cy) in mock-up units.
void caption (juce::Graphics& g, const juce::String& text, int cx, int cy, juce::Colour col, float size = 10.f)
{
    g.setColour (col);
    g.setFont (juce::FontOptions (size, juce::Font::bold));
    g.drawText (text, juce::Rectangle<float> ((float) cx * K - 60.f, (float) cy * K - 8.f, 120.f, 16.f), juce::Justification::centred, false);
}
}

FkLNF::FkLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffffe0a0));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffe0a0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2060));
}

void FkLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const float r = juce::jmin (c.getWidth(), c.getHeight()) * 0.5f;
    const auto ctr = c.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const auto col = ramp (pos);
    juce::Path track; track.addCentredArc (ctr.x, ctr.y, r, r, 0, a0, a1, true);
    g.setColour (juce::Colours::white.withAlpha (0.08f)); g.strokePath (track, juce::PathStrokeType (4.f));
    juce::Path val; val.addCentredArc (ctr.x, ctr.y, r, r, 0, a0, ang, true);
    g.setColour (col.withAlpha (0.3f)); g.strokePath (val, juce::PathStrokeType (8.f));
    g.setColour (col); g.strokePath (val, juce::PathStrokeType (3.5f));
    const float kr = r * 0.74f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffeaeaea), ctr.x - kr * 0.4f, ctr.y - kr * 0.5f, juce::Colour (0xff3d3d3d), ctr.x + kr * 0.6f, ctr.y + kr * 0.7f, false));
    g.fillEllipse (ctr.x - kr, ctr.y - kr, kr * 2, kr * 2);
    g.setColour (juce::Colours::black.withAlpha (0.7f)); g.drawEllipse (ctr.x - kr, ctr.y - kr, kr * 2, kr * 2, 1.4f);
    juce::Path p; p.addRoundedRectangle (-1.6f, -kr, 3.2f, kr * 0.42f, 1.4f);
    g.setColour (juce::Colour (0xff1a1a1a)); g.fillPath (p, juce::AffineTransform::rotation (ang).translated (ctr.x, ctr.y));
    juce::ignoreUnused (s);
}

void FkLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const bool wheel = s.getProperties()["wheel"];
    const float cx = (float) x + (float) w * 0.5f;
    if (wheel)
    {
        g.setColour (juce::Colour (0xff050505)); g.fillRoundedRectangle ((float) x + 6, (float) y, (float) w - 12, (float) h, 8.f);
        const float th = 52.f;
        auto cap = juce::Rectangle<float> (cx - 20.f, pos - th * 0.5f, 40.f, th);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5d5d5d), cap.getX(), 0, juce::Colour (0xff222222), cap.getRight(), 0, false));
        g.fillRoundedRectangle (cap, 8.f);
        g.setColour (juce::Colours::black); for (int i = 1; i < 7; ++i) g.drawHorizontalLine ((int) (cap.getY() + i * th / 7.f), cap.getX() + 4, cap.getRight() - 4);
        return;
    }
    g.setColour (juce::Colour (0xff060606)); g.fillRoundedRectangle (cx - 3.f, (float) y, 6.f, (float) h, 2.f);
    const float t = 1.f - (pos - (float) y) / (float) h;
    g.setColour (ramp (1.f - t).withAlpha (0.9f)); g.fillRoundedRectangle (cx - 2.f, pos, 4.f, (float) y + (float) h - pos, 2.f);
    for (int i = 0; i <= 8; ++i)   // graduations braise
    {
        const float yy = (float) y + (float) h * (float) i / 8.f;
        g.setColour (ramp ((float) i / 8.f).withAlpha (0.6f));
        g.fillRect (cx - 16.f, yy, 5.f, 2.f); g.fillRect (cx + 11.f, yy, 5.f, 2.f);
    }
    auto cap = juce::Rectangle<float> (cx - 12.f, pos - 7.f, 24.f, 14.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffdcdcdc), 0, cap.getY(), juce::Colour (0xff4b4b4b), 0, cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 3.f);
    g.setColour (juce::Colours::black); g.drawRoundedRectangle (cap, 3.f, 1.2f); g.drawHorizontalLine ((int) cap.getCentreY(), cap.getX() + 3, cap.getRight() - 3);
}

void FkLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffffe0a0)); g.fillPath (t);
}

void FkLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void FkLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff3a2060) : over ? juce::Colour (0xff2a1844) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
FkPanel::FkPanel (FkProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    logo = load (BinaryData::logo_png, BinaryData::logo_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::rate, "RATE");       addKnob (amount, P::amount, "AMOUNT");  addKnob (pitch, P::pitch, "TRANSPOSE");
    addKnob (glide, P::glide, "GLIDE");    addKnob (filter, P::filter, "CUTOFF");
    addKnob (vowel, P::t1, "VOWEL");       addKnob (body, P::t2, "BODY");       addKnob (bright, P::t3, "BRIGHT");
    addKnob (width, P::t4, "WIDTH");       addKnob (texture, P::t5, "TEXTURE");
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DELAY MIX");
    addKnob (dtime, P::dtime, "TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    const char* tn[4] = { "DEPTH", "DECAY", "RESO", "VEL" };
    const int ti[4] = { P::fenv, P::fdecay, P::reso, P::velocity };
    for (int i = 0; i < 4; ++i) addKnob (rsl[i], ti[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &vowel, &body, &bright, &width, &texture }) k->s.getProperties().set ("small", true);

    addCombo (cWave, P::wave, engineNames);  addCombo (cLfo, P::lfosrc, lfoNames);  addCombo (cDest, P::dest, destNames);
    addCombo (cFilt, P::ftype, filterNames); addCombo (cMono, P::mono, monoNames);

    for (auto* b : { &prev, &next, &folder, &save }) addAndMakeVisible (*b);
    addAndMakeVisible (expansion);
    expansion.getProperties().set ("flat", true);
    prev.onClick = [this] { stepPreset (-1); };
    next.onClick = [this] { stepPreset (1); };
    folder.onClick = [this] { showPresetMenu(); };
    save.onClick = [this] { savePreset(); };
    expansion.onClick = [this] { showExpansion(); };

    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (juce::FontOptions (19.f, juce::Font::plain));
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffe0a0));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setKeyWidth (27.4f);
    // JUCE 8 resets the first visible key to the start of the available range when scroll buttons are hidden,
    // so the playable range itself is restricted: MIDI 36..96 = 5 octaves (C2..C7), 36 white keys (fitted in resized()).
    keys.setAvailableRange (36, 96);
    keys.setOctaveForMiddleC (4);   // labels: middle C (MIDI 60) = C4, first key = C2
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

FkPanel::~FkPanel() { setLookAndFeel (nullptr); }

void FkPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
{
    k.label = label; k.lin = style == juce::Slider::LinearVertical;
    k.s.setSliderStyle (style);
    k.s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    k.s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    k.s.setPopupDisplayEnabled (true, true, this);
    k.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, ids[idx], k.s);
    addAndMakeVisible (k.s);
    all.push_back (&k);
}

void FkPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void FkPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void FkPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File FkPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("FUNKYKOPS").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void FkPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.fkpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .fkpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.fkpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void FkPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.fkpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("fkpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void FkPanel::loadUserPreset (const juce::File& f)
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

void FkPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void FkPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1c1c20), 0, 0, juce::Colour (0xff0b0b0e), 0, (float) getHeight(), false));
    g.fillAll();
    for (int side = 0; side < 2; ++side)   // rails metal
    {
        auto r = side == 0 ? R (8, 10, 125, 1470) : R (1870, 10, 125, 1470);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffa4a4aa), (float) r.getX(), 0, juce::Colour (0xff383840), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 6.f);
        g.setColour (juce::Colour (0xff17141c)); g.fillRoundedRectangle (r.reduced (30, 120).toFloat(), 10.f);
        for (int y : { 80, 745, 1405 }) { g.setColour (juce::Colour (0xff1d1d22)); g.fillEllipse ((float) r.getCentreX() - 11, y * K - 11, 22.f, 22.f); }
    }
    // haut-parleurs
    for (auto r : { R (615, 278, 140, 190), R (1245, 278, 572, 190) })
    {
        g.setColour (juce::Colour (0xff5a5a62)); g.fillRoundedRectangle (r.toFloat(), 6.f);
        g.setColour (juce::Colour (0xff0b0b0e)); g.fillRoundedRectangle (r.reduced (6).toFloat(), 4.f);
        for (float yy = (float) r.getY() + 14; yy < (float) r.getBottom() - 10; yy += 13.f)
            for (float xx = (float) r.getX() + 14; xx < (float) r.getRight() - 10; xx += 13.f)
            { g.setColour (juce::Colour (0xff35353d)); g.fillEllipse (xx, yy, 7.f, 7.f); g.setColour (juce::Colour (0xff050507)); g.fillEllipse (xx + 1, yy + 1, 5.f, 5.f); }
    }
    g.drawImage (camel, R (780, 215, 450, 470).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colours::white);
    g.drawImage (logo, R (1525, 75, 300, 150).toFloat(), juce::RectanglePlacement::centred);
    // titre
    g.setColour (ember.withAlpha (0.35f)); g.setFont (juce::FontOptions (72.f, juce::Font::bold)); g.drawFittedText ("FUNKYKOPS", R (700, 690, 600, 140), juce::Justification::centred, 1);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff4ecff), 0, 345.f, juce::Colour (0xffb890ff), 0, 400.f, false));
    g.setFont (juce::FontOptions (70.f, juce::Font::bold)); g.drawFittedText ("FUNKYKOPS", R (705, 692, 590, 130), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffb8b0c8)); g.setFont (juce::FontOptions (18.f));
    g.drawFittedText ("By Humpire", R (1090, 808, 240, 40), juce::Justification::centredRight, 1);
    // ecran preset
    auto scr = R (618, 85, 875, 105).toFloat();
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (scr.expanded (3.f), 12.f);
    g.setColour (juce::Colour (0xff75757d)); g.fillRoundedRectangle (scr, 10.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a1850), scr.getX(), 0, juce::Colour (0xff1a1030), scr.getRight(), 0, false)); g.fillRoundedRectangle (scr.reduced (6.f), 7.f);
    // cassette
    auto cas = R (180, 78, 410, 362).toFloat();
    g.setColour (ember.withAlpha (0.3f)); g.fillRoundedRectangle (cas.expanded (3.f), 12.f);
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, cas, juce::RectanglePlacement::centred);
    // panneau molettes
    g.setColour (juce::Colour (0xff0a0a0d)); g.fillRoundedRectangle (R (178, 1110, 320, 335).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff3a3a44)); g.drawRoundedRectangle (R (178, 1110, 320, 335).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffcfc8dc));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 9.f : (k->lin ? 11.f : 14.f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : -4), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    {   // section headers and captions
        const juce::Colour txt (0xffcfc8dc), dim (0xffa49cb8);
        header (g, "LFO", 325, 548, 110, txt, 16.f);                      // RATE + AMOUNT only (PITCH is the transpose knob)
        header (g, "AMP ENVELOPE", 402, 733, 215, amber, 13.f);
        header (g, "FILTER ENV", 1437, 490, 70, txt, 11.f);               // DEPTH + DECAY
        g.setColour (txt.withAlpha (0.3f));                               // resonance and velocity are not part of the filter envelope
        g.drawLine (1512.f * K, 512.f * K, 1512.f * K, 690.f * K, 1.f);
        caption (g, "WAVEFORM", 771, 1094, dim);
        caption (g, "LFO SOURCE", 1003, 1094, dim);
        caption (g, "LFO DESTINATION", 1235, 1094, dim);
        caption (g, "FILTER TYPE", 1468, 1094, dim);
        caption (g, "MONO / POLY", 1701, 1094, dim);
        caption (g, "PITCH BEND", 268, 1412, dim, 10.5f);
        caption (g, "MOD WHEEL", 396, 1412, dim, 10.5f);
    }
}

void FkPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 248, 630, 108);  place (amount, 403, 630, 108); place (pitch, 559, 630, 108);
    place (glide, 909, 895, 108); place (filter, 1094, 895, 108);
    place (vowel, 640, 905, 70);  place (body, 730, 905, 70);
    place (bright, 1200, 905, 70); place (width, 1275, 905, 70); place (texture, 1350, 905, 70);
    place (volume, 1756, 630, 108); place (reverb, 1459, 778, 108); place (chorus, 1611, 778, 108); place (dist, 1756, 778, 108);
    place (delay, 1459, 920, 108); place (dtime, 1611, 920, 108); place (feedback, 1756, 920, 108);
    const int ex[4] = { 240, 347, 456, 564 }, tx[4] = { 1400, 1475, 1550, 1625 };
    for (int i = 0; i < 4; ++i) { env[i].s.setBounds (R (ex[i] - 30, 755, 60, 210)); rsl[i].s.setBounds (R (tx[i] - 28, 510, 56, 170)); }
    bend.s.setBounds (C (268, 1255, 90, 250)); modw.s.setBounds (C (396, 1255, 90, 250));
    cWave.c.setBounds (R (667, 1022, 208, 50)); cLfo.c.setBounds (R (898, 1022, 210, 50)); cDest.c.setBounds (R (1130, 1022, 210, 50));
    cFilt.c.setBounds (R (1363, 1022, 210, 50)); cMono.c.setBounds (R (1596, 1022, 210, 50));
    folder.setBounds (R (632, 98, 150, 80)); save.setBounds (R (1385, 98, 105, 80));
    prev.setBounds (R (1262, 98, 55, 80)); next.setBounds (R (1322, 98, 55, 80));
    presetLabel.setBounds (R (790, 98, 460, 80));
    expansion.setBounds (R (180, 78, 410, 362));
    const auto kb = R (540, 1118, 1270, 335);
    keys.setKeyWidth ((float) kb.getWidth() / 36.f);   // 36 white keys between MIDI 36 and 96 fill the keyboard area exactly
    keys.setBounds (kb);
}
