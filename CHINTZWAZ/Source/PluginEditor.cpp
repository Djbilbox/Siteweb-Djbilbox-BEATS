#include "PluginEditor.h"
#include "BinaryData.h"
#include "FaceplateLayout.h"

using namespace Cz;

namespace
{
// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 48..96 = 4 octaves + 1 key
constexpr int kKeyLow = 48, kKeyHigh = 96;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }
const juce::Colour amber { 0xff40f0e0 }, ember { 0xffff80d0 }, dark { 0xff0a1a1c };
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
    g.setColour (col.withAlpha (0.6f));
    if (cx - tw * 0.5f - gap > px0) g.drawLine (px0, cy, cx - tw * 0.5f - gap, cy, 1.f);
    if (cx + tw * 0.5f + gap < px1) g.drawLine (cx + tw * 0.5f + gap, cy, px1, cy, 1.f);
}
}

CzLNF::CzLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffbffcf4));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffbffcf4));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff0f4a4a));
}

void CzLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    hfx::plateRotary (g, x, y, w, h, pos, a0, a1, s, ramp (pos));
}

void CzLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    if ((bool) s.getProperties()["wheel"]) { hfx::plateWheel (g, x, w, pos); return; }
    hfx::plateLinear (g, x, y, w, h, pos, s, faderCap, ramp ((pos - (float) y) / (float) juce::jmax (1, h)));
}

void CzLNF::drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&)
{
    // the box and its arrow are part of the faceplate: only the text (the combo's label) is drawn
}

void CzLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void CzLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff0f4a4a) : over ? juce::Colour (0xff0a3234) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
CzPanel::CzPanel (CzProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    logo = load (BinaryData::logo_png, BinaryData::logo_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::rate, "RATE");       addKnob (amount, P::amount, "AMOUNT");  addKnob (pitch, P::pitch, "TRANSPOSE");
    addKnob (glide, P::glide, "GLIDE");    addKnob (filter, P::filter, "CUTOFF");
    addKnob (tone, P::t1, "TONE");       addKnob (body, P::t2, "BODY");       addKnob (bright, P::t3, "BRIGHT");
    addKnob (width, P::t4, "WIDTH");       addKnob (texture, P::t5, "TEXTURE");
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DELAY");
    addKnob (dtime, P::dtime, "DLY TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "ENV AMT"); addKnob (resoK, P::reso, "RES");
    const char* tn[4] = { "ATTACK", "DECAY", "SUSTAIN", "VELOCITY" };
    const int ti[4] = { P::fatk, P::fdec, P::fsus, P::velocity };
    for (int i = 0; i < 4; ++i) addKnob (rsl[i], ti[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &tone, &body, &bright, &width, &texture, &fenvK, &resoK }) k->s.getProperties().set ("small", true);

    addCombo (cWave, P::wave, engineNames);  addCombo (cLfo, P::lfosrc, lfoNames);  addCombo (cDest, P::dest, destNames);
    addCombo (cFilt, P::ftype, filterNames); addCombo (cMono, P::mono, monoNames);

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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffbffcf4));
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
    overlay.a = amber; overlay.b = ember; overlay.theme = hfx::Theme::orientalGold;
    overlay.mascot = { Face::mascotX, Face::mascotY }; overlay.mascotR = Face::mascotR;
    overlay.setTitleFromFace (juce::ImageFileFormat::loadFrom (BinaryData::faceplate_jpg, BinaryData::faceplate_jpgSize),
                              { Face::titleX, Face::titleY, Face::titleW, Face::titleH });   // light from the real letters
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
    overlay.addKnob (tone.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (body.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (bright.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (width.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (texture.s, hfx::PremiumOverlay::plain);
    overlay.addKnob (resoK.s, hfx::PremiumOverlay::resonance);
    overlay.addKnob (fenvK.s, hfx::PremiumOverlay::filterEnv);
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

CzPanel::~CzPanel() { setLookAndFeel (nullptr); }

void CzPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void CzPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void CzPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void CzPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File CzPanel::userPresetDir()
{
    const auto root = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS");
    auto d = root.getChildFile ("CHINTAWAZ").getChildFile ("Presets");
    // the product was first spelt CHINTZWAZ: the user presets saved there move to the new folder once
    if (! d.exists() && root.getChildFile ("CHINTZWAZ").isDirectory()) root.getChildFile ("CHINTZWAZ").moveFileTo (root.getChildFile ("CHINTAWAZ"));
    d.createDirectory();
    return d;
}

void CzPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.czpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .czpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.czpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void CzPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.czpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("czpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void CzPanel::loadUserPreset (const juce::File& f)
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

void CzPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void CzPanel::paint (juce::Graphics& g)
{
    face.draw (g, getLocalBounds());   // the whole front panel is one photo-real render (see Assets/faceplate.jpg)
}

void CzPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 255, 655, 96);  place (amount, 392, 655, 96); place (pitch, 530, 655, 96);
    place (glide, 680, 865, 96); place (filter, 1325, 865, 96);
    place (volume, 1775, 480, 96); place (reverb, 1472, 735, 96); place (chorus, 1610, 735, 96); place (dist, 1748, 735, 96);
    place (delay, 1472, 890, 96); place (dtime, 1610, 890, 96); place (feedback, 1748, 890, 96);
    place (tone, 698, 435, 56); place (body, 698, 525, 56); place (bright, 698, 615, 56); place (width, 698, 705, 56);
    place (texture, 1301, 450, 56); place (resoK, 1301, 565, 56); place (fenvK, 1301, 680, 56);
    const int ex[4] = { 245, 343, 442, 540 }, tx[4] = { 1415, 1488, 1561, 1660 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 28, 801, 56, 189));
    for (int i = 0; i < 4; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 405, 56, 190));
    bend.s.setBounds (C (297, 1250, 90, 250)); modw.s.setBounds (C (445, 1250, 90, 250));
    cWave.c.setBounds (R (630, 995, 222, 57)); cLfo.c.setBounds (R (873, 995, 222, 57)); cDest.c.setBounds (R (1117, 995, 222, 57));
    cFilt.c.setBounds (R (1360, 995, 222, 57)); cMono.c.setBounds (R (1603, 995, 195, 57));
    folder.setBounds (R (645, 205, 100, 80)); save.setBounds (R (1275, 205, 90, 80));
    prev.setBounds (R (728, 205, 55, 80)); next.setBounds (R (1215, 205, 60, 80));
    presetLabel.setBounds (R (790, 205, 420, 66));
    expansion.setBounds (R (228, 140, 292, 392));
    keys.setBounds (R (608, 1105, 1192, 305));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly

    // on the faceplate: knobs centred on their caps (value ring just outside), faders on their slots
    for (const auto& cap : Face::caps)
        for (auto* k : { &rate, &amount, &pitch, &glide, &filter, &volume, &reverb, &chorus, &dist, &delay, &dtime, &feedback, &tone, &body, &bright, &width, &texture, &resoK, &fenvK })
            if (juce::String (cap.name) == knobName (k))
            {
                const float ring = cap.r + (cap.r > 14.f ? 4.5f : 2.6f), half = ring + 3.f;
                k->s.setBounds (juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre ({ cap.x, cap.y }).toNearestInt());
            }
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (juce::Rectangle<float> (28.f, Face::envSlots[i].bottom - Face::envSlots[i].top + 8.f).withCentre ({ Face::envSlots[i].x, (Face::envSlots[i].top + Face::envSlots[i].bottom) * 0.5f }).toNearestInt());
    for (int i = 0; i < 4; ++i) rsl[i].s.setBounds (juce::Rectangle<float> (28.f, Face::rslSlots[i].bottom - Face::rslSlots[i].top + 8.f).withCentre ({ Face::rslSlots[i].x, (Face::rslSlots[i].top + Face::rslSlots[i].bottom) * 0.5f }).toNearestInt());
    overlay.setBounds (getLocalBounds());
}

juce::String CzPanel::knobName (const Knob* k) const
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
    if (k == &tone) return "tone";
    if (k == &body) return "body";
    if (k == &bright) return "bright";
    if (k == &width) return "width";
    if (k == &texture) return "texture";
    if (k == &resoK) return "resoK";
    if (k == &fenvK) return "fenvK";
    return {};
}

void CzPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    juce::PopupMenu m;
    m.addItem (1, "Animations", true, overlay.animationsOn());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [this] (int r) { if (r == 1) overlay.setAnimations (! overlay.animationsOn()); });
}
