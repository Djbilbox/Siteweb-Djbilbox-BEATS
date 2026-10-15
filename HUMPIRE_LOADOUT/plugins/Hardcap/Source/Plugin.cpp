// HUMPIRE HARDCAP - limiteur master : lookahead, true-peak (interpolation cubique), auto-release, meter LUFS (BS.1770 / EBU R128)
#include <HumpireCore/GenericEditor.h>
#include <HumpireCore/Biquad.h>
#include <deque>

using namespace humpire;

class HardcapProcessor : public HumpireProcessor
{
public:
    HardcapProcessor() : HumpireProcessor ("HUMPIRE HARDCAP", "HARDCAP", makeLayout(), presets_()) {}
    juce::AudioProcessorEditor* createEditor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        l.add (mkFloat ("gain",    "Input Gain", 0.0f, 24.0f, 0.0f, 1.0f, "dB"));
        l.add (mkFloat ("ceiling", "Ceiling",   -6.0f, 0.0f, -1.0f, 1.0f, "dBTP"));
        l.add (mkFloat ("release", "Release",   10.0f, 500.0f, 100.0f, 0.5f, "ms", 0));
        l.add (mkFloat ("look",    "Lookahead", 1.0f, 5.0f, 2.0f, 1.0f, "ms"));
        l.add (mkFloat ("target",  "Target LUFS", -24.0f, -6.0f, -14.0f, 1.0f, "LUFS"));
        l.add (mkBool  ("autorel", "Auto Release", true));
        l.add (mkBool  ("truepeak", "True Peak", true));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        auto P = [] (const char* n, const char* c, float g, float ce, float r, float lk, float tg)
        { return fp (n, c, { { "gain", g }, { "ceiling", ce }, { "release", r }, { "look", lk }, { "target", tg }, { "autorel", 1 }, { "truepeak", 1 } }); };
        return {
            P ("Streaming -14",     "Master",    3, -1.0f, 120, 2, -14), P ("Club Loud -9",     "Master",    7, -0.5f, 80, 2, -9),
            P ("Afrotrap Master",   "Master",    5, -1.0f, 90, 2, -10),  P ("Deep House Smooth","Master",    3, -1.0f, 200, 3, -11),
            P ("Oriental Dynamic",  "Master",    2, -1.0f, 250, 3, -14), P ("G-Funk Warm",      "Master",    3, -1.0f, 150, 2, -12),
            P ("Hip-Hop Punch",     "Master",    5, -1.0f, 60, 1.5f, -9),P ("Bus Drums Catch",  "Bus Drums", 4, -2.0f, 50, 1.5f, -14),
            P ("808 Guard",         "808",       2, -1.5f, 140, 3, -14), P ("Trap Vocal Safety", "Trap Vocals", 3, -2.0f, 100, 2, -16),
            P ("Kick Ceiling",      "Kick",      3, -1.0f, 40, 1, -14),  P ("Safe -16 LUFS",    "Master",    0, -1.5f, 150, 2, -16),
        };
    }

    void prepare (double sr, int) override
    {
        fs = sr;
        maxL = (int) (0.005 * sr) + 4;
        delay.assign ((size_t) (2 * (maxL + 8)), 0.0f);  dPos = 0;
        gTarget.assign ((size_t) (maxL + 8), 1.0f);  gMin.assign ((size_t) (maxL + 8), 1.0f);  tPos = 0;
        boxSum = 0; env = 1.0f; grAvg = 0;
        for (auto& h : hist) h.fill (0.0f);
        // K-weighting (BS.1770) : shelf +4 dB @1.68k puis HP 38 Hz
        for (int c = 0; c < 2; ++c) { kShelf[c].set (Biquad::Type::highShelf, sr, 1681.97, 0.7072, 3.9998); kHp[c].set (Biquad::Type::highPass, sr, 38.135, 0.5003, 0); kShelf[c].reset(); kHp[c].reset(); }
        blockSamples = (int) (0.1 * sr);  sumSq = 0;  cnt = 0;
        sub.assign (30, 0.0);  blocks.clear();  integrated = shortTerm = momentary = -70.0f;  gatedSum = 0;  gatedN = 0;  lastLevel = 0;
        updateLatency();
    }

    void updateLatency()
    {
        lookSamples = juce::jlimit (1, maxL, (int) (get ("look") * 0.001f * (float) fs));
        L = juce::jmax (1, lookSamples / 2 + 1);
        setLatencySamples (L - 1);
    }

    void process (juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        const int n = buf.getNumSamples(), nch = juce::jmin (2, buf.getNumChannels());
        const int newL = juce::jmax (1, juce::jlimit (1, maxL, (int) (get ("look") * 0.001f * (float) fs)) / 2 + 1);
        if (newL != L) { L = newL; setLatencySamples (L - 1); }
        const float inG = juce::Decibels::decibelsToGain (get ("gain")), ceil = juce::Decibels::decibelsToGain (get ("ceiling"));
        const bool autoRel = getBool ("autorel"), tp = getBool ("truepeak");
        const float relMs = get ("release");
        const int ringN = (int) gTarget.size(), dN = ringN;
        float* ch[2] = { buf.getWritePointer (0), nch > 1 ? buf.getWritePointer (1) : nullptr };
        float minGr = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            // --- entree + historique 4 points par canal (true-peak)
            float peak = 0.0f;
            for (int c = 0; c < nch; ++c)
            {
                const float x = ch[c][i] * inG;
                auto& h = hist[(size_t) c];  h[0] = h[1]; h[1] = h[2]; h[2] = h[3]; h[3] = x;
                peak = juce::jmax (peak, std::abs (x));
                if (tp)   // Catmull-Rom entre h[1] et h[2] a 0.25 / 0.5 / 0.75
                    for (float f : { 0.25f, 0.5f, 0.75f })
                    {
                        const float a = -0.5f * h[0] + 1.5f * h[1] - 1.5f * h[2] + 0.5f * h[3], b = h[0] - 2.5f * h[1] + 2.0f * h[2] - 0.5f * h[3], cc = -0.5f * h[0] + 0.5f * h[2];
                        peak = juce::jmax (peak, std::abs (((a * f + b) * f + cc) * f + h[1]));
                    }
                delay[(size_t) (c * dN + dPos)] = x;
            }
            // --- gain cible
            const float gt = peak > ceil ? ceil / peak : 1.0f;
            gTarget[(size_t) tPos] = gt;
            // min glissant sur L derniers
            float m = 1.0f;
            for (int k = 0; k < L; ++k) m = juce::jmin (m, gTarget[(size_t) ((tPos - k + ringN) % ringN)]);
            gMin[(size_t) tPos] = m;
            float sum = 0.0f;                                         // moyenne glissante de gMin sur L
            for (int k = 0; k < L; ++k) sum += gMin[(size_t) ((tPos - k + ringN) % ringN)];
            const float avg = sum / (float) L;

            // --- release (attaque instantanee via lookahead)
            float rel = relMs;
            if (autoRel) { grAvg += ((1.0f - avg) - grAvg) * (1.0f / (0.5f * (float) fs)); rel = 40.0f + 260.0f * juce::jmin (1.0f, grAvg * 6.0f); }
            const float rc = 1.0f - std::exp (-1.0f / (0.001f * rel * (float) fs));
            env = avg < env ? avg : env + (avg - env) * rc;
            // --- sortie retardee de L-1
            for (int c = 0; c < nch; ++c)
            {
                float y = delay[(size_t) (c * dN + ((dPos - (L - 1) + dN) % dN))] * env;
                y = juce::jlimit (-ceil, ceil, y);
                ch[c][i] = y;
            }
            minGr = juce::jmin (minGr, 20.0f * std::log10 (juce::jmax (env, 1.0e-4f)));
            dPos = (dPos + 1) % dN;  tPos = (tPos + 1) % ringN;

            // --- LUFS : K-weighting + energie, blocs de 100 ms (4 blocs = 400 ms)
            float e = 0.0f;
            for (int c = 0; c < nch; ++c) { const float k = kHp[c].process (kShelf[c].process (ch[c][i])); e += k * k; }
            sumSq += e;
            if (++cnt >= blockSamples) { pushBlock ((double) sumSq / (double) cnt); sumSq = 0; cnt = 0; }
        }
        aux[0].store (minGr);
        aux[1].store (shortTerm);  aux[2].store (integrated);  aux[3].store (momentary);
    }

    float shortTerm = -70, integrated = -70, momentary = -70;   // ecrits par l'audio, lus par l'UI (valeurs dans aux)

