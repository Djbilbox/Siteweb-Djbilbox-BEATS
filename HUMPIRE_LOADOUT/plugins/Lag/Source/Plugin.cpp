// HUMPIRE LAG - delay stereo : sync BPM, feedback filtre, ducking, ping-pong, 4 snapshots A/B/C/D avec morphing
#include <HumpireCore/GenericEditor.h>
#include <HumpireCore/Biquad.h>

using namespace humpire;

namespace
{
constexpr const char* snapIds[7] = { "timeL", "timeR", "fb", "lowcut", "highcut", "duck", "mix" };
const juce::StringArray divNames { "1/16", "1/8", "1/8 D", "1/8 T", "1/4", "1/4 D", "1/2", "1/1" };
constexpr float divBeats[8] = { 0.25f, 0.5f, 0.75f, 1.0f / 3.0f, 1.0f, 1.5f, 2.0f, 4.0f };
}

class LagProcessor : public HumpireProcessor
{
public:
    LagProcessor() : HumpireProcessor ("HUMPIRE LAG", "LAG", makeLayout(), presets_()) {}
    double getTailLengthSeconds() const override { return 30.0; }
    juce::AudioProcessorEditor* createEditor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        l.add (mkFloat ("timeL", "Time L", 1.0f, 2000.0f, 375.0f, 0.4f, "ms", 0));
        l.add (mkFloat ("timeR", "Time R", 1.0f, 2000.0f, 375.0f, 0.4f, "ms", 0));
        l.add (mkBool  ("sync", "Sync", false));
        l.add (mkChoice ("divL", "Division L", divNames, 4));
        l.add (mkChoice ("divR", "Division R", divNames, 5));
        l.add (mkFloat ("fb", "Feedback", 0.0f, 0.95f, 0.35f, 1.0f, "", 2));
        l.add (mkFloat ("lowcut", "Low Cut", 20.0f, 1000.0f, 100.0f, 0.4f, "Hz", 0));
        l.add (mkFloat ("highcut", "High Cut", 1000.0f, 20000.0f, 8000.0f, 0.4f, "Hz", 0));
        l.add (mkFloat ("duck", "Ducking", 0.0f, 1.0f, 0.0f, 1.0f, "", 2));
        l.add (mkFloat ("mix", "Mix", 0.0f, 1.0f, 0.3f, 1.0f, "", 2));
        l.add (mkBool  ("pingpong", "Ping-Pong", false));
        l.add (mkFloat ("morph", "Morph", 20.0f, 3000.0f, 500.0f, 0.5f, "ms", 0));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        auto P = [] (const char* n, const char* c, float tl, float tr, float fb, float lc, float hc, float dk, float mx, bool sync, int dl, int dr, bool pp)
        { return fp (n, c, { { "timeL", tl }, { "timeR", tr }, { "fb", fb }, { "lowcut", lc }, { "highcut", hc }, { "duck", dk }, { "mix", mx },
                             { "sync", sync ? 1.f : 0.f }, { "divL", (float) dl }, { "divR", (float) dr }, { "pingpong", pp ? 1.f : 0.f } }); };
        return {
            P ("Trap Vocal 1/8",    "Trap Vocals", 250, 250, 0.35f, 150, 7000, 0.6f, 0.25f, true, 1, 1, false),
            P ("Ad-Lib Ping-Pong",  "Trap Vocals", 300, 300, 0.45f, 200, 6000, 0.3f, 0.30f, true, 4, 4, true),
            P ("Dotted Hook",       "Trap Vocals", 375, 375, 0.40f, 180, 8000, 0.5f, 0.28f, true, 5, 4, false),
            P ("808 Slap",          "808",       80,  95,  0.15f, 120, 3000, 0.0f, 0.15f, false, 1, 1, false),
            P ("Kick Echo Tight",   "Kick",      60,  60,  0.20f, 200, 5000, 0.0f, 0.12f, false, 0, 0, false),
            P ("Oud Wide Echo",     "Oud",       420, 560, 0.45f, 150, 7500, 0.2f, 0.30f, false, 4, 5, false),
            P ("Oud Desert",        "Oud",       700, 900, 0.55f, 250, 5000, 0.3f, 0.35f, false, 6, 5, true),
            P ("Darbouka Roll",     "Darbouka",  125, 187, 0.40f, 300, 9000, 0.0f, 0.25f, true, 0, 2, true),
            P ("Bus Drums Space",   "Bus Drums", 187, 281, 0.25f, 400, 6000, 0.4f, 0.18f, true, 1, 2, false),
            P ("Master Haze",       "Master",    500, 750, 0.30f, 500, 4000, 0.6f, 0.10f, false, 4, 5, false),
            P ("Dub Throw",         "Master",    375, 375, 0.75f, 300, 3500, 0.2f, 0.40f, true, 4, 4, true),
            P ("Infinite Ghost",    "Master",    1200, 1500, 0.90f, 400, 3000, 0.3f, 0.35f, false, 7, 6, true),
        };
    }

    void prepare (double sr, int) override
    {
        fs = sr;
        const size_t len = (size_t) (8.5 * sr);   // assez pour la division 1/1 jusqu'a ~28 BPM
        for (auto& b : buf) b.assign (len, 0.0f);
        wPos = 0;
        for (auto& s : tSm) s.reset (sr, 0.08);
        for (int c = 0; c < 2; ++c) { hp[c].reset(); lp[c].reset(); }
        env = 0; first = true;
    }

    void process (juce::AudioBuffer<float>& b, juce::MidiBuffer&) override
    {
        const int n = b.getNumSamples(), nch = juce::jmin (2, b.getNumChannels());
        double bpm = 120.0;
        if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) if (auto t = pos->getBpm()) bpm = *t;
        const bool sync = getBool ("sync"), pp = getBool ("pingpong");
        float tMs[2];
        tMs[0] = sync ? (float) (60000.0 / bpm) * divBeats[juce::jlimit (0, 7, juce::roundToInt (get ("divL")))] : get ("timeL");
        tMs[1] = sync ? (float) (60000.0 / bpm) * divBeats[juce::jlimit (0, 7, juce::roundToInt (get ("divR")))] : get ("timeR");
        for (int c = 0; c < 2; ++c)
        {
            const float tS = juce::jlimit (1.0f, (float) (buf[0].size() - 4), tMs[c] * 0.001f * (float) fs);
            if (first) tSm[c].setCurrentAndTargetValue (tS);  else tSm[c].setTargetValue (tS);
        }
        first = false;
        const float fb = get ("fb"), duck = get ("duck"), mix = get ("mix");
        for (int c = 0; c < 2; ++c) { hp[c].set (Biquad::Type::highPass, fs, get ("lowcut"), 0.707, 0); lp[c].set (Biquad::Type::lowPass, fs, get ("highcut"), 0.707, 0); }
        float* ch[2] = { b.getWritePointer (0), nch > 1 ? b.getWritePointer (1) : nullptr };
        const int len = (int) buf[0].size();

        for (int i = 0; i < n; ++i)
        {
            const float in[2] = { ch[0][i], nch > 1 ? ch[1][i] : ch[0][i] };
            env = juce::jmax (std::abs (in[0]) + std::abs (in[1]), env * 0.9997f);
            const float dk = 1.0f - duck * juce::jmin (1.0f, env * 3.0f);
            float wet[2];
            for (int c = 0; c < 2; ++c)
            {
                float p = (float) wPos - tSm[c].getNextValue();  while (p < 0) p += (float) len;  if (p >= (float) len) p -= (float) len;
                const int i0 = (int) p, i1 = (i0 + 1) % len;  const float f = p - (float) i0;
                wet[c] = buf[c][(size_t) i0] * (1.0f - f) + buf[c][(size_t) i1] * f;
            }
            for (int c = 0; c < 2; ++c)
            {
                const float src = pp ? (c == 0 ? in[0] + in[1] : 0.0f) * 0.5f : in[c];          // ping-pong : entree dans L seulement
                const float fbIn = pp ? wet[1 - c] : wet[c];
                const float filtered = lp[c].process (hp[c].process (fbIn));
                buf[c][(size_t) wPos] = std::tanh (src + filtered * fb);                          // tanh = limite douce du feedback
            }
            wPos = (wPos + 1) % len;
            for (int c = 0; c < (nch > 1 ? 2 : 1); ++c)
                ch[c][i] = in[c] + mix * wet[c] * dk;
        }
        aux[0].store (tMs[0]);  aux[1].store (tMs[1]);
    }

    // ---- snapshots (valeurs reelles), editees depuis l'UI
    std::array<std::array<float, 7>, 4> snaps {{ { 375, 375, 0.35f, 100, 8000, 0.0f, 0.30f },
                                                  { 250, 375, 0.50f, 200, 6000, 0.3f, 0.35f },
                                                  { 90,  120, 0.20f, 150, 10000, 0.0f, 0.25f },
                                                  { 800, 1200, 0.70f, 300, 4000, 0.5f, 0.45f } }};
    int activeSnap = 0;

    void saveExtraState (juce::ValueTree& s) override
    {
        juce::StringArray a;  for (auto& sn : snaps) for (float v : sn) a.add (juce::String (v, 4));
        s.setProperty ("snaps", a.joinIntoString (","), nullptr);
        s.setProperty ("snapActive", activeSnap, nullptr);
    }
    void loadExtraState (const juce::ValueTree& s) override
    {
        auto a = juce::StringArray::fromTokens (s.getProperty ("snaps").toString(), ",", "");
        if (a.size() == 28) { int k = 0; for (auto& sn : snaps) for (float& v : sn) v = a[k++].getFloatValue(); }
        activeSnap = juce::jlimit (0, 3, (int) s.getProperty ("snapActive", 0));
    }

    static const char* snapId (int i) { return snapIds[i]; }

