// HUMPIRE SKILLTREE - EQ 6 bandes + HP/LP, cible Stereo/Mid/Side, phase Minimum ou Lineaire (FIR 4096), analyseur FFT, noeuds interactifs
#include <HumpireCore/GenericEditor.h>
#include <HumpireCore/Biquad.h>

using namespace humpire;

namespace
{
constexpr int nBands = 6, kFir = 4096, kLat = kFir / 2;
constexpr float defF[nBands] = { 60, 200, 600, 2000, 6000, 12000 }, defQ[nBands] = { 0.7f, 1.0f, 1.0f, 1.0f, 1.0f, 0.7f };

struct Cascade { Biquad bq[10]; };      // 0-5 bandes, 6-7 passe-haut, 8-9 passe-bas (24 dB/oct)

inline void setIdentity (Biquad& b) { b.b0 = 1; b.b1 = b.b2 = b.a1 = b.a2 = 0; }
}

class SkilltreeProcessor : public HumpireProcessor, private juce::AsyncUpdater, private juce::AudioProcessorValueTreeState::Listener
{
public:
    SkilltreeProcessor() : HumpireProcessor ("HUMPIRE SKILLTREE", "SKILLTREE", makeLayout(), presets_())
    {
        for (auto* p : getParameters())
            if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (p)) apvts.addParameterListener (r->paramID, this);
        for (auto& r : ring) r.store (0.0f);
    }
    ~SkilltreeProcessor() override
    {
        cancelPendingUpdate();
        for (auto* p : getParameters())
            if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (p)) apvts.removeParameterListener (r->paramID, this);
    }
    juce::AudioProcessorEditor* createEditor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        for (int i = 0; i < nBands; ++i)
        {
            const juce::String n (i + 1), pre = "b" + n;
            l.add (mkFloat ((pre + "f").toRawUTF8(), ("Band " + n + " Freq").toRawUTF8(), 20.0f, 20000.0f, defF[i], 0.3f, "Hz", 0));
            l.add (mkFloat ((pre + "g").toRawUTF8(), ("Band " + n + " Gain").toRawUTF8(), -18.0f, 18.0f, 0.0f, 1.0f, "dB"));
            l.add (mkFloat ((pre + "q").toRawUTF8(), ("Band " + n + " Q").toRawUTF8(), 0.2f, 10.0f, defQ[i], 0.4f, "", 2));
            l.add (mkBool  ((pre + "on").toRawUTF8(), ("Band " + n + " On").toRawUTF8(), true));
        }
        l.add (mkFloat ("hp", "High Pass", 20.0f, 1000.0f, 20.0f, 0.3f, "Hz", 0));
        l.add (mkFloat ("lp", "Low Pass", 1000.0f, 20000.0f, 20000.0f, 0.3f, "Hz", 0));
        l.add (mkChoice ("target", "Target", { "Stereo", "Mid", "Side" }, 0));
        l.add (mkChoice ("phase", "Phase", { "Minimum", "Linear" }, 0));
        l.add (mkFloat ("out", "Output", -18.0f, 18.0f, 0.0f, 1.0f, "dB"));
        l.add (mkBool  ("analyzer", "Analyzer", true));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        using V = std::initializer_list<std::pair<const char*, float>>;
        auto P = [] (const char* n, const char* c, V v) { return fp (n, c, v); };
        return {
            P ("Voix Trap Air",     "Voix Trap", { { "hp", 90 }, { "b2g", -2 }, { "b2f", 250 }, { "b4g", 2.5f }, { "b4f", 3500 }, { "b6g", 3 }, { "b6f", 11000 } }),
            P ("Voix Radio",        "Voix Trap", { { "hp", 200 }, { "lp", 9000 }, { "b3g", 4 }, { "b3f", 1500 }, { "b3q", 1.4f } }),
            P ("808 Sub Boost",     "808",       { { "hp", 25 }, { "b1g", 4 }, { "b1f", 55 }, { "b2g", -3 }, { "b2f", 220 }, { "lp", 8000 } }),
            P ("808 Clarity",       "808",       { { "b2g", -4 }, { "b2f", 300 }, { "b4g", 2 }, { "b4f", 1200 } }),
            P ("Kick Punch",        "Kick",      { { "b1g", 3 }, { "b1f", 60 }, { "b2g", -4 }, { "b2f", 350 }, { "b4g", 3 }, { "b4f", 4000 } }),
            P ("Kick Click",        "Kick",      { { "hp", 30 }, { "b5g", 4 }, { "b5f", 5000 } }),
            P ("Oud Body",          "Oud",       { { "hp", 70 }, { "b2g", 2.5f }, { "b2f", 180 }, { "b3g", -2 }, { "b3f", 500 }, { "b5g", 2 }, { "b5f", 5000 } }),
            P ("Oud Air",           "Oud",       { { "hp", 80 }, { "b6g", 4 }, { "b6f", 10000 } }),
            P ("Darbouka Bite",     "Darbouka",  { { "hp", 100 }, { "b4g", 3 }, { "b4f", 2500 }, { "b5g", 3 }, { "b5f", 7000 } }),
            P ("Bus Drums Glue",    "Bus Drums", { { "b1g", 2 }, { "b1f", 70 }, { "b3g", -2 }, { "b3f", 450 }, { "b6g", 2 }, { "b6f", 10000 } }),
            P ("Master Wide Air",   "Master",    { { "target", 2 }, { "b6g", 4 }, { "b6f", 9000 }, { "hp", 120 } }),
            P ("Master Mono Bass",  "Master",    { { "target", 1 }, { "b1g", 1.5f }, { "b1f", 80 } }),
        };
    }

    // --- cascade courante depuis les parametres (sans allocation)
    Cascade buildCascade (double sr) const
    {
        Cascade c;
        for (int i = 0; i < nBands; ++i)
        {
            const juce::String pre = "b" + juce::String (i + 1);
            const bool on = getBool ((pre + "on").toRawUTF8());
            const float g = get ((pre + "g").toRawUTF8());
            if (! on || std::abs (g) < 0.01f) { setIdentity (c.bq[i]); continue; }
            c.bq[i].set (i == 0 ? Biquad::Type::lowShelf : (i == nBands - 1 ? Biquad::Type::highShelf : Biquad::Type::peak),
                         sr, get ((pre + "f").toRawUTF8()), get ((pre + "q").toRawUTF8()), g);
        }
        const float hp = get ("hp"), lp = get ("lp");
        if (hp > 20.5f) { c.bq[6].set (Biquad::Type::highPass, sr, hp, 0.5412, 0); c.bq[7].set (Biquad::Type::highPass, sr, hp, 1.3066, 0); }
        else { setIdentity (c.bq[6]); setIdentity (c.bq[7]); }
        if (lp < 19990.0f) { c.bq[8].set (Biquad::Type::lowPass, sr, lp, 0.5412, 0); c.bq[9].set (Biquad::Type::lowPass, sr, lp, 1.3066, 0); }
        else { setIdentity (c.bq[8]); setIdentity (c.bq[9]); }
        return c;
    }
    static double cascadeMag (const Cascade& c, double sr, double f)
    { double m = 1; for (auto& b : c.bq) m *= b.magnitude (sr, f); return m; }

    void prepare (double sr, int maxBlock) override
    {
        fs = sr;
        for (auto& c : conv) { c.prepare ({ sr, (juce::uint32) maxBlock, 1 }); c.reset(); }
        for (auto& d : dly) d.assign ((size_t) kFir, 0.0f);
        dPos = 0;
        for (auto& cc : chain) for (auto& b : cc.bq) b.reset();
        triggerAsyncUpdate();
    }

    void process (juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        const int n = buf.getNumSamples(), nch = juce::jmin (2, buf.getNumChannels());
        const bool lin = juce::roundToInt (get ("phase")) == 1;
        const int tgt = nch == 2 ? juce::roundToInt (get ("target")) : 0;
        const float outG = juce::Decibels::decibelsToGain (get ("out"));

        const auto fresh = buildCascade (fs);
        for (auto& cc : chain)
            for (int k = 0; k < 10; ++k) { auto& d = cc.bq[k]; const auto& s = fresh.bq[k]; d.b0 = s.b0; d.b1 = s.b1; d.b2 = s.b2; d.a1 = s.a1; d.a2 = s.a2; }

        float* ch[2] = { buf.getWritePointer (0), nch > 1 ? buf.getWritePointer (1) : nullptr };
        if (tgt != 0) for (int i = 0; i < n; ++i) { const float m = 0.5f * (ch[0][i] + ch[1][i]), s = 0.5f * (ch[0][i] - ch[1][i]); ch[0][i] = m; ch[1][i] = s; }

        for (int c = 0; c < nch; ++c)
        {
            const bool processed = tgt == 0 || (tgt == 1 && c == 0) || (tgt == 2 && c == 1);
            if (processed)
            {
                if (! lin) for (int i = 0; i < n; ++i) { float x = ch[c][i]; for (auto& b : chain[(size_t) c].bq) x = b.process (x); ch[c][i] = x; }
                else
                {
                    juce::dsp::AudioBlock<float> blk (&ch[c], 1, (size_t) n);
                    conv[c].process (juce::dsp::ProcessContextReplacing<float> (blk));
                }
            }
            else if (lin)        // compense la latence du FIR sur le canal non traite
                for (int i = 0; i < n; ++i)
                {
                    const int rp = (dPos + i - kLat + 4 * kFir) % kFir;
                    dly[(size_t) c][(size_t) ((dPos + i) % kFir)] = ch[c][i];
                    ch[c][i] = dly[(size_t) c][(size_t) rp];
                }
        }
        if (lin) dPos = (dPos + n) % kFir;
        if (tgt != 0) for (int i = 0; i < n; ++i) { const float m = ch[0][i], s = ch[1][i]; ch[0][i] = m + s; ch[1][i] = m - s; }

        for (int i = 0; i < n; ++i)
        {
            float mono = 0;
            for (int c = 0; c < nch; ++c) { ch[c][i] *= outG; mono += ch[c][i]; }
            const int h = head.load (std::memory_order_relaxed);
            ring[(size_t) h].store (mono / (float) nch, std::memory_order_relaxed);
            head.store ((h + 1) % (int) ring.size(), std::memory_order_release);
        }
    }

    std::array<std::atomic<float>, 8192> ring;
    std::atomic<int> head { 0 };
    double sampleRate() const { return fs; }

