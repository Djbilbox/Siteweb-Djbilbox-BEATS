#include "PluginEditor.h"
#include "InstrumentArt.h"

namespace humpire
{
using namespace theme;

namespace
{
juce::String U (const char* s) { return juce::String::fromUTF8 (s); }

void setParam (juce::AudioProcessorValueTreeState& a, const juce::String& id, float plainValue)
{
    if (auto* p = a.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
        p->endChangeGesture();
    }
}

float getParam (juce::AudioProcessorValueTreeState& a, const juce::String& id)
{
    return a.getRawParameterValue (id)->load();
}

void panelBox (juce::Graphics& g, juce::Rectangle<float> r, float radius = 10.f)
{
    g.setColour (panel.withAlpha (0.92f));
    g.fillRoundedRectangle (r, radius);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawRoundedRectangle (r.reduced (0.5f), radius, 1.0f);
}
} // namespace

// ============================================================ Canvas
Canvas::Canvas() { zellige = makeZelligeImage (1280, 760, 64.0f, gold.withAlpha (0.055f)); }

void Canvas::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.drawImageAt (zellige, 0, 0);
    // bandeau superieur
    g.setGradientFill (juce::ColourGradient (panel, 0, 0, bg, 0, 60.0f, false));
    g.fillRect (0, 0, 1280, 60);
    g.setColour (gold.withAlpha (0.55f));
    g.fillRect (0, 59, 1280, 1);
}

// ============================================================ Carte instrument
InstrumentCard::InstrumentCard (ChaabiProcessor& p, int i) : proc (p), idx (i) {}

void InstrumentCard::paint (juce::Graphics& g)
{
    auto& a = proc.apvts;
    const bool selected = (int) getParam (a, pid::selected) == idx;
    const bool multi = getParam (a, pid::multi) > 0.5f;
    const bool on = getParam (a, pid::inst (idx, "on")) > 0.5f;
    const auto col = juce::Colour (kInstruments[idx].colour);
    auto r = getLocalBounds().toFloat();

    panelBox (g, r, 9.f);
    if (selected)
    {
        g.setColour (gold.withAlpha (0.12f));
        g.fillRoundedRectangle (r, 9.f);
        g.setColour (gold);
        g.drawRoundedRectangle (r.reduced (1.f), 9.f, 1.6f);
    }

    auto left = r.removeFromLeft (r.getHeight()).reduced (7.f);
    InstrumentArt::draw (g, idx, left, col.withAlpha ((multi && ! on) ? 0.35f : 1.0f));

    auto text = r.reduced (6.f, 6.f);
    g.setColour (selected ? gold : this->findColour (juce::Label::textColourId));
    g.setFont (font (15.0f, true));
    g.drawText (instrumentName (idx), text.removeFromTop (20.f).toNearestInt(), juce::Justification::centredLeft, true);

    // etat de chargement + zone (multi)
    const auto st = proc.getSlotStatus (idx);
    juce::String sub = st.state == 2 ? U ("pr\xC3\xAAt") : st.state == 1 ? U ("chargement\xE2\x80\xA6") : st.state == 3 ? "SFZ ?" : U ("\xE2\x80\x94");
    if (multi)
        sub = (on ? juce::String ("ON  ") : juce::String ("off ")) + juce::MidiMessage::getMidiNoteName ((int) getParam (a, pid::inst (idx, "lo")), true, true, 5)
              + "-" + juce::MidiMessage::getMidiNoteName ((int) getParam (a, pid::inst (idx, "hi")), true, true, 5);
    g.setColour (st.state == 3 ? terracotta.brighter() : st.state == 2 ? ok : textDim);
    g.setFont (font (11.5f));
    g.drawText (sub, text.toNearestInt(), juce::Justification::centredLeft, true);
}

