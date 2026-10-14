// HUMPIRE CLUTCH - compresseur Clean / Punch / Crush (boitier hardware noir et or, barre de vie LED)
#include <HumpireCore/GenericEditor.h>

using namespace humpire;

class ClutchProcessor : public HumpireProcessor
{
public:
    ClutchProcessor() : HumpireProcessor ("HUMPIRE CLUTCH", "CLUTCH", makeLayout(), presets_()) {}

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        l.add (mkChoice ("mode", "Mode", { "Clean", "Punch", "Crush" }, 0));
        l.add (mkFloat ("thresh",  "Threshold", -40.0f, 0.0f, -18.0f, 1.0f, "dB"));
        l.add (mkFloat ("ratio",   "Ratio", 1.0f, 20.0f, 4.0f, 0.5f, ":1"));
        l.add (mkFloat ("attack",  "Attack", 0.1f, 100.0f, 10.0f, 0.4f, "ms"));
        l.add (mkFloat ("release", "Release", 10.0f, 1000.0f, 120.0f, 0.4f, "ms", 0));
        l.add (mkFloat ("knee",    "Knee", 0.0f, 24.0f, 6.0f, 1.0f, "dB"));
        l.add (mkFloat ("makeup",  "Makeup", -12.0f, 24.0f, 0.0f, 1.0f, "dB"));
        l.add (mkFloat ("schpf",   "SC HPF", 20.0f, 500.0f, 60.0f, 0.5f, "Hz", 0));
        l.add (mkFloat ("mix",     "Mix", 0.0f, 1.0f, 1.0f, 1.0f, "", 2));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        //                 nom                 categorie      mode  thr    ratio att   rel   knee mk   hpf mix
        auto P = [] (const char* n, const char* c, float m, float t, float r, float a, float rl, float k, float mk, float h, float mx)
        { return fp (n, c, { { "mode", m }, { "thresh", t }, { "ratio", r }, { "attack", a }, { "release", rl }, { "knee", k }, { "makeup", mk }, { "schpf", h }, { "mix", mx } }); };
        return {
            P ("Voix Trap Lead",    "Voix Trap", 0, -20, 4, 8,   140, 6, 4, 90, 1.0f),
            P ("Ad-Lib Squash",     "Voix Trap", 2, -26, 8, 2,   90,  3, 6, 120, 0.7f),
            P ("Vocal Glue",        "Voix Trap", 0, -16, 2.5f, 15, 200, 10, 2, 80, 1.0f),
            P ("808 Tamer",         "808",       0, -14, 6, 20,  250, 4, 2, 30, 1.0f),
            P ("808 Sustain",       "808",       1, -22, 3, 30,  400, 8, 5, 25, 1.0f),
            P ("Kick Punch",        "Kick",      1, -18, 5, 25,  80,  2, 3, 60, 1.0f),
            P ("Kick Crush",        "Kick",      2, -24, 10, 5,  60,  0, 5, 70, 0.6f),
            P ("Oud Smooth",        "Oud",       0, -18, 3, 12,  180, 12, 3, 100, 1.0f),
            P ("Darbouka Snap",     "Darbouka",  1, -20, 4, 18,  100, 3, 3, 150, 1.0f),
            P ("Bus Drums Glue",    "Bus Drums", 0, -14, 2, 30,  150, 10, 2, 90, 1.0f),
            P ("Parallel Smash",    "Bus Drums", 2, -30, 12, 1,  120, 0, 8, 100, 0.4f),
            P ("Master Gentle",     "Master",    0, -10, 1.8f, 30, 300, 14, 1, 40, 1.0f),
        };
    }

    juce::AudioProcessorEditor* createEditor() override;

    void prepare (double sr, int) override { fs = sr; grDb = 0.0f; for (auto& h : hpY) h = hpX = 0.0f; hpX = 0; flash = 0; }

    void process (juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        const int mode = juce::roundToInt (get ("mode"));
        const float thr = get ("thresh"), knee = get ("knee"), makeup = get ("makeup"), mix = get ("mix");
        float ratio = get ("ratio"), atk = get ("attack"), rel = get ("release");
        if (mode == 1) { atk = atk * 3.0f + 4.0f; rel *= 0.7f; }          // Punch : laisse passer l'attaque
        if (mode == 2) { ratio = juce::jmin (30.0f, ratio * 1.8f); atk = juce::jmax (0.05f, atk * 0.3f); }  // Crush
        const float aA = std::exp (-1.0f / (0.001f * atk * (float) fs)), aR = std::exp (-1.0f / (0.001f * rel * (float) fs));
        const float hpA = std::exp (-6.2831853f * get ("schpf") / (float) fs);
        const float mk = juce::Decibels::decibelsToGain (makeup);

        const int n = buf.getNumSamples(), nch = juce::jmin (2, buf.getNumChannels());
        float* ch[2] = { buf.getWritePointer (0), nch > 1 ? buf.getWritePointer (1) : nullptr };
        float minGr = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            float sc = 0.0f;
            for (int c = 0; c < nch; ++c)                     // sidechain HPF 1 pole, liaison stereo (max)
            {
                const float x = ch[c][i];
                hpY[c] = hpA * (hpY[c] + x - hpXs[c]);  hpXs[c] = x;
                sc = juce::jmax (sc, std::abs (hpY[c]));
            }
            const float lvl = 20.0f * std::log10 (juce::jmax (sc, 1.0e-6f));
            const float over = lvl - thr, w = knee;           // gain computer (soft knee)
            float outDb;
            if (2.0f * over < -w)            outDb = lvl;
            else if (2.0f * std::abs (over) <= w) outDb = lvl + (1.0f / ratio - 1.0f) * (over + w * 0.5f) * (over + w * 0.5f) / (2.0f * juce::jmax (w, 1.0e-3f));
            else                             outDb = thr + over / ratio;
            const float target = outDb - lvl;                // <= 0
            grDb = target < grDb ? aA * grDb + (1.0f - aA) * target : aR * grDb + (1.0f - aR) * target;
            minGr = juce::jmin (minGr, grDb);

            const float g = juce::Decibels::decibelsToGain (grDb) * mk;
            for (int c = 0; c < nch; ++c)
            {
                const float dry = ch[c][i];
                float wet = dry * g;
                if (mode == 2) wet = std::tanh (wet * 1.6f) * 0.75f;      // Crush : saturation
                ch[c][i] = dry + mix * (wet - dry);
            }
        }
        aux[0].store (minGr);
    }

