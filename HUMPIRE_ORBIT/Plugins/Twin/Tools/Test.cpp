// HUMPIRE TWIN - headless DSP / plugin test (console).
//   cmake --build <build> --target HumpireTwinTest && HumpireTwinTest
// Exit code 0 = every check passed.
#include "PluginProcessor.h"
#include "TwinPresets.h"
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <vector>

static int failures = 0;
#define CHECK(cond, ...) do { if (cond) std::printf ("  [OK]   " __VA_ARGS__); else { ++failures; std::printf ("  [FAIL] " __VA_ARGS__); } std::printf ("\n"); } while (0)

//==============================================================================
// Allocation counter: replaces global new/delete so the test can prove that processBlock allocates nothing.
static std::atomic<long> gAllocations { 0 };
static std::atomic<bool> gCounting { false };

void* operator new (std::size_t n)
{
    if (gCounting.load (std::memory_order_relaxed)) gAllocations.fetch_add (1, std::memory_order_relaxed);
    if (void* p = std::malloc (n != 0 ? n : 1)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
static constexpr double kSr = 48000.0;

static void setParam (TwinAudioProcessor& p, const char* id, float real)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (real));
}

static float getParam (TwinAudioProcessor& p, const char* id)
{
    return p.apvts.getRawParameterValue (id)->load();
}

/** Neutral, deterministic starting point: one double, centred, 20 ms, no humanize, full wet. */
static void neutral (TwinAudioProcessor& p)
{
    setParam (p, "doubles", 1.0f);    setParam (p, "detune", 0.0f);     setParam (p, "delay", 20.0f);
    setParam (p, "humanize", 0.0f);   setParam (p, "drift", 0.3f);      setParam (p, "width", 0.0f);
    setParam (p, "hpFreq", 20.0f);    setParam (p, "lpFreq", 20000.0f); setParam (p, "mix", 100.0f);
    setParam (p, "outGain", 0.0f);    setParam (p, "bypass", 0.0f);     setParam (p, "simple", 0.0f);
}

/** Vocal-like source: band-limited sawtooth (glottal-like) at f0 plus a little breath noise. */
static std::vector<float> vocal (double seconds, double f0, float amp = 0.5f, double sr = kSr)
{
    std::vector<float> x ((size_t) (sr * seconds));
    juce::Random rng (4242);
    double ph = 0.0;
    float lp = 0.0f;
    for (auto& s : x)
    {
        ph += f0 / sr;
        if (ph >= 1.0) ph -= 1.0;
        lp += ((float) (2.0 * ph - 1.0) - lp) * 0.2f;
        s = amp * (lp + 0.02f * (rng.nextFloat() * 2.0f - 1.0f));
    }
    return x;
}

/** Noise-free harmonic source (two cascaded one-pole filters on a sawtooth): every harmonic is a clean spectral line. */
static std::vector<float> harmonicTone (double seconds, double f0, double sr = kSr)
{
    std::vector<float> x ((size_t) (sr * seconds));
    double ph = 0.0;
    float a = 0.0f, b = 0.0f;
    for (auto& s : x)
    {
        ph += f0 / sr;
        if (ph >= 1.0) ph -= 1.0;
        a += ((float) (2.0 * ph - 1.0) - a) * 0.15f;
        b += (a - b) * 0.15f;
        s = 2.0f * b;
    }
    return x;
}

static std::vector<float> sine (double seconds, double f, float amp, double sr = kSr)
{
    std::vector<float> x ((size_t) (sr * seconds));
    for (size_t i = 0; i < x.size(); ++i)
        x[i] = amp * (float) std::sin (juce::MathConstants<double>::twoPi * f * (double) i / sr);
    return x;
}

/** Runs a mono signal through the stereo bus, `block` samples per processBlock call. */
static void render (TwinAudioProcessor& p, const std::vector<float>& in, int block,
                    std::vector<float>& L, std::vector<float>& R)
{
    L.assign (in.size(), 0.0f);
    R.assign (in.size(), 0.0f);
    juce::MidiBuffer midi;
    for (size_t start = 0; start < in.size(); start += (size_t) block)
    {
        const int n = (int) juce::jmin ((size_t) block, in.size() - start);
        juce::AudioBuffer<float> buf (2, n);
        for (int i = 0; i < n; ++i)
        {
            buf.setSample (0, i, in[start + (size_t) i]);
            buf.setSample (1, i, in[start + (size_t) i]);
        }
        p.processBlock (buf, midi);
        for (int i = 0; i < n; ++i)
        {
            L[start + (size_t) i] = buf.getSample (0, i);
            R[start + (size_t) i] = buf.getSample (1, i);
        }
    }
}

static float peakOf (const std::vector<float>& x, size_t from, size_t to)
{
    float m = 0.0f;
    for (size_t i = from; i < to && i < x.size(); ++i) m = juce::jmax (m, std::abs (x[i]));
    return m;
}

static double rmsOf (const std::vector<float>& x, size_t from, size_t to)
{
    double acc = 0.0;
    size_t n = 0;
    for (size_t i = from; i < to && i < x.size(); ++i, ++n) acc += (double) x[i] * (double) x[i];
    return n > 0 ? std::sqrt (acc / (double) n) : 0.0;
}

static size_t argMaxAbs (const std::vector<float>& x)
{
    size_t best = 0;
    for (size_t i = 1; i < x.size(); ++i)
        if (std::abs (x[i]) > std::abs (x[best])) best = i;
    return best;
}

