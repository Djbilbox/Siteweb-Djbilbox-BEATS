#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Hg;

namespace
{
// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 48..96 = 4 octaves + 1 key
constexpr int kKeyLow = 48, kKeyHigh = 96;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }
const juce::Colour amber { 0xffffe08a }, ember { 0xff2fe08a }, dark { 0xff090d0b };
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

HgLNF::HgLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffefe3b0));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffefe3b0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff1d4a30));
}

void HgLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void HgLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void HgLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffefe3b0)); g.fillPath (t);
}

void HgLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void HgLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff1d4a30) : over ? juce::Colour (0xff12301f) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
HgPanel::HgPanel (HgProcessor& p)
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
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DLY MIX");
    addKnob (dtime, P::dtime, "DLY TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "FLT ENV"); addKnob (resoK, P::reso, "RES");
    const char* tn[5] = { "ATK", "DEC", "SUS", "REL", "VELOCITY" };
    const int ti[5] = { P::fatk, P::fdec, P::fsus, P::frel, P::velocity };
    for (int i = 0; i < 5; ++i) addKnob (rsl[i], ti[i], tn[i], juce::Slider::LinearVertical);
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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffefe3b0));
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

HgPanel::~HgPanel() { setLookAndFeel (nullptr); }

void HgPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void HgPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void HgPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void HgPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File HgPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("HIGHROLLER_GOLD").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void HgPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.hgpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .hgpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.hgpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void HgPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.hgpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("hgpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void HgPanel::loadUserPreset (const juce::File& f)
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

void HgPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void HgPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff151816), 0, 0, juce::Colour (0xff060807), 0, (float) getHeight(), false));
    g.fillAll();
    g.setColour (juce::Colour (0xff0e1110)); g.fillRect (R (175, 35, 1655, 1440));
    for (int side = 0; side < 2; ++side)   // rails acier + poignee
    {
        auto r = side == 0 ? R (15, 20, 160, 1460) : R (1830, 20, 160, 1460);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff9a9c9e), (float) r.getX(), 0, juce::Colour (0xff2e3030), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 4.f);
        const auto hd = side == 0 ? R (58, 160, 85, 1180) : R (1858, 160, 85, 1180);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb8babc), (float) hd.getX(), 0, juce::Colour (0xff3a3c3e), (float) hd.getRight(), 0, false));
        g.fillRoundedRectangle (hd.toFloat(), 22.f);
        g.setColour (juce::Colour (0xff141616)); g.fillRoundedRectangle (hd.reduced (20, 14).toFloat(), 16.f);
        g.setColour ((side == 0 ? ember : amber).withAlpha (0.8f)); g.fillRect (side == 0 ? r.getRight() - 3 : r.getX(), r.getY() + 4, 3, r.getHeight() - 8);
        for (int y : { 85, 1410 }) { g.setColour (juce::Colour (0xff181a1a)); g.fillEllipse ((float) r.getCentreX() - 17, y * K - 17, 34.f, 34.f); g.setColour (juce::Colour (0xffa0a2a4)); g.fillEllipse ((float) r.getCentreX() - 12, y * K - 12, 24.f, 24.f); }
    }
    // haut-parleurs a grille doree
    for (auto r : { R (655, 325, 130, 360), R (1215, 325, 130, 360) })
    {
        g.setColour (juce::Colour (0xff3a3d3c)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        g.setColour (juce::Colour (0xff060707)); g.fillRoundedRectangle (r.reduced (4).toFloat(), 6.f);
        for (float yy = (float) r.getY() + 10; yy < (float) r.getBottom() - 8; yy += 9.f)
            for (float xx = (float) r.getX() + 9; xx < (float) r.getRight() - 8; xx += 9.f)
            { g.setColour (amber.withAlpha (0.22f)); g.fillEllipse (xx - 1.f, yy - 1.f, 7.f, 7.f); g.setColour (juce::Colour (0xff0c0e0d)); g.fillEllipse (xx, yy, 5.f, 5.f); }
    }
    g.drawImage (camel, R (820, 322, 364, 366).toFloat(), juce::RectanglePlacement::centred);
    {   // logo DB
        auto lr = R (1688, 132, 108, 90).toFloat();
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    {   // DJBILBOXBEATS
        juce::AttributedString as; as.setJustification (juce::Justification::centred);
        const auto fnt = juce::FontOptions (19.f, juce::Font::bold);
        as.append ("DJBILBOXBEATS", fnt, juce::Colour (0xffeee0a0));
        as.draw (g, R (1395, 140, 290, 70).toFloat());
    }
    // titre marquee or
    auto drawTitle = [&] (const juce::String& txt, juce::Rectangle<int> rc, float sz)
    {
        g.setFont (juce::FontOptions (sz, juce::Font::bold));
        for (float a : { 0.10f, 0.16f }) { g.setColour (ember.withAlpha (a)); g.drawFittedText (txt, rc.expanded (2, 1), juce::Justification::centred, 1); }
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff6e3a0), 0, (float) rc.getY(), juce::Colour (0xffb8902a), 0, (float) rc.getBottom(), false));
        g.drawFittedText (txt, rc, juce::Justification::centred, 1);
    };
    drawTitle ("HIGHROLLER", R (740, 722, 520, 100), 40.f);
    drawTitle ("GOLD", R (840, 812, 320, 95), 46.f);
    g.setColour (juce::Colour (0xffb8b8b0)); g.setFont (juce::FontOptions (19.f));
    g.drawFittedText ("By Humpire", R (880, 912, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (645, 115, 710, 120).toFloat();
    g.setColour (ember.withAlpha (0.3f)); g.fillRoundedRectangle (scr.expanded (4.f), 14.f);
    g.setGradientFill (juce::ColourGradient (amber, scr.getX(), 0, ember, scr.getRight(), 0, false)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (juce::Colour (0xff0a100d)); g.fillRoundedRectangle (scr.reduced (3.f), 10.f);
    g.setColour (juce::Colour (0xff0f1a14)); g.fillRoundedRectangle (R (800, 140, 440, 74).toFloat(), 4.f);
    {   // icones
        g.setColour (juce::Colour (0xffd8d0a0)); const auto f = R (686, 160, 44, 36).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1272, 156, 42, 44).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (amber);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (760, 0, 0, 0).getX() : (float) R (1240, 0, 0, 0).getX(), cy = (float) R (0, 177, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 10.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 10.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette (image avec plaque EXPANSION integree)
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, R (203, 122, 414, 340).toFloat(), juce::RectanglePlacement::centred);
    // plaque des molettes
    g.setColour (juce::Colour (0xff070909)); g.fillRoundedRectangle (R (215, 1055, 300, 365).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff2a2e2c)); g.drawRoundedRectangle (R (215, 1055, 300, 365).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffd2cfb4));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 11.f : 12.5f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    // section headers (rules + text), one per group of controls
    drawHeader (g, "LFO", amber, 15.f, 205, 470, 540);
    drawHeader (g, "TUNING", amber, 12.f, 486, 610, 540);   // the transpose knob is not an LFO control
    drawHeader (g, "AMP ENVELOPE", amber, 12.f, 215, 575, 745);
    drawHeader (g, "FILTER ENVELOPE", amber, 12.f, 1395, 1700, 300);
    // captions under the five drop-down menus
    g.setColour (juce::Colour (0xffd2cfb4).withAlpha (0.85f)); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    {
        const char* cap[5] = { "ENGINE", "LFO SHAPE", "LFO DESTINATION", "FILTER TYPE", "VOICE MODE" };
        const int cx[5] = { 758, 1000, 1243, 1479, 1696 };
        for (int i = 0; i < 5; ++i) g.drawFittedText (cap[i], R (cx[i] - 110, 1048, 220, 36), juce::Justification::centred, 1);
    }
    g.setColour (juce::Colour (0xffd2cfb4)); g.setFont (juce::FontOptions (11.5f));
    g.drawFittedText ("Pitch Bend", R (230, 1345, 140, 34), juce::Justification::centred, 1);
    g.drawFittedText ("Modulation", R (362, 1345, 140, 34), juce::Justification::centred, 1);
}

void HgPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 266, 627, 100);  place (amount, 407, 627, 100); place (pitch, 548, 627, 100);
    place (glide, 699, 895, 100); place (filter, 1305, 885, 100);
    place (tone, 840, 263, 52);  place (body, 920, 263, 52); place (bright, 1000, 263, 52); place (width, 1080, 263, 52); place (texture, 1160, 263, 52);
    place (resoK, 625, 740, 56); place (fenvK, 690, 740, 56); place (volume, 1318, 735, 76);
    place (reverb, 1455, 707, 100); place (chorus, 1595, 707, 100); place (dist, 1737, 707, 100);
    place (delay, 1455, 885, 100);  place (dtime, 1595, 885, 100); place (feedback, 1737, 885, 100);
    const int ex[4] = { 255, 355, 455, 555 }, tx[5] = { 1431, 1510, 1589, 1667, 1746 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 30, 775, 60, 182));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 332, 56, 225));
    bend.s.setBounds (C (297, 1230, 90, 240)); modw.s.setBounds (C (432, 1230, 90, 240));
    cWave.c.setBounds (R (648, 992, 220, 50)); cLfo.c.setBounds (R (890, 992, 220, 50)); cDest.c.setBounds (R (1133, 992, 220, 50));
    cFilt.c.setBounds (R (1376, 992, 205, 50)); cMono.c.setBounds (R (1604, 992, 184, 50));
    folder.setBounds (R (665, 135, 90, 85)); save.setBounds (R (1260, 135, 80, 85));
    prev.setBounds (R (740, 135, 55, 85)); next.setBounds (R (1215, 135, 55, 85));
    presetLabel.setBounds (R (800, 140, 440, 74));
    expansion.setBounds (R (300, 365, 220, 55));
    keys.setBounds (R (545, 1100, 1240, 318));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly
}
