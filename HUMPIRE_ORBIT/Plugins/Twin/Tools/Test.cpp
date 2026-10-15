// HUMPIRE TWIN - headless DSP test (console).
//   cmake --build <build> --target HumpireTwinTest && HumpireTwinTest
#include "PluginProcessor.h"
#include "TwinPresets.h"
#include <cmath>
#include <cstdio>
#include <vector>

static int failures = 0;
#define CHECK(cond, ...) do { if (cond) std::printf ("  [OK]   " __VA_ARGS__); else { ++failures; std::printf ("  [FAIL] " __VA_ARGS__); } std::printf ("\n"); } while (0)

static constexpr double kSr = 48000.0;

static void setParam (TwinAudioProcessor& p, const char* id, float real)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (real));
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
static std::vector<float> vocal (double seconds, double f0, float amp = 0.5f)
{
    std::vector<float> x ((size_t) (kSr * seconds));
    juce::Random rng (4242);
    double ph = 0.0;
    float lp = 0.0f;
    for (auto& s : x)
    {
        ph += f0 / kSr;
        if (ph >= 1.0) ph -= 1.0;
        lp += ((float) (2.0 * ph - 1.0) - lp) * 0.2f;
        s = amp * (lp + 0.02f * (rng.nextFloat() * 2.0f - 1.0f));
    }
    return x;
}

static std::vector<float> sine (double seconds, double f, float amp)
{
    std::vector<float> x ((size_t) (kSr * seconds));
    for (size_t i = 0; i < x.size(); ++i)
        x[i] = amp * (float) std::sin (juce::MathConstants<double>::twoPi * f * (double) i / kSr);
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

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    const size_t s1 = (size_t) kSr;   // one second

    std::printf ("== Factory presets ==\n");
    {
        const auto n = makeTwinFactoryPresets().size();
        CHECK (n == 20, "%d factory presets (expected 20)", (int) n);
    }

    std::printf ("== Safety: before prepareToPlay, zero samples, mono ==\n");
    {
        TwinAudioProcessor p;
        juce::MidiBuffer m;
        juce::AudioBuffer<float> b (2, 256);
        for (int i = 0; i < 256; ++i) { b.setSample (0, i, 0.25f); b.setSample (1, i, 0.25f); }
        p.processBlock (b, m);
        CHECK (b.getSample (0, 10) == 0.25f, "processBlock before prepareToPlay passes audio through, no crash");

        p.prepareToPlay (kSr, 512);
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
            for (size_t k = 0; k < L.size(); ++k)
            {
                if (! std::isfinite (L[k]) || ! std::isfinite (R[k]) || std::abs (L[k]) > 4.0f || std::abs (R[k]) > 4.0f)
                    ok = false;
                peak = juce::jmax (peak, std::abs (L[k]));
            }
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

    std::printf ("== Micro-detune: +30 cents shifts 220 Hz to 223.8 Hz ==\n");
    {
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "detune", 30.0f);
        p.prepareToPlay (kSr, 512);
        std::vector<float> L, R;
        render (p, sine (3.0, 220.0, 0.3f), 512, L, R);
        const double f = zeroCrossHz (L, kSr, s1, 3 * s1);
        const double ct = 1200.0 * std::log2 (f / 220.0);
        CHECK (std::abs (ct - 30.0) < 3.0, "measured %.2f Hz = %+.1f cents (target +30)", f, ct);
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

    std::printf ("== Block-size independence (1, 37, 512, 2048 samples) ==\n");
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
        for (int b : { 1, 37, 2048 })
        {
            const auto out = runWith (b);
            float d = 0.0f;
            for (size_t i = 0; i < ref.size() && i < out.size(); ++i) d = juce::jmax (d, std::abs (out[i] - ref[i]));
            CHECK (d < 1e-6f, "block %d: max deviation from 512-sample blocks %.2e", b, d);
        }
    }

    std::printf ("== Bypass is transparent ==\n");
    {
        TwinAudioProcessor p;
        neutral (p);
        setParam (p, "doubles", 4.0f);
        setParam (p, "bypass", 1.0f);
        p.prepareToPlay (kSr, 512);
        const auto in = vocal (0.5, 200.0);
        std::vector<float> L, R;
        render (p, in, 512, L, R);
        float d = 0.0f;
        for (size_t i = s1 / 4; i < in.size(); ++i) d = juce::jmax (d, std::abs (L[i] - in[i]));
        CHECK (d < 1e-4f, "bypass: max deviation from dry input %.2e", d);
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

    std::printf ("== Latency ==\n");
    {
        TwinAudioProcessor p;
        p.prepareToPlay (kSr, 512);
        CHECK (p.getLatencySamples() == 0, "reported latency = %d samples", p.getLatencySamples());
    }

    std::printf (failures == 0 ? "\nALL TESTS PASS\n" : "\n%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