void InstrumentCard::mouseUp (const juce::MouseEvent& e)
{
    if (! getLocalBounds().contains (e.getPosition())) return;
    const bool multi = getParam (proc.apvts, pid::multi) > 0.5f;
    if (multi && (e.mods.isRightButtonDown() || e.mods.isShiftDown() || e.x > getWidth() - 56))
        setParam (proc.apvts, pid::inst (idx, "on"), getParam (proc.apvts, pid::inst (idx, "on")) > 0.5f ? 0.f : 1.f);
    else
        setParam (proc.apvts, pid::selected, (float) idx);
}

// ============================================================ Visuel
void ArtView::paint (juce::Graphics& g)
{
    const int idx = (int) getParam (proc.apvts, pid::selected);
    auto r = getLocalBounds().toFloat();
    panelBox (g, r, 14.f);

    // halo terre cuite
    g.setGradientFill (juce::ColourGradient (terracotta.withAlpha (0.22f), r.getCentreX(), r.getCentreY(),
                                             juce::Colours::transparentBlack, r.getCentreX(), r.getBottom(), true));
    g.fillRoundedRectangle (r, 14.f);

    InstrumentArt::draw (g, idx, r.reduced (22.f), juce::Colour (kInstruments[idx].colour), true);
}

// ============================================================ Pad de phrase
PhrasePad::PhrasePad (ChaabiProcessor& p, int instrument, int index) : proc (p), inst (instrument), idx (index) {}

