#include "PluginEditor.h"
#include "BinaryData.h"

using namespace Pr;

namespace
{
const juce::Colour amber { 0xff9ff0ff }, ember { 0xff4ab8ff }, dark { 0xff0b1218 };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }
}

PrLNF::PrLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffd8f7ff));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffd8f7ff));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff1d4a66));
}

void PrLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
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

void PrLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
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

void PrLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.5f);
    g.setColour (amber.withAlpha (0.18f)); g.fillRoundedRectangle (r.expanded (1.5f), 9.f);
    g.setColour (dark); g.fillRoundedRectangle (r, 8.f);
    g.setGradientFill (juce::ColourGradient (amber, r.getX(), 0, ember, r.getRight(), 0, false));
    g.drawRoundedRectangle (r, 8.f, 1.7f);
    juce::Path t; const float cx = (float) w - 18.f, cy = (float) h * 0.5f;
    t.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (juce::Colour (0xffd8f7ff)); g.fillPath (t);
}

void PrLNF::positionComboBoxText (juce::ComboBox& b, juce::Label& l)
{
    l.setBounds (10, 1, b.getWidth() - 34, b.getHeight() - 2);
    l.setFont (getComboBoxFont (b));
    l.setJustificationType (juce::Justification::centredLeft);
}

void PrLNF::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    if (b.getProperties()["flat"]) return;   // bouton invisible (cassette)
    const auto r = b.getLocalBounds().toFloat().reduced (1.f);
    g.setColour (down ? juce::Colour (0xff1d4a66) : over ? juce::Colour (0xff153448) : dark.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.2f);
}

//==============================================================================
PrPanel::PrPanel (PrProcessor& p)
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
    addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DELAY");
    addKnob (dtime, P::dtime, "TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "ENV"); addKnob (resoK, P::reso, "RES");
    const char* tn[4] = { "", "", "", "" };
    const int ti[4] = { P::fatk, P::fdec, P::frel, P::velocity };
    for (int i = 0; i < 4; ++i) addKnob (rsl[i], ti[i], tn[i], juce::Slider::LinearVertical);
    addKnob (volume, P::volume, "VOLUME");
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
    presetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd8f7ff));
    presetLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetLabel);

    keys.setKeyWidth (33.5f);
    keys.setLowestVisibleKey (36);
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
}

PrPanel::~PrPanel() { setLookAndFeel (nullptr); }

void PrPanel::addKnob (Knob& k, int idx, const juce::String& label, juce::Slider::SliderStyle style)
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

void PrPanel::addCombo (Combo& c, int idx, const juce::StringArray& items)
{
    c.c.addItemList (items, 1);
    c.a = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, ids[idx], c.c);
    addAndMakeVisible (c.c);
}

void PrPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); }

void PrPanel::stepPreset (int d)
{
    const int n = proc.getNumPrograms();
    const int base = proc.presetIndex < 0 ? (d > 0 ? -1 : 0) : proc.presetIndex;
    proc.applyPreset (((base + d) % n + n) % n);
    refreshPresetLabel();
}

juce::File PrPanel::userPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("DJBILBOX BEATS").getChildFile ("PHANTOM_RIDER").getChildFile ("Presets");
    d.createDirectory();
    return d;
}

void PrPanel::showPresetMenu()
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
    auto files = userPresetDir().findChildFiles (juce::File::findFiles, false, "*.prpreset"); files.sort();
    juce::PopupMenu user;
    for (int i = 0; i < files.size(); ++i) user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    if (files.isEmpty()) user.addItem (-1, "(aucun preset utilisateur)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Ouvrir un fichier .prpreset...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Charger un preset", userPresetDir(), "*.prpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void PrPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Sauvegarder le preset", userPresetDir(), "*.prpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult(); if (f == juce::File()) return;
        if (auto xml = proc.apvts.copyState().createXml()) xml->writeTo (f.withFileExtension ("prpreset"));
        proc.presetName = f.getFileNameWithoutExtension(); refreshPresetLabel();
    });
}

void PrPanel::loadUserPreset (const juce::File& f)
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

void PrPanel::showExpansion()
{
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "EXPANSION VOL.1",
        "Les packs d'expansion arrivent bientot.\n\nhttps://djbilboxbeats.gumroad.com/");
}