private:
    void pushBlock (double ms)
    {
        for (size_t i = 0; i + 1 < sub.size(); ++i) sub[i] = sub[i + 1];
        sub.back() = ms;                                           // 30 blocs de 100 ms = 3 s
        ++fill100;
        auto lufs = [] (double mean) { return mean > 1e-10 ? (float) (-0.691 + 10.0 * std::log10 (mean)) : -70.0f; };
        double s4 = 0; for (size_t i = sub.size() - 4; i < sub.size(); ++i) s4 += sub[i];
        double s30 = 0; for (auto v : sub) s30 += v;
        momentary = lufs (s4 / 4.0);
        shortTerm = lufs (s30 / (double) sub.size());
        if (fill100 % 1 == 0 && momentary > -70.0f)                // gating absolu -70, relatif -10 LU (approx. sur blocs 400 ms)
        {
            blocks.push_back (s4 / 4.0);
            if (blocks.size() > 36000) blocks.pop_front();
            double abs = 0; int na = 0;
            for (auto v : blocks) if (lufs (v) > -70.0f) { abs += v; ++na; }
            if (na > 0)
            {
                const float rel = lufs (abs / na) - 10.0f;
                double g = 0; int ng = 0;
                for (auto v : blocks) if (lufs (v) > rel) { g += v; ++ng; }
                if (ng > 0) integrated = lufs (g / ng);
            }
        }
    }

    double fs = 44100;
    int maxL = 256, L = 50, lookSamples = 100, dPos = 0, tPos = 0, filled = 0, blockSamples = 4410, cnt = 0, fill100 = 0;
    std::vector<float> delay, gTarget, gMin;
    float boxSum = 0, env = 1, grAvg = 0, sumSq = 0, gatedSum = 0, lastLevel = 0;
    int gatedN = 0;
    std::array<std::array<float, 4>, 2> hist {};
    Biquad kShelf[2], kHp[2];
    std::vector<double> sub = std::vector<double> (30, 0.0);
    std::deque<double> blocks;
};

