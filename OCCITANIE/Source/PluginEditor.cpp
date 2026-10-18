#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Occ;

namespace
{
// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 4 octaves + 1 key
constexpr int kKeyLow = 48, kKeyHigh = 96;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }
const juce::Colour amber { 0xffffa63a }, ember { 0xffe0462a }, dark { 0xff120a0a };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }

// Section header: text centred between two thin rules. x0/x1/yc are mockup coordinates (see K).
void drawHeader (juce::Graphics& g, const juce::String& text, juce::Colour col, float size, int x0, int x1, int yc)
{
    const juce::Font font (juce::FontOptions (size, juce::Font::bold));
    const float tw = juce::GlyphArrangement::getStringWidth (font, text), gap = 7.f;
    const float px0 = (float) x0 * K, px1 = (float) x1 * K, cy = (float) yc * K, cx = (px0 + px1) * 0.5f;
    g.setFont (font); g.setColour (col);
    g.drawText (text, juce::Rectangle<float> (cx - tw * 0.5f - 3.f, cy - 10.f, tw + 6.f, 20.f), juce::Justification::centred, false);
    g.setColour (col.withAlpha (0.7f));
    if (cx - tw * 0.5f - gap > px0) g.drawLine (px0, cy, cx - tw * 0.5f - gap, cy, 1.f);
    if (cx + tw * 0.5f + gap < px1) g.drawLine (cx + tw * 0.5f + gap, cy, px1, cy, 1.f);
}
}

OccLNF::OccLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffffc58a));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffc58a));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff5a2a14));
}

void OccLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void OccLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void OccLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffffc58a)); g.fillPath (t);
}

void OccLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void OccLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff5a2a14) : over ? juce::Colour (0xff3a1c10) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
OccPanel::OccPanel (OccProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    logo = load (BinaryData::logo_png, BinaryData::logo_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::rate, "RATE");       addKnob (amount, P::amount, "AMOUNT");  addKnob (pitch, P::pitch, "TRANSPOSE");
    addKnob (glide, P::glide, "GLIDE");    addKnob (filter, P::filter, "CUTOFF");
    addKnob (width, P::t4, "WIDTH");       addKnob (texture, P::t5, "TEXTURE");
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DLY MIX");
    addKnob (dtime, P::dtime, "DLY TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    const char* tn[5] = { "REED", "BODY", "BRIGHT", "DRONE", "VELOCITY" };
    const int ti[5] = { P::t1, P::t2, P::t3, P::drone, P::velocity };
    for (int i = 0; i < 5; ++i) addKnob (timb[i], ti[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &width, &texture }) k->s.getProperties().set ("small", true);

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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffc58a));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setAvailableRange (kKeyLow, kKeyHigh);   // must come before sizing: with scroll buttons hidden the first visible key is the range start
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    keys.setLowestVisibleKey (kKeyLow);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

OccPanel::~OccPanel() { setLookAndFeel (nullptr); }

void OccPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void OccPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void OccPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void OccPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File OccPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("OCCITANIE").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void OccPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.occpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .occpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.occpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void OccPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.occpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("occpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void OccPanel::loadUserPreset (const juce::File& f)
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

void OccPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void OccPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff202020), 0, 0, juce::Colour (0xff0c0c0c), 0, (float) getHeight(), false));
    g.fillAll();
    for (int side = 0; side < 2; ++side)   // rails metal
    {
        auto r = side == 0 ? R (30, 25, 120, 1440) : R (1850, 25, 120, 1440);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffa0a0a0), (float) r.getX(), 0, juce::Colour (0xff383838), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 6.f);
        g.setColour (juce::Colour (0xff1a1010)); g.fillRoundedRectangle (r.reduced (28, 110).toFloat(), 8.f);
        for (int y : { 78, 1407 }) { g.setColour (juce::Colour (0xff1d1d1d)); g.fillEllipse ((float) r.getCentreX() - 11, y * K - 11, 22.f, 22.f); }
    }
    // haut-parleurs
    for (auto r : { R (545, 310, 185, 205), R (1270, 310, 190, 205), R (1505, 310, 285, 205) })
    {
        g.setColour (juce::Colour (0xff0a0606)); g.fillRoundedRectangle (r.toFloat(), 6.f);
        for (float yy = (float) r.getY() + 7; yy < (float) r.getBottom() - 6; yy += 11.f)
            for (float xx = (float) r.getX() + 8; xx < (float) r.getRight() - 6; xx += 11.f)
            { g.setColour (ramp ((yy - (float) r.getY()) / (float) r.getHeight()).withAlpha (0.18f)); g.fillEllipse (xx - 1, yy - 1, 8.f, 8.f); g.setColour (juce::Colour (0xff1d1212)); g.fillEllipse (xx, yy, 6.f, 6.f); }
    }
    // medaillon
    g.drawImage (camel, R (775, 215, 455, 535).toFloat(), juce::RectanglePlacement::centred);
    g.drawImage (logo, R (1585, 190, 88, 88).toFloat(), juce::RectanglePlacement::centred);
    auto txt = [&] (const juce::String& s, juce::Rectangle<int> r, float size, bool bold, juce::Colour c1, juce::Colour c2)
    {
        g.setGradientFill (juce::ColourGradient (c1, (float) r.getX(), 0, c2, (float) r.getRight(), 0, false));
        g.setFont (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
        g.drawFittedText (s, r, juce::Justification::centred, 1);
    };
    txt ("DJBILBOXBEATS", R (1430, 90, 390, 80), 34.f, true, juce::Colour (0xffffc58a), juce::Colour (0xffe9773a));
    // titre grave
    g.setColour (ember.withAlpha (0.22f)); g.setFont (juce::FontOptions (70.f, juce::Font::bold)); g.drawFittedText ("OCCITANIE", R (725, 725, 550, 135), juce::Justification::centred, 1);
    txt ("OCCITANIE", R (730, 730, 540, 125), 66.f, true, juce::Colour (0xffffc58a), juce::Colour (0xffe0462a));
    g.setColour (juce::Colour (0xffb8b0a8)); g.setFont (juce::FontOptions (18.f));
    g.drawFittedText ("By Humpire", R (850, 862, 300, 50), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (610, 75, 780, 110).toFloat();
    g.setColour (amber.withAlpha (0.2f)); g.fillRoundedRectangle (scr.expanded (3.f), 14.f);
    g.setColour (juce::Colour (0xff8a8a8a)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (juce::Colour (0xff1a0a0a)); g.fillRoundedRectangle (scr.reduced (6.f), 8.f);
    // cassette
    auto cas = R (207, 105, 280, 405).toFloat();
    g.setColour (amber.withAlpha (0.25f)); g.fillRoundedRectangle (cas.expanded (3.f), 10.f);
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, cas, juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffb0a8a0)); g.setFont (juce::FontOptions (16.f));
    g.drawFittedText ("EXPANSION", R (207, 45, 280, 50), juce::Justification::centred, 1);
    // panneau molettes
    g.setColour (juce::Colour (0xff0b0707)); g.fillRoundedRectangle (R (190, 1120, 325, 300).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff2e2a2a)); g.drawRoundedRectangle (R (190, 1120, 325, 300).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffd0c8c0));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 11.f : (k->lin ? (k->s.getHeight() < 80 ? 10.f : 12.f) : 14.f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : -4), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    // section headers (rules + text), one per group of controls
    const auto ink = juce::Colour (0xffffc58a);
    drawHeader (g, "LFO", ink, 14.f, 195, 474, 566);
    drawHeader (g, "AMP ENVELOPE", ink, 12.f, 205, 625, 752);
    drawHeader (g, "TIMBRE", ink, 11.f, 1290, 1515, 534);
    // captions under the five drop-down menus
    g.setColour (juce::Colour (0xffb0a8a0)); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    {
        const char* cap[5] = { "ENGINE", "LFO SHAPE", "LFO DESTINATION", "FILTER TYPE", "VOICE MODE" };
        const int cx[5] = { 345, 672, 999, 1327, 1654 };
        for (int i = 0; i < 5; ++i) g.drawFittedText (cap[i], R (cx[i] - 140, 1078, 280, 34), juce::Justification::centred, 1);
    }
    // wheel plate captions
    g.setColour (juce::Colour (0xffd0c8c0)); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    g.drawFittedText ("PITCH BEND", R (275 - 80, 1128, 160, 44), juce::Justification::centred, 1);
    g.drawFittedText ("MOD WHEEL", R (430 - 80, 1128, 160, 44), juce::Justification::centred, 1);
}

void OccPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 257, 650, 108);  place (amount, 412, 650, 108); place (pitch, 568, 650, 108);
    place (glide, 1273, 920, 108); place (filter, 1010, 945, 96);
    place (width, 790, 945, 82);   place (texture, 890, 945, 82);
    place (volume, 1775, 630, 108); place (reverb, 1433, 775, 108); place (chorus, 1587, 775, 108); place (dist, 1740, 775, 108);
    place (delay, 1433, 920, 108); place (dtime, 1587, 920, 108); place (feedback, 1740, 920, 108);
    const int ex[4] = { 253, 359, 466, 573 }, tx[5] = { 1330, 1402, 1474, 1565, 1650 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 30, 780, 60, 185));
    for (int i = 0; i < 5; ++i) timb[i].s.setBounds (R (tx[i] - 24, 548, 48, 135));
    bend.s.setBounds (C (275, 1290, 90, 210)); modw.s.setBounds (C (430, 1290, 90, 210));
    cWave.c.setBounds (R (203, 1016, 285, 58)); cLfo.c.setBounds (R (530, 1016, 285, 58)); cDest.c.setBounds (R (857, 1016, 285, 58));
    cFilt.c.setBounds (R (1185, 1016, 285, 58)); cMono.c.setBounds (R (1512, 1016, 285, 58));
    folder.setBounds (R (630, 92, 150, 76)); save.setBounds (R (1245, 92, 135, 76));
    prev.setBounds (R (785, 92, 70, 76)); next.setBounds (R (1150, 92, 70, 76));
    presetLabel.setBounds (R (860, 92, 280, 76));
    expansion.setBounds (R (207, 105, 280, 405));
    keys.setBounds (R (555, 1125, 1245, 330));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly
}