void PrPanel::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5d6a76), 0, 0, juce::Colour (0xff343c45), 0, (float) getHeight(), false));
    g.fillAll();
    juce::Random cl (7);   // texture de metal patine
    for (int i = 0; i < 160; ++i)
    {
        const float x = cl.nextFloat() * (float) getWidth(), y = cl.nextFloat() * (float) getHeight(), r = 15.f + cl.nextFloat() * 50.f;
        g.setColour (juce::Colours::black.withAlpha (0.025f)); g.fillEllipse (x - r, y - r, r * 2, r * 2);
    }
    for (int side = 0; side < 2; ++side)   // rails acier
    {
        auto r = side == 0 ? R (0, 0, 148, 1493) : R (1855, 0, 145, 1493);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb4b8be), (float) r.getX(), 0, juce::Colour (0xff4a4e56), (float) r.getRight(), 0, false));
        g.fillRect (r);
        g.setColour (juce::Colour (0xff23282f)); g.fillRoundedRectangle (r.reduced (38, 130).toFloat(), 10.f);
        g.setColour (juce::Colour (0xff3a424c)); g.fillRoundedRectangle (r.reduced (46, 138).toFloat(), 8.f);
        for (int y : { 62, 1425 }) { g.setColour (juce::Colour (0xff202228)); g.fillEllipse ((float) r.getCentreX() - 16, y * K - 16, 32.f, 32.f); g.setColour (juce::Colour (0xff9ea2aa)); g.fillEllipse ((float) r.getCentreX() - 11, y * K - 11, 22.f, 22.f); }
    }
    // grilles a pastilles noires
    for (auto r : { R (656, 248, 106, 368), R (1240, 248, 106, 368) })
        for (int cx = 0; cx < 4; ++cx)
            for (int cy = 0; cy < 17; ++cy)
            {
                const float x = (float) r.getX() + 4.f + cx * 11.5f, y = (float) r.getY() + 3.f + cy * 10.5f;
                g.setColour (juce::Colour (0xff0b0e12)); g.fillEllipse (x, y, 8.f, 8.f);
            }
    g.drawImage (camel, R (797, 252, 406, 406).toFloat(), juce::RectanglePlacement::centred);
    {   // logo DB
        auto lr = R (1698, 200, 104, 102).toFloat();
        g.setColour (amber.withAlpha (0.35f)); g.fillRoundedRectangle (lr.expanded (3.f), 8.f);
        g.setColour (juce::Colours::white); g.drawImage (logo, lr, juce::RectanglePlacement::centred);
    }
    for (float a : { 0.12f, 0.2f })
    { g.setColour (amber.withAlpha (a)); g.setFont (juce::FontOptions (29.f + (0.3f - a) * 6.f, juce::Font::bold)); g.drawFittedText ("DJBILBOXBEATS", R (1420, 92, 400, 70), juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xffd8f7ff)); g.setFont (juce::FontOptions (28.f, juce::Font::bold));
    g.drawFittedText ("DJBILBOXBEATS", R (1420, 92, 400, 70), juce::Justification::centred, 1);
    // titre fantome
    const auto titleR = R (700, 680, 600, 130);
    for (float a : { 0.10f, 0.16f, 0.25f })
    { g.setColour (amber.withAlpha (a)); g.setFont (juce::FontOptions (54.f + (0.3f - a) * 16.f, juce::Font::bold)); g.drawFittedText ("PHANTOM RIDER", titleR, juce::Justification::centred, 1); }
    g.setColour (juce::Colour (0xffeafcff)); g.setFont (juce::FontOptions (52.f, juce::Font::bold));
    g.drawFittedText ("PHANTOM RIDER", titleR, juce::Justification::centred, 1);
    g.setColour (juce::Colour (0xffd0dae4)); g.setFont (juce::FontOptions (19.f));
    g.drawFittedText ("By Humpire", R (880, 792, 240, 44), juce::Justification::centred, 1);
    // ecran preset
    auto scr = R (620, 75, 760, 113).toFloat();
    g.setColour (amber.withAlpha (0.35f)); g.fillRoundedRectangle (scr.expanded (4.f), 14.f);
    g.setColour (juce::Colour (0xff3a424c)); g.fillRoundedRectangle (scr, 12.f);
    g.setColour (juce::Colour (0xff0c1218)); g.fillRoundedRectangle (scr.reduced (7.f), 8.f);
    g.setColour (juce::Colour (0xff131e28)); g.fillRoundedRectangle (R (722, 92, 556, 82).toFloat(), 4.f);
    {   // icones
        g.setColour (juce::Colour (0xffa8b4c0)); const auto f = R (655, 110, 48, 40).toFloat();
        juce::Path fo; fo.addRoundedRectangle (f.getX(), f.getY() + 5, f.getWidth(), f.getHeight() - 5, 3.f); fo.addRoundedRectangle (f.getX(), f.getY(), f.getWidth() * 0.45f, 8.f, 2.f);
        g.strokePath (fo, juce::PathStrokeType (1.6f));
        const auto d = R (1302, 110, 42, 44).toFloat();
        g.drawRoundedRectangle (d, 3.f, 1.8f); g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY(), d.getWidth() * 0.55f, d.getHeight() * 0.35f, 1.5f);
        g.drawRect (d.getX() + d.getWidth() * 0.2f, d.getY() + d.getHeight() * 0.58f, d.getWidth() * 0.6f, d.getHeight() * 0.42f, 1.5f);
        g.setColour (amber);
        for (int sgn : { -1, 1 })
        { const float cx = sgn < 0 ? (float) R (752, 0, 0, 0).getX() : (float) R (1247, 0, 0, 0).getX(), cy = (float) R (0, 133, 0, 0).getY();
          juce::Path c; c.startNewSubPath (cx - sgn * 5.f, cy - 11.f); c.lineTo (cx + sgn * 5.f, cy); c.lineTo (cx - sgn * 5.f, cy + 11.f); g.strokePath (c, juce::PathStrokeType (2.2f)); }
    }
    // cassette
    {
        auto fr = R (213, 90, 307, 435).toFloat();
        g.setColour (amber.withAlpha (0.4f)); g.fillRoundedRectangle (fr.expanded (3.f), 14.f);
        g.setColour (juce::Colour (0xff2a323b)); g.fillRoundedRectangle (fr, 12.f);
        g.setColour (juce::Colours::white);   // drawImage applique l'alpha de la couleur courante
        g.drawImage (cassette, R (226, 102, 282, 412).toFloat(), juce::RectanglePlacement::centred);
        g.saveState(); g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi, (float) R (187, 308, 0, 0).getX(), (float) R (187, 308, 0, 0).getY()));
        g.setColour (juce::Colour (0xff141b22)); g.setFont (juce::FontOptions (13.f));
        g.drawFittedText ("EXPANSION", juce::Rectangle<int> ((int) (187 * K) - 60, (int) (308 * K) - 12, 120, 24), juce::Justification::centred, 1);
        g.restoreState();
    }
    // plaque des molettes
    g.setColour (juce::Colour (0xff10161c)); g.fillRoundedRectangle (R (183, 1135, 295, 340).toFloat(), 10.f);
    g.setColour (juce::Colour (0xff4a525c)); g.drawRoundedRectangle (R (183, 1135, 295, 340).toFloat(), 10.f, 2.f);
    // labels
    g.setColour (juce::Colour (0xff141b22));
    for (auto* k : all)
    {
        const auto r = k->s.getBounds();
        g.setFont (juce::FontOptions (k->s.getProperties()["small"] ? 10.f : (k->lin ? 11.f : 12.5f), juce::Font::bold));
        g.drawFittedText (k->label, r.getX() - 25, r.getBottom() + (k->lin ? 2 : 0), r.getWidth() + 50, 18, juce::Justification::centred, 1);
    }
    g.setFont (juce::FontOptions (14.f, juce::Font::bold));
    g.drawFittedText ("LFO", R (352, 562, 120, 40), juce::Justification::centred, 1);
    g.setFont (juce::FontOptions (11.f, juce::Font::bold));
    g.drawFittedText ("FILTER ENV A / D / R + VELOCITY", R (1370, 560, 400, 30), juce::Justification::centred, 1);
}