void PhrasePad::paint (juce::Graphics& g)
{
    const auto& ph = phrasesFor (inst)[(size_t) idx];
    auto r = getLocalBounds().toFloat();
    g.setColour (down ? gold.withAlpha (0.9f) : panelHi);
    g.fillRoundedRectangle (r, 8.f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (r.reduced (0.5f), 8.f, 1.f);

    auto handle = r.removeFromRight (34.f);
    g.setColour (down ? bg : text);
    g.setFont (font (12.5f, true));
    auto t = r.reduced (8.f, 4.f);
    g.drawText (juce::MidiMessage::getMidiNoteName (kPhraseLo + idx, true, true, 5), t.removeFromTop (16.f).toNearestInt(), juce::Justification::centredLeft);
    g.setFont (font (12.f));
    g.setColour (down ? bg : textDim);
    g.drawText (ph.name, t.toNearestInt(), juce::Justification::centredLeft, true);

    // poignee drag MIDI : 6 points
    g.setColour (down ? bg : gold);
    for (int cx = 0; cx < 2; ++cx)
        for (int cy = 0; cy < 3; ++cy)
            g.fillEllipse (handle.getCentreX() - 6.f + (float) cx * 8.f, handle.getCentreY() - 9.f + (float) cy * 8.f, 3.4f, 3.4f);
    g.setFont (font (8.5f, true));
    g.drawText ("MIDI", handle.removeFromBottom (12.f).toNearestInt(), juce::Justification::centred);
}

void PhrasePad::mouseDown (const juce::MouseEvent& e)
{
    exported = false;
    if (onHandle (e.getPosition())) return;
    down = true;
    proc.keyboardState.noteOn (1, kPhraseLo + idx, 0.8f);
    repaint();
}

void PhrasePad::mouseDrag (const juce::MouseEvent& e)
{
    if (onHandle (e.getMouseDownPosition()) && ! exported && e.getDistanceFromDragStart() > 6)
    {
        exported = true;
        const auto file = writePhraseMidiFile (phrasesFor (inst)[(size_t) idx], proc.getCurrentBpm());
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
    }
}

void PhrasePad::mouseUp (const juce::MouseEvent&)
{
    if (down) proc.keyboardState.noteOff (1, kPhraseLo + idx, 0.0f);
    down = false;
    repaint();
}

// ============================================================ Sequenceur
juce::Rectangle<float> SequencerView::cell (int lane, int step) const
{
    const float labelW = 54.f;
    const float cw = ((float) getWidth() - labelW) / (float) kSeqSteps;
    const float ch = (float) getHeight() / (float) kSeqLanes;
    return { labelW + (float) step * cw + 1.5f, (float) lane * ch + 1.5f, cw - 3.f, ch - 3.f };
}

void SequencerView::paint (juce::Graphics& g)
{
    const int playStep = proc.phrasePlayer->seqDisplayStep.load();
    for (int lane = 0; lane < kSeqLanes; ++lane)
    {
        auto lab = cell (lane, 0);
        g.setColour (textDim);
        g.setFont (font (11.f, true));
        g.drawText (kSeqLaneNames[inst][lane], juce::Rectangle<float> (0.f, lab.getY(), 52.f, lab.getHeight()).toNearestInt(), juce::Justification::centredLeft);

        for (int step = 0; step < kSeqSteps; ++step)
        {
            const int v = proc.getSeqStep (inst, lane, step);
            auto c = cell (lane, step);
            g.setColour (v == 2 ? gold : v == 1 ? terracotta : (step / 4) % 2 == 0 ? panelHi : panelHi.brighter (0.06f));
            g.fillRoundedRectangle (c, 3.f);
            if (step == playStep)
            {
                g.setColour (juce::Colours::white.withAlpha (0.28f));
                g.fillRoundedRectangle (c, 3.f);
            }
        }
    }
}

void SequencerView::mouseDown (const juce::MouseEvent& e)
{
    for (int lane = 0; lane < kSeqLanes; ++lane)
        for (int step = 0; step < kSeqSteps; ++step)
            if (cell (lane, step).contains (e.position))
            {
                const int cur = proc.getSeqStep (inst, lane, step);
                proc.setSeqStep (inst, lane, step, e.mods.isRightButtonDown() ? 0 : (cur + 1) % 3);
                repaint();
                return;
            }
}

// ============================================================ Clavier
void ZonedKeyboard::drawWhiteNote (int note, juce::Graphics& g, juce::Rectangle<float> area, bool down, bool over, juce::Colour line, juce::Colour txt)
{
    juce::MidiKeyboardComponent::drawWhiteNote (note, g, area, down, over, line, txt);
    if (zoneColour) { g.setColour (zoneColour (note)); g.fillRect (area.withTrimmedBottom (area.getHeight() * 0.62f)); }
}

void ZonedKeyboard::drawBlackNote (int note, juce::Graphics& g, juce::Rectangle<float> area, bool down, bool over, juce::Colour fill)
{
    juce::MidiKeyboardComponent::drawBlackNote (note, g, area, down, over, fill);
    if (zoneColour) { g.setColour (zoneColour (note)); g.fillRect (area.withTrimmedBottom (area.getHeight() * 0.5f)); }
}

// ============================================================ Editeur
HumpireKnob* ChaabiEditor::addGlobalKnob (const juce::String& label, const juce::String& paramId)
{
    auto k = std::make_unique<HumpireKnob> (label);
    content.addAndMakeVisible (*k);
    k->onDragStart = [this] { proc.undoManager.beginNewTransaction(); };
    globalAtts.push_back (std::make_unique<SliderAtt> (proc.apvts, paramId, *k));
    globalKnobs.push_back (std::move (k));
    return globalKnobs.back().get();
}

ChaabiEditor::ChaabiEditor (ChaabiProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      inMeter (p.midiActivity, p.midiActivity, "IN"), outMeter (p.meterL, p.meterR, "OUT"),
      art (p), loadBar (loadProgress), seqView (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&laf);
    addAndMakeVisible (content);

    auto add = [this] (juce::Component& c) { content.addAndMakeVisible (c); };

    // ---- barre superieure
    logo.setText ("HUMPIRE", juce::dontSendNotification);
    logo.setFont (font (28.f, true));
    logo.setColour (juce::Label::textColourId, gold);
    logo.setInterceptsMouseClicks (false, false);
    add (logo);

    for (auto* b : { &prevBtn, &nextBtn, &presetBtn, &saveBtn, &abBtn, &undoBtn, &redoBtn }) add (*b);
    add (bypassBtn); add (inMeter); add (outMeter);
    prevBtn.onClick = [this] { proc.presets->prev(); };
    nextBtn.onClick = [this] { proc.presets->next(); };
    presetBtn.onClick = [this] { showPresetMenu(); };
    saveBtn.onClick = [this] { askPresetName(); };
    abBtn.onClick = [this] { proc.presets->toggleAB(); };
    undoBtn.onClick = [this] { proc.undoManager.undo(); };
    redoBtn.onClick = [this] { proc.undoManager.redo(); };
    bypassAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::bypass, bypassBtn);

    // ---- instruments
    for (int i = 0; i < kNumInstruments; ++i)
    {
        cards[(size_t) i] = std::make_unique<InstrumentCard> (proc, i);
        add (*cards[(size_t) i]);
    }
    add (art);

    titleLabel.setFont (font (26.f, true));
    titleLabel.setColour (juce::Label::textColourId, gold);
    taglineLabel.setFont (font (13.f));
    taglineLabel.setColour (juce::Label::textColourId, textDim);
    statusLabel.setFont (font (12.f));
    statusLabel.setColour (juce::Label::textColourId, textDim);
    for (auto* l : { &titleLabel, &taglineLabel, &statusLabel }) { l->setInterceptsMouseClicks (false, false); add (*l); }
    add (loadBar); add (libraryBtn); add (multiBtn); add (activeBtn);
    libraryBtn.onClick = [this] { chooseLibrary(); };
    multiAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::multi, multiBtn);

    // ---- knobs de l'instrument (rattaches a l'instrument selectionne)
    const char* labels[10] = { "Volume", "Pan", "Reverb", "Delay", "Filtre", "Pitch", "Attack", "Release", "Zone d\xC3\xA9" "but", "Zone fin" };
    for (size_t k = 0; k < instKnobs.size(); ++k)
    {
        instKnobs[k] = std::make_unique<HumpireKnob> (U (labels[k]));
        instKnobs[k]->onDragStart = [this] { proc.undoManager.beginNewTransaction(); };
        add (*instKnobs[k]);
    }

    // ---- knobs globaux
    authKnob = std::make_unique<HumpireKnob> (U ("AUTHENTICIT\xC3\x89"));
    authKnob->onDragStart = [this] { proc.undoManager.beginNewTransaction(); };
    add (*authKnob);
    authAtt = std::make_unique<SliderAtt> (proc.apvts, pid::authent, *authKnob);
    addGlobalKnob ("Master", pid::master);
    addGlobalKnob ("Rev Taille", pid::revSize);
    addGlobalKnob ("Rev Amorti", pid::revDamp);
    addGlobalKnob ("Dly Temps", pid::dlyTime);
    addGlobalKnob ("Dly Feedback", pid::dlyFb);
    addGlobalKnob ("Tempo", pid::tempo);
    addGlobalKnob ("Polyphonie", pid::poly);
    addGlobalKnob ("Bend", pid::bendRange);

    // ---- onglets
    for (auto* b : { &tabPlay, &tabPhrases, &tabPatterns })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (77);
        b->onClick = [this] { layoutTabs(); };
        add (*b);
    }
    tabPlay.setToggleState (true, juce::dontSendNotification);
    add (playPanel); add (phrasePanel); add (patternPanel);

    playInfo.setFont (font (13.f));
    playInfo.setColour (juce::Label::textColourId, textDim);
    playInfo.setJustificationType (juce::Justification::topLeft);
    playPanel.addAndMakeVisible (playInfo);

    patternPanel.addAndMakeVisible (seqView);
    for (auto* b : { &seqDerb, &seqTar })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (88);
        patternPanel.addAndMakeVisible (*b);
    }
    seqDerb.setToggleState (true, juce::dontSendNotification);
    seqDerb.onClick = [this] { seqInst = 0; seqView.setInstrument (0); };
    seqTar.onClick  = [this] { seqInst = 1; seqView.setInstrument (1); };
    patternPanel.addAndMakeVisible (seqRun);
    seqRate.addItemList ({ "1/8", "1/16" }, 1);
    patternPanel.addAndMakeVisible (seqRate);
    patternPanel.addAndMakeVisible (seqSwingKnob);
    seqRunAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::seqOn, seqRun);
    seqRateAtt = std::make_unique<ComboAtt> (proc.apvts, pid::seqRate, seqRate);
    seqSwingAtt = std::make_unique<SliderAtt> (proc.apvts, pid::seqSwing, seqSwingKnob);

    // ---- clavier
    keyboard.setOctaveForMiddleC (5);   // convention FL Studio : C5 = 60
    keyboard.setAvailableRange (kKeyswitchLo, 96);
    keyboard.setKeyWidth ((float) (kW - 32) / 22.0f);
    keyboard.setLowestVisibleKey (kPhraseLo);
    keyboard.setScrollButtonsVisible (true);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, gold.withAlpha (0.8f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, gold.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffe8e0d0));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff1a1a1d));
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, bg);
    keyboard.zoneColour = [this] (int note) -> juce::Colour
    {
        if (note >= kPhraseLo && note <= kPhraseHi) return gold.withAlpha (0.5f);
        if (note >= kKeyswitchLo && note <= kKeyswitchHi) return juce::Colour (0xff4a9fb8).withAlpha (0.55f);
        const bool multi = getParam (proc.apvts, pid::multi) > 0.5f;
        if (! multi)
            return note >= kPlayLo ? juce::Colour (kInstruments[(int) getParam (proc.apvts, pid::selected)].colour).withAlpha (0.28f) : juce::Colours::transparentBlack;
        for (int i = 0; i < kNumInstruments; ++i)
            if (getParam (proc.apvts, pid::inst (i, "on")) > 0.5f
                && note >= (int) getParam (proc.apvts, pid::inst (i, "lo")) && note <= (int) getParam (proc.apvts, pid::inst (i, "hi")))
                return juce::Colour (kInstruments[i].colour).withAlpha (0.45f);
        return juce::Colours::transparentBlack;
    };
    add (keyboard);

    bindInstrumentKnobs();
    layoutTabs();
    rebuildPhrasePads();

    setResizable (true, true);
    setResizeLimits (kW / 2, kH / 2, kW * 2, kH * 2);
    getConstrainer()->setFixedAspectRatio ((double) kW / (double) kH);
    setSize (kW, kH);
    startTimerHz (15);
}

