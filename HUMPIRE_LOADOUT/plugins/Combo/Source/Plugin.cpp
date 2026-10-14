// HUMPIRE COMBO - performance : Stutter (sync BPM), Tape Stop, Filter Sweep, Reverse, 2 sticks XY, gachettes drive/mix, MIDI learn
#include <HumpireCore/GenericEditor.h>
#include <HumpireCore/XYPad.h>

using namespace humpire;

namespace { constexpr const char* fxIds[4] = { "stutter", "tapestop", "sweep", "reverse" }; }

class ComboProcessor : public HumpireProcessor
{
public:
    ComboProcessor() : HumpireProcessor ("HUMPIRE COMBO", "COMBO", makeLayout(), presets_(), true) {}
    juce::AudioProcessorEditor* createEditor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        for (int i = 0; i < 4; ++i)
            l.add (mkBool (fxIds[i], (juce::String ("FX ") + fxIds[i]).toRawUTF8(), false));
        l.add (mkFloat ("lx", "Stick L X (Stutter Rate)", 0.0f, 1.0f, 0.5f, 1.0f, "", 2));
        l.add (mkFloat ("ly", "Stick L Y (Filter)", 0.0f, 1.0f, 0.5f, 1.0f, "", 2));
        l.add (mkFloat ("rx", "Stick R X (Tape Time)", 0.0f, 1.0f, 0.5f, 1.0f, "", 2));
        l.add (mkFloat ("ry", "Stick R Y (Reverse Len)", 0.0f, 1.0f, 0.5f, 1.0f, "", 2));
        l.add (mkFloat ("drive", "Trigger L Drive", 0.0f, 1.0f, 0.0f, 1.0f, "", 2));
        l.add (mkFloat ("mix", "Trigger R Mix", 0.0f, 1.0f, 1.0f, 1.0f, "", 2));
        l.add (mkBool  ("learn", "MIDI Learn", false, false));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        auto P = [] (const char* n, const char* c, float lx, float ly, float rx, float ry, float dr, float mx)
        { return fp (n, c, { { "lx", lx }, { "ly", ly }, { "rx", rx }, { "ry", ry }, { "drive", dr }, { "mix", mx } }); };
        return {
            P ("Trap Stutter 1/16",   "Voix Trap", 0.30f, 0.5f, 0.3f, 0.5f, 0.0f, 1.0f), P ("Ad-Lib Throw",       "Voix Trap", 0.60f, 0.3f, 0.5f, 0.5f, 0.2f, 0.9f),
            P ("808 Tape Dive",       "808",       0.50f, 0.5f, 0.8f, 0.5f, 0.0f, 1.0f), P ("808 Slow Stop",       "808",       0.50f, 0.5f, 1.0f, 0.5f, 0.1f, 1.0f),
            P ("Kick Roll 1/32",      "Kick",      0.05f, 0.5f, 0.3f, 0.2f, 0.3f, 1.0f), P ("Kick Reverse",        "Kick",      0.50f, 0.5f, 0.3f, 0.3f, 0.0f, 1.0f),
            P ("Oud Reverse Swell",   "Oud",       0.50f, 0.5f, 0.4f, 0.8f, 0.0f, 0.8f), P ("Oud Filter Close",    "Oud",       0.50f, 0.15f, 0.4f, 0.5f, 0.0f, 1.0f),
            P ("Darbouka Stutter",    "Darbouka",  0.30f, 0.6f, 0.3f, 0.4f, 0.2f, 1.0f), P ("Bus Drums Freeze",    "Bus Drums", 0.80f, 0.5f, 0.4f, 0.5f, 0.0f, 1.0f),
            P ("Bus Filter Drop",     "Bus Drums", 0.50f, 0.05f, 0.4f, 0.5f, 0.0f, 1.0f),P ("Master Tape Stop",     "Master",    0.50f, 0.5f, 0.6f, 0.5f, 0.0f, 1.0f),
        };
    }

    void prepare (double sr, int) override
    {
        fs = sr;  len = (int) (4.0 * sr);
        for (auto& b : buf) b.assign ((size_t) len, 0.0f);
        wPos = 0;
        for (auto& f : svf) { f.reset(); f.prepare ({ sr, 512, 1 }); f.setType (juce::dsp::StateVariableTPTFilterType::lowpass); f.setResonance (2.5f); }
        for (auto& e : eng) e = 0.0f;
        sweepAmt = 0; was[0] = was[1] = was[2] = was[3] = false;
    }

    void process (juce::AudioBuffer<float>& b, juce::MidiBuffer& midi) override
    {
        // ---- MIDI : notes maintenues (defaut 36..39) + apprentissage
        for (const auto meta : midi)
        {
            const auto m = meta.getMessage();
            if (m.isNoteOn())
            {
                const int tgt = learnTarget.load();
                if (tgt >= 0) { noteMap[tgt].store (m.getNoteNumber()); learnTarget.store (-1); learnDone.store (true); }
                for (int i = 0; i < 4; ++i) if (m.getNoteNumber() == noteMap[i].load()) held |= (1 << i);
            }
            else if (m.isNoteOff())
                for (int i = 0; i < 4; ++i) if (m.getNoteNumber() == noteMap[i].load()) held &= ~(1 << i);
        }
        bool act[4];
        for (int i = 0; i < 4; ++i) act[i] = getBool (fxIds[i]) || (held & (1 << i));

        double bpm = 120.0;
        if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) if (auto t = pos->getBpm()) bpm = *t;
        const float lx = get ("lx"), ly = get ("ly"), rx = get ("rx"), ry = get ("ry"), drive = get ("drive"), mix = get ("mix");
        static constexpr float stutBeats[4] = { 0.125f, 0.25f, 0.5f, 1.0f }, revBeats[4] = { 0.25f, 0.5f, 1.0f, 2.0f };
        const int stutLen = juce::jlimit (64, len / 2, (int) (stutBeats[juce::jlimit (0, 3, (int) (lx * 4.0f))] * 60.0 / bpm * fs));
        const int revLen  = juce::jlimit (64, len / 2, (int) (revBeats[juce::jlimit (0, 3, (int) (ry * 4.0f))] * 60.0 / bpm * fs));
        const float stopTime = 0.1f + 1.9f * rx;
        const float cutTarget = 40.0f * std::pow (18000.0f / 40.0f, ly);

        // fronts montants -> capture
        if (act[0] && ! was[0]) { stutStart = (wPos - stutLen + len) % len; stutPos = 0; }
        if (act[1] && ! was[1]) { tapePos = (float) wPos; tapeSpeed = 1.0f; }
        if (act[3] && ! was[3]) { revPos = (float) (wPos - 1 + len); revCount = 0; }
        for (int i = 0; i < 4; ++i) was[i] = act[i];

        const int n = b.getNumSamples(), nch = juce::jmin (2, b.getNumChannels());
        float* ch[2] = { b.getWritePointer (0), nch > 1 ? b.getWritePointer (1) : nullptr };
        const float up = 1.0f - std::exp (-1.0f / (0.003f * (float) fs)), dn = 1.0f - std::exp (-1.0f / (0.012f * (float) fs));
        auto readBuf = [this] (int c, float p) { while (p < 0) p += (float) len; while (p >= (float) len) p -= (float) len;
                                                  const int i0 = (int) p, i1 = (i0 + 1) % len;  const float f = p - (float) i0;
                                                  return buf[(size_t) c][(size_t) i0] * (1 - f) + buf[(size_t) c][(size_t) i1] * f; };
        const float driveG = 1.0f + drive * 9.0f;

        for (int i = 0; i < n; ++i)
        {
            float dry[2] { ch[0][i], nch > 1 ? ch[1][i] : ch[0][i] };
            for (int c = 0; c < 2; ++c) buf[(size_t) c][(size_t) wPos] = dry[c];

            for (int k = 0; k < 4; ++k) eng[k] += ((act[k] ? 1.0f : 0.0f) - eng[k]) * (act[k] ? up : dn);
            sweepAmt += ((act[2] ? 1.0f : 0.0f) - sweepAmt) * (1.0f / (0.35f * (float) fs)) * (act[2] ? 1.0f : 3.0f);
            svf[0].setCutoffFrequency (18000.0f * std::pow (cutTarget / 18000.0f, juce::jlimit (0.0f, 1.0f, sweepAmt)));
            svf[1].setCutoffFrequency (svf[0].getCutoffFrequency());

            float s[2] { dry[0], dry[1] };
            // tape stop
            if (eng[1] > 0.001f)
            {
                tapeSpeed = juce::jmax (0.0f, tapeSpeed - 1.0f / (stopTime * (float) fs));
                if (! act[1]) tapeSpeed = 1.0f;
                tapePos += tapeSpeed;
                for (int c = 0; c < 2; ++c) s[c] += (readBuf (c, tapePos) - s[c]) * eng[1];
            }
            // stutter
            if (eng[0] > 0.001f)
            {
                const int idx = (stutStart + stutPos) % len;
                const float fade = juce::jmin (1.0f, juce::jmin ((float) stutPos, (float) (stutLen - stutPos)) / 48.0f);
                for (int c = 0; c < 2; ++c) s[c] += (buf[(size_t) c][(size_t) idx] * fade - s[c]) * eng[0];
                if (++stutPos >= stutLen) stutPos = 0;
            }
            // reverse
            if (eng[3] > 0.001f)
            {
                for (int c = 0; c < 2; ++c) s[c] += (readBuf (c, revPos) - s[c]) * eng[3];
                revPos -= 1.0f;
                if (++revCount >= revLen) { revCount = 0; revPos = (float) (wPos - 1 + len); }
            }
            // sweep
            float f2[2] { svf[0].processSample (0, s[0]), svf[1].processSample (0, s[1]) };
            for (int c = 0; c < 2; ++c) s[c] += (f2[c] - s[c]) * eng[2];

            const float any = juce::jmax (juce::jmax (eng[0], eng[1]), juce::jmax (eng[2], eng[3]));
            for (int c = 0; c < (nch > 1 ? 2 : 1); ++c)
            {
                float y = s[c];
                if (drive > 0.001f) y += any * (std::tanh (y * driveG) / std::sqrt (driveG) - y);
                ch[c][i] = dry[c] + any * mix * (y - dry[c]);
            }
            wPos = (wPos + 1) % len;
        }
        aux[0].store ((float) ((act[0] ? 1 : 0) | (act[1] ? 2 : 0) | (act[2] ? 4 : 0) | (act[3] ? 8 : 0)));
        aux[1].store ((float) stutLen / (float) fs);
        aux[2].store (cutTarget);
    }

    std::atomic<int> learnTarget { -1 }, noteMap[4] { 36, 37, 38, 39 };
    std::atomic<bool> learnDone { false };

    void saveExtraState (juce::ValueTree& s) override
    { s.setProperty ("notes", juce::String (noteMap[0].load()) + "," + juce::String (noteMap[1].load()) + "," + juce::String (noteMap[2].load()) + "," + juce::String (noteMap[3].load()), nullptr); }
    void loadExtraState (const juce::ValueTree& s) override
    {
        auto a = juce::StringArray::fromTokens (s.getProperty ("notes").toString(), ",", "");
        if (a.size() == 4) for (int i = 0; i < 4; ++i) noteMap[i].store (a[i].getIntValue());
    }

