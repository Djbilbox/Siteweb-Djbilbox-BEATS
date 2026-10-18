#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Wz;

namespace
{
// On-screen keyboard shows kKeyLow..kKeyHigh (middle C = C3 in JUCE naming): 36..84 = 4 octaves + 1 key
constexpr int kKeyLow = 36, kKeyHigh = 84;
int countWhiteKeys (int lo, int hi) { int n = 0; for (int k = lo; k <= hi; ++k) if (! juce::MidiMessage::isMidiNoteBlack (k)) ++n; return n; }
const juce::Colour green { 0xff8fe04f }, red { 0xffff5a4a }, dark { 0xff101010 }, bgDark { 0xff191919 };
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
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe9e9e9), ctr.x - kr * 0.4f, ctr.y - kr * 0.5f, juce::Colour (0xff3d3d3d), ctr.x + kr * 0.6f, ctr.y + kr * 0.7f, false));
    g.fillEllipse (ctr.x - kr, ctr.y - kr, kr * 2, kr * 2);
    g.setColour (juce::Colours::black.withAlpha (0.7f)); g.drawEllipse (ctr.x - kr, ctr.y - kr, kr * 2, kr * 2, 1.4f);
    juce::Path p; p.addRoundedRectangle (-1.6f, -kr, 3.2f, kr * 0.42f, 1.4f);
    g.setColour (juce::Colour (0xff1a1a1a)); g.fillPath (p, juce::AffineTransform::rotation (ang).translated (ctr.x, ctr.y));
    juce::ignoreUnused (s);
}

void WarzoneLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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
    for (int i = 0; i <= 8; ++i)   // graduations
    {
        const float yy = (float) y + (float) h * (float) i / 8.f;
        g.setColour (ramp ((float) i / 8.f).withAlpha (0.55f));
        g.fillRect (cx - 16.f, yy, 5.f, 2.f); g.fillRect (cx + 11.f, yy, 5.f, 2.f);
    }
    auto cap = juce::Rectangle<float> (cx - 12.f, pos - 7.f, 24.f, 14.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd9d9d9), 0, cap.getY(), juce::Colour (0xff4b4b4b), 0, cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 3.f);
    g.setColour (juce::Colours::black); g.drawRoundedRectangle (cap, 3.f, 1.2f); g.drawHorizontalLine ((int) cap.getCentreY(), cap.getX() + 3, cap.getRight() - 3);
}

void WarzoneLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (green.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (green, r.getX(), 0, red, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffb6e58a)); g.fillPath (t);
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
    const char* fn[4] = { "ATK", "DEC", "SUS", "VELOCITY" };
    const int fi[4] = { P::fattack, P::fdecay, P::fsustain, P::velocity };
    for (int i = 0; i < 4; ++i) addKnob (fenv[i], fi[i], fn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);   // wheels are captioned in paint()
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &unison, &detune, &dmix, &reso, &envdepth }) k->s.getProperties().set ("small", true);
    unison.s.setNumDecimalPlacesToDisplay (0);
    // maximum label width (px) per control: keeps neighbouring captions from touching
    for (auto& k : env) k.s.getProperties().set ("lw", 46);
    for (int i = 0; i < 3; ++i) fenv[i].s.getProperties().set ("lw", 34);
    fenv[3].s.getProperties().set ("lw", 60);
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
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff222222), 0, 0, juce::Colour (0xff0d0d0d), 0, (float) getHeight(), false));
    g.fillAll();
    for (int side = 0; side < 2; ++side)   // rails metal
    {
        auto r = side == 0 ? R (0, 0, 110, 1493) : R (1890, 0, 110, 1493);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff9a9a9a), (float) r.getX(), 0, juce::Colour (0xff3a3a3a), (float) r.getRight(), 0, false));
        g.fillRect (r);
        g.setColour (juce::Colour (0xff161616)); g.fillRoundedRectangle (r.reduced (20, 120).toFloat().withTrimmedBottom (80.f), 6.f);
        for (int y : { 60, 1433 }) { g.setColour (juce::Colour (0xff1d1d1d)); g.fillEllipse ((float) r.getCentreX() - 11, y * K - 11, 22.f, 22.f); }
    }
    // haut-parleurs
    for (auto r : { R (575, 255, 163, 343), R (1265, 255, 163, 343) })
    {
        g.setColour (juce::Colour (0xff0a0a0a)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        for (float yy = (float) r.getY() + 7; yy < (float) r.getBottom() - 8; yy += 10.f)
            for (float xx = (float) r.getX() + 8; xx < (float) r.getRight() - 8; xx += 10.f)
            { g.setColour (ramp ((xx - (float) r.getX()) / (float) r.getWidth()).withAlpha (0.12f)); g.fillEllipse (xx - 1, yy - 1, 8.f, 8.f); g.setColour (juce::Colour (0xff2a2a2a)); g.fillEllipse (xx, yy, 6.f, 6.f); }
    }
    // medaillon, logo
    g.drawImage (camel, R (790, 200, 420, 460).toFloat(), juce::RectanglePlacement::centred);
    g.drawImage (dbLogo, R (1603, 207, 110, 110).toFloat(), juce::RectanglePlacement::centred);
    auto txt = [&] (const juce::String& s, juce::Rectangle<int> r, float size, bool bold, juce::Colour c1, juce::Colour c2)
    {
        g.setGradientFill (juce::ColourGradient (c1, (float) r.getX(), 0, c2, (float) r.getRight(), 0, false));
        g.setFont (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
        g.drawFittedText (s, r, juce::Justification::centred, 1);
    };
    txt ("DJBILBOXBEATS", R (1470, 80, 380, 90), 34.f, true, green, red);
    txt ("WARZONE808", R (640, 665, 720, 150), 66.f, true, green.withAlpha (0.95f), red);
    g.setColour (juce::Colour (0xff9a9a9a)); g.setFont (juce::FontOptions (18.f));
    g.drawFittedText ("By Humpire", R (1100, 800, 260, 50), juce::Justification::centredRight, 1);
    // ecran preset
    auto scr = R (575, 68, 850, 110).toFloat();
    g.setColour (green.withAlpha (0.2f)); g.fillRoundedRectangle (scr.expanded (3.f), 14.f);
    g.setGradientFill (juce::ColourGradient (green, scr.getX(), 0, red, scr.getRight(), 0, false)); g.drawRoundedRectangle (scr, 12.f, 2.f);
    g.setColour (juce::Colour (0xff0a0f08)); g.fillRoundedRectangle (scr.reduced (6.f), 8.f);
    // cassette
    auto cas = R (165, 110, 315, 485).toFloat();
    g.setColour (green.withAlpha (0.25f)); g.fillRoundedRectangle (cas.expanded (3.f), 10.f);
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, cas.reduced (4.f), juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffb0b0b0)); g.setFont (juce::FontOptions (16.f));
    g.drawFittedText ("EXPANSION", R (165, 45, 315, 60), juce::Justification::centred, 1);
    // panneau molettes
    g.setColour (juce::Colour (0xff0b0b0b)); g.fillRoundedRectangle (R (148, 1110, 335, 330).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff2a2a2a)); g.drawRoundedRectangle (R (148, 1110, 335, 330).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffc8c8c8));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 11.f : (k->lin ? 9.5f : 14.f)));
        const int lw = (int) k->s.getProperties().getWithDefault ("lw", r.getWidth() + 50);
        g.drawFittedText (k->label, r.getCentreX() - lw / 2, r.getBottom() + (k->lin ? 2 : -4), lw, 18, juce::Justification::centred, 1);
    }
    // section headers (rules + text), one per group of controls
    const auto hdr = juce::Colour (0xffc8c8c8);
    drawHeader (g, "LFO", hdr, 16.f, 160, 432, 625);
    drawHeader (g, "AMP ENVELOPE", hdr, 12.f, 160, 570, 805);
    drawHeader (g, "FILTER ENVELOPE", hdr, 12.f, 1480, 1720, 345);
    // captions under the five drop-down menus
    g.setColour (juce::Colour (0xffb6e58a).withAlpha (0.85f)); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
    {
        const char* cap[5] = { "ENGINE", "LFO SHAPE", "LFO DESTINATION", "FILTER TYPE", "VOICE MODE" };
        const int cx[5] = { 703, 939, 1194, 1460, 1717 };
        for (int i = 0; i < 5; ++i) g.drawFittedText (cap[i], R (cx[i] - 110, 1088, 220, 28), juce::Justification::centred, 1);
    }
    g.setColour (juce::Colour (0xffc8c8c8)); g.setFont (juce::FontOptions (10.f));
    g.drawFittedText ("PITCH BEND", C (246, 1420, 130, 30), juce::Justification::centred, 1);
    g.drawFittedText ("MOD WHEEL", C (380, 1420, 130, 30), juce::Justification::centred, 1);
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
    const int ex[4] = { 220, 316, 413, 511 }, fx[4] = { 1529, 1602, 1677, 1790 };
    for (int i = 0; i < 4; ++i) { env[i].s.setBounds (R (ex[i] - 30, 825, 60, 205)); fenv[i].s.setBounds (R (fx[i] - 30, 370, 60, 210)); }
    bend.s.setBounds (C (246, 1278, 90, 250)); modw.s.setBounds (C (380, 1278, 90, 250));
    cWave.c.setBounds (R (600, 1028, 206, 56)); cLfo.c.setBounds (R (830, 1028, 218, 56)); cDest.c.setBounds (R (1072, 1028, 244, 56));
    cFilt.c.setBounds (R (1338, 1028, 244, 56)); cMono.c.setBounds (R (1606, 1028, 222, 56));
    folder.setBounds (R (595, 85, 150, 76)); save.setBounds (R (1255, 85, 150, 76));
    prev.setBounds (R (760, 85, 70, 76)); next.setBounds (R (1170, 85, 70, 76));
    presetLabel.setBounds (R (840, 85, 320, 76));
    expansion.setBounds (R (165, 110, 315, 485));
    keys.setBounds (R (515, 1120, 1317, 330));
    keys.setKeyWidth ((float) keys.getWidth() / (float) countWhiteKeys (kKeyLow, kKeyHigh));   // fit the whole range exactly
}