/** Frequency from positive-going zero crossings (linear interpolation) between two indices. */
static double zeroCrossHz (const std::vector<float>& x, double sr, size_t from, size_t to)
{
    double first = -1.0, last = -1.0;
    int count = 0;
    for (size_t i = from + 1; i < to && i < x.size(); ++i)
        if (x[i - 1] < 0.0f && x[i] >= 0.0f)
        {
            const double pos = (double) i - 1.0 + (double) (-x[i - 1]) / (double) (x[i] - x[i - 1]);
            if (first < 0.0) first = pos;
            else { last = pos; ++count; }
        }
    return count > 0 ? sr * count / (last - first) : 0.0;
}

/** Frequency of the spectral line closest to `centre` (Hann-windowed DFT scan, 0.02 Hz steps). */
static double lineHz (const std::vector<float>& x, double sr, size_t from, size_t to, double centre, double span)
{
    double bestMag = -1.0, bestF = centre;
    const double len = (double) (to - from);
    for (double f = centre - span; f <= centre + span; f += 0.02)
    {
        const double w = juce::MathConstants<double>::twoPi * f / sr;
        double re = 0.0, im = 0.0;
        for (size_t i = from; i < to; ++i)
        {
            const double win = 0.5 - 0.5 * std::cos (juce::MathConstants<double>::twoPi * (double) (i - from) / len);
            re += win * (double) x[i] * std::cos (w * (double) i);
            im += win * (double) x[i] * std::sin (w * (double) i);
        }
        const double mag = re * re + im * im;
        if (mag > bestMag) { bestMag = mag; bestF = f; }
    }
    return bestF;
}

static bool finiteAndBounded (const std::vector<float>& x, float limit = 4.0f)
{
    for (float v : x)
        if (! std::isfinite (v) || std::abs (v) > limit) return false;
    return true;
}

static bool allFinite (const juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float v = b.getSample (c, i);
            if (! std::isfinite (v) || std::abs (v) > 4.0f) return false;
        }
    return true;
}

static bool isPlainEnglish (const juce::String& s)
{
    if (s.isEmpty()) return false;
    for (auto c : s)
        if (c < 32 || c > 126) return false;
    return true;
}