private:
    double fs = 44100.0;
    float grDb = 0.0f, hpY[2] { 0, 0 }, hpXs[2] { 0, 0 }, hpX = 0, flash = 0;
};

class ClutchEditor : public GenericEditor
{
public:
    explicit ClutchEditor (ClutchProcessor& p) : GenericEditor (p, makeSpec())
    {
        hist.fill (0.0f);
        finishSetup();
    }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "CLUTCH";  s.subtitle = "DYNAMICS UNIT  //  CLEAN - PUNCH - CRUSH";
        s.style = BodyStyle::rack;
        s.body1 = juce::Colour (0xff25252b);  s.body2 = juce::Colour (0xff09090c);  s.accent = col::amber;
        s.knobs = { { "thresh", "THRESH" }, { "ratio", "RATIO" }, { "attack", "ATTACK" }, { "release", "RELEASE" },
                    { "knee", "KNEE", true }, { "schpf", "SC HPF", true }, { "makeup", "MAKEUP" }, { "mix", "MIX" } };
        s.buttons = { { "mode", "CLEAN" } };
        return s;
    }

protected:
    void tick() override
    {
        const float gr = -proc.aux[0].load();                         // >= 0
        if (gr > shown + 0.6f) flash = 1.0f;                          // coup encaisse
        shown = juce::jmax (gr, shown * 0.93f);
        flash *= 0.85f;
        hist[(size_t) (histPos++ % (int) hist.size())] = gr;
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float) override
    {
        using namespace juce;
        g.setColour (Colour (0xff0a0f0c));  g.fillRect (a);
        g.setColour (col::amber);  g.setFont (Theme::lcdFont (11.0f));
        g.drawText ("VITALITY", a.getX() + 18, a.getY() + 12, 200, 16, Justification::centredLeft);
        g.drawText ("GR -" + String (shown, 1) + " dB", a.getRight() - 220, a.getY() + 12, 200, 16, Justification::centredRight);

        // barre de vie : 32 segments, se vide avec la reduction de gain (0..24 dB)
        const int N = 32;
        const float life = 1.0f - jlimit (0.0f, 1.0f, shown / 24.0f);
        const auto bar = Rectangle<float> (a.getX() + 18, a.getY() + 40, a.getWidth() - 36, 44);
        const float sw = (bar.getWidth() - (N - 1) * 3.0f) / N;
        for (int i = 0; i < N; ++i)
        {
            const float f = (float) i / N;
            const auto c = f < 0.35f ? col::red : (f < 0.65f ? col::orange : col::amber);
            const bool on = f < life;
            g.setColour (on ? c.interpolatedWith (Colours::white, flash * 0.8f) : c.withAlpha (0.08f));
            g.fillRoundedRectangle (bar.getX() + i * (sw + 3.0f), bar.getY(), sw, bar.getHeight(), 2.0f);
        }
        if (flash > 0.2f) { g.setColour (Colours::white.withAlpha (flash * 0.9f)); g.setFont (Theme::lcdFont (16.0f));
                            g.drawText ("HIT!", a.getCentreX() - 60, a.getY() + 94, 120, 22, Justification::centred); }

        // historique de reduction
        const auto gr = Rectangle<float> (a.getX() + 18, a.getY() + 124, a.getWidth() - 36, a.getHeight() - 140);
        g.setColour (Colours::white.withAlpha (0.06f));  g.drawRect (gr, 1.0f);
        Path p;
        for (int i = 0; i < (int) hist.size(); ++i)
        {
            const float v = hist[(size_t) ((histPos + i) % (int) hist.size())];
            const float x = gr.getX() + gr.getWidth() * i / (float) (hist.size() - 1);
            const float y = gr.getY() + gr.getHeight() * jlimit (0.0f, 1.0f, v / 24.0f);
            i == 0 ? p.startNewSubPath (x, y) : p.lineTo (x, y);
        }
        g.setColour (col::amber);  g.strokePath (p, PathStrokeType (2.0f));
    }

private:
    float shown = 0.0f, flash = 0.0f;
    std::array<float, 240> hist;
    int histPos = 0;
};

juce::AudioProcessorEditor* ClutchProcessor::createEditor() { return new ClutchEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ClutchProcessor(); }