ChaabiEditor::~ChaabiEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void ChaabiEditor::resized()
{
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) kW));
    content.setBounds (0, 0, kW, kH);

    // ---- barre superieure
    logo.setBounds (18, 8, 190, 42);
    prevBtn.setBounds (236, 15, 30, 30);
    presetBtn.setBounds (270, 15, 330, 30);
    nextBtn.setBounds (604, 15, 30, 30);
    saveBtn.setBounds (642, 15, 58, 30);
    abBtn.setBounds (708, 15, 36, 30);
    undoBtn.setBounds (752, 15, 58, 30);
    redoBtn.setBounds (814, 15, 58, 30);
    bypassBtn.setBounds (884, 15, 84, 30);
    inMeter.setBounds (990, 12, 130, 16);
    outMeter.setBounds (990, 32, 130, 16);
    // (1130..1264 : reserve au futur bouton licence / Hub)

    // ---- colonne instruments
    for (int i = 0; i < kNumInstruments; ++i)
        cards[(size_t) i]->setBounds (16, 74 + i * 56, 244, 52);

    // ---- centre : visuel + infos
    art.setBounds (276, 74, 290, 246);
    titleLabel.setBounds (582, 78, 330, 34);
    taglineLabel.setBounds (582, 112, 330, 20);
    statusLabel.setBounds (582, 140, 330, 20);
    loadBar.setBounds (582, 166, 330, 14);
    multiBtn.setBounds (582, 196, 100, 30);
    activeBtn.setBounds (690, 196, 100, 30);
    libraryBtn.setBounds (582, 236, 208, 30);

    for (size_t k = 0; k < 8; ++k)
        instKnobs[k]->setBounds (276 + (int) k * 80, 332, 76, 92);
    instKnobs[8]->setBounds (276, 428, 76, 92);
    instKnobs[9]->setBounds (356, 428, 76, 92);

    // ---- colonne droite : macro + globaux
    authKnob->setBounds (944, 78, 172, 188);
    for (size_t k = 0; k < globalKnobs.size(); ++k)
        globalKnobs[k]->setBounds (922 + (int) (k % 4) * 86, 276 + (int) (k / 4) * 100, 82, 94);

    // ---- bas
    tabPlay.setBounds (16, 530, 110, 28);
    tabPhrases.setBounds (130, 530, 110, 28);
    tabPatterns.setBounds (244, 530, 110, 28);
    for (auto* c : { &playPanel, &phrasePanel, &patternPanel }) c->setBounds (16, 564, kW - 32, 86);
    playInfo.setBounds (4, 2, 900, 80);
    keyboard.setBounds (16, 656, kW - 32, 96);

    // pads de phrases
    for (size_t i = 0; i < pads.size(); ++i)
        pads[i]->setBounds ((int) i * 160, 6, 152, 72);

    // sequenceur
    seqDerb.setBounds (0, 4, 104, 36);
    seqTar.setBounds (0, 44, 104, 36);
    seqRun.setBounds (112, 4, 92, 36);
    seqRate.setBounds (112, 44, 92, 36);
    seqSwingKnob.setBounds (212, 0, 76, 86);
    seqView.setBounds (296, 0, patternPanel.getWidth() - 296, 86);
}

