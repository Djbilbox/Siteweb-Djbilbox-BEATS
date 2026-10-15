// HUMPIRE RESPAWN - reverb algorithmique FDN 8 lignes (matrice de Householder), pre-delay, early/tail, damping, EQ, ducking
#include <HumpireCore/GenericEditor.h>
#include <HumpireCore/Biquad.h>

using namespace humpire;

namespace
{
struct DelayLine
{
    std::vector<float> b;  int w = 0;
    void init (int n) { b.assign ((size_t) n + 4, 0.0f); w = 0; }
    void push (float x) noexcept { b[(size_t) w] = x; if (++w >= (int) b.size()) w = 0; }
    float read (float d) const noexcept          // d en echantillons (>=1), interpolation lineaire
    {
        const int n = (int) b.size();
        float p = (float) w - d;  while (p < 0) p += (float) n;
        const int i0 = (int) p, i1 = (i0 + 1) % n;  const float f = p - (float) i0;
        return b[(size_t) i0 % (size_t) n] * (1.0f - f) + b[(size_t) i1] * f;
    }
};
constexpr int N = 8;
constexpr float baseMs[N] = { 29.7f, 37.1f, 41.1f, 43.7f, 53.3f, 59.9f, 67.7f, 71.9f };
constexpr float earlyMs[6] = { 7.0f, 13.0f, 19.0f, 27.0f, 36.0f, 48.0f };
constexpr float earlyG[6]  = { 0.8f, 0.65f, 0.55f, 0.42f, 0.32f, 0.22f };
}