private:
    double fs = 44100;  int len = 176400, wPos = 0, held = 0;
    std::array<std::vector<float>, 2> buf;
    juce::dsp::StateVariableTPTFilter<float> svf[2];
    float eng[4] {}, sweepAmt = 0, tapePos = 0, tapeSpeed = 1, revPos = 0;
    int stutStart = 0, stutPos = 0, revCount = 0;
    bool was[4] {};
};

class ComboEditor : public GenericEditor
{
public:
    explicit ComboEditor (ComboProcessor& p) : GenericEditor (p, makeSpec()), combo (p),
        padL (p.apvts, "lx", "ly", true, true), padR (p.apvts, "rx", "ry", true, true)
    {
        padL.setBounds (110, 290, 200, 200);  padR.setBounds (510, 290, 200, 200);
        content.addAndMakeVisible (padL);  content.addAndMakeVisible (padR);
        for (int i = 0; i < 2; ++i)
        {
            auto& s = trig[i];
            s.setSliderStyle (juce::Slider::LinearVertical);  s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s.setColour (juce::Slider::thumbColourId, col::amber);  s.setColour (juce::Slider::trackColourId, col::orange);
            s.setColour (juce::Slider::backgroundColourId, juce::Colours::black);
            s.setBounds (i == 0 ? 345 : 435, 290, 40, 200);
            content.addAndMakeVisible (s);
            trigAtt[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, i == 0 ? "drive" : "mix", s);
            lbl[i].setText (i == 0 ? "L TRIG: DRIVE" : "R TRIG: MIX", juce::dontSendNotification);
            lbl[i].setJustificationType (juce::Justification::centred);  lbl[i].setColour (juce::Label::textColourId, col::amber);
            lbl[i].setFont (Theme::uiFont (10.0f, true));
            lbl[i].setBounds (i == 0 ? 320 : 410, 492, 90, 14);  content.addAndMakeVisible (lbl[i]);
        }
        const char* t[2] = { "L STICK  X: STUTTER RATE  Y: FILTER", "R STICK  X: TAPE TIME  Y: REVERSE LEN" };
        for (int i = 0; i < 2; ++i)
        { stk[i].setText (t[i], juce::dontSendNotification); stk[i].setJustificationType (juce::Justification::centred);
          stk[i].setColour (juce::Label::textColourId, col::amber); stk[i].setFont (Theme::uiFont (10.0f, true));
          stk[i].setBounds (i == 0 ? 60 : 460, 496, 300, 14); content.addAndMakeVisible (stk[i]); }
        finishSetup();
    }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "COMBO";  s.subtitle = "PERFORMANCE CONTROLLER  //  MIDI NOTES 36-39";
        s.style = BodyStyle::controller;
        s.body1 = juce::Colour (0xff1b1b1f);  s.body2 = juce::Colour (0xff060608);  s.accent = col::amber;
        s.display = { 270, 64, 280, 130 };
        s.buttons = { { "stutter", "STUTTER", true }, { "tapestop", "TAPE STOP", true }, { "sweep", "SWEEP", true }, { "reverse", "REVERSE", true }, { "learn", "MIDI LEARN" } };
        s.buttonY = 216;  s.knobY = 0;
        return s;
    }