void ChaabiEditor::layoutTabs()
{
    playPanel.setVisible (tabPlay.getToggleState());
    phrasePanel.setVisible (tabPhrases.getToggleState());
    patternPanel.setVisible (tabPatterns.getToggleState());
}

void ChaabiEditor::bindInstrumentKnobs()
{
    const int sel = (int) getParam (proc.apvts, pid::selected);
    shownSelected = sel;
    static const char* ids[10] = { "vol", "pan", "rev", "dly", "filter", "pitch", "atk", "rel", "lo", "hi" };

    for (size_t k = 0; k < instKnobs.size(); ++k)
    {
        instAtts[k].reset();
        instAtts[k] = std::make_unique<SliderAtt> (proc.apvts, pid::inst (sel, ids[k]), *instKnobs[k]);
    }
    activeAtt.reset();
    activeAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::inst (sel, "on"), activeBtn);

    titleLabel.setText (instrumentName (sel), juce::dontSendNotification);
    taglineLabel.setText (U (kInstruments[sel].tagline), juce::dontSendNotification);
    rebuildPhrasePads();
}

void ChaabiEditor::rebuildPhrasePads()
{
    pads.clear();
    phrasePanel.removeAllChildren();
    const int sel = (int) getParam (proc.apvts, pid::selected);
    const auto& list = phrasesFor (sel);
    for (int i = 0; i < (int) list.size(); ++i)
    {
        pads.push_back (std::make_unique<PhrasePad> (proc, sel, i));
        phrasePanel.addAndMakeVisible (*pads.back());
        pads.back()->setBounds (i * 160, 6, 152, 72);
    }
}

