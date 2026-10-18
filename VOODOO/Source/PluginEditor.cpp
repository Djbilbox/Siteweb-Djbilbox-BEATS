#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Voodoo;

namespace
{
const juce::Colour red { 0xffff3b52 }, purple { 0xffb44bff }, panel { 0xffe8e2e0 }, dark { 0xff1a0f1c };
constexpr float K = 0.5f; // mockup 2000 px -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }

// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 48..96 = 4 octaves + 1 key
constexpr int kKeyLow = 48, kKeyHigh = 96;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }

// Section header: text centred between two thin rules. x0/x1/yc are mockup coordinates (see K).
void drawHeader (juce::Graphics& g, const juce::String& text, juce::Colour col, float size, int x0, int x1, int yc)
{
    const juce::Font font (juce::FontOptions (size, juce::Font::bold));
    const float tw = juce::GlyphArrangement::getStringWidth (font, text), gap = 7.f;
    const float px0 = (float) x0 * K, px1 = (float) x1 * K, cy = (float) yc * K, cx = (px0 + px1) * 0.5f;
    g.setFont (font); g.setColour (col);
    g.drawText (text, juce::Rectangle<float> (cx - tw * 0.5f - 3.f, cy - 10.f, tw + 6.f, 20.f), juce::Justification::centred, false);
    g.setColour (col.withAlpha (0.6f));
    if (cx - tw * 0.5f - gap > px0) g.drawLine (px0, cy, cx - tw * 0.5f - gap, cy, 1.f);
    if (cx + tw * 0.5f + gap < px1) g.drawLine (cx + tw * 0.5f + gap, cy, px1, cy, 1.f);
}
}

VoodooLNF::VoodooLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffff8a98));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffb0ba));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff5a1f4a));
    setColour (juce::Slider::thumbColourId, juce::Colours::lightgrey);
}

void VoodooLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.f);
    const float r = juce::jmin (c.getWidth(), c.getHeight()) * 0.5f;
    const auto ctr = c.getCentre();
    const auto ring = s.getProperties()["purple"] ? purple : red;
    const float ang = a0 + pos * (a1 - a0);
    juce::Path track; track.addCentredArc (ctr.x, ctr.y, r, r, 0, a0, a1, true);
    g.setColour (ring.withAlpha (0.18f)); g.strokePath (track, juce::PathStrokeType (5.f));
    juce::Path val; val.addCentredArc (ctr.x, ctr.y, r, r, 0, a0, ang, true);
    g.setColour (ring.withAlpha (0.35f)); g.strokePath (val, juce::PathStrokeType (9.f));
    g.setColour (ring); g.strokePath (val, juce::PathStrokeType (4.5f));
    const float kr = r * 0.74f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff2f2f2), ctr.x - kr * 0.4f, ctr.y - kr * 0.5f, juce::Colour (0xff4a4a4a), ctr.x + kr * 0.6f, ctr.y + kr * 0.7f, false));
    g.fillEllipse (ctr.x - kr, ctr.y - kr, kr * 2, kr * 2);
    g.setColour (juce::Colours::black.withAlpha (0.6f)); g.drawEllipse (ctr.x - kr, ctr.y - kr, kr * 2, kr * 2, 1.5f);
    juce::Path p; p.addRoundedRectangle (-1.8f, -kr, 3.6f, kr * 0.45f, 1.5f);
    g.setColour (juce::Colour (0xff202020)); g.fillPath (p, juce::AffineTransform::rotation (ang).translated (ctr.x, ctr.y));
}

void VoodooLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const bool wheel = s.getProperties()["wheel"];
    const float cx = (float) x + (float) w * 0.5f;
    if (wheel)
    {
        g.setColour (juce::Colour (0xff0c0c0c)); g.fillRoundedRectangle ((float) x + 4, (float) y, (float) w - 8, (float) h, 8.f);
        const float th = 48.f;
        auto cap = juce::Rectangle<float> (cx - 20.f, pos - th * 0.5f, 40.f, th);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6a6a6a), cap.getX(), 0, juce::Colour (0xff2a2a2a), cap.getRight(), 0, false));
        g.fillRoundedRectangle (cap, 8.f);
        g.setColour (juce::Colours::black); for (int i = 1; i < 6; ++i) g.drawHorizontalLine ((int) (cap.getY() + i * th / 6.f), cap.getX() + 4, cap.getRight() - 4);
        return;
    }
    g.setColour (juce::Colour (0xff222222)); g.fillRoundedRectangle (cx - 2.5f, (float) y, 5.f, (float) h, 2.f);
    g.setColour (red.withAlpha (0.8f)); g.fillRoundedRectangle (cx - 2.5f, pos, 5.f, (float) y + (float) h - pos, 2.f);
    auto cap = juce::Rectangle<float> (cx - 12.f, pos - 7.f, 24.f, 14.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe0e0e0), 0, cap.getY(), juce::Colour (0xff555555), 0, cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 3.f);
    g.setColour (juce::Colours::black); g.drawRoundedRectangle (cap, 3.f, 1.2f);
    g.drawHorizontalLine ((int) cap.getCentreY(), cap.getX() + 3, cap.getRight() - 3);
}

void VoodooLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& b)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    const auto glow = b.getProperties()["purple"] ? purple : red;
    g.setColour (glow.withAlpha (0.25f)); g.fillRoundedRectangle (r.expanded (1.5f), 8.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 7.f);
    g.setColour (glow); g.drawRoundedRectangle (r, 7.f, 1.6f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffff8a98)); g.fillPath (t);
}

void VoodooLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void VoodooLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff5a1f4a) : over ? juce::Colour (0xff3a1a3a) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (red.withAlpha (0.8f)); g.drawRoundedRectangle (r, 6.f, 1.3f);
}

//==============================================================================
VoodooPanel::VoodooPanel (VoodooProcessor& p)
    : proc (p),
      keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    camel = juce::ImageFileFormat::loadFrom (BinaryData::camel_png, (size_t) BinaryData::camel_pngSize);
    logo = juce::ImageFileFormat::loadFrom (BinaryData::logo_png, (size_t) BinaryData::logo_pngSize);

    addKnob (rate, P::rate, "RATE", false);        addKnob (amount, P::amount, "AMOUNT", true);
    addKnob (pitch, P::pitch, "TRANSPOSE", false);     addKnob (glide, P::glide, "GLIDE", true);
    addKnob (filter, P::filter, "CUTOFF", false);  addKnob (volume, P::volume, "VOLUME", true);
    addKnob (reverb, P::reverb, "REVERB", false);  addKnob (chorus, P::chorus, "CHORUS", true);
    addKnob (dist, P::dist, "DISTORTION", false);  addKnob (delay, P::delay, "DLY MIX", true);
    addKnob (dtime, P::dtime, "DLY TIME", false);   addKnob (feedback, P::fb, "DLY FDBK", true);
    const char* en[4] = { "ATK", "DEC", "SUS", "REL" };
    for (int i = 0; i < 4; ++i)
    {
        addKnob (env[i], P::atk + i, en[i], false, juce::Slider::LinearVertical);
        env[i].s.getProperties().set ("fs", 13.f); env[i].s.getProperties().set ("lw", 34);   // caption size / max width (px): neighbours are only 33 px apart
    }
    const char* tm[5] = { "BREATH", "BODY", "BRIGHT", "WIDTH", "TEXTURE" };   // timbre sliders: names of Params t1..t5 (the sliders are too close together for one row of captions)
    for (int i = 0; i < 5; ++i)
    {
        addKnob (timb[i], P::t1 + i, tm[i], false, juce::Slider::LinearVertical);
        timb[i].s.getProperties().set ("fs", 11.f); timb[i].s.getProperties().set ("lw", 56); timb[i].s.getProperties().set ("row", i & 1);   // alternate two caption rows
    }
    addKnob (bend, P::bend, "PITCH BEND", false, juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "MOD WHEEL", false, juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    bend.s.setDoubleClickReturnValue (true, 0.0);
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); }; // retour au centre

    addCombo (cWave, P::wave, engineNames);  addCombo (cLfo, P::lfosrc, lfoNames);
    addCombo (cDest, P::dest, destNames);    cDest.c.getProperties().set ("purple", false);
    addCombo (cFilt, P::ftype, filterNames); cFilt.c.getProperties().set ("purple", true);
    addCombo (cMono, P::mono, monoNames);    cMono.c.getProperties().set ("purple", true);

    for (auto* b : { &prev, &next, &folder, &save, &expansion }) { addAndMakeVisible (*b); }
    prev.onClick = [this] { stepPreset (-1); };
    next.onClick = [this] { stepPreset (1); };
    folder.onClick = [this] { showPresetMenu(); };
    save.onClick = [this] { savePreset(); };
    expansion.onClick = [this] { showExpansion(); };

    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (juce::FontOptions (22.f, juce::Font::plain));
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffff8a98));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setAvailableRange (kKeyLow, kKeyHigh);   // must come before sizing: with scroll buttons hidden the first visible key is the range start
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, red.withAlpha (0.55f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, purple.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    keys.setLowestVisibleKey (kKeyLow);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

VoodooPanel::~VoodooPanel() { setLookAndFeel (nullptr); }

void VoodooPanel::addKnob (Knob& k, int idx, const juce::String& label, bool purple, juce::Slider::SliderStyle style)
{
    k.label = label;
    k.s.setSliderStyle (style);
    k.s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    k.s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    k.s.getProperties().set ("purple", purple);
    k.s.setPopupDisplayEnabled (true, true, this);
    k.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, ids[idx], k.s);
    addAndMakeVisible (k.s);
    all.push_back (&k);
}

void VoodooPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void VoodooPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void VoodooPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    proc.applyPreset ((proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex) + d < 0 ? n - 1 : ((proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex) + d) % n);
    refreshPresetLabel();
}

void VoodooPanel::showPresetMenu()
{
    juce::PopupMenu menu;
    juce::String cur; juce::PopupMenu sub; int idx = 0;
    const auto& list = presets();
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (cur != list[i].bank)
        {
            if (cur.isNotEmpty()) menu.addSubMenu (cur, sub);
            sub = juce::PopupMenu(); cur = list[i].bank;
        }
        sub.addItem ((int) i + 1, list[i].name, true, (int) i == proc.presetIndex);
        idx++;
    }
    menu.addSubMenu (cur, sub);
    menu.addSeparator();
    juce::PopupMenu user; auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.voodoopreset");
    files.sort();
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .voodoopreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.voodoopreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void VoodooPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.voodoopreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("voodoopreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

juce::File VoodooPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("VOODOO").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void VoodooPanel::loadUserPreset (const juce::File& f)
{
    if (auto xml = juce::XmlDocument::parse (f))
        if (xml->hasTagName (proc.apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            for (int i = 0; i < COUNT; ++i) // ne touche qu'aux parametres existants, valeurs bornees
                if (auto* prm = proc.apvts.getParameter (ids[i]))
                    if (auto child = vt.getChildWithProperty ("id", ids[i]); child.isValid() && i != P::bend && i != P::modw)
                        prm->setValueNotifyingHost (prm->convertTo0to1 ((float) child.getProperty ("value", prm->convertFrom0to1 (prm->getValue()))));
            proc.presetIndex = -1; proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
        }
}

void VoodooPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void VoodooPanel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (panel.brighter (0.1f), b.getX(), b.getY(), panel.darker (0.12f), b.getRight(), b.getBottom(), false));
    g.fillAll();
    // rails metal
    for (int side = 0; side < 2; ++side)
    {
        auto r = side == 0 ? R (0, 0, 70, 1493) : R (1930, 0, 70, 1493);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8a8a8a), (float) r.getX(), 0, juce::Colour (0xff3c3c3c), (float) r.getRight(), 0, false));
        g.fillRect (r);
        g.setColour (juce::Colour (0xff1a1a1a)); for (int y : { 70, 745, 1420 }) g.fillEllipse ((float) r.getCentreX() - 9, y * K - 9, 18.f, 18.f);
    }
    // haut-parleurs
    for (auto r : { R (612, 338, 140, 312), R (1248, 338, 140, 312) })
    {
        g.setColour (juce::Colour (0xffc9a3a8)); g.fillRoundedRectangle (r.toFloat(), 6.f);
        g.setColour (juce::Colour (0xff301820));
        for (float yy = (float) r.getY() + 8; yy < (float) r.getBottom() - 6; yy += 11.f)
            for (float xx = (float) r.getX() + 9; xx < (float) r.getRight() - 6; xx += 11.f) g.fillEllipse (xx, yy, 6.f, 6.f);
    }
    // medaillon chameau
    auto med = R (815, 305, 370, 410).toFloat();
    g.setColour (juce::Colour (0xff6a6a6a)); g.fillEllipse (med.expanded (6.f));
    { juce::Path clip; clip.addEllipse (med.reduced (3.f)); g.saveState(); g.reduceClipRegion (clip); g.drawImage (camel, med.reduced (3.f)); g.restoreState(); }
    // logo
    g.drawImage (logo, R (1440, 60, 380, 210).toFloat(), juce::RectanglePlacement::centred);
    // titre
    g.setColour (red); g.setFont (juce::FontOptions (64.f, juce::Font::bold));
    g.drawFittedText ("VOODOO", R (780, 745, 440, 140), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xff222222)); g.setFont (juce::FontOptions (19.f, juce::Font::bold));
    g.drawFittedText ("By Humpire", R (780, 895, 440, 70), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (610, 105, 780, 125).toFloat();
    g.setColour (red.withAlpha (0.3f)); g.fillRoundedRectangle (scr.expanded (4.f), 14.f);
    g.setColour (juce::Colour (0xff8a8a8a)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (dark); g.fillRoundedRectangle (scr.reduced (8.f), 8.f);
    // panneau molettes
    g.setColour (juce::Colour (0xff232023)); g.fillRoundedRectangle (R (205, 1090, 385, 345).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff6a6a6a)); g.drawRoundedRectangle (R (205, 1090, 385, 345).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xff1c1c1c)); g.setFont (juce::FontOptions (15.f, juce::Font::bold));
    for (auto* k : all)
    {
        if (k == &bend || k == &modw) continue;
        const auto r = k->s.getBounds();
        const bool lin = k->s.getSliderStyle() == juce::Slider::LinearVertical;
        const auto& pr = k->s.getProperties();
        g.setFont (juce::FontOptions ((float) pr.getWithDefault ("fs", 15.f), juce::Font::bold));
        const int lw = (int) pr.getWithDefault ("lw", r.getWidth() + 40), row = (int) pr.getWithDefault ("row", 0);
        g.drawFittedText (k->label, r.getCentreX() - lw / 2, r.getBottom() + (lin ? 2 : -2) + row * 12, lw, 20, juce::Justification::centred, 1);
    }
    // section headers (rules + text), one per group of controls
    const auto ink = juce::Colour (0xff1c1c1c);
    drawHeader (g, "LFO", ink, 16.f, 190, 548, 408);
    drawHeader (g, "EFFECTS", ink, 16.f, 1470, 1815, 408);
    drawHeader (g, "AMP ENVELOPE", ink, 14.f, 505, 790, 718);
    drawHeader (g, "TIMBRE", ink, 14.f, 1200, 1500, 718);
    // captions under the five drop-down menus
    g.setColour (ink); g.setFont (juce::FontOptions (11.f, juce::Font::bold));
    {
        const char* cap[5] = { "ENGINE", "LFO SHAPE", "LFO DESTINATION", "FILTER TYPE", "VOICE MODE" };
        const int cx[5] = { 347, 667, 1000, 1335, 1655 };
        for (int i = 0; i < 5; ++i) g.drawFittedText (cap[i], R (cx[i] - 140, 1058, 280, 28), juce::Justification::centred, 1);
    }
    g.setColour (juce::Colour (0xffcfcfcf));
    g.setFont (juce::FontOptions (12.f, juce::Font::bold));
    g.drawFittedText ("Pitch Bend", R (240, 1385, 170, 40), juce::Justification::centred, 1);
    g.drawFittedText ("Mod Wheel", R (385, 1385, 170, 40), juce::Justification::centred, 1);
}

void VoodooPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (R (cx - sz / 2, cy - sz / 2, sz, sz)); };
    place (rate, 280, 530, 150);   place (amount, 458, 530, 150);
    place (pitch, 272, 718, 140);  place (glide, 431, 718, 140);
    place (filter, 272, 888, 140); place (volume, 431, 888, 140);
    place (reverb, 1555, 520, 160); place (chorus, 1728, 520, 160);
    place (dist, 1569, 718, 140);   place (delay, 1727, 718, 140);
    place (dtime, 1569, 888, 140);  place (feedback, 1727, 888, 140);
    const int ex[4] = { 548, 614, 680, 746 }, tx[5] = { 1236, 1292, 1348, 1404, 1460 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 25, 745, 50, 170));   // slightly shorter than the knob columns: leaves room for two caption rows above the combos
    for (int i = 0; i < 5; ++i) timb[i].s.setBounds (R (tx[i] - 22, 745, 44, 170));
    bend.s.setBounds (R (290, 1135, 70, 250)); modw.s.setBounds (R (435, 1135, 70, 250));
    cWave.c.setBounds (R (207, 992, 280, 62)); cLfo.c.setBounds (R (527, 992, 280, 62)); cDest.c.setBounds (R (860, 992, 280, 62));
    cFilt.c.setBounds (R (1195, 992, 280, 62)); cMono.c.setBounds (R (1515, 992, 280, 62));
    folder.setBounds (R (632, 128, 130, 80)); save.setBounds (R (1236, 128, 130, 80));
    prev.setBounds (R (770, 128, 70, 80)); next.setBounds (R (1160, 128, 70, 80));
    presetLabel.setBounds (R (850, 128, 300, 80));
    expansion.setBounds (R (205, 250, 370, 100));
    keys.setBounds (R (628, 1090, 1172, 380));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly
}
