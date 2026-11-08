#include "PluginEditor.h"
#include "BinaryData.h"
#include "FaceplateLayout.h"

using namespace Wz;

namespace
{
// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 36..84 = 4 octaves + 1 key
constexpr int kKeyLow = 36, kKeyHigh = 84;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }
const juce::Colour green { 0xff8fe04f }, red { 0xffff5a4a }, dark { 0xff101010 }, bgDark { 0xff191919 };
const juce::Colour amber { 0xffff9a3a }, ember { 0xffff4a3a };   // the premium skin's two lights: tracer orange and red
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return green.interpolatedWith (red, juce::jlimit (0.f, 1.f, t)); }

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

WarzoneLNF::WarzoneLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffb6e58a));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffb6e58a));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a4a22));
}

void WarzoneLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    hfx::plateRotary (g, x, y, w, h, pos, a0, a1, s, ramp (pos));
}

void WarzoneLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    if ((bool) s.getProperties()["wheel"]) { hfx::plateWheel (g, x, w, pos); return; }
    hfx::plateLinear (g, x, y, w, h, pos, s, faderCap, ramp ((pos - (float) y) / (float) juce::jmax (1, h)));
}

void WarzoneLNF::drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&)
{
    // the box and its arrow are part of the faceplate: only the text (the combo's label) is drawn
}

void WarzoneLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void WarzoneLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff35451f) : over ? juce::Colour (0xff25301a) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (green.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
WarzonePanel::WarzonePanel (WarzoneProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    dbLogo = load (BinaryData::db_png, BinaryData::db_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::rate, "RATE");        addKnob (amount, P::amount, "AMOUNT");   addKnob (pitch, P::pitch, "TRANSPOSE");
    addKnob (glide, P::glide, "GLIDE");     addKnob (filter, P::filter, "CUTOFF");
    addKnob (reso, P::reso, "RESO");        addKnob (envdepth, P::envdepth, "ENV AMT");
    addKnob (unison, P::unison, "UNISON");  addKnob (detune, P::detune, "DETUNE");   addKnob (dmix, P::dmix, "DLY MIX");
    addKnob (volume, P::volume, "VOLUME");  addKnob (reverb, P::reverb, "REVERB");   addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION");  addKnob (dtime, P::dtime, "DLY TIME");     addKnob (feedback, P::feedback, "DLY FDBK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    const int ei[4] = { P::attack, P::decay, P::sustain, P::release };
    for (int i = 0; i < 4; ++i) addKnob (env[i], ei[i], en[i], juce::Slider::LinearVertical);
    const char* tn[4] = { "ATK", "DEC", "SUS", "VELOCITY" };
    const int fi[4] = { P::fattack, P::fdecay, P::fsustain, P::velocity };
    for (int i = 0; i < 4; ++i) addKnob (rsl[i], fi[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);   // wheels are captioned in paint()
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &unison, &detune, &dmix, &reso, &envdepth }) k->s.getProperties().set ("small", true);
    unison.s.setNumDecimalPlacesToDisplay (0);
    // maximum label width (px) per control: keeps neighbouring captions from touching
    for (auto& k : env) k.s.getProperties().set ("lw", 46);
    for (int i = 0; i < 3; ++i) rsl[i].s.getProperties().set ("lw", 34);
    rsl[3].s.getProperties().set ("lw", 60);
    pitch.s.getProperties().set ("lw", 70);

    addCombo (cWave, P::waveform, waveNames); addCombo (cLfo, P::lfosrc, lfoNames); addCombo (cDest, P::dest, destNames);
    addCombo (cFilt, P::ftype, filterNames);  addCombo (cMono, P::mono, monoNames);

    for (auto* b : { &prev, &next, &folder, &save }) addAndMakeVisible (*b);
    addAndMakeVisible (expansion);
    expansion.getProperties().set ("flat", true);
    prev.onClick = [this] { stepPreset (-1); };
    next.onClick = [this] { stepPreset (1); };
    folder.onClick = [this] { showPresetMenu(); };
    save.onClick = [this] { savePreset(); };
    expansion.onClick = [this] { showExpansionMenu(); };

    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (juce::FontOptions (17.f, juce::Font::plain));
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffb6e58a));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setAvailableRange (kKeyLow, kKeyHigh);   // must come before sizing: with scroll buttons hidden the first visible key is the range start
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, green.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, red.withAlpha (0.3f));
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
    overlay.a = amber; overlay.b = ember; overlay.theme = hfx::Theme::bassShake;
    overlay.mascot = { Face::mascotX, Face::mascotY }; overlay.mascotR = Face::mascotR;
    overlay.addKnob (rate.s, hfx::PremiumOverlay::lfoRate);
    overlay.addKnob (amount.s, hfx::PremiumOverlay::lfoAmount);
    overlay.addKnob (pitch.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (glide.s, hfx::PremiumOverlay::glide);
    overlay.addKnob (filter.s, hfx::PremiumOverlay::cutoff);
    overlay.addKnob (reso.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (envdepth.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (unison.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (detune.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (dmix.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (volume.s, hfx::PremiumOverlay::volume);
    overlay.addKnob (reverb.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (chorus.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (dist.s, hfx::PremiumOverlay::effect);
    overlay.addKnob (dtime.s, hfx::PremiumOverlay::delayTime);
    overlay.addKnob (feedback.s, hfx::PremiumOverlay::delayFeedback);
    overlay.addFader (env[0].s, true, 0, hfx::PremiumOverlay::ledColumns (Face::envSlots[0].x, Face::envSlots[0].top + 3.f, Face::envSlots[0].bottom - 3.f));
    overlay.addFader (env[1].s, true, 1, hfx::PremiumOverlay::ledColumns (Face::envSlots[1].x, Face::envSlots[1].top + 3.f, Face::envSlots[1].bottom - 3.f));
    overlay.addFader (env[2].s, true, 2, hfx::PremiumOverlay::ledColumns (Face::envSlots[2].x, Face::envSlots[2].top + 3.f, Face::envSlots[2].bottom - 3.f));
    overlay.addFader (env[3].s, true, 3, hfx::PremiumOverlay::ledColumns (Face::envSlots[3].x, Face::envSlots[3].top + 3.f, Face::envSlots[3].bottom - 3.f));
    overlay.addFader (rsl[0].s, false, 0, hfx::PremiumOverlay::ledColumns (Face::rslSlots[0].x, Face::rslSlots[0].top + 3.f, Face::rslSlots[0].bottom - 3.f));
    overlay.addFader (rsl[1].s, false, 1, hfx::PremiumOverlay::ledColumns (Face::rslSlots[1].x, Face::rslSlots[1].top + 3.f, Face::rslSlots[1].bottom - 3.f));
    overlay.addFader (rsl[2].s, false, 2, hfx::PremiumOverlay::ledColumns (Face::rslSlots[2].x, Face::rslSlots[2].top + 3.f, Face::rslSlots[2].bottom - 3.f));
    overlay.addVelocityFader (rsl[3].s, hfx::PremiumOverlay::ledColumns (Face::rslSlots[3].x, Face::rslSlots[3].top + 3.f, Face::rslSlots[3].bottom - 3.f));
    addAndMakeVisible (overlay);

    setSize (1000, 746);
}

WarzonePanel::~WarzonePanel() { setLookAndFeel (nullptr); }

void WarzonePanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void WarzonePanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void WarzonePanel::refreshPresetLabel() { presetLabel.setText (proc.presetName, juce::dontSendNotification); }

void WarzonePanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File userPresetDir() { return WarzoneProcessor::userDir ("Presets"); }

void WarzonePanel::showPresetMenu()
{
    juce::PopupMenu menu, sub; juce::String cur;
    const auto& list = proc.getEntries();
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (cur != list[i].bank) { if (cur.isNotEmpty()) menu.addSubMenu (cur, sub); sub = juce::PopupMenu(); cur = list[i].bank; }
        sub.addItem ((int) i + 1, list[i].name, true, (int) i == proc.presetIndex);
    }
    menu.addSubMenu (cur, sub);
    menu.addSeparator();
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.wz808preset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (10000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 10000 && r - 10000 < files.size()) loadUserPreset (files[r - 10000]);
        else if (r > 0 && r < 10000) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void WarzonePanel::showExpansionMenu()
{
    juce::PopupMenu m;
    m.addSectionHeader (proc.expansionName);
    m.addItem (1, "Load a .wz808exp pack...");
    m.addItem (2, "Open the packs folder");
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&expansion), [this] (int r)
    {
        if (r == 1)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load expansion pack", WarzoneProcessor::userDir ("Expansions"), "*.wz808exp");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc)
            {
                if (fc.getResult().existsAsFile())
                    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Expansion",
                        proc.loadExpansionFile (fc.getResult()) ? "Pack loaded: its presets are in the Presets menu." : "Invalid file (warzone808-expansion format).");
            });
        }
        else if (r == 2) WarzoneProcessor::userDir ("Expansions").revealToUser();
    });
}

void WarzonePanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.wz808preset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("wz808preset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void WarzonePanel::loadUserPreset (const juce::File& f)
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

void WarzonePanel::paint (juce::Graphics& g)
{
    face.draw (g, getLocalBounds());   // the whole front panel is one photo-real render (see Assets/faceplate.jpg)
}

void WarzonePanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 222, 716, 108); place (amount, 362, 716, 108); place (pitch, 503, 716, 108);
    place (glide, 902, 915, 108); place (filter, 1096, 915, 108);
    place (reso, 640, 915, 82);   place (envdepth, 745, 915, 82);
    place (unison, 1200, 915, 82); place (detune, 1285, 915, 82); place (dmix, 1370, 915, 82);
    place (volume, 1501, 716, 108); place (reverb, 1641, 716, 108); place (chorus, 1781, 716, 108);
    place (dist, 1501, 915, 108); place (dtime, 1641, 915, 108); place (feedback, 1781, 915, 108);
    const int ex[4] = { 220, 316, 413, 511 }, tx[4] = { 1529, 1602, 1677, 1790 };
    for (int i = 0; i < 4; ++i) { env[i].s.setBounds (R (ex[i] - 30, 825, 60, 205)); rsl[i].s.setBounds (R (tx[i] - 30, 370, 60, 210)); }
    bend.s.setBounds (C (246, 1278, 90, 250)); modw.s.setBounds (C (380, 1278, 90, 250));
    cWave.c.setBounds (R (600, 1028, 206, 56)); cLfo.c.setBounds (R (830, 1028, 218, 56)); cDest.c.setBounds (R (1072, 1028, 244, 56));
    cFilt.c.setBounds (R (1338, 1028, 244, 56)); cMono.c.setBounds (R (1606, 1028, 222, 56));
    folder.setBounds (R (595, 85, 150, 76)); save.setBounds (R (1255, 85, 150, 76));
    prev.setBounds (R (760, 85, 70, 76)); next.setBounds (R (1170, 85, 70, 76));
    presetLabel.setBounds (R (840, 85, 320, 76));
    expansion.setBounds (R (165, 110, 315, 485));
    keys.setBounds (R (515, 1120, 1317, 330));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly

    // on the faceplate: knobs centred on their caps (value ring just outside), faders on their slots
    for (const auto& cap : Face::caps)
        for (auto* k : { &rate, &amount, &pitch, &glide, &filter, &reso, &envdepth, &unison, &detune, &dmix, &volume, &reverb, &chorus, &dist, &dtime, &feedback })
            if (juce::String (cap.name) == knobName (k))
            {
                const float ring = cap.r + (cap.r > 14.f ? 4.5f : 2.6f), half = ring + 3.f;
                k->s.setBounds (juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre ({ cap.x, cap.y }).toNearestInt());
            }
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (juce::Rectangle<float> (28.f, Face::envSlots[i].bottom - Face::envSlots[i].top + 8.f).withCentre ({ Face::envSlots[i].x, (Face::envSlots[i].top + Face::envSlots[i].bottom) * 0.5f }).toNearestInt());
    for (int i = 0; i < 4; ++i) rsl[i].s.setBounds (juce::Rectangle<float> (28.f, Face::rslSlots[i].bottom - Face::rslSlots[i].top + 8.f).withCentre ({ Face::rslSlots[i].x, (Face::rslSlots[i].top + Face::rslSlots[i].bottom) * 0.5f }).toNearestInt());
    overlay.setBounds (getLocalBounds());
}

juce::String WarzonePanel::knobName (const Knob* k) const
{
    if (k == &rate) return "rate";
    if (k == &amount) return "amount";
    if (k == &pitch) return "pitch";
    if (k == &glide) return "glide";
    if (k == &filter) return "filter";
    if (k == &reso) return "reso";
    if (k == &envdepth) return "envdepth";
    if (k == &unison) return "unison";
    if (k == &detune) return "detune";
    if (k == &dmix) return "dmix";
    if (k == &volume) return "volume";
    if (k == &reverb) return "reverb";
    if (k == &chorus) return "chorus";
    if (k == &dist) return "dist";
    if (k == &dtime) return "dtime";
    if (k == &feedback) return "feedback";
    return {};
}

void WarzonePanel::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    juce::PopupMenu m;
    m.addItem (1, "Animations", true, overlay.animationsOn());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [this] (int r) { if (r == 1) overlay.setAnimations (! overlay.animationsOn()); });
}
