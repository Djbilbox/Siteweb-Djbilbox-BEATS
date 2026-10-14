#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Dz;

namespace
{
const juce::Colour amber { 0xffff5a4a }, ember { 0xff3fe06e }, dark { 0xff14160f };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }
}

DzLNF::DzLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffeee6d0));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffeee6d0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2a1d));
}

void DzLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void DzLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void DzLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffeee6d0)); g.fillPath (t);
}

void DzLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void DzLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff3a2a1d) : over ? juce::Colour (0xff242a18) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
DzPanel::DzPanel (DzProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    logo = load (BinaryData::logo_png, BinaryData::logo_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::rate, "RATE");       addKnob (amount, P::amount, "AMOUNT");  addKnob (pitch, P::pitch, "PITCH");
    addKnob (glide, P::glide, "GLIDE");    addKnob (filter, P::filter, "CUTOFF");
    addKnob (tone, P::t1, "TONE");       addKnob (body, P::t2, "BODY");       addKnob (bright, P::t3, "BRIGHT");
    addKnob (width, P::t4, "WIDTH");       addKnob (texture, P::t5, "TEXTURE");
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DELAY");
    addKnob (dtime, P::dtime, "TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "ENV"); addKnob (resoK, P::reso, "RESONANCE");
    const char* tn[5] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE", "VELOCITY" };
    const int ti[5] = { P::fatk, P::fdec, P::fsus, P::frel, P::velocity };
    for (int i = 0; i < 5; ++i) addKnob (rsl[i], ti[i], tn[i], juce::Slider::LinearVertical);
    addKnob (bend, P::bend, "", juce::Slider::LinearVertical);
    addKnob (modw, P::modw, "", juce::Slider::LinearVertical);
    bend.s.getProperties().set ("wheel", true); modw.s.getProperties().set ("wheel", true);
    all.erase (std::remove (all.begin(), all.end(), &bend), all.end()); all.erase (std::remove (all.begin(), all.end(), &modw), all.end());
    bend.s.onDragEnd = [this] { bend.s.setValue (0.0, juce::sendNotificationSync); };
    for (auto* k : { &tone, &body, &bright, &width, &texture, &fenvK, &delay }) k->s.getProperties().set ("small", true);

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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffeee6d0));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setKeyWidth (31.f);
    keys.setLowestVisibleKey (36);
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

DzPanel::~DzPanel() { setLookAndFeel (nullptr); }

void DzPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void DzPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void DzPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void DzPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File DzPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("DZPOWER").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void DzPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.dzpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(aucun preset utilisateur)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Ouvrir un fichier .dzpreset...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Charger un preset", userPresetDir(), "*.dzpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void DzPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Sauvegarder le preset", userPresetDir(), "*.dzpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("dzpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void DzPanel::loadUserPreset (const juce::File& f)
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

void DzPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Les packs d'expansion arrivent bientot.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void DzPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4e5638), 0, 0, juce::Colour (0xff2e3322), 0, (float) getHeight(), false));
    g.fillAll();
    juce::Random cl (3);   // metal patine / rayures
    for (int i = 0; i < 160; ++i)
    {
        const float x = cl.nextFloat() * (float) getWidth(), y = cl.nextFloat() * (float) getHeight(), r = 12.f + cl.nextFloat() * 45.f;
        g.setColour (juce::Colours::black.withAlpha (0.03f)); g.fillEllipse (x - r, y - r, r * 2, r * 2);
    }
    for (int side = 0; side < 2; ++side)   // rails sombres a plaques
    {
        auto r = side == 0 ? R (0, 0, 135, 1493) : R (1870, 0, 130, 1493);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3c3e40), (float) r.getX(), 0, juce::Colour (0xff1a1c1e), (float) r.getRight(), 0, false));
        g.fillRect (r);
        for (int y : { 150, 585, 1045 })
        {
            const auto pl = R (side == 0 ? 30 : 1900, y, 70, y == 585 ? 280 : 310);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8a8c90), (float) pl.getX(), 0, juce::Colour (0xff34363a), (float) pl.getRight(), 0, false));
            g.fillRoundedRectangle (pl.toFloat(), 8.f);
        }
        g.setColour ((side == 0 ? amber : ember).withAlpha (0.5f)); g.fillRect (side == 0 ? r.getRight() - 2 : r.getX(), 0, 2, r.getHeight());
        for (int y : { 80, 520, 985, 1410 }) { g.setColour (juce::Colour (0xff101214)); g.fillEllipse ((float) r.getCentreX() - 16, y * K - 16, 32.f, 32.f); g.setColour (juce::Colour (0xff8a8c90)); g.fillEllipse ((float) r.getCentreX() - 11, y * K - 11, 22.f, 22.f); }
    }
    // haut-parleurs perfores (portent les petits boutons)
    for (auto r : { R (575, 305, 225, 205), R (1200, 305, 230, 205) })
    {
        g.setColour (juce::Colour (0xff23261b)); g.fillRoundedRectangle (r.toFloat(), 14.f);
        for (float yy = (float) r.getY() + 8; yy < (float) r.getBottom() - 6; yy += 8.f)
            for (float xx = (float) r.getX() + 8; xx < (float) r.getRight() - 6; xx += 8.f)
            { g.setColour (juce::Colour (0xff0b0d08)); g.fillEllipse (xx, yy, 5.f, 5.f); }
    }
    g.drawImage (camel, R (815, 235, 372, 368).toFloat(), juce::RectanglePlacement::centred);
    {   // logo
        auto lr = R (1584, 78, 112, 108).toFloat();
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    for (float a : { 0.15f, 0.25f })
    { g.setColour (amber.withAlpha (a)); g.setFont (juce::FontOptions (23.f + (0.3f - a) * 6.f, juce::Font::bold)); g.drawFittedText ("DJBILBOXBEATS", R (1420, 195, 440, 75), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xffeee6d0)); g.setFont (juce::FontOptions (22.f, juce::Font::bold));
    g.drawFittedText ("DJBILBOXBEATS", R (1420, 195, 440, 75), juce::Justification::centred, 1);
    // titre electrique
    const auto titleR = R (700, 625, 600, 120);
    g.setFont (juce::FontOptions (56.f, juce::Font::bold));
    g.setColour (amber.withAlpha (0.35f)); g.drawFittedText ("DZPOWER", titleR.translated (-3, 0), juce::Justification::centred, 1);
    g.setColour (ember.withAlpha (0.35f)); g.drawFittedText ("DZPOWER", titleR.translated (3, 0), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xff101010)); for (int dx = -2; dx <= 2; dx += 2) for (int dy = -2; dy <= 2; dy += 2) g.drawFittedText ("DZPOWER", titleR.translated (dx, dy), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xfff4f2ea)); g.drawFittedText ("DZPOWER", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffd4d4c8)); g.setFont (juce::FontOptions (20.f, juce::Font::bold));
    g.drawFittedText ("By Humpire", R (880, 735, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (597, 72, 808, 118).toFloat();
    g.setGradientFill (juce::ColourGradient (amber, scr.getX(), 0, ember, scr.getRight(), 0, false)); g.fillRoundedRectangle (scr.expanded (2.f), 14.f);
    g.setColour (juce::Colour (0xff4a4c50)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (juce::Colour (0xff120e0e)); g.fillRoundedRectangle (scr.reduced (6.f), 9.f);
    g.setColour (juce::Colour (0xff1c1210)); g.fillRoundedRectangle (R (702, 100, 596, 66).toFloat(), 4.f);
    {   // icones
        g.setColour (juce::Colour (0xffa8a898)); const auto f = R (632, 112, 50, 40).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1324, 112, 44, 44).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (juce::Colour (0xffe8e0c8));
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (728, 0, 0, 0).getX() : (float) R (1272, 0, 0, 0).getX(), cy = (float) R (0, 133, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 10.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 10.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, R (205, 92, 317, 386).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffeee6d0)); g.setFont (juce::FontOptions (14.f));
    g.drawFittedText ("EXPANSION", R (240, 40, 230, 40), juce::Justification::centred, 1);
    // plaque des molettes
    g.setColour (juce::Colour (0xff0e100c)); g.fillRoundedRectangle (R (175, 1080, 367, 352).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff3a3e32)); g.drawRoundedRectangle (R (175, 1080, 367, 352).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffe6e0cc));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 9.5f : 12.f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    g.setFont (juce::FontOptions (15.f, juce::Font::bold));
    g.drawFittedText ("LFO", R (354, 535, 120, 40), juce::Justification::centred, 1);
    g.drawFittedText ("ASDR", R (1519, 488, 120, 40), juce::Justification::centred, 1);
    g.drawFittedText ("FILTER", R (1085, 772, 120, 40), juce::Justification::centred, 1);
    g.setFont (juce::FontOptions (12.f));
    g.drawFittedText ("DELAY", R (1583, 765, 120, 36), juce::Justification::centred, 1);
    g.setFont (juce::FontOptions (11.5f));
    g.drawFittedText ("Pitch Bend", R (221, 1378, 120, 40), juce::Justification::centred, 1);
    g.drawFittedText ("Modulation", R (372, 1378, 120, 40), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffb8b4a0));   // accolades
    g.drawLine ((float) R (259, 0, 0, 0).getX(), (float) R (0, 562, 0, 0).getY(), (float) R (340, 0, 0, 0).getX(), (float) R (0, 562, 0, 0).getY(), 1.f);
    g.drawLine ((float) R (490, 0, 0, 0).getX(), (float) R (0, 562, 0, 0).getY(), (float) R (572, 0, 0, 0).getX(), (float) R (0, 562, 0, 0).getY(), 1.f);
}

void DzPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 258, 648, 100);  place (amount, 413, 648, 100); place (pitch, 567, 648, 100);
    place (volume, 1575, 395, 100); place (reverb, 1752, 395, 100);
    place (glide, 825, 865, 100); place (filter, 1068, 865, 100); place (resoK, 1222, 865, 100);
    place (chorus, 1387, 838, 100); place (dist, 1515, 838, 100); place (dtime, 1643, 860, 100); place (feedback, 1771, 860, 100);
    place (tone, 640, 365, 64);  place (body, 740, 365, 64); place (bright, 640, 455, 64); place (width, 740, 455, 64);
    place (texture, 1262, 365, 64); place (fenvK, 1362, 365, 64); place (delay, 1312, 455, 64);
    const int ex[4] = { 257, 362, 467, 573 }, tx[5] = { 1388, 1485, 1583, 1680, 1776 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 28, 775, 56, 190));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 545, 56, 155));
    bend.s.setBounds (C (280, 1255, 90, 260)); modw.s.setBounds (C (432, 1255, 90, 260));
    cWave.c.setBounds (R (665, 985, 205, 57)); cLfo.c.setBounds (R (900, 985, 205, 57)); cDest.c.setBounds (R (1135, 985, 205, 57));
    cFilt.c.setBounds (R (1373, 985, 207, 57)); cMono.c.setBounds (R (1612, 985, 206, 57));
    folder.setBounds (R (632, 92, 100, 80)); save.setBounds (R (1310, 92, 80, 80));
    prev.setBounds (R (705, 92, 50, 80)); next.setBounds (R (1245, 92, 55, 80));
    presetLabel.setBounds (R (770, 100, 470, 66));
    expansion.setBounds (R (205, 92, 317, 386));
    keys.setBounds (R (583, 1110, 1232, 305));
}
