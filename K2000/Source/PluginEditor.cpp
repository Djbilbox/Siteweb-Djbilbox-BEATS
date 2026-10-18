#include "PluginEditor.h"
#include "BinaryData.h"

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

void K2kLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void K2kLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffc8f6ff)); g.fillPath (t);
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
    const char* ln[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (lfoEnv[i], P::lfoAtk + i, ln[i], juce::Slider::LinearVertical);
    const char* an[5] = { "ATTACK", "DECAY", "SUSTAIN", "BANK", "ADSR" };   // sit under the "VELOCITY" header
    const int ai[5] = { P::ampAtk, P::ampDec, P::ampSus, P::velBank, P::velAdsr };
    for (int i = 0; i < 5; ++i) addKnob (ampEnv[i], ai[i], an[i], juce::Slider::LinearVertical);
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
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c3036), 0, 0, juce::Colour (0xff14171b), 0, (float) getHeight(), false));
    g.fillAll();
    juce::Random cl (9);   // grain brosse / rayures
    for (int i = 0; i < 140; ++i)
    {
        const float x = cl.nextFloat() * (float) getWidth(), y = cl.nextFloat() * (float) getHeight(), r = 12.f + cl.nextFloat() * 40.f;
        g.setColour (juce::Colours::black.withAlpha (0.03f)); g.fillEllipse (x - r, y - r, r * 2, r * 2);
    }
    for (int side = 0; side < 2; ++side)   // rails chromes + poignee
    {
        auto r = side == 0 ? R (35, 30, 140, 1435) : R (1825, 30, 140, 1435);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb0b4b8), (float) r.getX(), 0, juce::Colour (0xff3c4044), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 6.f);
        const auto hd = side == 0 ? R (88, 170, 58, 1160) : R (1855, 170, 58, 1160);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd0d4d8), (float) hd.getX(), 0, juce::Colour (0xff4a4e52), (float) hd.getRight(), 0, false));
        g.fillRoundedRectangle (hd.toFloat(), 16.f);
        g.setColour (amber.withAlpha (0.7f)); g.fillRect (side == 0 ? r.getRight() - 3 : r.getX(), r.getY() + 4, 3, r.getHeight() - 8);
        for (int y : { 98, 1390 }) { g.setColour (juce::Colour (0xff181a1c)); g.fillEllipse ((float) r.getCentreX() - 17, y * K - 17, 34.f, 34.f); g.setColour (juce::Colour (0xff9a9ea2)); g.fillEllipse ((float) r.getCentreX() - 12, y * K - 12, 24.f, 24.f); }
    }
    // grilles de ventilation (portent les petits boutons)
    for (auto r : { R (680, 265, 125, 325), R (1195, 265, 125, 325) })
    {
        g.setColour (amber.withAlpha (0.35f)); g.fillRoundedRectangle (r.toFloat().expanded (2.f), 10.f);
        g.setColour (juce::Colour (0xff0a1418)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        for (float yy = (float) r.getY() + 8; yy < (float) r.getBottom() - 6; yy += 8.f)
            for (float xx = (float) r.getX() + 8; xx < (float) r.getRight() - 6; xx += 8.f)
            { g.setColour (juce::Colour (0xff1c4a54)); g.fillEllipse (xx - 1.f, yy - 1.f, 6.f, 6.f); g.setColour (juce::Colour (0xff04090b)); g.fillEllipse (xx, yy, 4.5f, 4.5f); }
    }
    g.drawImage (camel, R (843, 272, 314, 316).toFloat(), juce::RectanglePlacement::centred);
    {   // logo
        auto lr = R (1688, 98, 106, 102).toFloat();
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    g.setColour (juce::Colour (0xffb8c4cc)); g.setFont (juce::FontOptions (17.f, juce::Font::bold));
    g.drawFittedText ("DJBILBOXBEATS", R (1425, 125, 262, 60), juce::Justification::centred, 1);
    // titre K2000 : afficheur a LED cyan
    const auto titleR = R (780, 600, 440, 130);
    for (float a : { 0.10f, 0.16f, 0.26f })
    { g.setColour (amber.withAlpha (a)); g.setFont (juce::FontOptions (64.f + (0.3f - a) * 16.f, juce::Font::bold)); g.drawFittedText ("K2000", titleR, juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xff7af8ff)); g.setFont (juce::FontOptions (62.f, juce::Font::bold));
    g.drawFittedText ("K2000", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffc4ccd4)); g.setFont (juce::FontOptions (20.f));
    g.drawFittedText ("By Humpire", R (880, 722, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (590, 85, 820, 125).toFloat();
    g.setColour (amber.withAlpha (0.3f)); g.fillRoundedRectangle (scr.expanded (3.f), 14.f);
    g.setColour (juce::Colour (0xff484c52)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (juce::Colour (0xff0b1318)); g.fillRoundedRectangle (scr.reduced (6.f), 9.f);
    g.setColour (juce::Colour (0xff10202a)); g.fillRoundedRectangle (R (705, 110, 590, 75).toFloat(), 5.f);
    {   // icones
        g.setColour (juce::Colour (0xffa8b4bc)); const auto f = R (628, 125, 56, 46).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1318, 122, 50, 50).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (amber);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (733, 0, 0, 0).getX() : (float) R (1265, 0, 0, 0).getX(), cy = (float) R (0, 147, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 11.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 11.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, R (225, 92, 355, 410).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffc8d4dc)); g.setFont (juce::FontOptions (14.f));
    g.drawFittedText ("EXPANSION", R (270, 522, 220, 40), juce::Justification::centred, 1);
    // plaque des molettes
    g.setColour (juce::Colour (0xff080c0e)); g.fillRoundedRectangle (R (222, 1078, 365, 350).toFloat(), 12.f);
    g.setColour (juce::Colour (0xff2c3438)); g.drawRoundedRectangle (R (222, 1078, 365, 350).toFloat(), 12.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffc6d0d8));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 10.f : 12.5f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    // section headers (rules + text), one per group of controls
    drawHeader (g, "LFO", amber, 16.f, 215, 410, 605);
    drawHeader (g, "LFO ENVELOPE", amber, 12.f, 430, 790, 605);
    drawHeader (g, "AMP ENVELOPE", amber, 12.f, 1385, 1610, 295);
    drawHeader (g, "VELOCITY", amber, 12.f, 1630, 1770, 295);
    // captions under the five drop-down menus
    g.setColour (juce::Colour (0xffc6d0d8).withAlpha (0.85f)); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    {
        const char* cap[5] = { "WAVEFORM", "LFO SHAPE", "LFO DESTINATION", "FILTER TYPE", "VOICE MODE" };
        const int cx[5] = { 360, 680, 1000, 1320, 1640 };
        for (int i = 0; i < 5; ++i) g.drawFittedText (cap[i], R (cx[i] - 140, 1044, 280, 30), juce::Justification::centred, 1);
    }
    g.setColour (juce::Colour (0xffc6d0d8)); g.setFont (juce::FontOptions (10.f));
    g.drawFittedText ("LFO>PITCH", R (305, 782, 140, 40), juce::Justification::centred, 1);   // the toggle sends the LFO to pitch (it is not the pitch-bend wheel)
    g.setFont (juce::FontOptions (10.5f));
    g.drawFittedText ("PITCH BEND", R (245, 1368, 170, 40), juce::Justification::centred, 1);
    g.drawFittedText ("MODULATION", R (400, 1368, 170, 40), juce::Justification::centred, 1);
    g.setColour (amber.withAlpha (0.5f));   // liaison RATE - PITCH - AMOUNT
    g.drawLine ((float) R (345, 0, 0, 0).getX(), (float) R (0, 700, 0, 0).getY(), (float) R (378, 0, 0, 0).getX(), (float) R (0, 700, 0, 0).getY(), 1.f);
    g.drawLine ((float) R (378, 0, 0, 0).getX(), (float) R (0, 700, 0, 0).getY(), (float) R (378, 0, 0, 0).getX(), (float) R (0, 735, 0, 0).getY(), 1.f);
    g.drawLine ((float) R (345, 0, 0, 0).getX(), (float) R (0, 862, 0, 0).getY(), (float) R (378, 0, 0, 0).getX(), (float) R (0, 862, 0, 0).getY(), 1.f);
    g.drawLine ((float) R (378, 0, 0, 0).getX(), (float) R (0, 862, 0, 0).getY(), (float) R (378, 0, 0, 0).getX(), (float) R (0, 830, 0, 0).getY(), 1.f);   // stops under the "LFO>PITCH" caption
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
    const int lx[4] = { 465, 563, 661, 759 }, ax[5] = { 1416, 1497, 1576, 1657, 1738 };
    for (int i = 0; i < 4; ++i) lfoEnv[i].s.setBounds (R (lx[i] - 28, 668, 56, 232));
    for (int i = 0; i < 5; ++i) ampEnv[i].s.setBounds (R (ax[i] - 28, 338, 56, 232));
    bend.s.setBounds (C (330, 1250, 90, 260)); modw.s.setBounds (C (480, 1250, 90, 260));
    cWave.c.setBounds (R (225, 988, 270, 57)); cLfo.c.setBounds (R (545, 988, 270, 57)); cDest.c.setBounds (R (865, 988, 270, 57));
    cFilt.c.setBounds (R (1185, 988, 270, 57)); cMode.c.setBounds (R (1505, 988, 270, 57));
    folder.setBounds (R (612, 105, 100, 85)); save.setBounds (R (1305, 105, 80, 85));
    prev.setBounds (R (705, 105, 60, 85)); next.setBounds (R (1235, 105, 60, 85));
    presetLabel.setBounds (R (770, 112, 460, 70));
    expansion.setBounds (R (240, 100, 340, 410));
    keys.setBounds (R (610, 1090, 1165, 325));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly
}
