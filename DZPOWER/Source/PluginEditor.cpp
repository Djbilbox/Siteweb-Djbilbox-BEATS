#include "PluginEditor.h"
#include "BinaryData.h"
#include "FaceplateLayout.h"

using namespace Dz;

namespace
{
const juce::Colour amber { 0xffff5a4a }, ember { 0xff3fe06e }, dark { 0xff14160f };
constexpr float K = 0.5f;   // coordonnees maquette (2000 px) -> 1000 px
juce::Rectangle<int> R (int x, int y, int w, int h) { return { (int) (x * K), (int) (y * K), (int) (w * K), (int) (h * K) }; }
juce::Rectangle<int> C (int cx, int cy, int w, int h) { return R (cx - w / 2, cy - h / 2, w, h); }
juce::Colour ramp (float t) { return amber.interpolatedWith (ember, juce::jlimit (0.f, 1.f, t)); }

// Section header: text centred on (cx, cy) in mock-up units, thin rules running out to +/-half on both sides of the text.
void header (juce::Graphics& g, const juce::String& text, int cx, int cy, int half, juce::Colour col, float size = 13.f, bool rules = true)
{
    const juce::Font font (juce::FontOptions (size, juce::Font::bold));
    juce::GlyphArrangement ga; ga.addLineOfText (font, text, 0.f, 0.f);
    const float tw = ga.getBoundingBox (0, -1, true).getWidth();
    const float x = (float) cx * K, y = (float) cy * K, h = (float) half * K;
    g.setFont (font);
    g.setColour (col);
    g.drawText (text, juce::Rectangle<float> (x - tw * 0.5f - 6.f, y - 10.f, tw + 12.f, 20.f), juce::Justification::centred, false);
    if (rules && h > tw * 0.5f + 14.f)
    {
        g.setColour (col.withAlpha (0.45f));
        g.drawLine (x - h, y, x - tw * 0.5f - 8.f, y, 1.f);
        g.drawLine (x + tw * 0.5f + 8.f, y, x + h, y, 1.f);
    }
}

// Small caption centred on (cx, cy) in mock-up units.
void caption (juce::Graphics& g, const juce::String& text, int cx, int cy, juce::Colour col, float size = 10.f)
{
    g.setColour (col);
    g.setFont (juce::FontOptions (size, juce::Font::bold));
    g.drawText (text, juce::Rectangle<float> ((float) cx * K - 60.f, (float) cy * K - 8.f, 120.f, 16.f), juce::Justification::centred, false);
}
}

DzLNF::DzLNF()
{
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffeee6d0));
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffeee6d0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2a1d));
    faderImg = juce::ImageFileFormat::loadFrom (BinaryData::fader_png, BinaryData::fader_pngSize);
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
    const float glow = (float) s.getProperties().getWithDefault ("glow", 0.f);
    g.setColour (col.withAlpha (0.3f)); g.strokePath (val, juce::PathStrokeType (8.f));
    g.setColour (col); g.strokePath (val, juce::PathStrokeType (3.5f));
    if (glow > 0.45f)   // lit: a white-hot core along the arc (the halo itself is drawn by the panel, behind)
    { g.setColour (col.interpolatedWith (juce::Colours::white, 0.7f).withAlpha (juce::jmin (1.f, (glow - 0.45f) * 1.2f))); g.strokePath (val, juce::PathStrokeType (1.4f)); }
    // the knob cap is part of the faceplate photo; here only the lit index on it, turning with the value
    const float capR = r * 0.79f;
    const juce::Point<float> dir (std::sin (ang), -std::cos (ang));
    juce::Path idx; idx.startNewSubPath (ctr + dir * (capR * 0.3f)); idx.lineTo (ctr + dir * (capR * 0.86f));
    hfx::neonStroke (g, idx, col, r > 15.f ? 2.f : 1.5f, 0.6f + 0.6f * glow);
}

void DzLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const bool wheel = s.getProperties()["wheel"];
    const float cx = (float) x + (float) w * 0.5f;
    if (wheel)
    {
        const float th = 52.f;   // the slot is part of the faceplate: only the cap here
        auto cap = juce::Rectangle<float> (cx - 20.f, pos - th * 0.5f, 40.f, th);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5d5d5d), cap.getX(), 0, juce::Colour (0xff222222), cap.getRight(), 0, false));
        g.fillRoundedRectangle (cap, 8.f);
        g.setColour (juce::Colours::black); for (int i = 1; i < 7; ++i) g.drawHorizontalLine ((int) (cap.getY() + i * th / 7.f), cap.getX() + 4, cap.getRight() - 4);
        return;
    }
    // the slot and its LED dots are part of the faceplate (the panel lights the dots); here a thin lit strip inside
    // the slot below the cap, and the cap
    const float t = 1.f - (pos - (float) y) / (float) h;
    g.setColour (ramp (1.f - t).withAlpha (0.8f)); g.fillRoundedRectangle (cx - 1.f, pos, 2.f, (float) y + (float) h - pos, 1.f);
    const float led = (float) s.getProperties().getWithDefault ("led", 0.f);
    // photo-real cap with a soft shadow under it; its centre groove lights up while its stage plays
    const float cw = (float) w, ch = cw * (float) faderImg.getHeight() / (float) juce::jmax (1, faderImg.getWidth());
    const auto cap = juce::Rectangle<float> (cx - cw * 0.5f, pos - ch * 0.5f, cw, ch);
    hfx::glowSpot (g, cap.getCentre().translated (0.f, 4.f), cw * 0.55f, juce::Colours::black, 0.75f);
    g.setOpacity (1.f);
    g.drawImage (faderImg, cap, juce::RectanglePlacement::stretchToFit);
    if (led > 0.f)
    {
        juce::Path groove; groove.startNewSubPath (cap.getX() + cw * 0.22f, cap.getCentreY()); groove.lineTo (cap.getRight() - cw * 0.22f, cap.getCentreY());
        hfx::neonStroke (g, groove, ramp (1.f - led), 1.2f, 0.5f + led);
    }
}

void DzLNF::drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&)
{
    // the box and its arrow are part of the faceplate: only the text (the combo's label) is drawn
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
    faceImg = load (BinaryData::faceplate_jpg, BinaryData::faceplate_jpgSize);
    const juce::Colour ledCols[3] { juce::Colour (0xff35ff6a), juce::Colour (0xffffc23a), juce::Colour (0xffff3b3b) };
    for (int i = 0; i < 3; ++i)
    {
        ledSprite[i] = juce::Image (juce::Image::ARGB, 15, 15, true);
        juce::Graphics gs (ledSprite[i]);
        hfx::glowSpot (gs, { 7.5f, 7.5f }, 7.f, ledCols[i], 0.35f);
        gs.setColour (ledCols[i].withAlpha (0.95f)); gs.fillEllipse (5.f, 5.f, 5.f, 5.f);
    }

    addKnob (rate, P::rate, "RATE");       addKnob (amount, P::amount, "AMOUNT");  addKnob (pitch, P::pitch, "TRANSPOSE");
    addKnob (glide, P::glide, "GLIDE");    addKnob (filter, P::filter, "CUTOFF");
    addKnob (tone, P::t1, "TONE");       addKnob (body, P::t2, "BODY");       addKnob (bright, P::t3, "BRIGHT");
    addKnob (width, P::t4, "WIDTH");       addKnob (texture, P::t5, "TEXTURE");
    addKnob (volume, P::volume, "VOLUME"); addKnob (reverb, P::reverb, "REVERB");  addKnob (chorus, P::chorus, "CHORUS");
    addKnob (dist, P::dist, "DISTORTION"); addKnob (delay, P::delay, "DELAY MIX");
    addKnob (dtime, P::dtime, "TIME");     addKnob (feedback, P::fb, "FEEDBACK");
    const char* en[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    for (int i = 0; i < 4; ++i) addKnob (env[i], P::atk + i, en[i], juce::Slider::LinearVertical);
    addKnob (fenvK, P::fenv, "FLT ENV"); addKnob (resoK, P::reso, "RESONANCE");
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

    // JUCE 8 resets the first visible key to the start of the available range when scroll buttons are hidden,
    // so the playable range itself is restricted: MIDI 36..96 = 5 octaves (C2..C7), 36 white keys (fitted in resized()).
    keys.setAvailableRange (36, 96);
    keys.setOctaveForMiddleC (4);   // labels: middle C (MIDI 60) = C4, first key = C2
    keys.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, amber.withAlpha (0.6f));
    keys.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ember.withAlpha (0.3f));
    keys.setScrollButtonsVisible (false);
    keys.neon = ember; keys.neonHot = amber;
    keys.setOverlayOnly();   // the keys are part of the faceplate photo
    addAndMakeVisible (keys);

    refreshPresetLabel();
    setSize (1000, 746);
    lastTick = juce::Time::getMillisecondCounterHiRes();
    if (animationsOn()) startTimerHz (45);
}

DzPanel::~DzPanel() { stopTimer(); setLookAndFeel (nullptr); }

