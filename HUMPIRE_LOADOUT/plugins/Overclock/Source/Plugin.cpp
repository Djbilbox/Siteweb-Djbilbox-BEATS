// HUMPIRE OVERCLOCK - enhancer : densite, harmoniques, loudness percu (split 2 bandes + saturation sur-echantillonnee 2x)
#include <HumpireCore/GenericEditor.h>

using namespace humpire;

class OverclockProcessor : public HumpireProcessor
{
public:
    OverclockProcessor() : HumpireProcessor ("HUMPIRE OVERCLOCK", "OVERCLOCK", makeLayout(), presets_()),
        osLow (2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true),
        osHigh (2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true) {}

    juce::AudioProcessorEditor* createEditor() override;

    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout l;
        l.add (mkFloat ("power",   "Power",    0.0f, 1.0f, 0.35f, 1.0f, "", 2));
        l.add (mkFloat ("curve",   "Curve",    0.0f, 1.0f, 0.3f, 1.0f, "", 2));   // 0 = doux/asymetrique, 1 = dur
        l.add (mkFloat ("density", "Density",  0.0f, 1.0f, 0.8f, 1.0f, "", 2));
        l.add (mkFloat ("harm",    "Harmonics",0.0f, 1.0f, 0.3f, 1.0f, "", 2));
        l.add (mkFloat ("split",   "Split",    80.0f, 4000.0f, 400.0f, 0.4f, "Hz", 0));
        l.add (mkFloat ("out",     "Output",  -18.0f, 12.0f, 0.0f, 1.0f, "dB"));
        return l;
    }

    static std::vector<FactoryPreset> presets_()
    {
        auto P = [] (const char* n, const char* c, float pw, float cv, float d, float h, float sp, float o)
        { return fp (n, c, { { "power", pw }, { "curve", cv }, { "density", d }, { "harm", h }, { "split", sp }, { "out", o } }); };
        return {
            P ("Vocal Presence",  "Trap Vocals", 0.30f, 0.20f, 0.7f, 0.50f, 800,  -1),
            P ("Ad-Lib Heat",     "Trap Vocals", 0.55f, 0.40f, 0.9f, 0.40f, 600,  -2),
            P ("808 Harmonics",   "808",       0.45f, 0.25f, 1.0f, 0.10f, 120,  -2),
            P ("Sub Thicken",     "808",       0.30f, 0.10f, 0.8f, 0.00f, 150,  -1),
            P ("Kick Weight",     "Kick",      0.40f, 0.30f, 0.9f, 0.20f, 200,  -2),
            P ("Kick Click",      "Kick",      0.35f, 0.60f, 0.6f, 0.60f, 1500, -1),
            P ("Oud Warmth",      "Oud",       0.25f, 0.10f, 0.6f, 0.35f, 500,  0),
            P ("Darbouka Bite",   "Darbouka",  0.40f, 0.50f, 0.7f, 0.55f, 1200, -1),
            P ("Drum Bus Power",  "Bus Drums", 0.50f, 0.35f, 0.8f, 0.30f, 300,  -2),
            P ("Loud Master",     "Master",    0.30f, 0.20f, 0.7f, 0.25f, 250,  -1),
            P ("Overclocked!",    "Master",    0.90f, 0.70f, 1.0f, 0.60f, 400,  -4),
            P ("Air Only",        "Master",    0.20f, 0.00f, 0.4f, 0.80f, 3000, 0),
        };
    }

    void prepare (double sr, int maxBlock) override
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) maxBlock, 2 };
        lr.prepare (spec);  lr.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
        osLow.initProcessing ((size_t) maxBlock);  osHigh.initProcessing ((size_t) maxBlock);
        lowBuf.setSize (2, maxBlock);  highBuf.setSize (2, maxBlock);
        osLow.reset(); osHigh.reset(); lr.reset();
        dc[0] = dc[1] = dcX[0] = dcX[1] = 0.0f;
        dcR = (float) std::exp (-juce::MathConstants<double>::twoPi * 35.0 / sr);
        setLatencySamples (juce::roundToInt (osLow.getLatencyInSamples()));
    }

    void process (juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        const int n = buf.getNumSamples(), nch = juce::jmin (2, buf.getNumChannels());
        const float power = get ("power"), curve = get ("curve"), density = get ("density"), harm = get ("harm");
        const float outG = juce::Decibels::decibelsToGain (get ("out")) / (1.0f + 1.5f * power);   // compensation de loudness
        lr.setCutoffFrequency (get ("split"));

        // 1) split LR4 (somme plate en amplitude)
        for (int c = 0; c < nch; ++c)
        {
            const float* in = buf.getReadPointer (c);  float* lo = lowBuf.getWritePointer (c);  float* hi = highBuf.getWritePointer (c);
            for (int i = 0; i < n; ++i) lr.processSample (c, in[i], lo[i], hi[i]);
        }
        // LR4 : la branche haute doit etre inversee de phase pour sommer plat avec le passe-bas
        // (juce::LinkwitzRileyFilter fournit deja high = sortie coherente en phase) -> aucune inversion necessaire.

        const float gLow  = 1.0f + 11.0f * std::pow (power, 1.5f);
        const float gHigh = 1.0f + 9.0f * harm * (0.4f + power);
        const float bias  = 0.25f * (1.0f - curve);                 // asymetrie -> harmoniques paires

        auto shape = [curve, bias] (float x, float g)
        {
            const float xb = x + bias * g / (1.0f + g);
            const float soft = std::tanh (g * xb), hard = juce::jlimit (-1.0f, 1.0f, g * xb);
            return ((1.0f - curve) * soft + curve * hard) / std::tanh (g);   // pic <= ~1, petits signaux montes
        };

        auto run = [&] (juce::dsp::Oversampling<float>& os, juce::AudioBuffer<float>& b, float g, float amount, bool isHigh)
        {
            juce::dsp::AudioBlock<float> blk (b);
            auto sub = blk.getSubBlock (0, (size_t) n).getSubsetChannelBlock (0, (size_t) nch);
            const float off0 = shape (0.0f, g);          // silence -> 0 (pas d'offset DC statique)
            auto up = os.processSamplesUp (sub);
            for (size_t c = 0; c < up.getNumChannels(); ++c)
            {
                float* d = up.getChannelPointer (c);
                for (size_t i = 0; i < up.getNumSamples(); ++i)
                {
                    const float x = d[i], y = shape (x, g) - off0;
                    // bande haute : on n'ajoute que les harmoniques generees (y - x), bande basse : melange densite
                    d[i] = isHigh ? x + amount * (y - x) : x + amount * (y - x);
                }
            }
            os.processSamplesDown (sub);
        };
        run (osLow,  lowBuf,  gLow,  density, false);
        run (osHigh, highBuf, gHigh, juce::jmin (1.0f, harm * 1.4f), true);

        // 2) recombinaison + blocage DC + limite douce
        for (int c = 0; c < nch; ++c)
        {
            float* out = buf.getWritePointer (c);  const float* lo = lowBuf.getReadPointer (c);  const float* hi = highBuf.getReadPointer (c);
            for (int i = 0; i < n; ++i)
            {
                float y = (lo[i] + hi[i]);
                const float yd = y - dcX[c] + dcR * dc[c];  dcX[c] = y;  dc[c] = yd;  y = yd * outG;
                if (std::abs (y) > 0.9f) y = std::copysign (0.9f + 0.1f * std::tanh ((std::abs (y) - 0.9f) / 0.1f), y);
                out[i] = y;
            }
        }
        aux[0].store (power);
    }

