#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Gh;

namespace
{
const juce::Colour amber { 0xffffc040 }, ember { 0xffff8a20 }, dark { 0xff14100b };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }
}

GhLNF::GhLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffe8d8b0));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffe8d8b0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff4a3318));
}

void GhLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void GhLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void GhLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffe8d8b0)); g.fillPath (t);
}

void GhLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void GhLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff4a3318) : over ? juce::Colour (0xff33220f) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
GhPanel::GhPanel (GhProcessor& p)
    : proc (p), keys (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto load = [] (const void* d, int n) { return juce::ImageFileFormat::loadFrom (d, (size_t) n); };
    camel = load (BinaryData::camel_png, BinaryData::camel_pngSize);
    logo = load (BinaryData::logo_png, BinaryData::logo_pngSize);
    cassette = load (BinaryData::cassette_png, BinaryData::cassette_pngSize);

    addKnob (rate, P::rate, "RATE");       addKnob (amount, P::amount, "AMOUNT");  addKnob (pitch, P::pitch, "PITCH");
    addKnob (glide, P::glide, "GLIDE");    addKnob (filter, P::filter, "FILTER");
    addKnob (tone, P::t1, "TONE");       addKnob (body, P::t2, "BODY");       addKnob (bright, P::t3, "BRIGHT");
    addKnob (width, P::t4, "WIDTH");       addKnob (texture, P::t5, "TEXTURE");
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DELAY");
    addKnob (dtime, P::dtime, "TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "A", "D", "S", "R" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "ENV"); addKnob (resoK, P::reso, "RES");
    const char* tn[4] = { "A", "D", "S", "VEL" };
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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe8d8b0));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setKeyWidth (32.5f);
    keys.setLowestVisibleKey (36);
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

GhPanel::~GhPanel() { setLookAndFeel (nullptr); }

void GhPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void GhPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void GhPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void GhPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File GhPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("GOLDHAWK").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void GhPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.ghpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(aucun preset utilisateur)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Ouvrir un fichier .ghpreset...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Charger un preset", userPresetDir(), "*.ghpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void GhPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Sauvegarder le preset", userPresetDir(), "*.ghpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("ghpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void GhPanel::loadUserPreset (const juce::File& f)
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

void GhPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Les packs d'expansion arrivent bientot.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void GhPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a3b26), 0, 0, juce::Colour (0xff17120d), 0, (float) getHeight(), false));
    g.fillAll();
    juce::Random cl (5);   // reflets brosses
    for (int i = 0; i < 120; ++i)
    {
        const float x = cl.nextFloat() * (float) getWidth(), y = cl.nextFloat() * (float) getHeight() * 0.7f, r = 20.f + cl.nextFloat() * 60.f;
        g.setColour (amber.withAlpha (0.012f)); g.fillEllipse (x - r, y - r, r * 2, r * 2);
    }
    for (int side = 0; side < 2; ++side)   // rails acier a plaques
    {
        auto r = side == 0 ? R (0, 0, 158, 1470) : R (1845, 0, 155, 1470);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffa4a49c), (float) r.getX(), 0, juce::Colour (0xff3a3a36), (float) r.getRight(), 0, false));
        g.fillRect (r);
        for (int y : { 170, 600, 1010 })
        {
            const auto pl = R (side == 0 ? 42 : 1884, y, 74, y == 600 ? 330 : 300);
            g.setColour (juce::Colour (0xff28261f)); g.fillRoundedRectangle (pl.toFloat(), 10.f);
            g.setColour (juce::Colour (0xff6c6a60)); g.drawRoundedRectangle (pl.toFloat().reduced (2.f), 9.f, 1.5f);
        }
        for (int y : { 100, 530, 940, 1355 }) { g.setColour (juce::Colour (0xff1c1a16)); g.fillEllipse ((float) r.getCentreX() - 16, y * K - 16, 32.f, 32.f); g.setColour (juce::Colour (0xff8a8880)); g.fillEllipse ((float) r.getCentreX() - 11, y * K - 11, 22.f, 22.f); }
    }
    // haut-parleurs a perforations dorees
    for (auto r : { R (660, 330, 115, 350), R (1228, 330, 114, 350) })
    {
        g.setColour (juce::Colour (0xff2a2620)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        g.setColour (juce::Colour (0xff0a0908)); g.fillRoundedRectangle (r.reduced (4).toFloat(), 6.f);
        for (float yy = (float) r.getY() + 10; yy < (float) r.getBottom() - 8; yy += 9.5f)
            for (float xx = (float) r.getX() + 10; xx < (float) r.getRight() - 8; xx += 9.5f)
            { g.setColour (juce::Colour (0xff3a342a)); g.fillEllipse (xx - 1.f, yy - 1.f, 7.f, 7.f); g.setColour (juce::Colour (0xff050403)); g.fillEllipse (xx, yy, 5.f, 5.f); }
    }
    g.drawImage (camel, R (812, 318, 376, 374).toFloat(), juce::RectanglePlacement::centred);
    {   // logo
        auto lr = R (1642, 128, 162, 160).toFloat();
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    {   // DJBILBOXBEATS
        juce::AttributedString as; as.setJustification (juce::Justification::centred);
        const auto fnt = juce::FontOptions (22.f, juce::Font::bold);
        as.append ("DJ", fnt, juce::Colour (0xffe0d0a8)); as.append ("BILBOX", fnt, juce::Colour (0xfffff0c8)); as.append ("BEATS", fnt, juce::Colour (0xffe0d0a8));
        as.draw (g, R (1380, 40, 440, 70).toFloat());
    }
    // titre dore
    const auto titleR = R (700, 710, 600, 100);
    g.setFont (juce::FontOptions (46.f, juce::Font::bold | juce::Font::italic));
    for (float a : { 0.12f, 0.2f }) { g.setColour (amber.withAlpha (a)); g.drawFittedText ("GOLDHAWK", titleR.expanded (3, 2), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xff3a2606));
    for (int dx = -1; dx <= 1; dx += 2) for (int dy = -1; dy <= 1; dy += 2) g.drawFittedText ("GOLDHAWK", titleR.translated (dx * 2, dy * 2), juce::Justification::centred, 1);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffff0b0), 0, 358.f, juce::Colour (0xffe8a820), 0, 392.f, false));
    g.drawFittedText ("GOLDHAWK", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffc8c0b0)); g.setFont (juce::FontOptions (20.f, juce::Font::bold));
    g.drawFittedText ("By Humpire", R (880, 800, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (635, 140, 960, 130).toFloat();
    g.setColour (amber.withAlpha (0.25f)); g.fillRoundedRectangle (scr.expanded (4.f), 14.f);
    g.setColour (amber.withAlpha (0.9f)); g.drawRoundedRectangle (scr, 12.f, 2.5f);
    g.setColour (juce::Colour (0xff17120c)); g.fillRoundedRectangle (scr.reduced (4.f), 10.f);
    g.setColour (juce::Colour (0xff231a0f)); g.fillRoundedRectangle (R (820, 165, 600, 80).toFloat(), 4.f);
    {   // icones
        g.setColour (amber); const auto f = R (685, 180, 46, 38).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1445, 178, 42, 44).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (770, 0, 0, 0).getX() : (float) R (1405, 0, 0, 0).getX(), cy = (float) R (0, 200, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 10.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 10.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, R (196, 58, 398, 344).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffd8ccb0)); g.setFont (juce::FontOptions (14.f));
    g.drawFittedText ("EXPANSION", R (270, 8, 250, 44), juce::Justification::centred, 1);
    // plaque des molettes
    g.setColour (juce::Colour (0xff0c0a08)); g.fillRoundedRectangle (R (190, 1085, 295, 335).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff302a20)); g.drawRoundedRectangle (R (190, 1085, 295, 335).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffd8ccb0));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 11.5f : 12.5f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    g.setFont (juce::FontOptions (15.f));
    g.drawFittedText ("LFO", R (340, 588, 120, 36), juce::Justification::centred, 1);
    g.drawFittedText ("ENVELOPE", R (1500, 340, 180, 40), juce::Justification::centred, 1);
    g.setFont (juce::FontOptions (12.5f));
    g.drawFittedText ("DELAY", R (1602, 855, 120, 40), juce::Justification::centred, 1);
    const char* cap[4] = { "WAVEFORM", "LFO SOURCE", "DESTINATION", "MONO/POLY" };
    for (int i = 0; i < 4; ++i) g.drawFittedText (cap[i], R (770, 872 + i * 52, 190, 44), juce::Justification::centredLeft, 1);
    g.drawFittedText ("FILTER TYPE", R (1370, 975, 170, 40), juce::Justification::centred, 1);
}

void GhPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 257, 682, 100);  place (amount, 400, 682, 100); place (pitch, 545, 682, 100);
    place (glide, 697, 950, 100); place (filter, 1305, 950, 100);
    place (volume, 1735, 495, 100); place (reverb, 1450, 752, 100); place (chorus, 1590, 752, 100); place (dist, 1730, 752, 100);
    place (dtime, 1590, 950, 100); place (feedback, 1730, 950, 100);
    place (tone, 260, 442, 54);  place (body, 350, 442, 54); place (bright, 440, 442, 54); place (width, 530, 442, 54);
    place (texture, 260, 522, 54); place (resoK, 350, 522, 54); place (fenvK, 440, 522, 54); place (delay, 530, 522, 54);
    const int ex[4] = { 270, 358, 445, 533 }, tx[4] = { 1418, 1485, 1552, 1618 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 28, 810, 56, 180));
    for (int i = 0; i < 4; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 402, 56, 190));
    bend.s.setBounds (C (275, 1255, 90, 270)); modw.s.setBounds (C (400, 1255, 90, 270));
    cWave.c.setBounds (R (960, 872, 258, 46)); cLfo.c.setBounds (R (960, 924, 258, 46)); cDest.c.setBounds (R (960, 976, 258, 46));
    cMono.c.setBounds (R (960, 1028, 258, 46)); cFilt.c.setBounds (R (1362, 925, 180, 50));
    folder.setBounds (R (660, 160, 90, 85)); save.setBounds (R (1430, 160, 80, 85));
    prev.setBounds (R (745, 160, 55, 85)); next.setBounds (R (1380, 160, 55, 85));
    presetLabel.setBounds (R (820, 165, 560, 80));
    expansion.setBounds (R (196, 58, 398, 344));
    keys.setBounds (R (518, 1090, 1277, 325));
}