float DzPanel::param (int index) const { return proc.apvts.getRawParameterValue (ids[index])->load(); }
bool DzPanel::animationsOn() const { return (bool) proc.apvts.state.getProperty ("animations", true); }

void DzPanel::setAnimations (bool on)
{
    proc.apvts.state.setProperty ("animations", on, nullptr);
    if (on) { lastTick = juce::Time::getMillisecondCounterHiRes(); startTimerHz (45); }
    else stopTimer();
    repaint();
}

void DzPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    juce::PopupMenu m;
    m.addItem (1, "Animations", true, animationsOn());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [this] (int r) { if (r == 1) setAnimations (! animationsOn()); });
}

void DzPanel::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    float dt = (float) juce::jlimit (0.0, 0.1, (now - lastTick) * 0.001);
    lastTick = now;
    // 45 frames per second while playing; quiet: 15 are plenty for slow breathing light
    if (! view.sounding && view.sinceHit > 2.f && (++skip % 3) != 0) { idleDt += dt; return; }
    dt += idleDt; idleDt = 0.f;
    advance (dt);
}

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

void DzPanel::refreshPresetLabel() { presetLabel.setText (proc.currentPresetName(), juce::dontSendNotification); presetFlash = 1.f; }

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
    if (files.isEmpty()) user.addItem (-1, "(no user presets)", false);
    menu.addSubMenu ("User", user);
    menu.addItem (999, "Open a .dzpreset file...");
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&folder), [this, files] (int r)
    {
        if (r >= 1000 && r - 1000 < files.size()) loadUserPreset (files[r - 1000]);
        else if (r == 999)
        {
            chooser = std::make_unique<juce::FileChooser> ("Load preset", userPresetDir(), "*.dzpreset");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) loadUserPreset (fc.getResult()); });
        }
        else if (r > 0 && r < 999) { proc.applyPreset (r - 1); refreshPresetLabel(); }
    });
}

void DzPanel::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> ("Save preset", userPresetDir(), "*.dzpreset");
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
        "Expansion packs are coming soon.\n\nhttps://djbilboxbeats.gumroad.com/");
}

namespace
{
// fixed places of the moving parts, in panel pixels
const juce::Point<float> medallion { 500.5f, 209.5f };
constexpr float medallionR = 92.f;
juce::Rectangle<float> grilles (int i) { return (i == 0 ? R (575, 305, 225, 205) : R (1200, 305, 230, 205)).toFloat(); }
// the LED dots beside fader i (0..3 amp envelope, 4..8 filter envelope), row 0 at the top
juce::Point<float> faderDot (int i, int side, int row)
{
    const bool amp = i < 4;
    const float top = amp ? Face::envLedTop : Face::rslLedTop, bottom = amp ? Face::envLedBottom : Face::rslLedBottom;
    return { Face::slotX[i] + (side == 0 ? -Face::ledDx : Face::ledDx), top + (bottom - top) * (float) row / (float) (Face::ledRows - 1) };
}
}

void DzPanel::paint (juce::Graphics& g)
{
    const float sc = g.getInternalContext().getPhysicalPixelScaleFactor();
    if (bgCache.isNull() || std::abs (sc - bgScale) > 0.01f)
    {
        bgScale = sc;
        bgCache = juce::Image (juce::Image::RGB, juce::jmax (1, juce::roundToInt ((float) getWidth() * sc)), juce::jmax (1, juce::roundToInt ((float) getHeight() * sc)), false);
        juce::Graphics gb (bgCache);
        gb.addTransform (juce::AffineTransform::scale (sc));
        paintStatic (gb);
    }
    {   // the cache already has the screen's pixel size: a plain copy, no smoothing (much cheaper)
        juce::Graphics::ScopedSaveState ss (g);
        g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
        g.drawImage (bgCache, getLocalBounds().toFloat());
    }
    paintLive (g);
}