private:
    void parameterChanged (const juce::String&, float) override { triggerAsyncUpdate(); }

    // thread message : (re)calcule le FIR phase-lineaire et la latence declaree
    void handleAsyncUpdate() override
    {
        const bool lin = juce::roundToInt (get ("phase")) == 1;
        setLatencySamples (lin ? kLat : 0);
        if (! lin || fs <= 0) return;

        const auto cas = buildCascade (fs);
        juce::dsp::FFT fft (12);
        std::vector<float> data ((size_t) kFir * 2, 0.0f);
        std::vector<double> mag ((size_t) kFir / 2 + 1);
        double e2 = 0;
        for (int k = 0; k <= kFir / 2; ++k)
        {
            const double f = juce::jmax (1.0, (double) k * fs / (double) kFir);
            mag[(size_t) k] = cascadeMag (cas, fs, f);
            data[(size_t) k * 2] = (float) mag[(size_t) k];
            e2 += mag[(size_t) k] * mag[(size_t) k] * (k == 0 || k == kFir / 2 ? 1.0 : 2.0);
        }
        fft.performRealOnlyInverseTransform (data.data());          // IR a phase nulle (reelle, symetrique)
        double h2 = 0;  for (int i = 0; i < kFir; ++i) h2 += (double) data[(size_t) i] * data[(size_t) i];
        const float scale = h2 > 1e-20 ? (float) std::sqrt ((e2 / (double) kFir) / h2) : 1.0f;     // Parseval : gain independant de la convention FFT

        juce::AudioBuffer<float> ir (1, kFir);
        for (int i = 0; i < kFir; ++i)
        {
            const float w = 0.5f - 0.5f * std::cos (6.2831853f * (float) i / (float) (kFir - 1));       // Hann
            ir.setSample (0, i, data[(size_t) ((i - kLat + kFir) % kFir)] * scale * w);
        }
        for (auto& c : conv)
        {
            juce::AudioBuffer<float> copy (ir);
            c.loadImpulseResponse (std::move (copy), fs, juce::dsp::Convolution::Stereo::no, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
        }
    }

    double fs = 44100;
    Cascade chain[2];
    juce::dsp::Convolution conv[2];
    std::vector<float> dly[2];
    int dPos = 0;
};