private:
    juce::dsp::LinkwitzRileyFilter<float> lr;
    juce::dsp::Oversampling<float> osLow, osHigh;
    juce::AudioBuffer<float> lowBuf, highBuf;
    float dc[2] { 0, 0 }, dcX[2] { 0, 0 }, dcR = 0.995f;
};

class OverclockEditor : public GenericEditor
{
public:
    explicit OverclockEditor (OverclockProcessor& p) : GenericEditor (p, makeSpec()) { finishSetup(); }

    static UiSpec makeSpec()
    {
        UiSpec s;
        s.title = "OVERCLOCK";  s.subtitle = "POWER ENHANCER  //  DENSITY - HARMONICS - LOUDNESS";
        s.style = BodyStyle::gamer;
        s.body1 = juce::Colour (0xff1d1d24);  s.body2 = juce::Colour (0xff08080b);  s.accent = col::orange;
        s.knobs = { { "power", "POWER" }, { "density", "DENSITY" }, { "curve", "CURVE" }, { "harm", "HARMONICS", true },
                    { "split", "SPLIT", true }, { "out", "OUTPUT" } };
        return s;
    }

protected:
    void tick() override
    {
        const float p = proc.aux[0].load();
        shownPower += (p - shownPower) * 0.15f;
        speed += ((0.04f + shownPower * 0.5f) - speed) * 0.05f;      // le ventilateur accelere avec Power
        angle += speed;
    }