void DzPanel::paintStatic (juce::Graphics& g)
{
    // the whole front panel is one photo-real render (Assets/faceplate.jpg): knob caps, slots, screens, texts, artwork
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (faceImg, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
}

//============================================================================== light and motion
void DzPanel::advance (float dt)
{
    view.update (proc.live, dt);
    const float lv = view.level, hit = view.hit;
    const auto gold = juce::Colour (0xffffd34a);

    // knob light: every arc glows a little, more with the sound; some follow what they control
    for (auto* k : all)
        if (! k->lin) { k->glow = 0.28f + 0.55f * lv; k->mod = -1.f; k->orbit = -1.f; }
    volume.glow = 0.3f + 1.0f * lv;
    reverb.glow = param (P::reverb) > 0.01f ? 0.3f + 0.9f * lv : 0.2f;
    chorus.glow = param (P::chorus) > 0.01f ? 0.3f + 0.9f * lv : 0.2f;
    dist.glow = param (P::dist) > 0.01f ? 0.3f + 1.1f * lv * (0.5f + param (P::dist)) : 0.2f;
    // LFO: a dot circles RATE at the real LFO speed, AMOUNT breathes with the LFO
    lfoIdlePhase = std::fmod (lfoIdlePhase + dt * param (P::rate), 1.f);
    rate.orbit = view.sounding ? view.lfoPhase : lfoIdlePhase;
    amount.glow = 0.28f + 0.7f * param (P::amount) * (0.5f + 0.5f * (view.sounding ? view.lfo : std::sin (juce::MathConstants<float>::twoPi * lfoIdlePhase)));
    // filter: CUTOFF shows where the filter really is, FLT ENV and RESONANCE light with the filter envelope
    filter.glow = 0.3f + 0.6f * lv + 0.6f * view.filt * param (P::fenv);
    if (view.sounding) filter.mod = (float) filter.s.valueToProportionOfLength (20.0 * std::pow (1000.0, (double) view.cutoff));
    fenvK.glow = 0.25f + 1.1f * view.filt * juce::jmax (0.2f, param (P::fenv));
    resoK.glow = 0.25f + 0.9f * lv * (0.3f + param (P::reso));
    glide.glow = param (P::glide) > 0.001f ? 0.28f + 0.9f * hit : 0.28f;
    // delay: TIME and FEEDBACK flash on every echo of the last note
    echo = 0.f;
    if (param (P::delay) > 0.01f)
    {
        const float T = juce::jmax (0.03f, param (P::dtime)), fb = param (P::fb);
        float a = 1.f;
        for (int k = 1; k <= 6; ++k) { a *= fb; const float d = (view.sinceHit - (float) k * T) / 0.05f; echo += a * std::exp (-d * d); }
        echo = juce::jmin (1.f, echo * param (P::delay) * 2.f);
    }
    dtime.glow = 0.28f + 1.1f * echo; feedback.glow = 0.28f + 1.1f * echo; delay.glow = 0.28f + 0.6f * lv + 0.6f * echo;
    for (auto* k : all) k->s.getProperties().set ("glow", k->glow);

    // envelope faders: the LEDs of the stage playing now rise with the envelope
    for (int i = 0; i < 4; ++i)
        env[i].s.getProperties().set ("led", view.ampStage == i ? juce::jmax (0.08f, view.amp) : 0.f);
    for (int i = 0; i < 4; ++i)
        rsl[i].s.getProperties().set ("led", view.filtStage == i ? juce::jmax (0.08f, view.filt) : 0.f);
    rsl[4].s.getProperties().set ("led", hit * 0.9f);

    // electricity: arcs jump from the medallion on every note, and now and then by themselves
    auto arc = [this] (float strength)
    {
        const float a = rnd.nextFloat() * juce::MathConstants<float>::twoPi, spread = 0.25f + rnd.nextFloat() * 0.5f;
        const auto p0 = medallion + juce::Point<float> (std::cos (a), std::sin (a)) * (medallionR + 2.f);
        const auto p1 = medallion + juce::Point<float> (std::cos (a + spread), std::sin (a + spread)) * (medallionR + 24.f + 30.f * strength);
        const juce::Colour cols[3] { amber, ember, juce::Colour (0xffffd34a) };
        bolts.add (hfx::lightning (p0, p1, rnd, 0.35f, 14.f), cols[rnd.nextInt (3)], 0.12f + 0.12f * strength, 1.2f + strength);
    };
    if (view.newNote)
    {
        arc (1.f); arc (0.6f);
        sparks.emit ({ medallion.x - 60.f, medallion.y + 70.f, 120.f, 20.f }, 10, { 0.f, -70.f }, 120.f, 1.1f, 2.4f, gold);
        if (rnd.nextFloat() < 0.35f)   // a crack of electricity across the title
        {
            const auto b = title.getBounds();
            const float y0 = b.getY() + b.getHeight() * (0.2f + 0.6f * rnd.nextFloat()), y1 = b.getY() + b.getHeight() * (0.2f + 0.6f * rnd.nextFloat());
            bolts.add (hfx::lightning ({ b.getX() + 6.f, y0 }, { b.getRight() - 6.f, y1 }, rnd, 0.12f), juce::Colour (0xfffff2c8), 0.14f, 1.1f);
        }
    }
    if ((idleBolt -= dt) <= 0.f) { arc (0.3f); idleBolt = 2.5f + rnd.nextFloat() * 4.f; }

    // keys: sparks rise from every key that goes down
    for (int n = 36; n <= 96; ++n)
    {
        const bool down = proc.keyboardState.isNoteOnForChannels (0xffff, n);
        if (down && ! keyWasDown[(size_t) n])
        {
            const auto kr = keys.getRectangleForKey (n).translated ((float) keys.getX(), (float) keys.getY());
            sparks.emit ({ kr.getX(), kr.getY() - 2.f, kr.getWidth(), 4.f }, 7, { 0.f, -110.f }, 70.f, 0.9f, 2.f, n % 2 ? amber : ember);
        }
        keyWasDown[(size_t) n] = down;
    }
    // a few embers float up all the time
    if ((idleSpark -= dt) <= 0.f)
    {
        sparks.emit ({ 70.f, 520.f, 860.f, 30.f }, 1, { 0.f, -25.f }, 20.f, 4.f, 1.6f, rnd.nextBool() ? amber : ember);
        idleSpark = view.sounding ? 0.06f : 0.35f;
    }
    sparks.update (dt);
    bolts.update (dt);
    keys.advance (dt);

    // the cassette turns while something plays
    reelSpeed += ((view.sounding ? 1.f : 0.f) - reelSpeed) * (1.f - std::exp (-dt / 0.4f));
    reelAngle += dt * reelSpeed * juce::MathConstants<float>::twoPi * 0.55f;
    presetFlash *= std::exp (-dt / 0.5f);
    invalidateChanges();
}

juce::Point<float> DzPanel::runnerPos() const
{
    juce::Path frame; frame.addRoundedRectangle (R (597, 72, 808, 118).toFloat().expanded (1.f), 14.f);
    return frame.getPointAlongPath (std::fmod (view.time * 0.18f, 1.f) * frame.getLength());
}

void DzPanel::invalidateChanges()
{
    // a full repaint of the panel 30-60 times per second costs far too much CPU: only what changed is redrawn
    if (view.time < 1.5f) { repaint(); prevMoving.clear(); return; }   // neon ignition: everything flickers
    juce::RectangleList<int> dirty, moving;
    auto add = [] (juce::RectangleList<int>& l, juce::Rectangle<float> r) { l.add (r.getSmallestIntegerContainer()); };
    auto square = [] (juce::Point<float> c, float half) { return juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre (c); };

    add (dirty, title.getBounds().expanded (44.f));                          // breathing neon
    add (dirty, square (medallion, medallionR + 48.f));                       // halo
    if (view.sinceHit < 0.7f) add (moving, square (medallion, medallionR + view.sinceHit * 170.f + 14.f));   // shock ring

    const float meter = view.levelL + view.levelR + view.holdL + view.holdR;
    if (std::abs (meter - shownMeter) > 0.002f)
    {
        for (int i = 0; i < 2; ++i) add (dirty, grilles (i).expanded (6.f));
        add (dirty, R (1490, 282, 340, 22).toFloat().expanded (4.f));
        shownMeter = meter;
    }
    const float rail = view.level + view.hit;
    if (std::abs (rail - shownRail) > 0.004f || view.sinceHit < 1.f)
    {
        for (float x : { 66.5f, 936.f }) add (dirty, { x - 30.f, 0.f, 60.f, (float) getHeight() });
        shownRail = rail;
    }
    for (auto* k : all)
    {
        if (k->lin)
        {
            const float led = (float) k->s.getProperties().getWithDefault ("led", 0.f);
            if (std::abs (led - k->shownLed) > 0.004f) { add (dirty, k->s.getBounds().toFloat().expanded (22.f, 8.f)); k->shownLed = led; }
            continue;
        }
        if (std::abs (k->glow - k->shownGlow) > 0.004f || k->orbit >= 0.f || k->mod >= 0.f || k->shownMod)
        {
            add (dirty, k->s.getBounds().toFloat().expanded (18.f));
            k->shownGlow = k->glow; k->shownMod = k->mod >= 0.f;
        }
    }
    const bool lit = std::any_of (keys.heat.begin(), keys.heat.end(), [] (float h) { return h > 0.f; });
    if (lit || keysLit) keys.repaint();
    keysLit = lit;
    const float line = view.level + 0.3f * view.hit;
    if (std::abs (line - shownLine) > 0.004f)                                         // neon line on top of the keyboard
    {
        add (dirty, keys.getBounds().toFloat().withHeight (1.f).expanded (2.f, 22.f));
        shownLine = line;
    }
    if (reelSpeed > 0.002f) for (auto rl : Face::reels) add (dirty, square ({ rl.x, rl.y }, 20.f));
    add (moving, square (runnerPos(), 18.f));
    if (presetFlash > 0.01f) add (dirty, square (presetLabel.getBounds().toFloat().getCentre(), 152.f));
    for (auto& p : sparks.ps) add (moving, square (p.pos, p.size * 3.f + 2.f));
    for (auto& b : bolts.bolts) add (moving, b.path.getBounds().expanded (16.f));

    dirty.add (moving); dirty.add (prevMoving);
    prevMoving = moving;
    dirty.consolidate();
    for (auto& r : dirty) repaint (r);
}

void DzPanel::paintLive (juce::Graphics& g)
{
    const float lv = view.level, hit = view.hit, ign = view.ignition(), t = view.time;

    // neon strips along the rails, a pulse runs down them on every note
    for (int side = 0; side < 2; ++side)
    {
        const float x = side == 0 ? 66.5f : 936.f;
        juce::Path strip; strip.startNewSubPath (x, 0.f); strip.lineTo (x, (float) getHeight());
        const auto c = side == 0 ? amber : ember;
        if (! g.clipRegionIntersects (juce::Rectangle<int> ((int) x - 30, 0, 60, getHeight()))) continue;
        hfx::neonStroke (g, strip, c, 2.f, ign * (0.3f + 0.8f * lv + 0.4f * hit));
        if (view.sinceHit < 0.9f) hfx::glowSpot (g, { x, view.sinceHit * 900.f }, 26.f, c, 0.9f * (1.f - view.sinceHit / 0.9f));
    }

    // speaker grilles = LED meters: the holes of the faceplate light up from the bottom with the level (left grille =
    // left channel), green -> amber -> red, the middle columns a little higher, the peak row stays lit a moment
    for (int i = 0; i < 2; ++i)
    {
        const auto r = grilles (i);
        if (! g.clipRegionIntersects (r.expanded (8.f).getSmallestIntegerContainer())) continue;
        const float level = i == 0 ? view.levelL : view.levelR, hold = i == 0 ? view.holdL : view.holdR;
        const auto* holes = i == 0 ? Face::grille0 : Face::grille1;
        const int n = i == 0 ? (int) std::size (Face::grille0) : (int) std::size (Face::grille1);
        float top = 1e9f, bottom = -1e9f, left = 1e9f, right = -1e9f;
        for (int k = 0; k < n; ++k) { top = juce::jmin (top, holes[k].y); bottom = juce::jmax (bottom, holes[k].y); left = juce::jmin (left, holes[k].x); right = juce::jmax (right, holes[k].x); }
        const float span = juce::jmax (1.f, bottom - top), midX = (left + right) * 0.5f, halfW = juce::jmax (1.f, (right - left) * 0.5f);
        for (int k = 0; k < n; ++k)
        {
            const float h = (bottom - holes[k].y) / span;   // 0 at the bottom, 1 at the top
            const float centre = 1.f - std::abs (holes[k].x - midX) / halfW;
            const float colLevel = juce::jlimit (0.f, 1.f, level * (0.7f + 0.3f * centre) + 0.06f * std::sin (t * 7.f + holes[k].x * 0.21f) * level);
            const bool lit = h <= colLevel + 0.02f, peak = std::abs (h - hold) < 5.f / span && hold > 0.05f;
            if (! (lit || peak)) continue;
            g.setOpacity (peak && ! lit ? 0.8f : 1.f);
            g.drawImageAt (ledSprite[h < 0.6f ? 0 : h < 0.82f ? 1 : 2], juce::roundToInt (holes[k].x) - 7, juce::roundToInt (holes[k].y) - 7);
        }
    }
    // envelope faders: the faceplate's LED dots beside the stage playing now rise with the envelope
    for (int i = 0; i < 9; ++i)
    {
        const auto& k = i < 4 ? env[i] : rsl[i - 4];
        const float led = (float) k.s.getProperties().getWithDefault ("led", 0.f);
        if (led <= 0.f || ! g.clipRegionIntersects (k.s.getBounds().expanded (12))) continue;
        for (int row = 0; row < Face::ledRows; ++row)
        {
            const float h = (float) (Face::ledRows - 1 - row) / (float) (Face::ledRows - 1);
            if (h > led + 0.02f) continue;
            for (int side = 0; side < 2; ++side)
            {
                const auto p = faderDot (i, side, row);
                g.setOpacity (1.f);
                g.drawImageAt (ledSprite[h < 0.55f ? 0 : h < 0.8f ? 1 : 2], juce::roundToInt (p.x) - 7, juce::roundToInt (p.y) - 7);
            }
        }
    }

    // medallion: a halo breathing with the sound, a shock ring on every note
    if (g.clipRegionIntersects (juce::Rectangle<float> (2.f * (medallionR + 48.f), 2.f * (medallionR + 48.f)).withCentre (medallion).getSmallestIntegerContainer()))
    {
        const float mix = 0.5f + 0.5f * std::sin (t * 0.7f), k = ign * (0.3f + 1.1f * lv + 0.7f * hit);
        haloA.draw (g, medallion, medallionR, 34.f, amber, k * (1.f - mix));
        haloB.draw (g, medallion, medallionR, 34.f, ember, k * mix);
    }
    if (view.sinceHit < 0.6f)
    {
        juce::Path ring; const float rr = medallionR + view.sinceHit * 170.f;
        ring.addEllipse (medallion.x - rr, medallion.y - rr, rr * 2.f, rr * 2.f);
        hfx::neonStroke (g, ring, amber.interpolatedWith (ember, view.sinceHit / 0.6f), 1.6f, 0.9f * (1.f - view.sinceHit / 0.6f));
    }

    // knob halos behind the knobs; the LFO dot around RATE; the live cutoff marker on CUTOFF
    for (auto* k : all)
    {
        if (k->lin || ! g.clipRegionIntersects (k->s.getBounds().expanded (18))) continue;
        const auto b = k->s.getBounds().toFloat().reduced (3.f);
        const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const auto rp = k->s.getRotaryParameters();
        const float pos = (float) k->s.valueToProportionOfLength (k->s.getValue());
        const float a = rp.startAngleRadians + pos * (rp.endAngleRadians - rp.startAngleRadians);
        juce::Path val; val.addCentredArc (c.x, c.y, r, r, 0.f, rp.startAngleRadians, a, true);
        hfx::neonStroke (g, val, ramp (pos), 3.5f, ign * k->glow, false);
        if (k->orbit >= 0.f)
        {
            const float oa = juce::MathConstants<float>::twoPi * k->orbit - juce::MathConstants<float>::halfPi;
            const auto p = c + juce::Point<float> (std::cos (oa), std::sin (oa)) * (r + 5.f);
            hfx::glowSpot (g, p, 9.f, ember, 0.9f * ign);
            g.setColour (juce::Colours::white); g.fillEllipse (p.x - 1.8f, p.y - 1.8f, 3.6f, 3.6f);
        }
        if (k->mod >= 0.f)
        {
            const float ma = rp.startAngleRadians + juce::jlimit (0.f, 1.f, k->mod) * (rp.endAngleRadians - rp.startAngleRadians);
            const auto p = c + juce::Point<float> (std::sin (ma), -std::cos (ma)) * r;
            hfx::glowSpot (g, p, 10.f, juce::Colours::white, 0.8f);
        }
    }

    // neon title: ignites when the plug-in opens, breathes, flares with the sound; a band of light sweeps it every 7 s
    if (g.clipRegionIntersects (title.getBounds().expanded (44.f).getSmallestIntegerContainer()))
    {
        const auto b = title.getBounds();
        juce::ColourGradient brush (amber, b.getX(), 0.f, ember, b.getRight(), 0.f, false);
        const float flicker = rnd.nextFloat() < 0.004f ? 0.55f : 1.f;
        const float sweep = std::fmod (t, 7.f) / 1.3f;
        title.draw (g, brush, juce::Colour (0xfffbf6ea), ign * flicker * (0.55f + 0.15f * view.breathe (3.5f, 0.f, 1.f) + 0.55f * lv + 0.45f * hit), sweep < 1.2f ? sweep : -1.f);
    }

    // cassette reels: the hubs turn while notes play
    for (auto rl : Face::reels)
    {
        const juce::Point<float> c { rl.x, rl.y };
        if (! g.clipRegionIntersects (juce::Rectangle<int> ((int) c.x - 20, (int) c.y - 20, 40, 40))) continue;
        g.setColour (juce::Colour (0xff1b1b1b)); g.fillEllipse (c.x - 10.f, c.y - 10.f, 20.f, 20.f);
        g.setColour (juce::Colour (0xffe8e4dc)); g.drawEllipse (c.x - 7.6f, c.y - 7.6f, 15.2f, 15.2f, 2.2f);
        for (int i = 0; i < 6; ++i)
        {
            const float a = reelAngle + (float) i * juce::MathConstants<float>::twoPi / 6.f;
            juce::Path tooth; tooth.addRectangle (-1.1f, -7.2f, 2.2f, 3.2f);
            g.fillPath (tooth, juce::AffineTransform::rotation (a).translated (c.x, c.y));
        }
        if (reelSpeed > 0.05f) hfx::glowSpot (g, c, 16.f, amber, 0.25f * reelSpeed);
    }

    // preset screen: a light runs around its frame, the name lights up when the preset changes
    {
        const float f = std::fmod (t * 0.18f, 1.f);
        const auto rp = runnerPos();
        if (g.clipRegionIntersects (juce::Rectangle<int> ((int) rp.x - 18, (int) rp.y - 18, 36, 36)))
            hfx::glowSpot (g, rp, 16.f, f < 0.5f ? amber : ember, 0.8f * ign);
        if (presetFlash > 0.02f) hfx::glowSpot (g, presetLabel.getBounds().toFloat().getCentre(), 150.f, juce::Colour (0xfffff2c8), 0.18f * presetFlash);
    }

    // stereo LED meter under the brand name
    {
        const auto m = R (1490, 282, 340, 22).toFloat();
        if (g.clipRegionIntersects (m.expanded (4.f).getSmallestIntegerContainer()))
        {
            hfx::ledBar (g, m.withHeight (4.5f), view.levelL, view.holdL);
            hfx::ledBar (g, m.withTrimmedTop (6.5f), view.levelR, view.holdR);
        }
    }

    // keyboard: a neon line along its top edge
    {
        juce::Path top; top.startNewSubPath ((float) keys.getX(), (float) keys.getY() - 1.f); top.lineTo ((float) keys.getRight(), (float) keys.getY() - 1.f);
        if (g.clipRegionIntersects (keys.getBounds().withHeight (1).expanded (2, 22)))
            hfx::neonStroke (g, top, ember.interpolatedWith (amber, hit), 1.4f, ign * (0.25f + 0.9f * lv));
    }

    sparks.draw (g);
    bolts.draw (g);
}

void DzPanel::resized()
{
    // knobs: centred on their caps in the faceplate, the value ring just outside the cap
    Knob* byName[] = { &rate, &amount, &pitch, &volume, &reverb, &glide, &filter, &resoK, &chorus, &dist, &dtime, &feedback,
                       &tone, &body, &bright, &width, &texture, &fenvK, &delay };
    for (size_t i = 0; i < std::size (Face::caps); ++i)
    {
        const auto& c = Face::caps[i];
        const float ring = c.r + (c.r > 14.f ? 4.5f : 2.6f), half = ring + 3.f;
        byName[i]->s.setBounds (juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre ({ c.x, c.y }).toNearestInt());
    }
    // faders: on the slots of the faceplate, travelling over the slot's length
    for (int i = 0; i < 4; ++i) env[i].s.setBounds (juce::Rectangle<int> (28, 86).withCentre ({ juce::roundToInt (Face::slotX[i]), 435 }));
    for (int i = 0; i < 5; ++i) rsl[i].s.setBounds (juce::Rectangle<int> (28, 66).withCentre ({ juce::roundToInt (Face::slotX[4 + i]), 311 }));
    bend.s.setBounds (C (280, 1255, 90, 260)); modw.s.setBounds (C (432, 1255, 90, 260));
    cWave.c.setBounds (R (665, 985, 205, 57)); cLfo.c.setBounds (R (900, 985, 205, 57)); cDest.c.setBounds (R (1135, 985, 205, 57));
    cFilt.c.setBounds (R (1373, 985, 207, 57)); cMono.c.setBounds (R (1612, 985, 206, 57));
    folder.setBounds (R (632, 92, 100, 80)); save.setBounds (R (1310, 92, 80, 80));
    prev.setBounds (R (705, 92, 50, 80)); next.setBounds (R (1245, 92, 55, 80));
    presetLabel.setBounds (R (770, 100, 470, 66));
    expansion.setBounds (R (205, 92, 317, 386));
    const auto kb = R (583, 1110, 1232, 305);
    keys.setKeyWidth ((float) kb.getWidth() / 36.f);   // 36 white keys between MIDI 36 and 96 fill the keyboard area exactly
    keys.setBounds (kb);
    title.set ("DZPOWER", juce::FontOptions (56.f, juce::Font::bold), R (700, 625, 600, 120).toFloat());
    bgCache = {};
}