protected:
    void tick() override
    {
        mask = (int) combo.aux[0].load();
        if (combo.getBool ("learn") && combo.learnTarget.load() < 0)
            for (int i = 0; i < 4; ++i) if (combo.getBool (fxIds[i])) combo.learnTarget.store (i);
        if (combo.learnDone.exchange (false))
            if (auto* pl = combo.apvts.getParameter ("learn")) pl->setValueNotifyingHost (0.0f);
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float t) override
    {
        using namespace juce;
        g.setColour (Colour (0xff020203));  g.fillRect (a);
        g.setColour (col::amber);  g.setFont (Theme::lcdFont (9.0f));
        const char* names[4] = { "STUTTER", "TAPE", "SWEEP", "REVERSE" };
        for (int i = 0; i < 4; ++i)
        {
            const bool on = (mask >> i) & 1;
            g.setColour (on ? col::amber : col::amber.withAlpha (0.18f));
            g.drawText (names[i], (int) a.getX() + 12, (int) a.getY() + 12 + i * 16, 110, 14, Justification::centredLeft);
            if (on) g.fillEllipse (a.getX() + 128, a.getY() + 15 + i * 16, 8, 8);
        }
        g.setColour (col::text.withAlpha (0.8f));  g.setFont (Theme::lcdFont (8.0f));
        g.drawText (String (combo.aux[1].load() * 1000.0f, 0) + " MS", (int) a.getX() + 150, (int) a.getY() + 12, 120, 12, Justification::centredRight);
        g.drawText (String (combo.aux[2].load(), 0) + " HZ", (int) a.getX() + 150, (int) a.getY() + 28, 120, 12, Justification::centredRight);
        if (combo.getBool ("learn")) { g.setColour (col::violet.withAlpha (0.5f + 0.5f * std::sin (t * 8.0f))); g.drawText ("LEARN: PRESS FX THEN NOTE", a.toNearestInt().removeFromBottom (24), Justification::centred); }
        else { g.setColour (col::textDim); g.drawText ("NOTES " + String (combo.noteMap[0].load()) + "-" + String (combo.noteMap[3].load()), a.toNearestInt().removeFromBottom (24), Justification::centred); }
    }

private:
    ComboProcessor& combo;
    XYPad padL, padR;
    juce::Slider trig[2];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trigAtt[2];
    juce::Label lbl[2], stk[2];
    int mask = 0;
};

juce::AudioProcessorEditor* ComboProcessor::createEditor() { return new ComboEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ComboProcessor(); }