    void paintDisplay (juce::Graphics& g, juce::Rectangle<float> a, float t) override
    {
        using namespace juce;
        const auto c = a.getCentre();
        // anneau lumineux ambre -> rouge
        const auto ringCol = col::amber.interpolatedWith (col::red, shownPower);
        const float rr = a.getHeight() * 0.40f;
        Path track;  track.addCentredArc (c.x - 150, c.y, rr, rr, 0, degreesToRadians (225.0f), degreesToRadians (495.0f), true);
        g.setColour (Colours::white.withAlpha (0.07f));  g.strokePath (track, PathStrokeType (14.0f, PathStrokeType::curved, PathStrokeType::rounded));
        Path v;  v.addCentredArc (c.x - 150, c.y, rr, rr, 0, degreesToRadians (225.0f), degreesToRadians (225.0f + 270.0f * shownPower), true);
        g.setColour (ringCol.withAlpha (0.25f));  g.strokePath (v, PathStrokeType (26.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (ringCol);                    g.strokePath (v, PathStrokeType (14.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setFont (Theme::lcdFont (22.0f));  g.setColour (ringCol);
        g.drawText (String (roundToInt (shownPower * 100.0f)), (int) (c.x - 150 - 60), (int) (c.y - 14), 120, 28, Justification::centred);
        g.setFont (Theme::lcdFont (9.0f));  g.setColour (col::textDim);
        g.drawText ("POWER %", (int) (c.x - 150 - 60), (int) (c.y + 20), 120, 14, Justification::centred);

        // ventilateur anime : 7 pales
        const Point<float> fc (c.x + 170, c.y);
        const float fr = a.getHeight() * 0.38f;
        g.setColour (Colour (0xff0e0e12));  g.fillEllipse (fc.x - fr - 6, fc.y - fr - 6, (fr + 6) * 2, (fr + 6) * 2);
        for (int i = 0; i < 7; ++i)
        {
            Path blade;  blade.addPieSegment (fc.x - fr, fc.y - fr, fr * 2, fr * 2, 0.0f, 0.55f, 0.18f);
            g.setColour (Colour (0xff3a3a44).withMultipliedBrightness (1.0f + 0.15f * (float) (i % 2)));
            g.fillPath (blade, AffineTransform::rotation (angle + (float) i * MathConstants<float>::twoPi / 7.0f, fc.x, fc.y));
        }
        g.setColour (Colour (0xff17171c));  g.fillEllipse (fc.x - 16, fc.y - 16, 32, 32);
        g.setColour (ringCol);  g.drawEllipse (fc.x - 16, fc.y - 16, 32, 32, 2.0f);
        g.setColour (Colours::white.withAlpha (0.08f));
        g.drawEllipse (fc.x - fr - 4, fc.y - fr - 4, (fr + 4) * 2, (fr + 4) * 2, 3.0f);
        ignoreUnused (t);
    }

private:
    float shownPower = 0, speed = 0.04f, angle = 0;
};

juce::AudioProcessorEditor* OverclockProcessor::createEditor() { return new OverclockEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OverclockProcessor(); }