//==============================================================================
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    const size_t s1 = (size_t) kSr;   // one second

    std::printf ("== Factory presets: 20 presets, English categories and names, values inside the parameter ranges ==\n");
    {
        const auto list = makeTwinFactoryPresets();
        CHECK (list.size() == 20, "%d factory presets (expected 20)", (int) list.size());

        const juce::StringArray allowed { "Trap Vocals", "Afrotrap", "Oriental / Rai", "Deep House", "G-Funk", "R&B",
                                          "Pop Vocals", "Spoken Word / Podcast", "Ambient / Cinematic", "Utility" };
        juce::StringArray categories, names;
        bool englishOk = true, allowedOk = true, rangeOk = true, idsOk = true;
        TwinAudioProcessor p;
        for (const auto& pr : list)
        {
            englishOk = englishOk && isPlainEnglish (pr.category) && isPlainEnglish (pr.name);
            allowedOk = allowedOk && allowed.contains (pr.category);
            categories.addIfNotAlreadyThere (pr.category);
            names.add (pr.name);
            for (const auto& [pid, value] : pr.values)
            {
                auto* prm = p.apvts.getParameter (pid);
                idsOk = idsOk && prm != nullptr;
                if (prm != nullptr)
                {
                    const float back = prm->convertFrom0to1 (prm->convertTo0to1 (value));
                    rangeOk = rangeOk && std::abs (back - value) < 0.051f + 0.001f * std::abs (value);   // survives clamping to [min, max]
                }
            }
        }
        names.removeDuplicates (false);
        CHECK (englishOk, "all category and preset names are plain English / ASCII");
        CHECK (allowedOk, "every category is one of the %d expected English categories", allowed.size());
        CHECK (categories.size() >= 8, "%d distinct categories", categories.size());
        CHECK (names.size() == 20, "%d unique preset names", names.size());
        CHECK (idsOk && rangeOk, "every preset value targets an existing parameter and lies inside its range");
        CHECK (p.presets.getNumPresets() >= 20, "PresetManager exposes %d presets", p.presets.getNumPresets());
    }

    std::printf ("== Parameters: English names, display text and typed values round-trip ==\n");
    {
        TwinAudioProcessor p;
        bool namesOk = true, textOk = true, typedOk = true;
        for (auto* base : p.getParameters())
        {
            namesOk = namesOk && isPlainEnglish (base->getName (64));
            if (auto* fp = dynamic_cast<juce::AudioParameterFloat*> (base))
            {
                for (float n : { 0.0f, 0.13f, 0.5f, 0.77f, 1.0f })
                {
                    const auto txt = base->getText (n, 0);
                    const float back = fp->convertFrom0to1 (base->getValueForText (txt));
                    const float orig = fp->convertFrom0to1 (n);
                    const float range = fp->getNormalisableRange().end - fp->getNormalisableRange().start;
                    // the text shows at most 3 significant digits for kHz, so allow a tolerance of 0.6 % of the range
                    const bool ok = std::abs (back - orig) <= juce::jmax (fp->getNormalisableRange().interval, 0.006f * range);
                    if (! ok) std::printf ("    %s: %.4f -> \"%s\" -> %.4f\n", fp->paramID.toRawUTF8(), orig, txt.toRawUTF8(), back);
                    textOk = textOk && ok;
                }
            }
        }
        CHECK (namesOk, "parameter names are plain English");
        CHECK (textOk, "display text of every float parameter parses back to the same value");

        auto typed = [&] (const char* id, const char* text, float expected, float tol)
        {
            auto* prm = p.apvts.getParameter (id);
            const float got = prm->convertFrom0to1 (prm->getValueForText (text));
            const bool ok = std::abs (got - expected) <= tol;
            if (! ok) std::printf ("    typed \"%s\" into %s -> %.3f (expected %.3f)\n", text, id, got, expected);
            typedOk = typedOk && ok;
        };
        typed ("detune", "20 ct", 20.0f, 0.06f);   typed ("detune", "7.5", 7.5f, 0.06f);
        typed ("delay", "25 ms", 25.0f, 0.06f);    typed ("delay", "12,5", 12.5f, 0.06f);
        typed ("humanize", "70 %", 70.0f, 0.51f);  typed ("width", "100%", 100.0f, 0.51f);
        typed ("mix", "35", 35.0f, 0.51f);         typed ("drift", "0.5 Hz", 0.5f, 0.011f);
        typed ("hpFreq", "300 Hz", 300.0f, 1.0f);  typed ("hpFreq", "0.3k", 300.0f, 1.0f);
        typed ("lpFreq", "8 kHz", 8000.0f, 1.0f);  typed ("lpFreq", "1.5k", 1500.0f, 1.0f);
        typed ("lpFreq", "9000", 9000.0f, 1.0f);   typed ("lpFreq", "12", 12000.0f, 1.0f);
        typed ("outGain", "-6 dB", -6.0f, 0.06f);  typed ("outGain", "+3.5", 3.5f, 0.06f);
        typed ("doubles", "3", 3.0f, 0.01f);       typed ("doubles", "9", 4.0f, 0.01f);
        CHECK (typedOk, "typed values (units, comma decimals, k suffix, clamping) are understood");

        auto* d = p.apvts.getParameter ("doubles");
        auto* dt = p.apvts.getParameter ("detune");
        CHECK (d->getText (d->convertTo0to1 (3.0f), 0) == "3", "integer parameter 'doubles' shows no decimals (\"%s\")", d->getText (d->convertTo0to1 (3.0f), 0).toRawUTF8());
        CHECK (p.apvts.getParameter ("mix")->getText (p.apvts.getParameter ("mix")->convertTo0to1 (40.0f), 0) == "40 %", "percent shows \"40 %%\"");
        CHECK (dt->getText (dt->convertTo0to1 (12.0f), 0) == "12.0 ct", "detune shows \"%s\"", dt->getText (dt->convertTo0to1 (12.0f), 0).toRawUTF8());
        auto* lp = p.apvts.getParameter ("lpFreq");
        CHECK (lp->getText (lp->convertTo0to1 (12000.0f), 0) == "12.0 kHz", "low-pass shows \"%s\"", lp->getText (lp->convertTo0to1 (12000.0f), 0).toRawUTF8());
        auto* og = p.apvts.getParameter ("outGain");
        CHECK (og->getText (og->convertTo0to1 (0.0f), 0) == "0.0 dB", "output shows \"%s\" (no negative zero)", og->getText (og->convertTo0to1 (0.0f), 0).toRawUTF8());
    }

    std::printf ("== Safety: before prepareToPlay, zero samples, mono, blocks larger than the prepared size ==\n");
    {
        TwinAudioProcessor p;
        juce::MidiBuffer m;
        juce::AudioBuffer<float> b (2, 256);
        for (int i = 0; i < 256; ++i) { b.setSample (0, i, 0.25f); b.setSample (1, i, -0.5f); }
        p.processBlock (b, m);
        CHECK (b.getSample (0, 10) == 0.25f && b.getSample (1, 10) == -0.5f, "processBlock before prepareToPlay passes audio through, no crash");

        juce::AudioBuffer<float> bigBefore (2, 20000);
        bigBefore.clear();
        p.processBlock (bigBefore, m);
        juce::AudioBuffer<float> monoBefore (1, 100), zeroBefore (2, 0);
        p.processBlock (monoBefore, m);
        p.processBlock (zeroBefore, m);
        CHECK (true, "huge / mono / empty blocks before prepareToPlay do not crash");

        p.prepareToPlay (kSr, 64);   // prepared for 64 samples only
        juce::AudioBuffer<float> zero (2, 0);
        p.processBlock (zero, m);
        CHECK (true, "zero-sample block accepted");

        juce::AudioBuffer<float> mono (1, 512);
        const auto in = vocal (0.2, 200.0);
        for (int i = 0; i < 512; ++i) mono.setSample (0, i, in[(size_t) i]);
        p.processBlock (mono, m);
        float rmsMono = 0.0f;
        for (int i = 0; i < 512; ++i) rmsMono += mono.getSample (0, i) * mono.getSample (0, i);
        CHECK (allFinite (mono) && rmsMono > 0.0f, "mono buffer: finite output, energy %.4f", rmsMono);

        juce::AudioBuffer<float> huge (2, 100000);
        const auto big = vocal (100000.0 / kSr, 180.0);
        for (int i = 0; i < 100000; ++i) { huge.setSample (0, i, big[(size_t) i]); huge.setSample (1, i, big[(size_t) i]); }
        p.processBlock (huge, m);
        CHECK (allFinite (huge), "a 100000-sample block (prepared for 64) is processed without a problem");
    }

    std::printf ("== Real-time safety: no heap allocation while processing and while parameters change ==\n");
    {
        TwinAudioProcessor p;
        p.prepareToPlay (kSr, 512);
        juce::MidiBuffer m;
        juce::AudioBuffer<float> b (2, 512);
        const auto in = vocal (0.5, 190.0);
        juce::Random rng (99);
        long total = 0;
        for (int block = 0; block < 120; ++block)
        {
            // parameter changes happen outside the counted section, as they do on the message thread
            if (block % 3 == 0)
            {
                setParam (p, "doubles", (float) (1 + rng.nextInt (4)));
                setParam (p, "detune", rng.nextFloat() * 50.0f);
                setParam (p, "delay", 5.0f + rng.nextFloat() * 35.0f);
                setParam (p, "humanize", rng.nextFloat() * 100.0f);
                setParam (p, "mix", rng.nextFloat() * 100.0f);
                setParam (p, "hpFreq", 20.0f + rng.nextFloat() * 700.0f);
                setParam (p, "bypass", rng.nextInt (5) == 0 ? 1.0f : 0.0f);
            }
            for (int i = 0; i < 512; ++i)
            {
                const float v = in[(size_t) ((block * 512 + i) % (int) in.size())];
                b.setSample (0, i, v);
                b.setSample (1, i, v);
            }
            gAllocations = 0;
            gCounting = true;
            p.processBlock (b, m);
            gCounting = false;
            total += gAllocations.load();
        }
        CHECK (total == 0, "%ld heap allocations in 120 processBlock calls with changing parameters", total);

        // the counter itself must work, otherwise the zero above proves nothing
        gAllocations = 0;
        gCounting = true;
        {
            std::vector<float> probe ((size_t) (64 + rng.nextInt (64)));
            volatile float sink = probe[0];
            (void) sink;
            const juce::String text = juce::String ("probe ") + juce::String (rng.nextInt (1000));
            volatile int len = text.length();
            (void) len;
        }
        gCounting = false;
        CHECK (gAllocations.load() > 0, "allocation counter is live (%ld allocations seen for a std::vector and a juce::String)", gAllocations.load());
    }

    std::printf ("== 20 factory presets render without NaN / Inf ==\n");
    {
        TwinAudioProcessor p;
        p.prepareToPlay (kSr, 512);
        const auto in = vocal (0.5, 180.0);
        std::vector<float> L, R;
        bool ok = true;
        float peak = 0.0f;
        const int n = p.presets.getNumPresets();
        for (int i = 0; i < n; ++i)
        {
            p.presets.loadPreset (i);
            render (p, in, 512, L, R);
            ok = ok && finiteAndBounded (L) && finiteAndBounded (R);
            peak = juce::jmax (peak, peakOf (L, 0, L.size()), peakOf (R, 0, R.size()));
        }
        CHECK (ok && n >= 20, "%d presets rendered, no NaN/Inf, output peak %.3f", n, peak);
    }

    std::printf ("== Micro-delay: impulse returns after the requested delay ==\n");
    {
        TwinAudioProcessor p;
        neutral (p);
        p.prepareToPlay (kSr, 512);
        std::vector<float> in ((size_t) (0.2 * kSr), 0.0f);
        in[0] = 1.0f;
        std::vector<float> L, R;
        render (p, in, 512, L, R);
        const size_t at = argMaxAbs (L);
        const double want = 20.0 * kSr / 1000.0;
        CHECK (std::abs ((double) at - want) <= 2.0, "impulse at %zu samples (expected %.0f = 20 ms)", at, want);
    }

    std::printf ("== Micro-detune is a true pitch ratio (cents), not a constant Hz shift ==\n");
    {
        for (double f : { 110.0, 165.0, 220.0, 330.0 })
        {
            TwinAudioProcessor p;
            neutral (p);
            setParam (p, "detune", 30.0f);
            p.prepareToPlay (kSr, 512);
            std::vector<float> L, R;
            render (p, sine (3.0, f, 0.3f), 512, L, R);
            const double m = zeroCrossHz (L, kSr, s1, 3 * s1);
            const double ct = 1200.0 * std::log2 (m / f);
            CHECK (std::abs (ct - 30.0) < 1.5, "%.0f Hz sine, detune +30 ct: measured %.2f Hz = %+.2f cents", f, m, ct);
        }

        // harmonic voice: every harmonic must move by the same number of cents
        for (double f0 : { 100.0, 140.0, 210.0 })
        {
            TwinAudioProcessor p;
            neutral (p);
            setParam (p, "detune", 15.0f);
            p.prepareToPlay (kSr, 512);
            std::vector<float> L, R;
            render (p, harmonicTone (4.0, f0), 512, L, R);
            const double ratio = std::pow (2.0, 15.0 / 1200.0);
            double worst = 0.0;
            for (int h : { 1, 2, 5 })
            {
                const double fm = lineHz (L, kSr, s1, 4 * s1, (double) h * f0 * ratio, 2.0);
                worst = juce::jmax (worst, std::abs (1200.0 * std::log2 (fm / ((double) h * f0)) - 15.0));
            }
            CHECK (worst < 1.5, "harmonic source f0 = %.0f Hz, detune +15 ct: harmonics 1, 2 and 5 within %.2f ct of +15", f0, worst);
        }

        // negative detune, and a short delay (the offset range has to widen to span a pitch period)
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "detune", 20.0f);
        setParam (p, "delay", 6.0f);
        p.prepareToPlay (kSr, 512);
        std::vector<float> L, R;
        render (p, sine (3.0, 150.0, 0.3f), 512, L, R);
        const double ct = 1200.0 * std::log2 (zeroCrossHz (L, kSr, s1, 3 * s1) / 150.0);
        CHECK (std::abs (ct - 20.0) < 2.0, "6 ms delay, 150 Hz, detune +20 ct: measured %+.2f cents", ct);
    }

    std::printf ("== Detuned voice stays clean: no level dips or clicks at the hand-over between taps ==\n");
    {
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "detune", 25.0f);
        setParam (p, "delay", 15.0f);
        p.prepareToPlay (kSr, 512);
        std::vector<float> L, R;
        render (p, sine (6.0, 200.0, 0.4f), 512, L, R);
        double lo = 1e9, hi = 0.0;
        for (size_t st = s1; st + 240 <= L.size(); st += 240)
        {
            const double r = rmsOf (L, st, st + 240);
            lo = juce::jmin (lo, r);
            hi = juce::jmax (hi, r);
        }
        double maxStep = 0.0;
        for (size_t i = s1 + 1; i < L.size(); ++i) maxStep = juce::jmax (maxStep, (double) std::abs (L[i] - L[i - 1]));
        const double expectedStep = 0.4 * juce::MathConstants<double>::twoPi * 200.0 / kSr;   // steepest slope of the input sine
        const double nominal = 0.4 / std::sqrt (2.0) * std::cos (juce::MathConstants<double>::pi * 0.25);   // sine RMS x centre pan gain
        CHECK (lo > 0.9 * hi && lo > 0.9 * nominal && hi < 1.1 * nominal, "5 ms RMS stays within %.3f..%.3f of the nominal %.3f", lo, hi, nominal);
        CHECK (maxStep < 1.15 * expectedStep, "largest sample step %.4f (sine slope %.4f): no clicks", maxStep, expectedStep);
    }

    std::printf ("== Stereo width: 0 %% = centred doubles (L == R), 100 %% = spread ==\n");
    {
        const auto in = vocal (1.0, 200.0);
        std::vector<float> L, R;

        TwinAudioProcessor narrow;
        neutral (narrow);
        setParam (narrow, "doubles", 2.0f);
        setParam (narrow, "width", 0.0f);
        narrow.prepareToPlay (kSr, 512);
        render (narrow, in, 512, L, R);
        float maxDiff = 0.0f;
        for (size_t i = s1 / 4; i < L.size(); ++i) maxDiff = juce::jmax (maxDiff, std::abs (L[i] - R[i]));
        CHECK (maxDiff < 1e-4f, "width 0 %%: max |L-R| = %.2e", maxDiff);

        TwinAudioProcessor wide;
        neutral (wide);
        setParam (wide, "doubles", 2.0f);
        setParam (wide, "width", 100.0f);
        wide.prepareToPlay (kSr, 512);
        render (wide, in, 512, L, R);
        float wideDiff = 0.0f;
        for (size_t i = s1 / 4; i < L.size(); ++i) wideDiff = juce::jmax (wideDiff, std::abs (L[i] - R[i]));
        const float pk = peakOf (L, s1 / 4, L.size());
        CHECK (wideDiff > 0.2f * pk, "width 100 %%: max |L-R| = %.3f (peak %.3f)", wideDiff, pk);
    }

    std::printf ("== Double filter: low-pass and high-pass attenuate their stop band ==\n");
    {
        auto rmsWith = [&] (const char* param, float value, double f)
        {
            TwinAudioProcessor p;
            neutral (p);
            setParam (p, param, value);
            p.prepareToPlay (kSr, 512);
            std::vector<float> L, R;
            render (p, sine (1.0, f, 0.3f), 512, L, R);
            return rmsOf (L, s1 / 2, L.size());
        };
        const double lpOpen = rmsWith ("lpFreq", 20000.0f, 5000.0);
        const double lpClosed = rmsWith ("lpFreq", 1500.0f, 5000.0);
        CHECK (lpClosed < 0.25 * lpOpen, "low-pass 1.5 kHz vs 5 kHz tone: ratio %.3f", lpClosed / lpOpen);
        const double hpOpen = rmsWith ("hpFreq", 20.0f, 100.0);
        const double hpClosed = rmsWith ("hpFreq", 800.0f, 100.0);
        CHECK (hpClosed < 0.25 * hpOpen, "high-pass 800 Hz vs 100 Hz tone: ratio %.3f", hpClosed / hpOpen);
    }

    std::printf ("== Humanize: micro-delay wanders randomly inside 5-40 ms ==\n");
    {
        auto delayRange = [&] (float humanize, float driftHz, float& lo, float& hi)
        {
            TwinAudioProcessor p;
            neutral (p);
            setParam (p, "humanize", humanize);
            setParam (p, "drift", driftHz);
            p.prepareToPlay (kSr, 512);
            const auto in = sine (10.0, 220.0, 0.3f);
            juce::MidiBuffer m;
            lo = 1e9f;
            hi = -1e9f;
            for (size_t start = 0; start + 512 <= in.size(); start += 512)
            {
                juce::AudioBuffer<float> b (2, 512);
                for (int i = 0; i < 512; ++i) { b.setSample (0, i, in[start + (size_t) i]); b.setSample (1, i, in[start + (size_t) i]); }
                p.processBlock (b, m);
                const float d = p.ui.delayMs[0].load();
                lo = juce::jmin (lo, d);
                hi = juce::jmax (hi, d);
            }
        };
        float lo = 0.0f, hi = 0.0f;
        delayRange (100.0f, 1.0f, lo, hi);
        CHECK (hi - lo > 3.0f && lo >= 5.0f - 1e-3f && hi <= 40.0f + 1e-3f,
               "humanize 100 %%: micro-delay sweeps %.2f..%.2f ms", lo, hi);
        delayRange (0.0f, 1.0f, lo, hi);
        CHECK (hi - lo < 1e-3f, "humanize 0 %%: micro-delay is static (%.4f ms spread)", hi - lo);
    }

    std::printf ("== Voice count follows the Doubles selector ==\n");
    {
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "doubles", 4.0f);
        p.prepareToPlay (kSr, 512);
        const std::vector<float> in (512, 0.1f);
        std::vector<float> L, R;
        render (p, in, 512, L, R);
        const int four = p.ui.voices.load();
        setParam (p, "doubles", 3.0f);
        render (p, in, 512, L, R);
        const int three = p.ui.voices.load();
        CHECK (four == 4 && three == 3, "doubles 4 -> %d active, doubles 3 -> %d active", four, three);
    }

    std::printf ("== Block-size independence (prepared for 512; blocks of 1, 37, 512, 2048, 8192 samples) ==\n");
    {
        auto runWith = [&] (int block)
        {
            TwinAudioProcessor p;
            neutral (p);
            setParam (p, "doubles", 4.0f);
            setParam (p, "detune", 15.0f);
            setParam (p, "delay", 25.0f);
            setParam (p, "humanize", 50.0f);
            setParam (p, "drift", 0.5f);
            setParam (p, "width", 100.0f);
            setParam (p, "mix", 60.0f);
            p.prepareToPlay (kSr, 512);
            std::vector<float> L, R;
            render (p, vocal (0.5, 200.0), block, L, R);
            return L;
        };
        const auto ref = runWith (512);
        for (int b : { 1, 37, 2048, 8192 })
        {
            const auto out = runWith (b);
            float d = 0.0f;
            for (size_t i = 0; i < ref.size() && i < out.size(); ++i) d = juce::jmax (d, std::abs (out[i] - ref[i]));
            CHECK (d < 1e-6f, "block %d: max deviation from 512-sample blocks %.2e", b, d);
        }
    }

    std::printf ("== Bypass: output equals input exactly ==\n");
    {
        const auto in = vocal (0.6, 200.0);
        std::vector<float> L, R;

        // bypass already on when the host prepares the plugin, aggressive settings, large blocks
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "doubles", 4.0f);
        setParam (p, "detune", 40.0f);
        setParam (p, "mix", 100.0f);
        setParam (p, "outGain", 9.0f);
        setParam (p, "bypass", 1.0f);
        p.prepareToPlay (kSr, 512);
        render (p, in, 2048, L, R);
        float dL = 0.0f, dR = 0.0f;
        for (size_t i = 0; i < in.size(); ++i) { dL = juce::jmax (dL, std::abs (L[i] - in[i])); dR = juce::jmax (dR, std::abs (R[i] - in[i])); }
        CHECK (dL == 0.0f && dR == 0.0f, "bypass on from the start, mix 100 %%, +9 dB output: max deviation L %.1e / R %.1e (bit exact)", dL, dR);

        // bypass switched on while running: after the 15 ms ramp the output is the exact input again
        TwinAudioProcessor q;
        neutral (q);
        setParam (q, "doubles", 3.0f);
        setParam (q, "outGain", -6.0f);
        q.prepareToPlay (kSr, 512);
        const auto in2 = vocal (1.5, 170.0);
        std::vector<float> A, B;
        const size_t half = in2.size() / 2;
        std::vector<float> first (in2.begin(), in2.begin() + (long) half), second (in2.begin() + (long) half, in2.end());
        render (q, first, 300, A, B);
        float processedDiff = 0.0f;
        for (size_t i = s1 / 4; i < half; ++i) processedDiff = juce::jmax (processedDiff, std::abs (A[i] - first[i]));
        setParam (q, "bypass", 1.0f);
        render (q, second, 300, A, B);
        const size_t settle = (size_t) (0.05 * kSr);
        float bypassDiff = 0.0f;
        for (size_t i = settle; i < second.size(); ++i) bypassDiff = juce::jmax (bypassDiff, std::abs (A[i] - second[i]), std::abs (B[i] - second[i]));
        CHECK (processedDiff > 0.01f, "bypass off: the doubles are audible (deviation from dry %.3f)", processedDiff);
        CHECK (bypassDiff == 0.0f, "bypass switched on while playing: exact input after the ramp (deviation %.1e)", bypassDiff);

        // and back off again: the wet signal returns
        setParam (q, "bypass", 0.0f);
        render (q, second, 300, A, B);
        float backDiff = 0.0f;
        for (size_t i = settle; i < second.size(); ++i) backDiff = juce::jmax (backDiff, std::abs (A[i] - second[i]));
        CHECK (backDiff > 0.01f, "bypass switched off again: processed signal is back (deviation %.3f)", backDiff);

        // processBlockBypassed (used by some wrappers) and mono
        TwinAudioProcessor r;
        neutral (r);
        setParam (r, "bypass", 1.0f);
        r.prepareToPlay (kSr, 512);
        juce::MidiBuffer m;
        juce::AudioBuffer<float> mono (1, 777), ref (1, 777);
        for (int i = 0; i < 777; ++i) { mono.setSample (0, i, in[(size_t) i]); ref.setSample (0, i, in[(size_t) i]); }
        r.processBlockBypassed (mono, m);
        float dm = 0.0f;
        for (int i = 0; i < 777; ++i) dm = juce::jmax (dm, std::abs (mono.getSample (0, i) - ref.getSample (0, i)));
        CHECK (dm == 0.0f, "mono + processBlockBypassed: exact input (%.1e)", dm);
        CHECK (r.getLatencySamples() == 0, "bypass keeps the latency consistent (0 samples both ways)");
        CHECK (r.getBypassParameter() != nullptr, "bypass parameter is exposed to the host");
    }

    std::printf ("== State save / restore round-trips every parameter ==\n");
    {
        TwinAudioProcessor a;
        setParam (a, "doubles", 4.0f);   setParam (a, "detune", 37.3f);   setParam (a, "delay", 31.4f);
        setParam (a, "humanize", 83.0f); setParam (a, "drift", 1.07f);    setParam (a, "width", 27.0f);
        setParam (a, "hpFreq", 245.0f);  setParam (a, "lpFreq", 7321.0f); setParam (a, "mix", 66.0f);
        setParam (a, "outGain", -7.3f);  setParam (a, "bypass", 1.0f);    setParam (a, "simple", 1.0f);
        juce::MemoryBlock blob;
        a.getStateInformation (blob);

        TwinAudioProcessor b;
        b.setStateInformation (blob.getData(), (int) blob.getSize());
        bool same = true;
        int count = 0;
        for (auto* base : a.getParameters())
        {
            auto* pa = dynamic_cast<juce::RangedAudioParameter*> (base);
            for (auto* other : b.getParameters())
                if (auto* pb = dynamic_cast<juce::RangedAudioParameter*> (other); pb != nullptr && pb->paramID == pa->paramID)
                {
                    ++count;
                    if (std::abs (pa->getValue() - pb->getValue()) > 1.0e-6f) { same = false; std::printf ("    %s differs\n", pa->paramID.toRawUTF8()); }
                }
        }
        CHECK (same && count == a.getParameters().size(), "all %d parameters restored identically (%d compared)", a.getParameters().size(), count);
        CHECK (getParam (b, "doubles") == 4.0f && std::abs (getParam (b, "detune") - 37.3f) < 0.06f
               && getParam (b, "bypass") > 0.5f && getParam (b, "simple") > 0.5f, "spot check: doubles %.0f, detune %.2f, bypass %.0f, simple %.0f",
               getParam (b, "doubles"), getParam (b, "detune"), getParam (b, "bypass"), getParam (b, "simple"));

        // a second round trip is stable, and garbage is ignored
        juce::MemoryBlock blob2;
        b.getStateInformation (blob2);
        CHECK (blob2.getSize() > 0, "state blob is not empty (%d bytes)", (int) blob2.getSize());
        const float before = getParam (b, "mix");
        b.setStateInformation (nullptr, 0);
        const char junk[] = "this is not a state";
        b.setStateInformation (junk, (int) sizeof (junk));
        CHECK (getParam (b, "mix") == before, "empty / garbage state is ignored");
    }

    std::printf ("== Silence in, silence out (4 doubles, humanize 100 %%) ==\n");
    {
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "doubles", 4.0f);
        setParam (p, "humanize", 100.0f);
        p.prepareToPlay (kSr, 512);
        const std::vector<float> in (s1 / 4, 0.0f);
        std::vector<float> L, R;
        render (p, in, 512, L, R);
        const float m = juce::jmax (peakOf (L, 0, L.size()), peakOf (R, 0, R.size()));
        CHECK (m < 1e-9f, "silence -> silence (peak %.2e)", m);
    }

    std::printf ("== NaN / Inf in the input do not poison the plugin ==\n");
    {
        TwinAudioProcessor p;
        p.prepareToPlay (kSr, 512);
        auto in = vocal (1.0, 180.0);
        in[1000] = std::nanf ("");
        in[1001] = INFINITY;
        in[1002] = -INFINITY;
        std::vector<float> L, R;
        render (p, in, 512, L, R);
        bool ok = true;
        for (size_t i = 5000; i < L.size(); ++i) ok = ok && std::isfinite (L[i]) && std::isfinite (R[i]);
        CHECK (ok, "output is finite again right after a burst of NaN / Inf samples");
    }

    std::printf ("== Parameter automation fuzz: random jumps every block stay finite and bounded ==\n");
    {
        bool ok = true;
        float peak = 0.0f;
        for (int block : { 64, 480, 1000 })
        {
            TwinAudioProcessor p;
            p.prepareToPlay (kSr, block);
            juce::Random rng (1234 + block);
            const auto in = vocal (4.0, 140.0, 0.6f);
            juce::MidiBuffer m;
            for (size_t start = 0; start + (size_t) block <= in.size(); start += (size_t) block)
            {
                if (rng.nextInt (3) == 0)
                {
                    setParam (p, "doubles", (float) (1 + rng.nextInt (4)));
                    setParam (p, "detune", rng.nextFloat() * 50.0f);
                    setParam (p, "delay", 5.0f + rng.nextFloat() * 35.0f);
                    setParam (p, "humanize", rng.nextFloat() * 100.0f);
                    setParam (p, "drift", 0.05f + rng.nextFloat() * 1.45f);
                    setParam (p, "width", rng.nextFloat() * 100.0f);
                    setParam (p, "hpFreq", 20.0f + rng.nextFloat() * 780.0f);
                    setParam (p, "lpFreq", 1500.0f + rng.nextFloat() * 18500.0f);
                    setParam (p, "mix", rng.nextFloat() * 100.0f);
                    setParam (p, "outGain", -24.0f + rng.nextFloat() * 36.0f);
                    setParam (p, "bypass", rng.nextInt (6) == 0 ? 1.0f : 0.0f);
                }
                juce::AudioBuffer<float> b (2, block);
                for (int i = 0; i < block; ++i) { b.setSample (0, i, in[start + (size_t) i]); b.setSample (1, i, in[start + (size_t) i]); }
                p.processBlock (b, m);
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < block; ++i)
                    {
                        const float v = b.getSample (c, i);
                        ok = ok && std::isfinite (v) && std::abs (v) < 12.0f;
                        peak = juce::jmax (peak, std::abs (v));
                    }
            }
        }
        CHECK (ok, "3 x 4 s of random automation: finite, peak %.2f", peak);
    }

    std::printf ("== Other sample rates: delay, pitch and stability ==\n");
    {
        for (double sr : { 22050.0, 44100.0, 96000.0, 192000.0 })
        {
            const size_t one = (size_t) sr;
            {
                TwinAudioProcessor p;
                neutral (p);
                p.prepareToPlay (sr, 256);
                std::vector<float> in ((size_t) (0.2 * sr), 0.0f), L, R;
                in[0] = 1.0f;
                render (p, in, 256, L, R);
                const double want = 20.0 * sr / 1000.0;
                CHECK (std::abs ((double) argMaxAbs (L) - want) <= 2.0, "%.0f Hz: impulse at %zu samples (expected %.0f)", sr, argMaxAbs (L), want);
            }
            {
                TwinAudioProcessor p;
                neutral (p);
                setParam (p, "detune", 25.0f);
                p.prepareToPlay (sr, 256);
                std::vector<float> L, R;
                render (p, sine (3.0, 220.0, 0.3f, sr), 256, L, R);
                const double ct = 1200.0 * std::log2 (zeroCrossHz (L, sr, one, 3 * one) / 220.0);
                CHECK (std::abs (ct - 25.0) < 2.0, "%.0f Hz: 220 Hz sine, detune +25 ct measured %+.2f cents", sr, ct);
            }
            {
                TwinAudioProcessor p;
                p.prepareToPlay (sr, 256);
                setParam (p, "doubles", 4.0f);
                setParam (p, "humanize", 100.0f);
                std::vector<float> L, R;
                render (p, vocal (1.0, 150.0, 0.5f, sr), 256, L, R);
                CHECK (finiteAndBounded (L) && finiteAndBounded (R), "%.0f Hz: 4 doubles, humanize 100 %%: finite and bounded", sr);
            }
        }
    }

    std::printf ("== Latency and CPU ==\n");
    {
        TwinAudioProcessor p;
        p.prepareToPlay (kSr, 512);
        CHECK (p.getLatencySamples() == 0, "reported latency = %d samples", p.getLatencySamples());
        CHECK (p.getTailLengthSeconds() >= 0.06, "tail length %.0f ms covers the longest double", p.getTailLengthSeconds() * 1000.0);

        setParam (p, "doubles", 4.0f);
        setParam (p, "detune", 30.0f);
        setParam (p, "humanize", 100.0f);
        setParam (p, "drift", 1.5f);
        setParam (p, "mix", 100.0f);
        const auto in = vocal (10.0, 120.0);
        std::vector<float> L, R;
        const double t0 = juce::Time::getMillisecondCounterHiRes();
        render (p, in, 512, L, R);
        const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
        std::printf ("  [INFO] 10 s of audio, 4 doubles, humanize 100 %%, detune 30 ct: %.0f ms = %.0fx real time (this machine, one core)\n", ms, 10000.0 / ms);
        CHECK (ms < 5000.0, "processing is much faster than real time");
    }

    std::printf ("== Editor: builds, paints and is destroyed cleanly ==\n");
    {
        TwinAudioProcessor p;
        p.prepareToPlay (kSr, 512);
        setParam (p, "simple", 0.0f);
        const auto in = vocal (0.3, 200.0);
        std::vector<float> L, R;
        render (p, in, 512, L, R);
        {
            std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
            ed->setSize (960, 600);
            ed->resized();
            auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            setParam (p, "simple", 1.0f);
            setParam (p, "doubles", 3.0f);
            auto img2 = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            CHECK (img.getWidth() == 960 && img2.getHeight() == 600, "editor snapshot %dx%d (Advanced and Simple view)", img.getWidth(), img.getHeight());
        }
        CHECK (true, "editor destroyed without a crash");
    }

    std::printf (failures == 0 ? "\nALL TESTS PASS\n" : "\n%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