class HardcapEditor : public GenericEditor
{
public:
    explicit HardcapEditor (HardcapProcessor& p) : GenericEditor (p, makeSpec()) { finishSetup(); }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "HARDCAP";  s.subtitle = "MASTER LIMITER  //  TRUE-PEAK - LUFS EBU R128";
        s.style = BodyStyle::speaker;
        s.body1 = juce::Colour (0xff17171c);  s.body2 = juce::Colour (0xff050507);  s.accent = col::amber;
        s.knobs = { { "gain", "GAIN" }, { "ceiling", "CEILING" }, { "release", "RELEASE" }, { "look", "LOOKAHEAD", true }, { "target", "TARGET" } };
        s.buttons = { { "autorel", "AUTO REL" }, { "truepeak", "TRUE PEAK" } };
        s.buttonY = 440;
        return s;
    }

protected:
    void tick() override
    {
        gr = juce::jmax (-proc.aux[0].load(), gr * 0.9f);
        st = proc.aux[1].load();  integ = proc.aux[2].load();  mom = proc.aux[3].load();
        const float target = proc.get ("target");
        const bool ok = integ > -69.0f && std::abs (integ - target) <= 0.5f;
        okTime = ok ? okTime + 1 : 0;
        if (okTime == 60) levelUp = 90;                                  // LEVEL UP apres 2 s dans la cible
        if (levelUp > 0) --levelUp;
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float) override
    {
        using namespace juce;
        g.setColour (Colour (0xff09090c));  g.fillRect (a);
        const float target = proc.get ("target");
        auto big = [&] (const String& label, float v, float x, bool hi)
        {
            g.setColour (col::textDim);  g.setFont (Theme::uiFont (11.0f, true));
            g.drawText (label, (int) x, (int) a.getY() + 14, 200, 14, Justification::centred);
            g.setColour (hi ? col::amber : col::text);  g.setFont (Theme::lcdFont (26.0f));
            g.drawText (v <= -69.0f ? "---" : String (v, 1), (int) x, (int) a.getY() + 34, 200, 36, Justification::centred);
        };
        big ("MOMENTARY", mom, a.getX() + 20, false);
        big ("SHORT-TERM", st, a.getCentreX() - 100, false);
        big ("INTEGRATED", integ, a.getRight() - 220, true);

        // bande de LED de niveau (-30..-0 LUFS) + repere cible
        const auto bar = Rectangle<float> (a.getX() + 24, a.getY() + 100, a.getWidth() - 48, 28);
        const int N = 48;
        const float lv = jlimit (0.0f, 1.0f, (st + 30.0f) / 30.0f), tg = jlimit (0.0f, 1.0f, (target + 30.0f) / 30.0f);
        for (int i = 0; i < N; ++i)
        {
            const float f = (float) i / N;
            const auto c = f < tg - 0.05f ? col::amber : (f < tg + 0.05f ? col::violet : col::red);
            g.setColour (f < lv ? c : c.withAlpha (0.08f));
            g.fillRoundedRectangle (bar.getX() + i * (bar.getWidth() / N), bar.getY(), bar.getWidth() / N - 2.0f, bar.getHeight(), 2.0f);
        }
        g.setColour (Colours::white);  g.fillRect (bar.getX() + tg * bar.getWidth() - 1, bar.getY() - 6, 2.0f, bar.getHeight() + 12);
        g.setFont (Theme::uiFont (10.0f, true));  g.setColour (col::textDim);
        g.drawText ("TARGET " + String (target, 0) + " LUFS", (int) (bar.getX() + tg * bar.getWidth() - 60), (int) bar.getBottom() + 8, 120, 14, Justification::centred);

        // reduction de gain
        g.setColour (col::textDim);  g.drawText ("GAIN REDUCTION", (int) a.getX() + 24, (int) a.getY() + 170, 160, 14, Justification::centredLeft);
        const auto grBar = Rectangle<float> (a.getX() + 24, a.getY() + 188, a.getWidth() - 48, 14);
        g.setColour (Colours::white.withAlpha (0.08f));  g.fillRoundedRectangle (grBar, 3.0f);
        g.setColour (col::orange);  g.fillRoundedRectangle (grBar.withWidth (grBar.getWidth() * jlimit (0.0f, 1.0f, gr / 12.0f)), 3.0f);
        g.setColour (col::text);  g.drawText ("-" + String (gr, 1) + " dB", grBar.toNearestInt().translated (0, 14), Justification::centredRight);

        if (levelUp > 0)
        {
            const float k = levelUp / 90.0f;
            g.setColour (Colours::black.withAlpha (0.55f * k));  g.fillRect (a);
            g.setColour (col::amber.withAlpha (jmin (1.0f, k * 2.0f)));  g.setFont (Theme::lcdFont (34.0f));
            g.drawText ("LEVEL UP!", a.translated (0, -(1.0f - k) * 30.0f).toNearestInt(), Justification::centred);
        }
    }

private:
    float gr = 0, st = -70, integ = -70, mom = -70;
    int okTime = 0, levelUp = 0;
};

juce::AudioProcessorEditor* HardcapProcessor::createEditor() { return new HardcapEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new HardcapProcessor(); }