void ChaabiEditor::timerCallback()
{
    if ((int) getParam (proc.apvts, pid::selected) != shownSelected)
        bindInstrumentKnobs();

    const int sel = shownSelected;
    const auto st = proc.getSlotStatus (sel);
    loadProgress = st.state == 1 ? (double) st.progress : (st.state == 2 ? 1.0 : 0.0);
    loadBar.setVisible (st.state == 1);
    statusLabel.setText (st.message, juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, st.state == 3 ? terracotta.brighter (0.3f) : st.state == 2 ? ok : textDim);

    presetBtn.setButtonText (proc.presets->getCurrentName());
    abBtn.setButtonText (proc.presets->isB() ? "B" : "A");
    abBtn.setToggleState (proc.presets->isB(), juce::dontSendNotification);
    undoBtn.setEnabled (proc.undoManager.canUndo());
    redoBtn.setEnabled (proc.undoManager.canRedo());

    const bool multi = getParam (proc.apvts, pid::multi) > 0.5f;
    activeBtn.setVisible (multi);
    instKnobs[8]->setVisible (multi);
    instKnobs[9]->setVisible (multi);

    juce::String info;
    info << U ("Biblioth\xC3\xA8que : ") << proc.getLibraryRoot().getFullPathName() << "\n"
         << U ("Zones clavier :  dor\xC3\xA9 = phrases (") << juce::MidiMessage::getMidiNoteName (kPhraseLo, true, true, 5) << "-"
         << juce::MidiMessage::getMidiNoteName (kPhraseHi, true, true, 5) << U (")   \xE2\x80\xA2   bleu = keyswitch articulations (")
         << juce::MidiMessage::getMidiNoteName (kKeyswitchLo, true, true, 5) << "-" << juce::MidiMessage::getMidiNoteName (kKeyswitchHi, true, true, 5)
         << U (")   \xE2\x80\xA2   couleur instrument = zone de jeu.\n")
         << U ("Phrases : jouez une touche dor\xC3\xA9" "e (maintenue = boucle, synchronis\xC3\xA9" "e sur l'h\xC3\xB4te). Glissez la poign\xC3\xA9" "e MIDI d'un pad vers le DAW.");
    playInfo.setText (info, juce::dontSendNotification);

    keyboard.repaint();
    for (auto& c : cards) c->repaint();
    art.repaint();
    seqView.repaint();
}

