#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Cp;

namespace
{
const juce::Colour amber { 0xffff4df5 }, ember { 0xffffe14a }, dark { 0xff0d0a12 };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }
}

CpLNF::CpLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffffc2fb));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffffc2fb));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff4a1d52));
}

void CpLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void CpLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void CpLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffffc2fb)); g.fillPath (t);
}

void CpLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void CpLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff4a1d52) : over ? juce::Colour (0xff321338) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
CpPanel::CpPanel (CpProcessor& p)
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
    const char* tn[5] = { "", "", "", "", "" };
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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffffc2fb));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setKeyWidth (21.4f);
    keys.setLowestVisibleKey (36);
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

CpPanel::~CpPanel() { setLookAndFeel (nullptr); }

void CpPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void CpPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void CpPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void CpPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File CpPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("CYBERPUNK_LEAD").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void CpPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.cppreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(aucun preset utilisateur)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Ouvrir un fichier .cppreset...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Charger un preset", userPresetDir(), "*.cppreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void CpPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Sauvegarder le preset", userPresetDir(), "*.cppreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("cppreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void CpPanel::loadUserPreset (const juce::File& f)
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

void CpPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Les packs d'expansion arrivent bientot.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void CpPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff181519), 0, 0, juce::Colour (0xff09070b), 0, (float) getHeight(), false));
    g.fillAll();
    g.setColour (juce::Colour (0xff131116)); g.fillRoundedRectangle (R (175, 45, 1650, 1410).toFloat(), 4.f);
    for (int side = 0; side < 2; ++side)   // rails acier + poignee
    {
        auto r = side == 0 ? R (40, 35, 135, 1425) : R (1825, 35, 135, 1425);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8a8a92), (float) r.getX(), 0, juce::Colour (0xff2f2f36), (float) r.getRight(), 0, false));
        g.fillRoundedRectangle (r.toFloat(), 5.f);
        const auto hd = side == 0 ? R (88, 178, 62, 1110) : R (1850, 178, 62, 1110);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc4c4cc), (float) hd.getX(), 0, juce::Colour (0xff3a3a42), (float) hd.getRight(), 0, false));
        g.fillRoundedRectangle (hd.toFloat(), 14.f);
        g.setColour ((side == 0 ? amber : ember).withAlpha (0.85f)); g.fillRect (side == 0 ? r.getRight() - 3 : r.getX(), r.getY(), 3, r.getHeight());
        for (int y : { 100, 1395 }) { g.setColour (juce::Colour (0xff1a1a1f)); g.fillEllipse ((float) r.getCentreX() - 15, y * K - 15, 30.f, 30.f); g.setColour (juce::Colour (0xff9a9aa2)); g.fillEllipse ((float) r.getCentreX() - 10, y * K - 10, 20.f, 20.f); }
    }
    // haut-parleurs a pastilles roses
    for (auto r : { R (660, 235, 135, 330), R (1207, 235, 135, 330) })
    {
        g.setColour (juce::Colour (0xff070509)); g.fillRoundedRectangle (r.toFloat(), 8.f);
        for (int cx = 0; cx < 6; ++cx)
            for (int cy = 0; cy < 14; ++cy)
            {
                const float x = (float) r.getX() + 6.f + cx * 11.f, y = (float) r.getY() + 6.f + cy * 11.5f;
                g.setColour (amber.withAlpha (0.55f)); g.fillEllipse (x, y, 4.f, 4.f);
            }
    }
    g.drawImage (camel, R (824, 222, 352, 352).toFloat(), juce::RectanglePlacement::centred);
    {   // logo DB
        auto lr = R (1556, 176, 88, 86).toFloat();
        g.setGradientFill (juce::ColourGradient (amber, lr.getX(), lr.getY(), ember, lr.getRight(), lr.getBottom(), false)); g.fillRoundedRectangle (lr.expanded (2.f), 6.f);
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff0f0f4), 0, 50.f, juce::Colour (0xff8e8e98), 0, 80.f, false));
    g.setFont (juce::FontOptions (28.f, juce::Font::bold));
    g.drawFittedText ("DJBILBOXBEATS", R (1400, 95, 400, 70), juce::Justification::centred, 1);
    // titre glitch jaune
    const auto titleR = R (690, 575, 620, 110);
    g.setFont (juce::FontOptions (56.f, juce::Font::bold | juce::Font::italic));
    g.setColour (juce::Colour (0xff00e5ff).withAlpha (0.8f)); g.drawFittedText ("Cyberpunk Lead", titleR.translated (-2, 0), juce::Justification::centred, 1);
    g.setColour (amber.withAlpha (0.8f)); g.drawFittedText ("Cyberpunk Lead", titleR.translated (2, 1), juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xfffff25a)); g.drawFittedText ("Cyberpunk Lead", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffb8b0c0)); g.setFont (juce::FontOptions (19.f));
    g.drawFittedText ("By Humpire", R (1090, 690, 200, 40), juce::Justification::centredRight, 1);
    // ecran preset
    auto scr = R (635, 95, 730, 85).toFloat();
    g.setGradientFill (juce::ColourGradient (amber, scr.getX(), 0, ember, scr.getRight(), 0, false)); g.fillRoundedRectangle (scr.expanded (2.f), 12.f);
    g.setColour (juce::Colour (0xff0e0a12)); g.fillRoundedRectangle (scr.reduced (2.f), 11.f);
    g.setColour (juce::Colour (0xff1a1020)); g.fillRoundedRectangle (R (742, 108, 520, 58).toFloat(), 5.f);
    {   // icones
        g.setColour (juce::Colour (0xff9a96a4)); const auto f = R (664, 118, 40, 34).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1304, 118, 38, 38).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (ember);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (735, 0, 0, 0).getX() : (float) R (1265, 0, 0, 0).getX(), cy = (float) R (0, 137, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 11.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 11.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    {
        auto fr = R (272, 94, 285, 352).toFloat();
        g.setGradientFill (juce::ColourGradient (amber, fr.getX(), fr.getY(), ember, fr.getRight(), fr.getBottom(), false)); g.fillRoundedRectangle (fr, 12.f);
        g.setColour (juce::Colour (0xff0c0a0e)); g.fillRoundedRectangle (fr.reduced (2.5f), 10.f);
        g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
        g.drawImage (cassette, R (282, 102, 270, 338).toFloat(), juce::RectanglePlacement::centred);
        g.saveState(); g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi, (float) R (245, 265, 0, 0).getX(), (float) R (245, 265, 0, 0).getY()));
        g.setColour (juce::Colour (0xffcfc8d8)); g.setFont (juce::FontOptions (13.f));
        g.drawFittedText ("EXPANSION", juce::Rectangle<int> ((int) (245 * K) - 60, (int) (265 * K) - 12, 120, 24), juce::Justification::centred, 1);
        g.restoreState();
    }
    // plaque des molettes
    g.setColour (juce::Colour (0xff060508)); g.fillRoundedRectangle (R (228, 1052, 370, 353).toFloat(), 12.f);
    g.setColour (juce::Colour (0xff3a3640)); g.drawRoundedRectangle (R (228, 1052, 370, 353).toFloat(), 12.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xffc8c2d2));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 11.f : 12.5f)));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    g.setFont (juce::FontOptions (14.f));
    g.drawFittedText ("LFO", R (367, 455, 120, 40), juce::Justification::centred, 1);
    g.setFont (juce::FontOptions (12.5f));
    g.drawFittedText ("ASDR+VELOCITY", R (1470, 545, 220, 30), juce::Justification::centred, 1);
    g.drawFittedText ("PITCH BEND", R (255, 1350, 160, 34), juce::Justification::centred, 1);
    g.drawFittedText ("MODULATION", R (412, 1350, 160, 34), juce::Justification::centred, 1);
}

void CpPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 284, 548, 100);  place (amount, 426, 548, 100); place (pitch, 568, 548, 100);
    place (glide, 913, 808, 100); place (filter, 1085, 808, 100);
    place (tone, 722, 790, 56);  place (body, 812, 790, 56); place (bright, 722, 872, 56); place (width, 812, 872, 56);
    place (texture, 1165, 770, 56); place (resoK, 1245, 770, 56); place (fenvK, 1325, 770, 56);
    place (volume, 1416, 658, 100); place (reverb, 1563, 658, 100); place (chorus, 1709, 658, 100);
    place (dist, 1416, 832, 100); place (dtime, 1563, 832, 100); place (feedback, 1709, 832, 100);
    place (delay, 1245, 862, 76);
    const int ex[4] = { 293, 428, 537, 652 }, tx[5] = { 1430, 1505, 1580, 1656, 1735 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 30, 665, 60, 215));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 315, 56, 225));
    bend.s.setBounds (C (333, 1220, 90, 250)); modw.s.setBounds (C (490, 1220, 90, 250));
    cWave.c.setBounds (R (232, 950, 265, 55)); cLfo.c.setBounds (R (549, 950, 266, 55)); cDest.c.setBounds (R (866, 950, 266, 55));
    cFilt.c.setBounds (R (1183, 950, 266, 55)); cMono.c.setBounds (R (1500, 950, 266, 55));
    folder.setBounds (R (650, 100, 80, 75)); save.setBounds (R (1285, 100, 70, 75));
    prev.setBounds (R (715, 100, 40, 75)); next.setBounds (R (1245, 100, 40, 75));
    presetLabel.setBounds (R (742, 108, 520, 58));
    expansion.setBounds (R (272, 94, 285, 352));
    keys.setBounds (R (650, 1055, 1115, 370));
}
