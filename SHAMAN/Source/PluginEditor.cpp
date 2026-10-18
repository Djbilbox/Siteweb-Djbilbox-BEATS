#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Sh;

namespace
{
const juce::Colour amber { 0xffffb030 }, ember { 0xffff6a10 }, dark { 0xff140c08 };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }

// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 36..84 = 4 octaves + 1 key
constexpr int kKeyLow = 36, kKeyHigh = 84;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }

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

// Captions: each one is centred at cx on a text row. Captions that would touch a neighbour on the same row shrink
// (both of them, proportionally) until they clear each other; the others keep their natural size.
struct Cap
{
    juce::String text; juce::Rectangle<int> box; juce::Colour col; float wanted = 10.f; bool bold = false; float size = 0.f;
    float width (float sz) const { return juce::GlyphArrangement::getStringWidth (juce::Font (juce::FontOptions (sz, bold ? juce::Font::bold : juce::Font::plain)), text); }
};
void drawCaptions (juce::Graphics& g, std::vector<Cap>& caps)
{
    for (auto& c : caps) c.size = c.wanted;
    for (int pass = 0; pass < 3; ++pass)
        for (size_t i = 0; i < caps.size(); ++i)
            for (size_t j = i + 1; j < caps.size(); ++j)
            {
                auto& a = caps[i]; auto& b = caps[j];
                if (std::abs (a.box.getY() - b.box.getY()) >= 12) continue;
                const float d = (float) std::abs (a.box.getCentreX() - b.box.getCentreX()), wa = a.width (a.size), wb = b.width (b.size);
                if ((wa + wb) * 0.5f + 4.f > d && wa + wb > 0.f)
                {
                    const float f = juce::jmax (0.5f, 2.f * (d - 4.f) / (wa + wb));
                    a.size = juce::jmax (7.f, a.size * f); b.size = juce::jmax (7.f, b.size * f);
                }
            }
    for (auto& c : caps)
    {
        g.setColour (c.col); g.setFont (juce::FontOptions (c.size, c.bold ? juce::Font::bold : juce::Font::plain));
        g.drawText (c.text, c.box, juce::Justification::centred, false);
    }
}
}

ShLNF::ShLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffffc878));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffc878));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff4a2a10));
}

void ShLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void ShLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void ShLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffffc878)); g.fillPath (t);
}

void ShLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void ShLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff4a2a10) : over ? juce::Colour (0xff301c0a) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
ShPanel::ShPanel (ShProcessor& p)
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
    addKnob (dtime, P::dtime, "DLY TIME");  addKnob (feedback, P::fb, "DLY FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "ENV AMT"); addKnob (resoK, P::reso, "RES");
    const char* tn[5] = { "ATK", "DEC", "SUS", "REL", "VELOCITY" };
    const int ti[5] = { P::fatk, P::fdec, P::fsus, P::frel, P::velocity };
    for (int i = 0; i < 5; ++i) addKnob (rsl[i], ti[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &tone, &body, &bright, &width, &texture, &fenvK, &resoK, &delay }) k->s.getProperties().set ("small", true);

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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffc878));
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

ShPanel::~ShPanel() { setLookAndFeel (nullptr); }

void ShPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void ShPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void ShPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void ShPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File ShPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("SHAMAN").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void ShPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.shpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .shpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.shpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void ShPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.shpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("shpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void ShPanel::loadUserPreset (const juce::File& f)
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

void ShPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void ShPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff566838), 0, 0, juce::Colour (0xff2f3a22), 0, (float) getHeight(), false));
    g.fillAll();
    juce::Random cl (13);   // taches de patine / rouille
    for (int i = 0; i < 220; ++i)
    {
        const float x = cl.nextFloat() * (float) getWidth(), y = cl.nextFloat() * (float) getHeight(), r = 8.f + cl.nextFloat() * 40.f;
        g.setColour ((i % 3 == 0 ? juce::Colour (0xffaaa070) : juce::Colours::black).withAlpha (0.035f)); g.fillEllipse (x - r, y - r, r * 2, r * 2);
    }
    g.setColour (juce::Colour (0xff1a1d12));   // separations de panneaux
    for (int y : { 365, 525, 1095 }) g.fillRect (R (140, y - 3, 1720, 6));
    for (int side = 0; side < 2; ++side)   // rails acier
    {
        auto r = side == 0 ? R (25, 15, 112, 1465) : R (1865, 15, 112, 1465);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffa4a8a8), (float) r.getX(), 0, juce::Colour (0xff383c3c), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 8.f);
        g.setColour (amber.withAlpha (0.45f)); g.fillRect (side == 0 ? r.getRight() - 2 : r.getX(), r.getY() + 8, 2, r.getHeight() - 16);
        for (int y : { 128, 495, 975, 1395 }) { g.setColour (juce::Colour (0xff1c1e1e)); g.fillEllipse ((float) r.getCentreX() - 17, y * K - 17, 34.f, 34.f); g.setColour (juce::Colour (0xff9a9c9c)); g.fillEllipse ((float) r.getCentreX() - 12, y * K - 12, 24.f, 24.f); }
    }
    // haut-parleurs orange (portent les petits boutons)
    for (auto r : { R (205, 385, 565, 125), R (1232, 385, 563, 125) })
    {
        g.setColour (juce::Colour (0xff1a1410)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        for (float yy = (float) r.getY() + 8; yy < (float) r.getBottom() - 6; yy += 8.f)
            for (float xx = (float) r.getX() + 8; xx < (float) r.getRight() - 6; xx += 8.f)
            { g.setColour (ember.withAlpha (0.4f)); g.fillEllipse (xx - 1.f, yy - 1.f, 6.f, 6.f); g.setColour (juce::Colour (0xff0a0705)); g.fillEllipse (xx, yy, 4.5f, 4.5f); }
    }
    g.drawImage (camel, R (812, 292, 376, 376).toFloat(), juce::RectanglePlacement::centred);
    {   // logo
        auto lr = R (1552, 100, 132, 130).toFloat();
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    for (float a : { 0.2f, 0.35f })
    { g.setColour (ember.withAlpha (a)); g.setFont (juce::FontOptions (30.f + (0.4f - a) * 4.f, juce::Font::bold)); g.drawFittedText ("DJBILBOX BEATS", R (1390, 240, 460, 75), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xffffc878)); g.setFont (juce::FontOptions (29.f, juce::Font::bold));
    g.drawFittedText ("DJBILBOX BEATS", R (1390, 240, 460, 75), juce::Justification::centred, 1);
    // titre tribal incandescent
    const auto titleR = R (740, 690, 520, 110);
    g.setFont (juce::FontOptions (50.f, juce::Font::bold));
    for (float a : { 0.12f, 0.2f, 0.3f }) { g.setColour (ember.withAlpha (a)); g.drawFittedText ("SHAMAN", titleR.expanded (4, 2), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xff3a1204)); for (int dx = -2; dx <= 2; dx += 2) for (int dy = -2; dy <= 2; dy += 2) g.drawFittedText ("SHAMAN", titleR.translated (dx, dy), juce::Justification::centred, 1);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffffe0a0), 0, 345.f, juce::Colour (0xffff8a20), 0, 385.f, false));
    g.drawFittedText ("SHAMAN", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffd8d0b8)); g.setFont (juce::FontOptions (20.f));
    g.drawFittedText ("By Humpire", R (880, 798, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (640, 118, 728, 122).toFloat();
    g.setColour (juce::Colour (0xff6a6e6c)); g.fillRoundedRectangle (scr.expanded (3.f), 12.f);
    g.setColour (juce::Colour (0xff180d08)); g.fillRoundedRectangle (scr, 10.f);
    g.setColour (ember.withAlpha (0.35f)); g.drawRoundedRectangle (scr.reduced (3.f), 9.f, 1.5f);
    g.setColour (juce::Colour (0xff241408)); g.fillRoundedRectangle (R (800, 138, 410, 84).toFloat(), 4.f);
    {   // icones
        g.setColour (amber); const auto f = R (673, 148, 50, 40).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1285, 150, 48, 48).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (762, 0, 0, 0).getX() : (float) R (1248, 0, 0, 0).getX(), cy = (float) R (0, 182, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 11.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 11.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, R (180, 92, 416, 260).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffe8dcc0)); g.setFont (juce::FontOptions (14.f));
    g.drawFittedText ("EXPANSION", R (300, 50, 220, 40), juce::Justification::centred, 1);
    // plaque des molettes
    g.setColour (juce::Colour (0xff17140e)); g.fillRoundedRectangle (R (218, 1125, 402, 285).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff5a5a50)); g.drawRoundedRectangle (R (218, 1125, 402, 285).toFloat(), 10.f, 2.f);
    // captions of the knobs and sliders (no overlapping texts: neighbours on the same row shrink to fit)
    const auto ink = juce::Colour (0xffe8e0c8);
    std::vector<Cap> caps;
    for (auto* k : all)
    {
        if (k->label.isEmpty()) continue;
        const auto r = k->s.getBounds();
        const bool small = k->s.getProperties()["small"];
        caps.push_back ({ k->label, { r.getCentreX() - 60, r.getBottom() + (k->lin ? 2 : 0), 120, 18 },
                          small ? juce::Colour (0xffffd8a0) : ink, small ? 10.f : (k->lin ? 10.5f : 12.5f), false });
    }
    // captions of the five drop-down menus (under each menu)
    {
        const char* ct[5] = { "ENGINE", "LFO SHAPE", "LFO DEST", "FILTER TYPE", "VOICE MODE" };
        Combo* cc[5] = { &cWave, &cLfo, &cDest, &cFilt, &cMono };
        for (int i = 0; i < 5; ++i)
        {
            const auto b = cc[i]->c.getBounds();
            caps.push_back ({ ct[i], { b.getCentreX() - 70, b.getBottom() + 2, 140, 15 }, juce::Colour (0xffffd8a0), 10.f, false });
        }
    }
    drawCaptions (g, caps);
    // section headers (rules + text), one per group of controls
    drawHeader (g, "LFO", ink, 15.f, 318, 587, 548);
    drawHeader (g, "AMP ENVELOPE", ink, 12.f, 318, 720, 735);
    drawHeader (g, "FILTER ENVELOPE", ink, 12.f, 1250, 1550, 735);
    g.setColour (ink); g.setFont (juce::FontOptions (12.f));
    g.drawFittedText ("Pitch Bend", R (275, 1368, 140, 40), juce::Justification::centred, 1);
    g.drawFittedText ("Modulation", R (420, 1368, 140, 40), juce::Justification::centred, 1);
}

void ShPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 380, 625, 100);  place (amount, 525, 625, 100); place (pitch, 672, 625, 100);
    place (volume, 1322, 625, 100); place (reverb, 1470, 625, 100); place (chorus, 1613, 625, 100); place (dist, 1757, 625, 100);
    place (dtime, 1757, 760, 100); place (feedback, 1757, 900, 100);
    place (glide, 903, 900, 100); place (filter, 1095, 900, 100);
    place (tone, 275, 448, 64); place (body, 385, 448, 64); place (bright, 495, 448, 64); place (width, 605, 448, 64);
    place (texture, 1300, 448, 64); place (resoK, 1410, 448, 64); place (fenvK, 1520, 448, 64); place (delay, 1630, 448, 64);
    const int ex[4] = { 357, 465, 573, 680 }, tx[5] = { 1290, 1363, 1436, 1510, 1612 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 28, 760, 56, 187));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 760, 56, 187));
    bend.s.setBounds (C (345, 1255, 90, 245)); modw.s.setBounds (C (490, 1255, 90, 245));
    cWave.c.setBounds (R (215, 1008, 290, 50)); cLfo.c.setBounds (R (540, 1008, 290, 50)); cDest.c.setBounds (R (864, 1008, 290, 50));
    cFilt.c.setBounds (R (1190, 1008, 290, 50)); cMono.c.setBounds (R (1520, 1008, 270, 50));
    folder.setBounds (R (658, 138, 80, 84)); save.setBounds (R (1270, 138, 80, 84));
    prev.setBounds (R (735, 138, 60, 84)); next.setBounds (R (1215, 138, 60, 84));
    presetLabel.setBounds (R (800, 140, 410, 80));
    expansion.setBounds (R (225, 100, 365, 240));
    keys.setBounds (R (652, 1135, 1148, 290));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly
}