private:
    double fs = 44100;
    std::array<std::vector<float>, 2> buf;
    int wPos = 0;
    juce::SmoothedValue<float> tSm[2];
    Biquad hp[2], lp[2];
    float env = 0;  bool first = true;
};

class LagEditor : public GenericEditor
{
public:
    explicit LagEditor (LagProcessor& p) : GenericEditor (p, makeSpec()), lag (p)
    {
        const char* names[4] = { "A", "B", "C", "D" };
        for (int i = 0; i < 4; ++i)
        {
            slot[i] = std::make_unique<ConsoleButton> (names[i], ConsoleButton::Shape::pill, juce::Colour (0xff3a3a44));
            slot[i]->setGlyph (juce::String ("SLOT ") + names[i]);  slot[i]->setShowLed (true);
            slot[i]->setBounds (200 + i * 100, 502, 80, 30);
            slot[i]->onClick = [this, i] { pickSlot (i); };
            content.addAndMakeVisible (*slot[i]);
        }
        store = std::make_unique<ConsoleButton> ("STORE", ConsoleButton::Shape::pill, juce::Colour (0xff8a5cff));
        store->setGlyph ("STORE");  store->setClickingTogglesState (true);  store->setBounds (610, 502, 80, 30);
        content.addAndMakeVisible (*store);
        refreshLeds();
        finishSetup();
    }

