#include "PluginEditor.h"
#include "BinaryData.h"
#include "FaceplateLayout.h"

using namespace K2k;

namespace
{
// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 36..84 = 4 octaves + 1 key
constexpr int kKeyLow = 36, kKeyHigh = 84;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }
const juce::Colour amber { 0xff00f0ff }, ember { 0xff4ab0ff }, dark { 0xff0a1218 };
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

K2kLNF::K2kLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffc8f6ff));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffc8f6ff));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff0f4a58));
}

void K2kLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    hfx::plateRotary (g, x, y, w, h, pos, a0, a1, s, ramp (pos));
}

void K2kLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    if ((bool) s.getProperties()["wheel"]) { hfx::plateWheel (g, x, w, pos); return; }
    hfx::plateLinear (g, x, y, w, h, pos, s, faderCap, ramp ((pos - (float) y) / (float) juce::jmax (1, h)));
}

void K2kLNF::drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&)
{
    // the box and its arrow are part of the faceplate: only the text (the combo's label) is drawn
}

void K2kLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void K2kLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff0f4a58) : over ? juce::Colour (0xff0a3038) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}


void K2kLNF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    const auto r = b.getLocalBounds().toFloat().reduced (2.f);
    const float d = juce::jmin (r.getWidth(), r.getHeight());
    const auto c = r.withSizeKeepingCentre (d, d);
    const bool on = b.getToggleState();
    g.setColour (on ? amber.withAlpha (0.35f) : juce::Colours::transparentBlack); g.fillEllipse (c.expanded (3.f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe0e0e0), c.getX() + d * 0.3f, c.getY() + d * 0.2f, juce::Colour (0xff3c3c3c), c.getRight(), c.getBottom(), false));
    g.fillEllipse (c);
    g.setColour (on ? amber : juce::Colour (0xff1a2a30)); g.fillEllipse (c.reduced (d * 0.3f));
    g.setColour (juce::Colours::black.withAlpha (0.7f)); g.drawEllipse (c, 1.2f);
}