void ChaabiEditor::showPresetMenu()
{
    auto& pm = *proc.presets;
    std::vector<std::pair<juce::String, juce::PopupMenu>> groups;
    for (int i = 0; i < pm.getNumPresets(); ++i)
    {
        auto cat = pm.getCategory (i);
        auto it = std::find_if (groups.begin(), groups.end(), [&] (auto& g) { return g.first == cat; });
        if (it == groups.end()) { groups.push_back ({ cat, juce::PopupMenu() }); it = groups.end() - 1; }
        it->second.addItem (i + 1, pm.getName (i), true, i == pm.getCurrentIndex());
    }
    juce::PopupMenu menu;
    for (auto& g : groups) menu.addSubMenu (g.first, g.second);
    menu.addSeparator();
    menu.addItem (100000, U ("Enregistrer le preset\xE2\x80\xA6"));

    juce::Component::SafePointer<ChaabiEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetBtn), [safe] (int r)
    {
        if (safe == nullptr || r == 0) return;
        if (r == 100000) safe->askPresetName();
        else safe->proc.presets->loadIndex (r - 1);
    });
}

void ChaabiEditor::askPresetName()
{
    auto* w = new juce::AlertWindow (U ("Enregistrer le preset"), U ("Nom du preset :"), juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("n", proc.presets->getCurrentName());
    w->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Annuler", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe = juce::Component::SafePointer<ChaabiEditor> (this), w] (int r)
    {
        if (r == 1 && safe != nullptr) safe->proc.presets->saveUser (w->getTextEditorContents ("n"));
    }), true);
}

void ChaabiEditor::chooseLibrary()
{
    chooser = std::make_unique<juce::FileChooser> (U ("Dossier racine des samples HUMPIRE"), proc.getLibraryRoot());
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [this] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f != juce::File()) proc.setLibraryRoot (f);
                          });
}
} // namespace humpire