// ======================================================================= editeur
class SkilltreeEditor : public GenericEditor
{
public:
    explicit SkilltreeEditor (SkilltreeProcessor& p) : GenericEditor (p, makeSpec()), sk (p), fft (11), pad (*this)
    {
        pad.setBounds (spec.display);
        content.addAndMakeVisible (pad);
        spectrum.fill (-100.0f);
        finishSetup();
    }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "SKILLTREE";  s.subtitle = "6-BAND EQ  //  DRAG NODES  -  WHEEL = Q  -  DOUBLE-CLICK = ON/OFF";
        s.style = BodyStyle::arcade;
        s.body1 = juce::Colour (0xff1c1a24);  s.body2 = juce::Colour (0xff07060a);  s.accent = col::amber;
        s.display = { 70, 62, 680, 250 };
        s.knobs = { { "hp", "HIGH PASS" }, { "out", "OUTPUT" }, { "lp", "LOW PASS" } };
        s.buttons = { { "target", "STEREO" }, { "phase", "MINIMUM" }, { "analyzer", "ANALYZER" } };
        s.knobY = 336;  s.buttonY = 452;
        return s;
    }

    // ---- mapping ecran <-> parametres
    float freqToX (float f, juce::Rectangle<float> a) const { return a.getX() + a.getWidth() * std::log (f / 20.0f) / std::log (1000.0f); }
    float xToFreq (float x, juce::Rectangle<float> a) const { return 20.0f * std::exp (juce::jlimit (0.0f, 1.0f, (x - a.getX()) / a.getWidth()) * std::log (1000.0f)); }
    float dbToY (float db, juce::Rectangle<float> a) const { return a.getCentreY() - db / 18.0f * a.getHeight() * 0.5f; }
    float yToDb (float y, juce::Rectangle<float> a) const { return juce::jlimit (-18.0f, 18.0f, (a.getCentreY() - y) / (a.getHeight() * 0.5f) * 18.0f); }

    void setReal (const juce::String& id, float v, bool gesture, bool begin)
    {
        if (auto* p = sk.apvts.getParameter (id))
        {
            if (gesture && begin) p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (v));
        }
    }
    void endGestures() { for (int i = 1; i <= nBands; ++i) for (auto* s : { "f", "g", "q" }) if (auto* p = sk.apvts.getParameter ("b" + juce::String (i) + s)) p->endChangeGesture(); }

    int nodeAt (juce::Point<float> pt) const
    {
        const auto a = spec.display.toFloat();
        int best = -1;  float bd = 18.0f;
        for (int i = 0; i < nBands; ++i)
        {
            const juce::String pre = "b" + juce::String (i + 1);
            const juce::Point<float> np (freqToX (sk.get ((pre + "f").toRawUTF8()), a) , dbToY (sk.get ((pre + "g").toRawUTF8()), a));
            const float d = np.translated (-a.getX(), -a.getY()).getDistanceFrom (pt);
            if (d < bd) { bd = d; best = i; }
        }
        return best;
    }

    int selected = -1, hovered = -1;

