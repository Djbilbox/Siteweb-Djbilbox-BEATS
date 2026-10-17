#include "PluginEditor.h"

using namespace humpire;
using namespace saturn;

SaturnEditor::SaturnEditor (SaturnAudioProcessor& p)
    : HumpireEditorBase (p, 960, 600),
      topBar (p.apvts, p.presets, "SATURN - Harmony", p.inLevel, p.outLevel),
      orbit (p),
      waveform (p.waveform, theme::violet),
      readout (p),
      xpBar (p.outLevel),
      modeAttach (*p.apvts.getParameter (id::mode),
                  [this] (float v) { applyMode (v > 0.5f); }, &p.undoManager),
      simpleAttach (*p.apvts.getParameter (id::simple),
                    [this] (float v) { applySimple (v > 0.5f); }, nullptr)
{
    // --- main zones -------------------------------------------------------------
    content.addAndMakeVisible (topBar);     topBar.setBounds (0, 0, 960, 56);
    content.addAndMakeVisible (orbit);      orbit.setBounds (16, 68, 600, 400);
    content.addAndMakeVisible (waveform);   waveform.setBounds (16, 480, 600, 52);
    content.addAndMakeVisible (readout);    readout.setBounds (16, 540, 600, 40);
    content.addAndMakeVisible (xpBar);      xpBar.setBounds (632, 548, 312, 36);

    // --- interval mode / simple -------------------------------------------------
    for (auto* b : { &degreesBtn, &semitonesBtn, &simpleBtn })
        content.addAndMakeVisible (*b);
    degreesBtn.setBounds (632, 68, 92, 28);
    semitonesBtn.setBounds (728, 68, 92, 28);
    simpleBtn.setBounds (856, 68, 88, 28);
    simpleBtn.setClickingTogglesState (true);
    degreesBtn.setTooltip ("Intervals follow the scale degrees of the sung note");
    semitonesBtn.setTooltip ("Intervals are fixed numbers of semitones");
    simpleBtn.setTooltip ("Show the main controls only");
    degreesBtn.onClick   = [this] { modeAttach.setValueAsCompleteGesture (0.0f); };
    semitonesBtn.onClick = [this] { modeAttach.setValueAsCompleteGesture (1.0f); };
    simpleBtn.onClick    = [this] { simpleAttach.setValueAsCompleteGesture (simpleBtn.getToggleState() ? 1.0f : 0.0f); };

    // --- key / scale -----------------------------------------------------------
    content.addAndMakeVisible (keyBox);
    content.addAndMakeVisible (scaleBox);
    keyBox.addItemList ({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
    for (int i = 0; i < (int) NumScales; ++i)
        scaleBox.addItem (kScales[i].name, i + 1);
    keyBox.setTooltip ("Tonic of the scale");
    scaleBox.setTooltip ("Scale or maqam used for the harmony");
    keyAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, id::key, keyBox);
    scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, id::scale, scaleBox);
    keyBox.setBounds (632, 104, 86, 28);
    scaleBox.setBounds (724, 104, 220, 28);

    // --- voice rows: [on] [interval] [level] [pan] [detune] --------------------
    for (int v = 0; v < kNumVoices; ++v)
    {
        const int rowY = 144 + v * 68;
        const auto col = saturnVoiceColour (v);

        auto* btn = voiceBtns.add (new juce::ToggleButton ("V" + juce::String (v + 1)));
        btn->setColour (juce::ToggleButton::tickColourId, col);
        btn->setTooltip ("Enable harmony voice " + juce::String (v + 1));
        content.addAndMakeVisible (btn);
        btn->setBounds (632, rowY + 18, 46, 28);
        voiceAttach.add (new juce::AudioProcessorValueTreeState::ButtonAttachment (p.apvts, voiceId (v, vOn), *btn));

        auto addKnob = [&] (juce::OwnedArray<OrbitKnob>& arr, const char* name, const char* tip, VoiceParam param, int slot)
        {
            auto* k = arr.add (new OrbitKnob (name, col));
            content.addAndMakeVisible (k);
            k->attach (p.apvts, voiceId (v, param));
            k->setTooltip (tip);
            k->setBounds (682 + slot * 66, rowY, 64, 64);
        };
        addKnob (semiKnobs,   "Semi",   "Interval in semitones",                      vSemi,   0);
        addKnob (degKnobs,    "Deg",    "Interval in steps of the selected scale",    vDeg,    0);
        addKnob (levelKnobs,  "Level",  "Level of this voice (minimum = silent)",     vLevel,  1);
        addKnob (panKnobs,    "Pan",    "Stereo position of this voice",              vPan,    2);
        addKnob (detuneKnobs, "Detune", "Fine tuning of this voice in cents",         vDetune, 3);
    }

    // --- global knobs ----------------------------------------------------------
    struct Global { OrbitKnob* k; const char* paramId; int x; };
    const Global globals[] = {
        { &glideKnob,   id::glide,   632 },
        { &mixKnob,     id::mix,     710 },
        { &inGainKnob,  id::inGain,  788 },
        { &outGainKnob, id::outGain, 866 },
    };
    for (const auto& gk : globals)
    {
        content.addAndMakeVisible (*gk.k);
        gk.k->attach (p.apvts, gk.paramId);
        gk.k->setBounds (gk.x, 424, 78, 88);
    }
    glideKnob.setTooltip ("Glide between successive interval changes");
    mixKnob.setTooltip ("0 % = dry only, 100 % = dry plus the harmony voices");
    inGainKnob.setTooltip ("Input trim before pitch detection");
    outGainKnob.setTooltip ("Output level");

    modeAttach.sendInitialUpdate();
    simpleAttach.sendInitialUpdate();
}

void SaturnEditor::applyMode (bool semitones)
{
    degreesBtn.setToggleState (! semitones, juce::dontSendNotification);
    semitonesBtn.setToggleState (semitones, juce::dontSendNotification);
    for (int v = 0; v < kNumVoices; ++v)
    {
        semiKnobs[v]->setVisible (semitones);
        degKnobs[v]->setVisible (! semitones);
    }
}

void SaturnEditor::applySimple (bool simple)
{
    simpleBtn.setToggleState (simple, juce::dontSendNotification);
    // Simple: hides the secondary controls (glide, input/output trims, pan, detune); all four voices stay visible.
    for (auto* k : { &glideKnob, &inGainKnob, &outGainKnob })
        k->setVisible (! simple);
    for (int v = 0; v < kNumVoices; ++v)
    {
        panKnobs[v]->setVisible (! simple);
        detuneKnobs[v]->setVisible (! simple);
    }
}