class RespawnProcessor : public HumpireProcessor
{
public:
    RespawnProcessor() : HumpireProcessor ("HUMPIRE RESPAWN", "RESPAWN", makeLayout(), presets_()) {}
    double getTailLengthSeconds() const override { return 21.0; }
    juce::AudioProcessorEditor* createEditor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        l.add (mkFloat ("decay",   "Decay",     0.2f, 20.0f, 2.0f, 0.4f, "s", 1));
        l.add (mkFloat ("size",    "Size",      0.5f, 2.0f, 1.0f, 1.0f, "", 2));
        l.add (mkFloat ("predelay","Pre-Delay", 0.0f, 200.0f, 15.0f, 0.6f, "ms", 0));
        l.add (mkFloat ("damp",    "Damping",   0.0f, 1.0f, 0.4f, 1.0f, "", 2));
        l.add (mkFloat ("early",   "Early",     0.0f, 1.0f, 0.5f, 1.0f, "", 2));
        l.add (mkFloat ("mod",     "Modulation",0.0f, 1.0f, 0.3f, 1.0f, "", 2));
        l.add (mkFloat ("lowcut",  "Low Cut",   20.0f, 1000.0f, 80.0f, 0.4f, "Hz", 0));
        l.add (mkFloat ("highcut", "High Cut",  2000.0f, 20000.0f, 12000.0f, 0.4f, "Hz", 0));
        l.add (mkFloat ("duck",    "Ducking",   0.0f, 1.0f, 0.0f, 1.0f, "", 2));
        l.add (mkFloat ("mix",     "Mix",       0.0f, 1.0f, 0.3f, 1.0f, "", 2));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        auto P = [] (const char* n, const char* c, float dc, float sz, float pd, float dp, float er, float md, float lc, float hc, float dk, float mx)
        { return fp (n, c, { { "decay", dc }, { "size", sz }, { "predelay", pd }, { "damp", dp }, { "early", er }, { "mod", md }, { "lowcut", lc }, { "highcut", hc }, { "duck", dk }, { "mix", mx } }); };
        return {
            P ("Plate Vocal",        "Plate",      1.6f, 0.7f, 10,  0.35f, 0.3f, 0.35f, 120, 14000, 0.2f, 0.25f),
            P ("Plate Snare",        "Plate",      1.1f, 0.6f, 0,   0.3f,  0.2f, 0.3f,  200, 15000, 0.0f, 0.3f),
            P ("Hall Medium",        "Hall",       2.6f, 1.0f, 25,  0.45f, 0.5f, 0.3f,  100, 11000, 0.0f, 0.25f),
            P ("Hall Large",         "Hall",       4.5f, 1.4f, 40,  0.5f,  0.5f, 0.35f, 90,  9000,  0.1f, 0.3f),
            P ("Hammam Steam",       "Hammam",     6.0f, 0.9f, 20,  0.75f, 0.2f, 0.5f,  150, 6000,  0.0f, 0.4f),
            P ("Hammam Chant",       "Hammam",     3.8f, 0.8f, 30,  0.65f, 0.3f, 0.45f, 120, 7000,  0.3f, 0.35f),
            P ("Cathedral Choir",   "Cathedral", 9.5f, 1.8f, 60,  0.5f,  0.3f, 0.4f,  110, 8000,  0.0f, 0.4f),
            P ("Cathedral Abyss",   "Cathedral", 15.0f, 2.0f, 80, 0.6f,  0.2f, 0.5f,  140, 6500,  0.2f, 0.5f),
            P ("Trap Vocal Space",    "Trap Vocals",  2.2f, 1.1f, 35,  0.5f,  0.3f, 0.3f,  250, 10000, 0.7f, 0.28f),
            P ("808 Room",           "808",        0.6f, 0.5f, 0,   0.6f,  0.6f, 0.1f,  300, 5000,  0.0f, 0.12f),
            P ("Oud Chamber",        "Oud",        2.0f, 0.9f, 18,  0.55f, 0.45f,0.35f, 100, 9500,  0.0f, 0.3f),
            P ("Darbouka Courtyard", "Darbouka",   1.4f, 1.2f, 12,  0.4f,  0.55f,0.25f, 150, 12000, 0.2f, 0.25f),
        };
    }

    void prepare (double sr, int) override
    {
        fs = sr;
        for (auto& d : lines) d.init ((int) (0.2 * sr));
        for (auto& d : pre)   d.init ((int) (0.25 * sr));
        for (auto& d : erIn)  d.init ((int) (0.115 * sr));
        for (auto& z : lp) z = 0.0f;
        env = 0.0f;  for (auto& p : lfoPh) p = 0.0f;
        for (int c = 0; c < 2; ++c) { lc[c].reset(); hc[c].reset(); }
    }

    void process (juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        const int n = buf.getNumSamples(), nch = juce::jmin (2, buf.getNumChannels());
        const float decay = get ("decay"), size = get ("size"), damp = get ("damp"), early = get ("early"), mod = get ("mod");
        const float duck = get ("duck"), mix = get ("mix");
        const float preS = get ("predelay") * 0.001f * (float) fs;
        const float fcDamp = 18000.0f * std::pow (1500.0f / 18000.0f, damp);
        const float dampA = std::exp (-6.2831853f * fcDamp / (float) fs);
        float g[N], dl[N];
        for (int i = 0; i < N; ++i)
        {
            dl[i] = baseMs[i] * 0.001f * size * (float) fs;
            g[i] = std::pow (10.0f, -3.0f * (dl[i] / (float) fs) / decay);
        }
        for (int c = 0; c < 2; ++c) { lc[c].set (Biquad::Type::highPass, fs, get ("lowcut"), 0.707, 0); hc[c].set (Biquad::Type::lowPass, fs, get ("highcut"), 0.707, 0); }
        float* ch[2] = { buf.getWritePointer (0), nch > 1 ? buf.getWritePointer (1) : nullptr };
        float wetPeak = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            const float inL = ch[0][i], inR = nch > 1 ? ch[1][i] : inL;
            env = juce::jmax (std::abs (inL) + std::abs (inR), env * 0.9995f);       // suiveur d'enveloppe pour ducking

            pre[0].push (inL);  pre[1].push (inR);
            const float pL = pre[0].read (juce::jmax (1.0f, preS)), pR = pre[1].read (juce::jmax (1.0f, preS));
            erIn[0].push (pL);  erIn[1].push (pR);

            float eL = 0, eR = 0;                                           // premieres reflexions
            for (int k = 0; k < 6; ++k)
            {
                const float d = earlyMs[k] * 0.001f * size * (float) fs;
                eL += erIn[0].read (d) * earlyG[k] * (k % 2 ? -1.0f : 1.0f);
                eR += erIn[1].read (d * 1.13f) * earlyG[k] * (k % 2 ? 1.0f : -1.0f);
            }

            float y[N], sum = 0.0f;
            for (int k = 0; k < N; ++k)
            {
                lfoPh[k] += 0.37f * (1.0f + 0.13f * (float) k) / (float) fs;  if (lfoPh[k] >= 1.0f) lfoPh[k] -= 1.0f;
                const float lfo = std::sin (6.2831853f * (lfoPh[k] + 0.125f * (float) k));
                y[k] = lines[(size_t) k].read (dl[k] + mod * 0.0004f * (float) fs * lfo);
                lp[k] = (1.0f - dampA) * y[k] + dampA * lp[k];               // damping HF
                y[k] = lp[k] * g[k];
                sum += y[k];
            }
            const float hh = sum * (2.0f / (float) N);                       // Householder
            for (int k = 0; k < N; ++k)
            {
                const float in = (k % 2 == 0 ? pL : pR) * 0.5f;
                lines[(size_t) k].push (y[k] - hh + in);
            }
            float tL = 0, tR = 0;
            for (int k = 0; k < N; ++k) (k % 2 == 0 ? tL : tR) += y[k] * (k % 4 < 2 ? 1.0f : -1.0f);
            float wL = tL * 0.35f + eL * early * 0.35f, wR = tR * 0.35f + eR * early * 0.35f;
            wL = hc[0].process (lc[0].process (wL));  wR = hc[1].process (lc[1].process (wR));
            const float dk = 1.0f - duck * juce::jmin (1.0f, env * 3.0f);
            wL *= dk;  wR *= dk;
            wetPeak = juce::jmax (wetPeak, std::abs (wL), std::abs (wR));

            if (nch > 1) { ch[0][i] = inL * (1.0f - mix) + mix * wL;  ch[1][i] = inR * (1.0f - mix) + mix * wR; }
            else ch[0][i] = inL * (1.0f - mix) + mix * 0.5f * (wL + wR);
        }
        aux[0].store (wetPeak);
    }