protected:
    void tick() override
    {
        // analyseur FFT 2048 pts (fenetre de Hann), lissage temporel
        std::array<float, 4096> data {};
        const int h = sk.head.load (std::memory_order_acquire), sz = (int) sk.ring.size();
        for (int i = 0; i < 2048; ++i)
        {
            const float w = 0.5f - 0.5f * std::cos (6.2831853f * (float) i / 2047.0f);
            data[(size_t) i] = sk.ring[(size_t) ((h - 2048 + i + sz) % sz)].load (std::memory_order_relaxed) * w;
        }
        fft.performFrequencyOnlyForwardTransform (data.data());
        for (int k = 0; k < 1024; ++k)
        {
            const float db = juce::Decibels::gainToDecibels (data[(size_t) k] / 512.0f, -100.0f);
            spectrum[(size_t) k] = db > spectrum[(size_t) k] ? db : spectrum[(size_t) k] * 0.85f + db * 0.15f;
        }
        const char* ph[] = { "MINIMUM", "LINEAR" };  (void) ph;
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float) override
    {
        using namespace juce;
        const double sr = sk.sampleRate();
        g.setGradientFill (ColourGradient (Colour (0xff0a1210), a.getCentreX(), a.getCentreY(), Colour (0xff030605), a.getX(), a.getY(), true));
        g.fillRect (a);
        // grille
        g.setFont (Theme::uiFont (9.0f, true));
        for (float f : { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f })
        {
            const float x = freqToX (f, a);
            g.setColour (Colours::white.withAlpha (0.06f));  g.drawVerticalLine ((int) x, a.getY(), a.getBottom());
            g.setColour (col::textDim);  g.drawText (f >= 1000 ? String (f / 1000, 0) + "k" : String ((int) f), (int) x - 14, (int) a.getBottom() - 14, 28, 12, Justification::centred);
        }
        for (float db : { -12.0f, -6.0f, 0.0f, 6.0f, 12.0f })
        {
            g.setColour (Colours::white.withAlpha (db == 0.0f ? 0.18f : 0.06f));  g.drawHorizontalLine ((int) dbToY (db, a), a.getX(), a.getRight());
            g.setColour (col::textDim);  g.drawText (String ((int) db), (int) a.getX() + 4, (int) dbToY (db, a) - 12, 24, 11, Justification::centredLeft);
        }
        // spectre
        if (sk.getBool ("analyzer"))
        {
            Path sp;  sp.startNewSubPath (a.getX(), a.getBottom());
            for (int k = 2; k < 1024; ++k)
            {
                const float f = (float) k * (float) sr / 2048.0f;
                if (f < 20.0f || f > 20000.0f) continue;
                sp.lineTo (freqToX (f, a), a.getBottom() - a.getHeight() * jlimit (0.0f, 1.0f, (spectrum[(size_t) k] + 90.0f) / 90.0f) * 0.85f);
            }
            sp.lineTo (a.getRight(), a.getBottom());  sp.closeSubPath();
            g.setColour (col::violet.withAlpha (0.28f));  g.fillPath (sp);
            g.setColour (col::violet.withAlpha (0.8f));   g.strokePath (sp, PathStrokeType (1.0f));
        }
        // courbe de reponse
        const auto cas = sk.buildCascade (sr);
        Path curve;
        for (int px = 0; px <= (int) a.getWidth(); px += 2)
        {
            const float f = xToFreq (a.getX() + (float) px, a);
            const float db = jlimit (-18.0f, 18.0f, (float) (20.0 * std::log10 (jmax (1.0e-5, SkilltreeProcessor::cascadeMag (cas, sr, f)))));
            const float y = dbToY (db, a);
            px == 0 ? curve.startNewSubPath (a.getX(), y) : curve.lineTo (a.getX() + (float) px, y);
        }
        g.setColour (col::amber.withAlpha (0.25f));  g.strokePath (curve, PathStrokeType (7.0f));
        g.setColour (col::amber);                     g.strokePath (curve, PathStrokeType (2.2f));

        // arbre de talents : noeuds relies par ordre de frequence
        struct N { float x, y; int i; bool on; };
        std::vector<N> nodes;
        for (int i = 0; i < nBands; ++i)
        {
            const String pre = "b" + String (i + 1);
            nodes.push_back ({ freqToX (sk.get ((pre + "f").toRawUTF8()), a), dbToY (sk.get ((pre + "g").toRawUTF8()), a), i, sk.getBool ((pre + "on").toRawUTF8()) });
        }
        std::sort (nodes.begin(), nodes.end(), [] (const N& p, const N& q) { return p.x < q.x; });
        for (size_t k = 0; k + 1 < nodes.size(); ++k)
        {
            g.setColour (col::violet.withAlpha (nodes[k].on && nodes[k + 1].on ? 0.55f : 0.15f));
            g.drawLine (nodes[k].x, nodes[k].y, nodes[k + 1].x, nodes[k + 1].y, 1.5f);
        }
        for (auto& nd : nodes)
        {
            const bool act = nd.i == selected || nd.i == hovered;
            const auto c = nd.on ? col::amber : col::textDim;
            if (nd.on) { g.setColour (c.withAlpha (act ? 0.5f : 0.25f));  g.fillEllipse (nd.x - 16, nd.y - 16, 32, 32); }
            g.setColour (Colour (0xff15110a));  g.fillEllipse (nd.x - 9, nd.y - 9, 18, 18);
            g.setColour (c);  g.drawEllipse (nd.x - 9, nd.y - 9, 18, 18, act ? 3.0f : 2.0f);
            g.setFont (Theme::uiFont (10.0f, true));  g.drawText (String (nd.i + 1), (int) nd.x - 9, (int) nd.y - 9, 18, 18, Justification::centred);
        }
        if (hovered >= 0 || selected >= 0)
        {
            const int i = selected >= 0 ? selected : hovered;  const String pre = "b" + String (i + 1);
            g.setColour (col::text);  g.setFont (Theme::lcdFont (9.0f));
            g.drawText ("B" + String (i + 1) + "  " + String (sk.get ((pre + "f").toRawUTF8()), 0) + " HZ  " + String (sk.get ((pre + "g").toRawUTF8()), 1) + " DB  Q " + String (sk.get ((pre + "q").toRawUTF8()), 2),
                        a.toNearestInt().removeFromTop (22).reduced (12, 0), Justification::centredLeft);
        }
        if (sk.get ("phase") > 0.5f) { g.setColour (col::violet); g.setFont (Theme::lcdFont (8.0f)); g.drawText ("LINEAR PHASE  LAT 2048", a.toNearestInt().removeFromTop (22).reduced (12, 0), Justification::centredRight); }
    }