    ~LagEditor() override
    {
        if (morphing) for (int k = 0; k < 7; ++k) if (auto* prm = lag.apvts.getParameter (LagProcessor::snapId (k))) prm->endChangeGesture();
    }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "LAG";  s.subtitle = "STEREO DELAY  //  CARTRIDGE SNAPSHOTS";
        s.style = BodyStyle::cartridge;
        s.body1 = juce::Colour (0xff2b2b33);  s.body2 = juce::Colour (0xff0c0c10);  s.accent = col::amber;
        s.knobs = { { "timeL", "TIME L" }, { "timeR", "TIME R" }, { "fb", "FEEDBACK" }, { "lowcut", "LOW CUT", true },
                    { "highcut", "HIGH CUT", true }, { "duck", "DUCK", true }, { "mix", "MIX" }, { "morph", "MORPH", true } };
        s.buttons = { { "sync", "SYNC" }, { "pingpong", "PING-PONG" }, { "divL", "1/4" }, { "divR", "1/4 D" } };
        s.knobY = 312;  s.buttonY = 440;
        return s;
    }

protected:
    void tick() override
    {
        if (morphing)
        {
            morphT += 1.0f / 30.0f / juce::jmax (0.02f, lag.get ("morph") * 0.001f);
            const float k = juce::jmin (1.0f, morphT), e = k * k * (3.0f - 2.0f * k);        // smoothstep
            for (int i = 0; i < 7; ++i)
                if (auto* prm = lag.apvts.getParameter (LagProcessor::snapId (i)))
                    prm->setValueNotifyingHost (prm->convertTo0to1 (from[(size_t) i] + (lag.snaps[(size_t) lag.activeSnap][(size_t) i] - from[(size_t) i]) * e));
            if (k >= 1.0f) { morphing = false; for (int i = 0; i < 7; ++i) if (auto* prm = lag.apvts.getParameter (LagProcessor::snapId (i))) prm->endChangeGesture(); }
        }
        for (int i = 0; i < 4; ++i) slot[i]->setToggleState (i == lag.activeSnap, juce::dontSendNotification);
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float t) override
    {
        using namespace juce;
        g.setColour (Colour (0xff08090c));  g.fillRect (a);
        const float tL = jmax (1.0f, lag.aux[0].load()), tR = jmax (1.0f, lag.aux[1].load()), fb = lag.get ("fb");
        const float span = jmax (tL, tR) * 7.0f;
        const auto lane = [&] (float y, float tm, Colour c, const char* label)
        {
            g.setColour (Colours::white.withAlpha (0.08f));  g.drawLine (a.getX() + 20, y, a.getRight() - 20, y, 1.0f);
            g.setColour (col::textDim);  g.setFont (Theme::uiFont (10.0f, true));  g.drawText (label, (int) a.getX() + 20, (int) y - 66, 30, 12, Justification::centredLeft);
            // source + repetitions qui s'estompent en fantomes
            for (int k = 0; k <= 8; ++k)
            {
                const float x = a.getX() + 40 + (a.getWidth() - 80) * (k * tm) / span;
                if (x > a.getRight() - 20) break;
                const float amp = std::pow (fb, (float) k), h = 10.0f + 54.0f * amp;
                for (int gl = 3; gl >= 0; --gl)                                  // halo "fantome"
                {
                    g.setColour (c.withAlpha (amp * (gl == 0 ? 0.9f : 0.12f)));
                    g.fillRoundedRectangle (x - 4.0f - (float) gl * 3.0f * (float) k * 0.15f, y - h, 8.0f + (float) gl * 6.0f, h, 3.0f);
                }
            }
            // impulsion qui parcourt la ligne
            const float ph = std::fmod (t * 1000.0f / span * 0.7f, 1.0f);
            g.setColour (c.withAlpha (0.8f));  g.fillEllipse (a.getX() + 40 + (a.getWidth() - 80) * ph - 4, y - 4, 8, 8);
        };
        lane (a.getY() + 112, tL, col::amber, "L");
        lane (a.getY() + 214, tR, col::violet, "R");
        g.setColour (col::text);  g.setFont (Theme::lcdFont (11.0f));
        g.drawText (String (roundToInt (tL)) + " ms", (int) a.getX() + 60, (int) a.getY() + 14, 140, 16, Justification::centredLeft);
        g.drawText (String (roundToInt (tR)) + " ms", (int) a.getRight() - 200, (int) a.getY() + 14, 140, 16, Justification::centredRight);
    }

private:
    void pickSlot (int i)
    {
        if (store->getToggleState())                         // STORE : ecrit les reglages courants dans le slot
        {
            for (int k = 0; k < 7; ++k) lag.snaps[(size_t) i][(size_t) k] = lag.get (LagProcessor::snapId (k));
            store->setToggleState (false, juce::dontSendNotification);
            lag.activeSnap = i;
            return;
        }
        for (int k = 0; k < 7; ++k) from[(size_t) k] = lag.get (LagProcessor::snapId (k));
        lag.activeSnap = i;  morphT = 0;
        if (! morphing) for (int k = 0; k < 7; ++k) if (auto* prm = lag.apvts.getParameter (LagProcessor::snapId (k))) prm->beginChangeGesture();
        morphing = true;
    }
    void refreshLeds() { for (int i = 0; i < 4; ++i) slot[i]->setToggleState (i == lag.activeSnap, juce::dontSendNotification); }

    LagProcessor& lag;
    std::unique_ptr<ConsoleButton> slot[4], store;
    std::array<float, 7> from {};
    float morphT = 0;  bool morphing = false;
};

juce::AudioProcessorEditor* LagProcessor::createEditor() { return new LagEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new LagProcessor(); }