private:
    double fs = 44100;
    DelayLine lines[N], pre[2], erIn[2];
    float lp[N] {}, lfoPh[N] {}, env = 0;
    Biquad lc[2], hc[2];
};

class RespawnEditor : public GenericEditor
{
public:
    explicit RespawnEditor (RespawnProcessor& p) : GenericEditor (p, makeSpec())
    {
        juce::Random r;
        for (auto& q : parts) q = { r.nextFloat(), r.nextFloat(), r.nextFloat() };
        finishSetup();
    }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "RESPAWN";  s.subtitle = "ALGORITHMIC REVERB  //  FDN 8";
        s.style = BodyStyle::amp;
        s.body1 = juce::Colour (0xff2a2119);  s.body2 = juce::Colour (0xff0d0a08);  s.accent = col::amber;
        s.knobs = { { "decay", "DECAY" }, { "size", "SIZE" }, { "predelay", "PRE-DLY" }, { "damp", "DAMP" }, { "early", "EARLY", true },
                    { "mod", "MOD", true }, { "lowcut", "LOW CUT", true }, { "highcut", "HIGH CUT", true }, { "duck", "DUCK", true }, { "mix", "MIX" } };
        return s;
    }

protected:
    void tick() override
    {
        const float l = proc.aux[0].load();
        level = juce::jmax (l * 4.0f, level * 0.9f);
        decayNorm += (juce::jlimit (0.0f, 1.0f, proc.get ("decay") / 15.0f) - decayNorm) * 0.1f;
        for (auto& q : parts) { q.life += 0.008f + level * 0.01f; if (q.life > 1.0f) { q.life = 0.0f; q.x = juce::Random::getSystemRandom().nextFloat(); q.y = juce::Random::getSystemRandom().nextFloat(); } }
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float t) override
    {
        using namespace juce;
        g.setGradientFill (ColourGradient (Colour (0xff130f0b), a.getCentreX(), a.getCentreY(), Colour (0xff050403), a.getX(), a.getY(), true));
        g.fillRect (a);
        const auto c = a.getCentre();
        // ondes lumineuses : amplitude et nombre d'anneaux suivent le decay
        const int rings = 4 + (int) (decayNorm * 8.0f);
        for (int i = 0; i < rings; ++i)
        {
            const float ph = std::fmod (t * (0.25f + 0.2f * (1.0f - decayNorm)) + (float) i / rings, 1.0f);
            const float r = 40.0f + ph * (a.getHeight() * 0.55f + decayNorm * a.getWidth() * 0.25f);
            g.setColour (col::amber.interpolatedWith (col::orange, ph).withAlpha ((1.0f - ph) * (0.15f + 0.5f * jmin (1.0f, level + decayNorm * 0.4f))));
            g.drawEllipse (c.x - r, c.y - r * 0.8f, r * 2, r * 1.6f, 2.0f + (1.0f - ph) * 2.0f);
        }
        // cone de haut-parleur (pulse avec le niveau)
        const float pulse = 1.0f + level * 0.06f;
        const float cr = 62.0f * pulse;
        g.setGradientFill (ColourGradient (Colour (0xff6a5a45), c.x - cr * 0.4f, c.y - cr * 0.5f, Colour (0xff17120d), c.x + cr * 0.6f, c.y + cr * 0.7f, true));
        g.fillEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2);
        for (int i = 1; i <= 5; ++i) { g.setColour (Colours::black.withAlpha (0.25f)); g.drawEllipse (c.x - cr * i / 5.5f, c.y - cr * i / 5.5f, cr * 2 * i / 5.5f, cr * 2 * i / 5.5f, 1.0f); }
        g.setColour (Colour (0xff2b241b));  g.fillEllipse (c.x - 18, c.y - 18, 36, 36);                     // dust cap
        g.setColour (col::amber.withAlpha (0.8f));  g.drawEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2, 3.0f);
        // particules qui reapparaissent
        for (auto& q : parts)
        {
            const float al = std::sin (q.life * MathConstants<float>::pi) * (0.3f + level);
            g.setColour (col::amber.withAlpha (jlimit (0.0f, 1.0f, al)));
            g.fillEllipse (a.getX() + q.x * a.getWidth() - 2, a.getY() + q.y * a.getHeight() - 2 - q.life * 18.0f, 4, 4);
        }
    }

private:
    struct Part { float x, y, life; };
    std::array<Part, 48> parts;
    float level = 0, decayNorm = 0;
};

juce::AudioProcessorEditor* RespawnProcessor::createEditor() { return new RespawnEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RespawnProcessor(); }
