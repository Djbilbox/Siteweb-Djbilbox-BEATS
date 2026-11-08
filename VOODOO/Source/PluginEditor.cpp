#include "PluginEditor.h"
#include "BinaryData.h"
#include "FaceplateLayout.h"

using namespace Voodoo;

namespace
{
const juce::Colour red { 0xffff3b52 }, purple { 0xffb44bff }, panel { 0xffe8e2e0 }, dark { 0xff1a0f1c };
const juce::Colour amber { 0xffff3b52 }, ember { 0xffb44bff };   // the premium skin's two lights: crimson and violet
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }
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
    hfx::plateRotary (g, x, y, w, h, pos, a0, a1, s, ramp (pos));
}

void VoodooLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    if ((bool) s.getProperties()["wheel"]) { hfx::plateWheel (g, x, w, pos); return; }
    hfx::plateLinear (g, x, y, w, h, pos, s, faderCap, ramp ((pos - (float) y) / (float) juce::jmax (1, h)));
}

void VoodooLNF::drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&)
{
    // the box and its arrow are part of the faceplate: only the text (the combo's label) is drawn
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
    const char* tn[5] = { "BREATH", "BODY", "BRIGHT", "WIDTH", "TEXTURE" };   // timbre sliders: names of Params t1..t5 (the sliders are too close together for one row of captions)
    for (int i = 0; i < 5; ++i)
    {
        addKnob (rsl[i], P::t1 + i, tn[i], false, juce::Slider::LinearVertical);
        rsl[i].s.getProperties().set ("fs", 11.f); rsl[i].s.getProperties().set ("lw", 56); rsl[i].s.getProperties().set ("row", i & 1);   // alternate two caption rows
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
    // premium skin: the photo-real faceplate, the live light on top (see ALL_PLUGINS/HumpireFX/PremiumSkin.h)
    face.setImage (juce::ImageFileFormat::loadFrom (BinaryData::faceplate_jpg, BinaryData::faceplate_jpgSize));
    face.setMascot (camel, { Face::mascotX, Face::mascotY }, Face::mascotR);   // the original artwork in the medallion
    lnf.faderCap = juce::ImageFileFormat::loadFrom (BinaryData::fader_png, BinaryData::fader_pngSize);
    keys.neon = ember; keys.neonHot = amber;
    keys.setOverlayOnly();   // the keys are part of the faceplate
    overlay.a = amber; overlay.b = ember; overlay.theme = hfx::Theme::ghostFlame;
    overlay.mascot = { Face::mascotX, Face::mascotY }; overlay.mascotR = Face::mascotR;
    overlay.addKnob (rate.s, hfx::PremiumOverlay::lfoRate);
    overlay.addKnob (amount.s, hfx::PremiumOverlay::lfoAmount);
    overlay.addKnob (pitch.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (glide.s, hfx::PremiumOverlay::glide);
    overlay.addKnob (filter.s, hfx::PremiumOverlay::cutoff);
    overlay.addKnob (volume.s, hfx::PremiumOverlay::volume);
    overlay.addKnob (reverb.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (chorus.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (dist.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (delay.s, hfx::PremiumOverlay::delayMix);
    overlay.addKnob (dtime.s, hfx::PremiumOverlay::delayTime);
    overlay.addKnob (feedback.s, hfx::PremiumOverlay::delayFeedback);
    overlay.addFader (env[0].s, true, 0, hfx::PremiumOverlay::ledColumns (Face::envSlots[0].x, Face::envSlots[0].top + 3.f, Face::envSlots[0].bottom - 3.f));
    overlay.addFader (env[1].s, true, 1, hfx::PremiumOverlay::ledColumns (Face::envSlots[1].x, Face::envSlots[1].top + 3.f, Face::envSlots[1].bottom - 3.f));
    overlay.addFader (env[2].s, true, 2, hfx::PremiumOverlay::ledColumns (Face::envSlots[2].x, Face::envSlots[2].top + 3.f, Face::envSlots[2].bottom - 3.f));
    overlay.addFader (env[3].s, true, 3, hfx::PremiumOverlay::ledColumns (Face::envSlots[3].x, Face::envSlots[3].top + 3.f, Face::envSlots[3].bottom - 3.f));
    overlay.addPlainFader (rsl[0].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[0].x, Face::rslSlots[0].top + 3.f, Face::rslSlots[0].bottom - 3.f));   // a timbre fader (BREATH)
    overlay.addPlainFader (rsl[1].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[1].x, Face::rslSlots[1].top + 3.f, Face::rslSlots[1].bottom - 3.f));   // a timbre fader (BODY)
    overlay.addPlainFader (rsl[2].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[2].x, Face::rslSlots[2].top + 3.f, Face::rslSlots[2].bottom - 3.f));   // a timbre fader (BRIGHT)
    overlay.addPlainFader (rsl[3].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[3].x, Face::rslSlots[3].top + 3.f, Face::rslSlots[3].bottom - 3.f));   // a timbre fader (WIDTH)
    overlay.addPlainFader (rsl[4].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[4].x, Face::rslSlots[4].top + 3.f, Face::rslSlots[4].bottom - 3.f));   // a timbre fader (TEXTURE)
    addAndMakeVisible (overlay);

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
    face.draw (g, getLocalBounds());   // the whole front panel is one photo-real render (see Assets/faceplate.jpg)
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
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 22, 745, 44, 170));
    bend.s.setBounds (R (290, 1135, 70, 250)); modw.s.setBounds (R (435, 1135, 70, 250));
    cWave.c.setBounds (R (207, 992, 280, 62)); cLfo.c.setBounds (R (527, 992, 280, 62)); cDest.c.setBounds (R (860, 992, 280, 62));
    cFilt.c.setBounds (R (1195, 992, 280, 62)); cMono.c.setBounds (R (1515, 992, 280, 62));
    folder.setBounds (R (632, 128, 130, 80)); save.setBounds (R (1236, 128, 130, 80));
    prev.setBounds (R (770, 128, 70, 80)); next.setBounds (R (1160, 128, 70, 80));
    presetLabel.setBounds (R (850, 128, 300, 80));
    expansion.setBounds (R (205, 250, 370, 100));
    keys.setBounds (R (628, 1090, 1172, 380));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly

    // on the faceplate: knobs centred on their caps (value ring just outside), faders on their slots
    for (const auto& cap : Face::caps)
        for (auto* k : { &rate, &amount, &pitch, &glide, &filter, &volume, &reverb, &chorus, &dist, &delay, &dtime, &feedback })
            if (juce::String (cap.name) == knobName (k))
            {
                const float ring = cap.r + (cap.r > 14.f ? 4.5f : 2.6f), half = ring + 3.f;
                k->s.setBounds (juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre ({ cap.x, cap.y }).toNearestInt());
            }
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (juce::Rectangle<float> (28.f, Face::envSlots[i].bottom - Face::envSlots[i].top + 8.f).withCentre ({ Face::envSlots[i].x, (Face::envSlots[i].top + Face::envSlots[i].bottom) * 0.5f }).toNearestInt());
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (juce::Rectangle<float> (28.f, Face::rslSlots[i].bottom - Face::rslSlots[i].top + 8.f).withCentre ({ Face::rslSlots[i].x, (Face::rslSlots[i].top + Face::rslSlots[i].bottom) * 0.5f }).toNearestInt());
    overlay.setBounds (getLocalBounds());
}

juce::String VoodooPanel::knobName (const Knob* k) const
{
    if (k == &rate) return "rate";
    if (k == &amount) return "amount";
    if (k == &pitch) return "pitch";
    if (k == &glide) return "glide";
    if (k == &filter) return "filter";
    if (k == &volume) return "volume";
    if (k == &reverb) return "reverb";
    if (k == &chorus) return "chorus";
    if (k == &dist) return "dist";
    if (k == &delay) return "delay";
    if (k == &dtime) return "dtime";
    if (k == &feedback) return "feedback";
    return {};
}

void VoodooPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    juce::PopupMenu m;
    m.addItem (1, "Animations", true, overlay.animationsOn());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [this] (int r) { if (r == 1) overlay.setAnimations (! overlay.animationsOn()); });
}