private:
    struct Pad : juce::Component
    {
        explicit Pad (SkilltreeEditor& e) : ed (e) {}
        void mouseMove (const juce::MouseEvent& m) override { ed.hovered = ed.nodeAt (m.position); }
        void mouseExit (const juce::MouseEvent&) override  { ed.hovered = -1; }
        void mouseDown (const juce::MouseEvent& m) override { ed.selected = ed.nodeAt (m.position); began = false; }
        void mouseDrag (const juce::MouseEvent& m) override
        {
            if (ed.selected < 0) return;
            const auto a = juce::Rectangle<float> (0, 0, (float) getWidth(), (float) getHeight());
            const juce::String pre = "b" + juce::String (ed.selected + 1);
            ed.setReal (pre + "f", ed.xToFreq (m.position.x, a), true, ! began);
            ed.setReal (pre + "g", ed.yToDb (m.position.y, a), true, ! began);
            began = true;
        }
        void mouseUp (const juce::MouseEvent&) override { ed.endGestures(); ed.selected = -1; began = false; }
        void mouseDoubleClick (const juce::MouseEvent& m) override
        {
            const int i = ed.nodeAt (m.position);
            if (i < 0) return;
            const juce::String id = "b" + juce::String (i + 1) + "on";
            if (auto* p = ed.sk.apvts.getParameter (id)) { p->beginChangeGesture(); p->setValueNotifyingHost (p->getValue() > 0.5f ? 0.0f : 1.0f); p->endChangeGesture(); }
        }
        void mouseWheelMove (const juce::MouseEvent& m, const juce::MouseWheelDetails& w) override
        {
            const int i = ed.nodeAt (m.position);
            if (i < 0) return;
            const juce::String id = "b" + juce::String (i + 1) + "q";
            if (auto* p = ed.sk.apvts.getParameter (id))
            {
                const float q = p->convertFrom0to1 (p->getValue()) * std::exp (w.deltaY * 0.8f);
                p->beginChangeGesture();  p->setValueNotifyingHost (p->convertTo0to1 (q));  p->endChangeGesture();
            }
        }
        SkilltreeEditor& ed;  bool began = false;
    };

    SkilltreeProcessor& sk;
    juce::dsp::FFT fft;
    std::array<float, 1024> spectrum;
    Pad pad;
};

juce::AudioProcessorEditor* SkilltreeProcessor::createEditor() { return new SkilltreeEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SkilltreeProcessor(); }