void PrPanel::resized()
{
    auto place = [] (Knob& k, int cx, int cy, int sz) { k.s.setBounds (C (cx, cy, sz, sz)); };
    place (rate, 262, 665, 104);  place (amount, 412, 665, 104); place (pitch, 562, 665, 104);
    place (glide, 846, 895, 104); place (filter, 1153, 895, 104);
    place (tone, 685, 855, 56);  place (body, 755, 855, 56); place (bright, 685, 940, 56); place (width, 755, 940, 56);
    place (texture, 1240, 850, 56); place (resoK, 1315, 850, 56); place (fenvK, 1277, 935, 56);
    place (volume, 1747, 455, 104);
    place (reverb, 1436, 697, 104); place (chorus, 1587, 697, 104); place (dist, 1738, 697, 104);
    place (delay, 1436, 895, 104);  place (dtime, 1587, 895, 104); place (feedback, 1738, 895, 104);
    const int ex[4] = { 257, 362, 466, 571 }, tx[4] = { 1441, 1509, 1576, 1643 };
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (R (ex[i] - 30, 782, 60, 178));
    for (int i = 0; i < 4; ++i) rsl[i].s.setBounds (R (tx[i] - 28, 342, 56, 192));
    bend.s.setBounds (C (262, 1280, 90, 260)); modw.s.setBounds (C (389, 1280, 90, 260));
    cWave.c.setBounds (R (187, 1035, 298, 62)); cLfo.c.setBounds (R (519, 1035, 298, 62)); cDest.c.setBounds (R (851, 1035, 298, 62));
    cFilt.c.setBounds (R (1183, 1035, 298, 62)); cMono.c.setBounds (R (1515, 1035, 298, 62));
    folder.setBounds (R (634, 90, 104, 86)); save.setBounds (R (1282, 90, 90, 86));
    prev.setBounds (R (720, 90, 64, 86)); next.setBounds (R (1215, 90, 64, 86));
    presetLabel.setBounds (R (790, 92, 420, 82));
    expansion.setBounds (R (213, 90, 307, 435));
    keys.setBounds (R (517, 1137, 1288, 340));
}
