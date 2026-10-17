#include "PluginEditor.h"
#include "BinaryData.h"

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

void CzLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void CzLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffbffcf4)); g.fillPath (t);
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
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("CHINTZWAZ").getChildFile ("Presets");
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
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffa5727e), 0, 0, juce::Colour (0xff80525e), 0, (float) getHeight(), false));
    g.fillAll();
    juce::Random fl (21);   // papier peint a fleurs (motifs doux)
    for (int i = 0; i < 70; ++i)
    {
        const float x = fl.nextFloat() * (float) getWidth(), y = fl.nextFloat() * (float) getHeight(), r = 8.f + fl.nextFloat() * 22.f;
        g.setColour (juce::Colour (0xff6e4450).withAlpha (0.35f));
        for (int k = 0; k < 5; ++k) { const float an = k * 1.2566f; g.fillEllipse (x + std::cos (an) * r * 0.8f - r * 0.5f, y + std::sin (an) * r * 0.8f - r * 0.5f, r, r); }
    }
    // plaque rose metal brosse
    auto plate = R (155, 60, 1695, 1385).toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe6bfc8), plate.getX(), plate.getY(), juce::Colour (0xffb98593), plate.getX(), plate.getBottom(), false));
    g.fillRoundedRectangle (plate, 4.f);
    for (int side = 0; side < 2; ++side)   // rails argent
    {
        auto r = side == 0 ? R (45, 60, 110, 1385) : R (1850, 60, 105, 1385);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffcfd2d6), (float) r.getX(), 0, juce::Colour (0xff767a80), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 4.f);
        g.setColour (juce::Colour (0xffd8a8b6)); g.fillRoundedRectangle (r.reduced (28, 130).toFloat(), 8.f);
        g.setColour (amber.withAlpha (0.8f)); g.drawRoundedRectangle (r.reduced (28, 130).toFloat(), 8.f, 1.5f);
        for (int y : { 125, 1385 }) { g.setColour (juce::Colour (0xff3c3c42)); g.fillEllipse ((float) r.getCentreX() - 15, y * K - 15, 30.f, 30.f); g.setColour (juce::Colour (0xffb0b2b8)); g.fillEllipse ((float) r.getCentreX() - 10, y * K - 10, 20.f, 20.f); }
    }
    // haut-parleurs perfores (portent les petits boutons)
    for (auto r : { R (637, 375, 123, 400), R (1240, 375, 122, 400) })
    {
        g.setColour (juce::Colour (0xff8f6471)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        g.setColour (juce::Colour (0xff1c1214)); g.fillRoundedRectangle (r.reduced (4).toFloat(), 6.f);
        for (float yy = (float) r.getY() + 9; yy < (float) r.getBottom() - 6; yy += 8.5f)
            for (float xx = (float) r.getX() + 9; xx < (float) r.getRight() - 6; xx += 8.5f)
            { g.setColour (juce::Colour (0xff4a343c)); g.fillEllipse (xx - 1.f, yy - 1.f, 6.f, 6.f); g.setColour (juce::Colour (0xff0a0607)); g.fillEllipse (xx, yy, 4.5f, 4.5f); }
    }
    g.drawImage (camel, R (812, 367, 376, 376).toFloat(), juce::RectanglePlacement::centred);
    {   // logo
        auto lr = R (1726, 96, 84, 80).toFloat();
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    for (float a : { 0.2f, 0.35f })
    { g.setColour (amber.withAlpha (a)); g.setFont (juce::FontOptions (28.f + (0.4f - a) * 4.f, juce::Font::bold)); g.drawFittedText ("DJBILBOXBEATS", R (1400, 195, 420, 80), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xff2a1d22)); g.setFont (juce::FontOptions (27.f, juce::Font::bold));
    g.drawFittedText ("DJBILBOXBEATS", R (1400, 195, 420, 80), juce::Justification::centred, 1);
    // titre floral neon
    const auto titleR = R (730, 780, 540, 110);
    g.setFont (juce::FontOptions (46.f, juce::Font::bold));
    for (float a : { 0.15f, 0.3f }) { g.setColour (amber.withAlpha (a)); g.drawFittedText ("CHINTZWAZ", titleR.expanded (3, 2), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xff1c8f88)); for (int dx = -2; dx <= 2; dx += 2) for (int dy = -2; dy <= 2; dy += 2) g.drawFittedText ("CHINTZWAZ", titleR.translated (dx, dy), juce::Justification::centred, 1);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffff0f4), 0, 393.f, juce::Colour (0xfff0a8b8), 0, 430.f, false));
    g.drawFittedText ("CHINTZWAZ", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xff2a1d22)); g.setFont (juce::FontOptions (20.f));
    g.drawFittedText ("By Humpire", R (880, 888, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (625, 185, 753, 110).toFloat();
    g.setColour (ember.withAlpha (0.6f)); g.fillRoundedRectangle (scr.expanded (3.f), 14.f);
    g.setColour (amber.withAlpha (0.9f)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (juce::Colour (0xff0c1c20)); g.fillRoundedRectangle (scr.reduced (4.f), 9.f);
    g.setColour (juce::Colour (0xff102a30)); g.fillRoundedRectangle (R (780, 205, 400, 66).toFloat(), 4.f);
    {   // icones
        g.setColour (amber); const auto f = R (660, 213, 50, 40).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1292, 213, 46, 46).toFloat(); g.setColour (juce::Colour (0xffd8e0e4));
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (juce::Colour (0xffd8e0e4));
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (753, 0, 0, 0).getX() : (float) R (1245, 0, 0, 0).getX(), cy = (float) R (0, 238, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 10.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 10.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    {
        auto fr = R (228, 140, 292, 392).toFloat();
        g.setColour (amber.withAlpha (0.6f)); g.fillRoundedRectangle (fr.expanded (3.f), 12.f);
        g.setColour (juce::Colour (0xff10181a)); g.fillRoundedRectangle (fr, 10.f);
        g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
        g.drawImage (cassette, R (226, 138, 360, 396).toFloat(), juce::RectanglePlacement::centred);
        g.setColour (juce::Colour (0xff2a1d22)); g.setFont (juce::FontOptions (14.f, juce::Font::bold));
        g.drawFittedText ("EXPANSION", R (250, 90, 250, 44), juce::Justification::centred, 1);
    }
    // plaque des molettes
    g.setColour (juce::Colour (0xff18151a)); g.fillRoundedRectangle (R (185, 1085, 381, 335).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff5a4a52)); g.drawRoundedRectangle (R (185, 1085, 381, 335).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xff2a1d22));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setColour (k->s.getProperties()["small"] ? juce::Colour (0xffd8f8f4) : juce::Colour (0xff2a1d22));
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 9.f : 12.5f), juce::Font::bold));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    // section headers (rules + text), one per group of controls
    const auto ink = juce::Colour (0xff2a1d22);
    drawHeader (g, "LFO", ink, 15.f, 205, 442, 575);
    drawHeader (g, "AMP ENVELOPE", ink, 12.f, 205, 580, 760);
    drawHeader (g, "FILTER ENVELOPE", ink, 12.f, 1378, 1600, 375);
    drawHeader (g, "EFFECTS", ink, 12.f, 1424, 1796, 664);
    // captions under the five drop-down menus
    g.setColour (ink); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    {
        const char* cap[5] = { "ENGINE", "LFO SHAPE", "LFO DESTINATION", "FILTER TYPE", "VOICE MODE" };
        const int cx[5] = { 741, 984, 1228, 1471, 1700 };
        for (int i = 0; i < 5; ++i) g.drawFittedText (cap[i], R (cx[i] - 110, 1058, 220, 30), juce::Justification::centred, 1);
    }
    g.setColour (juce::Colour (0xffd8e0e4)); g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawFittedText ("PITCH BEND", C (297, 1388, 180, 40), juce::Justification::centred, 1);
    g.drawFittedText ("MOD WHEEL", C (445, 1388, 180, 40), juce::Justification::centred, 1);
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
}