//==============================================================================
K2kPanel::K2kPanel (K2000AudioProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    logo = load (BinaryData::logo_png, BinaryData::logo_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::lfoRate, "RATE");  addKnob (amount, P::lfoAmount, "AMOUNT");   // under the "LFO" header
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB"); addKnob (chorus, P::chorus, "CHORUS"); addKnob (dist, P::dist, "DISTORTION");
    addKnob (glide, P::glide, "GLIDE"); addKnob (cutoff, P::cutoff, "CUTOFF"); addKnob (drive, P::drive, "DRIVE");
    addKnob (dmix, P::delayMix, "DLY MIX"); addKnob (dtime, P::delayTime, "DLY TIME"); addKnob (dfb, P::delayFb, "FEEDBACK");
    addKnob (res, P::res, "RES"); addKnob (fenvK, P::fenv, "FLT ENV"); addKnob (rel, P::ampRel, "AMP REL"); addKnob (detune, P::detune, "DETUNE");
    for (auto* k : { &res, &fenvK, &rel, &detune }) k->s.getProperties().set ("small", true);
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::lfoAtk + i, en[i], juce::Slider::LinearVertical);
    const char* tn[5] = { "ATTACK", "DECAY", "SUSTAIN", "BANK", "ADSR" };   // sit under the "VELOCITY" header
    const int ai[5] = { P::ampAtk, P::ampDec, P::ampSus, P::velBank, P::velAdsr };
    for (int i = 0; i < 5; ++i) addKnob (rsl[i], ai[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };

    addCombo (cWave, P::wave, waveNames);  addCombo (cLfo, P::lfoSrc, lfoNames);  addCombo (cDest, P::lfoDst, destNames);
    addCombo (cFilt, P::ftype, filterNames); addCombo (cMode, P::vmode, modeNames);
    addAndMakeVisible (pitchSwitch);
    pitchAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, ids[P::lfoPitch], pitchSwitch);

    for (auto* b : { &prev, &next, &folder, &save }) { addAndMakeVisible (*b); b->setButtonText (""); b->getProperties().set ("flat", true); }
    addAndMakeVisible (expansion);
    expansion.getProperties().set ("flat", true);
    prev.onClick = [this] { stepPreset (-1); };
    next.onClick = [this] { stepPreset (1); };
    folder.onClick = [this] { showPresetMenu(); };
    save.onClick = [this] { savePreset(); };
    expansion.onClick = [this] { showExpansion(); };

    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (juce::FontOptions (19.f, juce::Font::plain));
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffc8f6ff));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setAvailableRange (kKeyLow, kKeyHigh);   // must come before sizing: with scroll buttons hidden the first visible key is the range start
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
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
    overlay.a = amber; overlay.b = ember; overlay.theme = hfx::Theme::scanner;
    overlay.mascot = { Face::mascotX, Face::mascotY }; overlay.mascotR = Face::mascotR;
    overlay.setTitleFromFace (juce::ImageFileFormat::loadFrom (BinaryData::faceplate_jpg, BinaryData::faceplate_jpgSize),
                              { Face::titleX, Face::titleY, Face::titleW, Face::titleH });   // light from the real letters
    overlay.addKnob (rate.s, hfx::PremiumOverlay::lfoRate);
    overlay.addKnob (amount.s, hfx::PremiumOverlay::lfoAmount);
    overlay.addKnob (volume.s, hfx::PremiumOverlay::volume);
    overlay.addKnob (reverb.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (chorus.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (dist.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (glide.s, hfx::PremiumOverlay::glide);
    overlay.addKnob (cutoff.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (drive.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (dmix.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (dtime.s, hfx::PremiumOverlay::delayTime);
    overlay.addKnob (dfb.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (res.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (fenvK.s, hfx::PremiumOverlay::filterEnv);
    overlay.addKnob (rel.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (detune.s, hfx::PremiumOverlay::plain);
    overlay.addFader (env[0].s, true, 0, hfx::PremiumOverlay::ledColumns (Face::envSlots[0].x, Face::envSlots[0].top + 3.f, Face::envSlots[0].bottom - 3.f));
    overlay.addFader (env[1].s, true, 1, hfx::PremiumOverlay::ledColumns (Face::envSlots[1].x, Face::envSlots[1].top + 3.f, Face::envSlots[1].bottom - 3.f));
    overlay.addFader (env[2].s, true, 2, hfx::PremiumOverlay::ledColumns (Face::envSlots[2].x, Face::envSlots[2].top + 3.f, Face::envSlots[2].bottom - 3.f));
    overlay.addFader (env[3].s, true, 3, hfx::PremiumOverlay::ledColumns (Face::envSlots[3].x, Face::envSlots[3].top + 3.f, Face::envSlots[3].bottom - 3.f));
    overlay.addFader (rsl[0].s, false, 0, hfx::PremiumOverlay::ledColumns (Face::rslSlots[0].x, Face::rslSlots[0].top + 3.f, Face::rslSlots[0].bottom - 3.f));
    overlay.addFader (rsl[1].s, false, 1, hfx::PremiumOverlay::ledColumns (Face::rslSlots[1].x, Face::rslSlots[1].top + 3.f, Face::rslSlots[1].bottom - 3.f));
    overlay.addFader (rsl[2].s, false, 2, hfx::PremiumOverlay::ledColumns (Face::rslSlots[2].x, Face::rslSlots[2].top + 3.f, Face::rslSlots[2].bottom - 3.f));
    overlay.addPlainFader (rsl[3].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[3].x, Face::rslSlots[3].top + 3.f, Face::rslSlots[3].bottom - 3.f));   // a timbre fader (BANK)
    overlay.addPlainFader (rsl[4].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[4].x, Face::rslSlots[4].top + 3.f, Face::rslSlots[4].bottom - 3.f));   // a timbre fader (ADSR)
    addAndMakeVisible (overlay);

    setSize (1000, 746);
}

K2kPanel::~K2kPanel() { setLookAndFeel (nullptr); }

void K2kPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void K2kPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void K2kPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void K2kPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File K2kPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("K2000").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void K2kPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.k2preset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .k2preset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.k2preset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void K2kPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.k2preset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("k2preset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void K2kPanel::loadUserPreset (const juce::File& f)
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

void K2kPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void K2kPanel::paint (juce::Graphics& g)
{
    face.draw (g, getLocalBounds());   // the whole front panel is one photo-real render (see Assets/faceplate.jpg)
}

void K2kPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 280, 697, 100);  place (amount, 280, 862, 100);
    pitchSwitch.setBounds (C (377, 757, 56, 56));
    place (volume, 1290, 697, 100); place (reverb, 1435, 697, 100); place (chorus, 1575, 697, 100); place (dist, 1718, 697, 100);
    place (glide, 908, 862, 100); place (cutoff, 1092, 862, 100); place (drive, 1289, 862, 100);
    place (dmix, 1434, 862, 100); place (dtime, 1575, 862, 100); place (dfb, 1718, 862, 100);
    place (res, 742, 385, 64); place (fenvK, 742, 490, 64); place (rel, 1257, 385, 64); place (detune, 1257, 490, 64);
    const int ex[4] = { 465, 563, 661, 759 }, tx[5] = { 1416, 1497, 1576, 1657, 1738 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 28, 668, 56, 232));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 338, 56, 232));
    bend.s.setBounds (C (330, 1250, 90, 260)); modw.s.setBounds (C (480, 1250, 90, 260));
    cWave.c.setBounds (R (225, 988, 270, 57)); cLfo.c.setBounds (R (545, 988, 270, 57)); cDest.c.setBounds (R (865, 988, 270, 57));
    cFilt.c.setBounds (R (1185, 988, 270, 57)); cMode.c.setBounds (R (1505, 988, 270, 57));
    folder.setBounds (R (612, 105, 100, 85)); save.setBounds (R (1305, 105, 80, 85));
    prev.setBounds (R (705, 105, 60, 85)); next.setBounds (R (1235, 105, 60, 85));
    presetLabel.setBounds (R (770, 112, 460, 70));
    expansion.setBounds (R (240, 100, 340, 410));
    keys.setBounds (R (610, 1090, 1165, 325));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly

    // on the faceplate: knobs centred on their caps (value ring just outside), faders on their slots
    for (const auto& cap : Face::caps)
        for (auto* k : { &rate, &amount, &volume, &reverb, &chorus, &dist, &glide, &cutoff, &drive, &dmix, &dtime, &dfb, &res, &fenvK, &rel, &detune })
            if (juce::String (cap.name) == knobName (k))
            {
                const float ring = cap.r + (cap.r > 14.f ? 4.5f : 2.6f), half = ring + 3.f;
                k->s.setBounds (juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre ({ cap.x, cap.y }).toNearestInt());
            }
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (juce::Rectangle<float> (28.f, Face::envSlots[i].bottom - Face::envSlots[i].top + 8.f).withCentre ({ Face::envSlots[i].x, (Face::envSlots[i].top + Face::envSlots[i].bottom) * 0.5f }).toNearestInt());
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (juce::Rectangle<float> (28.f, Face::rslSlots[i].bottom - Face::rslSlots[i].top + 8.f).withCentre ({ Face::rslSlots[i].x, (Face::rslSlots[i].top + Face::rslSlots[i].bottom) * 0.5f }).toNearestInt());
    overlay.setBounds (getLocalBounds());
}

juce::String K2kPanel::knobName (const Knob* k) const
{
    if (k == &rate) return "rate";
    if (k == &amount) return "amount";
    if (k == &volume) return "volume";
    if (k == &reverb) return "reverb";
    if (k == &chorus) return "chorus";
    if (k == &dist) return "dist";
    if (k == &glide) return "glide";
    if (k == &cutoff) return "cutoff";
    if (k == &drive) return "drive";
    if (k == &dmix) return "dmix";
    if (k == &dtime) return "dtime";
    if (k == &dfb) return "dfb";
    if (k == &res) return "res";
    if (k == &fenvK) return "fenvK";
    if (k == &rel) return "rel";
    if (k == &detune) return "detune";
    return {};
}

void K2kPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    juce::PopupMenu m;
    m.addItem (1, "Animations", true, overlay.animationsOn());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [this] (int r) { if (r == 1) overlay.setAnimations (! overlay.animationsOn()); });
}
