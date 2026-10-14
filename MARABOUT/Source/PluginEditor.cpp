#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Mb;

namespace
{
const juce::Colour amber { 0xffffd36a }, ember { 0xffb44bff }, dark { 0xff0f0a14 };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }
}

MbLNF::MbLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffffe0a0));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffe0a0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2060));
}

void MbLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void MbLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void MbLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffffe0a0)); g.fillPath (t);
}

void MbLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void MbLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff3a2060) : over ? juce::Colour (0xff2a1844) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
MbPanel::MbPanel (MbProcessor& p)
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
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "ENV"); addKnob (resoK, P::reso, "RES");
    const char* tn[5] = { "A", "D", "S", "R", "VELOCITY" };
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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffe0a0));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setKeyWidth (27.7f);
    keys.setLowestVisibleKey (36);
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

MbPanel::~MbPanel() { setLookAndFeel (nullptr); }

void MbPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void MbPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void MbPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void MbPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File MbPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("MARABOUT").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void MbPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.mbpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(aucun preset utilisateur)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Ouvrir un fichier .mbpreset...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Charger un preset", userPresetDir(), "*.mbpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void MbPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Sauvegarder le preset", userPresetDir(), "*.mbpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("mbpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void MbPanel::loadUserPreset (const juce::File& f)
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

void MbPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Les packs d'expansion arrivent bientot.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void MbPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff17122b), 0, 0, juce::Colour (0xff09070f), 0, (float) getHeight(), false));
    g.fillAll();
    // plaque centrale violette brossee
    auto plate = R (165, 85, 1675, 1320).toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2b2150), plate.getX(), plate.getY(), juce::Colour (0xff1a1436), plate.getX(), plate.getBottom(), false));
    g.fillRoundedRectangle (plate, 4.f);
    for (int side = 0; side < 2; ++side)   // rails metal
    {
        auto r = side == 0 ? R (50, 65, 115, 1355) : R (1840, 65, 110, 1355);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffa4a4aa), (float) r.getX(), 0, juce::Colour (0xff3d3d46), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 6.f);
        g.setColour (juce::Colour (0xff2a1d44)); g.fillRoundedRectangle (r.reduced (28, 140).toFloat(), 10.f);
        g.setColour (ember.withAlpha (0.8f)); g.fillRect (side == 0 ? r.getRight() - 3 : r.getX(), r.getY(), 3, r.getHeight());
        for (int y : { 150, 1345 }) { g.setColour (juce::Colour (0xff1d1d22)); g.fillEllipse ((float) r.getCentreX() - 14, y * K - 14, 28.f, 28.f); g.setColour (juce::Colour (0xff8a8a92)); g.fillEllipse ((float) r.getCentreX() - 9, y * K - 9, 18.f, 18.f); }
    }
    // grilles : 4 fentes verticales de chaque cote (2 eclairees)
    for (int side = 0; side < 2; ++side)
        for (int k = 0; k < 4; ++k)
        {
            const int x = (side == 0 ? 645 : 1245) + k * 37;
            auto slot = R (x - 9, 265, 18, 260).toFloat();
            g.setColour (juce::Colour (0xff0b0816)); g.fillRoundedRectangle (slot, 7.f);
            const bool lit = (k == 1 || k == 3);
            for (float yy = slot.getY() + 6; yy < slot.getBottom() - 6; yy += 6.5f)
            { g.setColour (lit ? amber.withAlpha (0.55f) : juce::Colour (0xff3a3450)); g.fillEllipse (slot.getCentreX() - 2.5f, yy, 5.f, 5.f); }
        }
    g.drawImage (camel, R (818, 250, 364, 362).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colours::white);
    g.drawImage (logo, R (1540, 230, 156, 152).toFloat(), juce::RectanglePlacement::centred);
    {   // DJBILBOXBEATS
        juce::AttributedString as; as.setJustification (juce::Justification::centred);
        const auto fnt = juce::FontOptions (23.f, juce::Font::bold);
        as.append ("DJ", fnt, juce::Colour (0xffcdb4ff)); as.append ("BILBOX", fnt, juce::Colour (0xffffd9a8)); as.append ("BEATS", fnt, juce::Colour (0xffb890ff));
        as.draw (g, R (1425, 130, 390, 70).toFloat());
    }
    // titre neon
    const auto titleR = R (700, 618, 600, 110);
    for (float a : { 0.10f, 0.14f, 0.2f })
    { g.setColour (ember.withAlpha (a)); g.setFont (juce::FontOptions (66.f + (0.3f - a) * 20.f, juce::Font::bold)); g.drawFittedText ("MARABOUT", titleR, juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xfff2e6ff)); g.setFont (juce::FontOptions (62.f, juce::Font::bold));
    g.drawFittedText ("MARABOUT", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffb8b0c8)); g.setFont (juce::FontOptions (20.f));
    g.drawFittedText ("By Humpire", R (880, 722, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (610, 125, 780, 93).toFloat();
    g.setColour (amber.withAlpha (0.2f)); g.fillRoundedRectangle (scr.expanded (3.f), 12.f);
    g.setColour (juce::Colour (0xff120c22)); g.fillRoundedRectangle (scr, 10.f);
    g.setColour (amber.withAlpha (0.85f)); g.drawRoundedRectangle (scr, 10.f, 1.6f);
    g.setColour (juce::Colour (0xff1c1236)); g.fillRoundedRectangle (R (705, 142, 590, 60).toFloat(), 6.f);
    {   // icones : dossier, chevrons, disquette
        g.setColour (amber); const auto f = R (632, 148, 52, 44).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 6, f.getWidth(), f.getHeight() - 6, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 9.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1318, 146, 46, 48).toFloat(); g.setColour (ember);
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (amber);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? R (728, 0, 0, 0).getX() * 1.f : R (1268, 0, 0, 0).getX() * 1.f, cy = (float) R (0, 172, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 11.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 11.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette (verticale)
    g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
    g.drawImage (cassette, R (236, 122, 272, 402).toFloat(), juce::RectanglePlacement::centred);
    g.setColour (juce::Colour (0xffcfc8dc)); g.setFont (juce::FontOptions (15.f));
    g.drawFittedText ("EXPANSION", R (250, 532, 240, 36), juce::Justification::centred, 1);
    // plaque des molettes
    {
        auto pl = R (215, 1052, 315, 308).toFloat();
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff9a9aa2), pl.getX(), pl.getY(), juce::Colour (0xff4a4a52), pl.getRight(), pl.getBottom(), false));
        g.fillRoundedRectangle (pl, 8.f);
        g.setColour (juce::Colour (0xff1d1d22)); g.drawRoundedRectangle (pl, 8.f, 1.5f);
        for (auto p : { pl.getTopLeft() + juce::Point<float> (10, 10), pl.getTopRight() + juce::Point<float> (-10, 10), pl.getBottomLeft() + juce::Point<float> (10, -10), pl.getBottomRight() + juce::Point<float> (-10, -10) })
        { g.setColour (juce::Colour (0xff2b2b30)); g.fillEllipse (p.x - 6, p.y - 6, 12, 12); }
    }
    // labels
    g.setColour (juce::Colour (0xffcfc8dc));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 11.f : 12.5f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    g.setFont (juce::FontOptions (11.f));
    g.drawFittedText ("FILTER ENVELOPE + VELOCITY", R (1320, 540, 360, 26), juce::Justification::centred, 1);
    g.setColour (amber.withAlpha (0.8f)); g.setFont (juce::FontOptions (22.f, juce::Font::bold));
    g.drawFittedText ("FX", R (1325, 800, 80, 50), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xff17122b)); g.setFont (juce::FontOptions (13.f));
    g.drawFittedText ("Pitch Bend", R (225, 1310, 160, 36), juce::Justification::centred, 1);
    g.drawFittedText ("Modulation", R (360, 1310, 160, 36), juce::Justification::centred, 1);
}

void MbPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 278, 632, 96);  place (amount, 431, 632, 96); place (pitch, 584, 632, 96);
    place (glide, 898, 840, 100); place (filter, 1101, 840, 100);
    place (tone, 690, 790, 64);  place (body, 770, 790, 64); place (bright, 690, 878, 64); place (width, 770, 878, 64);
    place (resoK, 1190, 790, 64); place (fenvK, 1262, 790, 64); place (texture, 1226, 878, 64);
    place (volume, 1725, 632, 96);
    place (reverb, 1477, 760, 80); place (chorus, 1601, 760, 80); place (dist, 1725, 760, 80);
    place (delay, 1477, 873, 80);  place (dtime, 1601, 873, 80); place (feedback, 1725, 873, 80);
    const int ex[4] = { 278, 380, 483, 585 }, tx[5] = { 1361, 1418, 1474, 1531, 1604 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 30, 735, 60, 155));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 570, 56, 105));
    bend.s.setBounds (C (305, 1190, 90, 230)); modw.s.setBounds (C (437, 1190, 90, 230));
    cWave.c.setBounds (R (222, 960, 295, 55)); cLfo.c.setBounds (R (538, 960, 295, 55)); cDest.c.setBounds (R (853, 960, 295, 55));
    cFilt.c.setBounds (R (1169, 960, 295, 55)); cMono.c.setBounds (R (1485, 960, 295, 55));
    folder.setBounds (R (625, 135, 75, 75)); save.setBounds (R (1305, 135, 75, 75));
    prev.setBounds (R (695, 135, 70, 75)); next.setBounds (R (1235, 135, 70, 75));
    presetLabel.setBounds (R (770, 142, 460, 60));
    expansion.setBounds (R (236, 122, 272, 402));
    keys.setBounds (R (570, 1068, 1220, 317));
}
