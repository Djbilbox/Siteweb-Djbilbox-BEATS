// Headless DSP test of HUMPIRE SATURN.
//   cmake --build /tmp/claude-0/orbit_Saturn --target HumpireSaturnTest
//   HumpireSaturnTest                    -> runs all checks, returns 0 when everything passes
//   HumpireSaturnTest --snapshot <dir>   -> writes editor PNG captures into <dir>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SaturnPresets.h"
#include "PitchDetector.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <new>
#include <set>
#include <string>
#include <vector>

//==============================================================================
// Allocation counter: every global operator new is counted while `tracking` is set on the calling thread.
// Used to prove that processBlock never allocates.
namespace alloc
{
    std::atomic<long> count { 0 };
    thread_local bool tracking = false;
}

void* operator new (std::size_t n)
{
    if (alloc::tracking) ++alloc::count;
    if (void* p = std::malloc (n != 0 ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[] (std::size_t n) { return ::operator new (n); }
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
namespace
{
    constexpr double kSampleRate = 48000.0;
    int failures = 0;

    void section (const char* title) { std::printf ("== %s ==\n", title); }

    void check (bool ok, const std::string& what)
    {
        std::printf ("  [%s] %s\n", ok ? "OK  " : "FAIL", what.c_str());
        if (! ok) ++failures;
    }

    std::string fmt (const char* pattern, double a = 0.0, double b = 0.0, double c = 0.0)
    {
        char buf[320];
        std::snprintf (buf, sizeof buf, pattern, a, b, c);
        return buf;
    }

    void setParam (SaturnAudioProcessor& p, const juce::String& id, float value)
    {
        if (auto* prm = p.apvts.getParameter (id))
            prm->setValueNotifyingHost (prm->convertTo0to1 (value));
    }

    void setVoice (SaturnAudioProcessor& p, int v, bool on, int semi, int deg, float levelDb, float pan, float detune)
    {
        setParam (p, saturn::voiceId (v, saturn::vOn), on ? 1.0f : 0.0f);
        setParam (p, saturn::voiceId (v, saturn::vSemi), (float) semi);
        setParam (p, saturn::voiceId (v, saturn::vDeg), (float) deg);
        setParam (p, saturn::voiceId (v, saturn::vLevel), levelDb);
        setParam (p, saturn::voiceId (v, saturn::vPan), pan);
        setParam (p, saturn::voiceId (v, saturn::vDetune), detune);
    }

    void silenceVoices (SaturnAudioProcessor& p)
    {
        for (int v = 0; v < saturn::kNumVoices; ++v)
            setVoice (p, v, false, 0, 0, -6.0f, 0.0f, 0.0f);
    }

    /** Neutral starting point: 0 dB in/out, full mix, C major, 30 ms glide, no bypass, all voices off. */
    void resetGlobal (SaturnAudioProcessor& p, bool degrees)
    {
        setParam (p, saturn::id::mode, degrees ? 0.0f : 1.0f);
        setParam (p, saturn::id::key, 0.0f);
        setParam (p, saturn::id::scale, (float) saturn::Major);
        setParam (p, saturn::id::glide, 30.0f);
        setParam (p, saturn::id::mix, 100.0f);
        setParam (p, saturn::id::inGain, 0.0f);
        setParam (p, saturn::id::outGain, 0.0f);
        setParam (p, saturn::id::bypass, 0.0f);
        silenceVoices (p);
    }

    std::vector<float> sine (double hz, double seconds, float amp, double sr = kSampleRate)
    {
        std::vector<float> x ((size_t) (sr * seconds));
        for (size_t i = 0; i < x.size(); ++i)
            x[i] = amp * (float) std::sin (2.0 * saturn::kPi * hz * (double) i / sr);
        return x;
    }

    /** True f0 of the vocal-like signal at time t (seconds): 190 Hz, 5.2 Hz vibrato (+/-2.5 %), slow drift (+/-4 %). */
    enum class Pitch { Steady, Vibrato, Glide };

    double vocalF0 (double t, Pitch kind = Pitch::Vibrato)
    {
        if (kind == Pitch::Steady) return 190.0;
        if (kind == Pitch::Glide) return 150.0 + 25.0 * t;           // 150 Hz -> 250 Hz in 4 s: crosses several grain-length steps
        return 190.0 * (1.0 + 0.025 * std::sin (2.0 * saturn::kPi * 5.2 * t))
                     * (1.0 + 0.04 * std::sin (2.0 * saturn::kPi * 0.7 * t));
    }

    /** Glottal-like sawtooth (190 Hz steady, with vibrato, or gliding), a formant-like low-pass and breath noise. */
    std::vector<float> vocalLike (double seconds, Pitch kind = Pitch::Vibrato)
    {
        std::vector<float> x ((size_t) (kSampleRate * seconds));
        double phase = 0.0, lp1 = 0.0, lp2 = 0.0, lp3 = 0.0;
        unsigned int rng = 0x2545F491u;
        for (size_t i = 0; i < x.size(); ++i)
        {
            phase += vocalF0 ((double) i / kSampleRate, kind) / kSampleRate;
            if (phase >= 1.0) phase -= 1.0;
            const double saw = 2.0 * phase - 1.0;
            lp1 += 0.12 * (saw - lp1);
            lp2 += 0.12 * (lp1 - lp2);
            lp3 += 0.20 * (lp2 - lp3);
            rng = rng * 1664525u + 1013904223u;
            const double noise = (double) (rng >> 8) / 16777216.0 * 2.0 - 1.0;
            x[i] = (float) (0.5 * lp3 + 0.02 * noise);
        }
        return x;
    }

    struct Rendered { std::vector<float> left, right; };

    /** Feeds `input` through processBlock in blocks of `block` samples. Stereo input = the same signal on both channels. */
    Rendered render (SaturnAudioProcessor& p, const std::vector<float>& input, int block, bool stereo = true)
    {
        Rendered r;
        r.left.reserve (input.size());
        r.right.reserve (input.size());
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> buf (stereo ? 2 : 1, block);
        for (size_t pos = 0; pos < input.size(); pos += (size_t) block)
        {
            const int n = (int) std::min<size_t> ((size_t) block, input.size() - pos);
            buf.setSize (stereo ? 2 : 1, n, false, false, true);
            for (int c = 0; c < buf.getNumChannels(); ++c)
                juce::FloatVectorOperations::copy (buf.getWritePointer (c), input.data() + pos, n);
            p.processBlock (buf, midi);
            for (int i = 0; i < n; ++i)
            {
                r.left.push_back (buf.getSample (0, i));
                if (stereo) r.right.push_back (buf.getSample (1, i));
            }
        }
        return r;
    }

    /** Amplitude of the component at `hz` in x[from, from + count) (Goertzel). */
    double tone (const std::vector<float>& x, size_t from, size_t count, double hz, double sr = kSampleRate)
    {
        const double w = 2.0 * saturn::kPi * hz / sr, coeff = 2.0 * std::cos (w);
        double s1 = 0.0, s2 = 0.0;
        for (size_t i = 0; i < count; ++i)
        {
            const double s0 = (double) x[from + i] + coeff * s1 - s2;
            s2 = s1;
            s1 = s0;
        }
        const double power = s1 * s1 + s2 * s2 - coeff * s1 * s2;
        return 2.0 * std::sqrt (std::max (0.0, power)) / (double) count;
    }

    bool allFinite (const std::vector<float>& x, float limit)
    {
        for (float v : x)
            if (! std::isfinite (v) || std::abs (v) > limit) return false;
        return true;
    }

    size_t sec (double s, double sr = kSampleRate) { return (size_t) (s * sr); }

    /** Largest |out(n) - in(n - latency)| over n in [from, size). Samples before n = latency count as 0 input. */
    double maxDelayedError (const std::vector<float>& out, const std::vector<float>& in, int latency, size_t from)
    {
        double maxErr = 0.0;
        for (size_t n = from; n < out.size() && n < in.size(); ++n)
        {
            const double ref = n >= (size_t) latency ? (double) in[n - (size_t) latency] : 0.0;
            maxErr = std::max (maxErr, std::abs ((double) out[n] - ref));
        }
        return maxErr;
    }

    std::unique_ptr<SaturnAudioProcessor> makeProcessor (double sr = kSampleRate, int block = 512)
    {
        auto p = std::make_unique<SaturnAudioProcessor>();
        p->prepareToPlay (sr, block);
        return p;
    }

    void snapshot (const juce::File& dir, const char* name, bool degrees, bool simple)
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, degrees);
        setVoice (p, 0, true, 4, 2, -6.0f, -35.0f, 0.0f);
        setVoice (p, 1, true, 7, 4, -6.0f, 35.0f, 0.0f);
        setVoice (p, 2, true, -12, -7, -9.0f, -20.0f, 0.0f);
        setParam (p, saturn::id::simple, simple ? 1.0f : 0.0f);
        const auto in = vocalLike (2.0);
        render (p, in, 512);

        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (960, 600);
        ed->resized();
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        juce::PNGImageFormat png;
        auto f = dir.getChildFile (name);
        f.deleteFile();
        juce::FileOutputStream os (f);
        png.writeImageToStream (img, os);
        std::printf ("  snapshot -> %s\n", f.getFullPathName().toRawUTF8());
    }

    bool isAscii (const juce::String& s)
    {
        for (auto c = s.getCharPointer(); ! c.isEmpty(); ++c)
            if ((juce::uint32) *c > 127) return false;
        return true;
    }

    /** Sets every parameter (except those in `skip`) to a pseudo-random normalised value. */
    void randomiseParameters (SaturnAudioProcessor& p, juce::Random& rng, const juce::StringArray& skip = {})
    {
        for (auto* prm : p.getParameters())
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
                if (! skip.contains (rp->paramID))
                    rp->setValueNotifyingHost (rng.nextFloat());
    }
}

//==============================================================================
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;

    if (argc >= 3 && juce::String (argv[1]) == "--snapshot")
    {
        const juce::File dir (argv[2]);
        snapshot (dir, "saturn_degrees.png", true, false);
        snapshot (dir, "saturn_simple.png", true, true);
        snapshot (dir, "saturn_semitones.png", false, false);
        return 0;
    }

    //--------------------------------------------------------------------------
    section ("1. processBlock before prepareToPlay (stereo, mono, 0 samples, huge block, bypass)");
    {
        {
            SaturnAudioProcessor p;
            juce::AudioBuffer<float> b (2, 256);
            const auto x = sine (220.0, 0.05, 0.5f, 44100.0);
            for (int i = 0; i < 256; ++i) { b.setSample (0, i, x[(size_t) i]); b.setSample (1, i, x[(size_t) i]); }
            juce::MidiBuffer m;
            p.processBlock (b, m);
            bool ok = true;
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 256; ++i) if (! std::isfinite (b.getSample (c, i))) ok = false;
            check (ok, "stereo: no crash, finite output");
        }
        {
            SaturnAudioProcessor p;
            juce::AudioBuffer<float> b (1, 300);
            for (int i = 0; i < 300; ++i) b.setSample (0, i, 0.3f * std::sin (0.05f * (float) i));
            juce::MidiBuffer m;
            p.processBlock (b, m);
            bool ok = true;
            for (int i = 0; i < 300; ++i) if (! std::isfinite (b.getSample (0, i))) ok = false;
            check (ok, "mono: no crash, finite output");
        }
        {
            SaturnAudioProcessor p;
            juce::AudioBuffer<float> b (2, 0);
            juce::MidiBuffer m;
            p.processBlock (b, m);
            check (true, "0 samples: accepted");
        }
        {
            SaturnAudioProcessor p;                                          // never prepared, huge block
            setParam (p, saturn::id::mix, 0.0f);
            const auto in = vocalLike (1.0);
            const auto out = render (p, in, 40000);
            check (allFinite (out.left, 4.0f), "block of 40000 samples before prepare: finite");
            check (maxDelayedError (out.left, in, p.getLatencySamples(), 0) == 0.0,
                   fmt ("unprepared plugin still delivers the aligned dry signal (latency %.0f)", (double) p.getLatencySamples()));
        }
        {
            SaturnAudioProcessor p;                                          // bypass before prepare
            setVoice (p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
            setParam (p, saturn::id::bypass, 1.0f);
            const auto in = sine (220.0, 0.5, 0.5f, 44100.0);
            const auto out = render (p, in, 256);
            check (maxDelayedError (out.left, in, p.getLatencySamples(), 0) == 0.0, "bypass before prepare = delayed input, bit-exact");
        }
    }

    //--------------------------------------------------------------------------
    section ("2. Latency at several sample rates");
    int latency48 = 0;
    {
        for (double sr : { 22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
        {
            auto p = makeProcessor (sr);
            const int L = p->getLatencySamples();
            std::printf ("  %.0f Hz: latency %d samples (%.2f ms)\n", sr, L, 1000.0 * L / sr);
            check (L > (int) (0.009 * sr) && L < (int) (0.014 * sr), fmt ("latency of %.0f samples is 9-14 ms at %.0f Hz", (double) L, sr));
            if (sr == kSampleRate) latency48 = L;
        }
        check (latency48 > 300 && latency48 < 700, fmt ("48 kHz latency %.0f samples (expected 300-700)", (double) latency48));
    }

    //--------------------------------------------------------------------------
    section ("3. Mix 0 percent is bit-exact dry, aligned on the latency");
    {
        auto p = makeProcessor();
        resetGlobal (*p, true);
        setParam (*p, saturn::id::mix, 0.0f);
        setVoice (*p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const auto in = sine (220.0, 1.0, 0.5f);
        const auto out = render (*p, in, 512);
        const double maxErr = maxDelayedError (out.left, in, p->getLatencySamples(), 0);
        check (maxErr < 1e-5, fmt ("max |out(n) - in(n - latency)| = %.2e", maxErr));
    }

    //--------------------------------------------------------------------------
    section ("4. Unison voice lines up with the dry signal");
    {
        auto p = makeProcessor();
        resetGlobal (*p, false);                                 // semitone mode
        setVoice (*p, 0, true, 0, 0, 0.0f, 0.0f, 0.0f);          // unison, 0 dB, centre pan
        const auto in = sine (220.0, 2.0, 0.5f);
        const auto out = render (*p, in, 512);
        const int L = p->getLatencySamples();
        // dry (1.0) + centre voice (cos 45 deg = 0.7071) -> 1.7071 * delayed input
        double maxErr = 0.0;
        for (size_t n = sec (1.5); n < sec (1.6); ++n)
            maxErr = std::max (maxErr, std::abs ((double) out.left[n] - 1.70711 * (double) in[n - (size_t) L]));
        check (maxErr < 3e-3, fmt ("unison voice aligned with dry (max error %.2e)", maxErr));
    }

    //--------------------------------------------------------------------------
    section ("5. Scale degrees: A3 (220 Hz) + 2 degrees in C major = C4");
    {
        auto p = makeProcessor();
        resetGlobal (*p, true);
        setVoice (*p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const auto out = render (*p, sine (220.0, 2.5, 0.5f), 512);
        const size_t from = sec (1.5), count = sec (1.0);
        const double atC4 = tone (out.left, from, count, 261.63);
        const double offTone = tone (out.left, from, count, 240.0);
        std::printf ("  amplitude at 261.63 Hz = %.3f, at 240 Hz = %.5f, tracked note = %.2f (A3 = 57)\n",
                     atC4, offTone, (double) p->uiTrackedMidi.load());
        check (atC4 > 0.2, fmt ("harmony present at 261.63 Hz (%.3f)", atC4));
        check (atC4 > 20.0 * offTone, "no energy off the expected interval");
        check (std::abs (p->uiTrackedMidi.load() - 57.0f) < 0.5f, "input note tracked as A3");
    }

    //--------------------------------------------------------------------------
    section ("6. Semitone mode: +4 semitones on 220 Hz = 277.18 Hz");
    {
        auto p = makeProcessor();
        resetGlobal (*p, false);
        setVoice (*p, 0, true, 4, 0, 0.0f, 0.0f, 0.0f);
        const auto out = render (*p, sine (220.0, 2.5, 0.5f), 512);
        const double atE = tone (out.left, sec (1.5), sec (1.0), 277.18);
        const double near = tone (out.left, sec (1.5), sec (1.0), 262.0);
        check (atE > 0.2 && near < 0.2 * atE, fmt ("277.18 Hz present (%.3f), 262 Hz absent (%.3f)", atE, near));
    }

    //--------------------------------------------------------------------------
    section ("7. Detune: +50 cents on 220 Hz = 226.27 Hz");
    {
        auto p = makeProcessor();
        resetGlobal (*p, false);
        setVoice (*p, 0, true, 0, 0, 0.0f, 0.0f, 50.0f);
        const auto out = render (*p, sine (220.0, 2.5, 0.5f), 512);
        const double amp = tone (out.left, sec (1.5), sec (1.0), 226.27);
        check (amp > 0.2, fmt ("detuned voice at 226.27 Hz (%.3f)", amp));
    }

    //--------------------------------------------------------------------------
    section ("8. Pan: voice hard left leaves the right channel clean");
    {
        auto p = makeProcessor();
        resetGlobal (*p, false);
        setVoice (*p, 0, true, 4, 0, 0.0f, -100.0f, 0.0f);
        const auto out = render (*p, sine (220.0, 2.5, 0.5f), 512);
        const double left = tone (out.left, sec (1.5), sec (1.0), 277.18);
        const double right = tone (out.right, sec (1.5), sec (1.0), 277.18);
        check (left > 0.2 && right < 0.01, fmt ("left %.3f, right %.4f at 277.18 Hz", left, right));
    }

    //--------------------------------------------------------------------------
    section ("9. Mono host: harmony still present");
    {
        auto p = makeProcessor();
        resetGlobal (*p, true);
        setVoice (*p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const auto out = render (*p, sine (220.0, 2.5, 0.5f), 512, false);
        const double atC4 = tone (out.left, sec (1.5), sec (1.0), 261.63);
        check (atC4 > 0.15 && allFinite (out.left, 4.0f), fmt ("mono harmony at 261.63 Hz (%.3f)", atC4));
    }

    //--------------------------------------------------------------------------
    section ("10. Block size invariance, including blocks far larger than the prepared 512");
    {
        const auto in = vocalLike (1.0);
        auto configure = [] (SaturnAudioProcessor& p) {
            resetGlobal (p, true);
            setVoice (p, 0, true, 4, 2, -6.0f, -35.0f, 0.0f);
            setVoice (p, 1, true, 7, 4, -8.0f, 35.0f, 4.0f);
        };
        auto ref = makeProcessor();
        configure (*ref);
        const auto reference = render (*ref, in, 512);
        for (int block : { 1, 37, 2048, 4097, 16384, 48000 })
        {
            auto p = makeProcessor (kSampleRate, 512);
            configure (*p);
            const auto out = render (*p, in, block);
            double maxDiff = 0.0;
            for (size_t i = 0; i < reference.left.size(); ++i)
                maxDiff = std::max (maxDiff, std::abs ((double) out.left[i] - (double) reference.left[i]));
            check (out.left.size() == reference.left.size() && maxDiff < 1e-6,
                   fmt ("block %.0f matches block 512 (max diff %.2e)", (double) block, maxDiff));
        }

        // irregular block sizes, 0 samples included
        auto p = makeProcessor();
        configure (*p);
        juce::Random rng (99);
        juce::MidiBuffer midi;
        std::vector<float> out;
        size_t pos = 0;
        while (pos < in.size())
        {
            const int want = rng.nextInt (5) == 0 ? 0 : 1 + rng.nextInt (3000);
            const int n = (int) std::min<size_t> ((size_t) want, in.size() - pos);
            juce::AudioBuffer<float> b (2, n);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < n; ++i) b.setSample (c, i, in[pos + (size_t) i]);
            p->processBlock (b, midi);
            for (int i = 0; i < n; ++i) out.push_back (b.getSample (0, i));
            pos += (size_t) n;
        }
        double maxDiff = 0.0;
        for (size_t i = 0; i < out.size(); ++i)
            maxDiff = std::max (maxDiff, std::abs ((double) out[i] - (double) reference.left[i]));
        check (out.size() == in.size() && maxDiff < 1e-6, fmt ("irregular blocks (0..3000) match (max diff %.2e)", maxDiff));
    }

    //--------------------------------------------------------------------------
    section ("11. Silence in, silence out");
    {
        auto p = makeProcessor();
        resetGlobal (*p, true);
        for (int v = 0; v < saturn::kNumVoices; ++v) setVoice (*p, v, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const std::vector<float> zeros ((size_t) sec (0.5), 0.0f);
        const auto out = render (*p, zeros, 512);
        bool ok = true;
        for (float v : out.left) if (std::abs (v) > 1e-7f) ok = false;
        check (ok, "silence -> silence");
    }

    //--------------------------------------------------------------------------
    section ("12. Bypass: the output equals the input (latency-aligned), whatever the trims and voices do");
    {
        const auto in = vocalLike (1.5);
        auto configure = [] (SaturnAudioProcessor& p) {
            resetGlobal (p, true);
            setVoice (p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
            setVoice (p, 1, true, 7, 4, 0.0f, 50.0f, 0.0f);
            setParam (p, saturn::id::inGain, 6.0f);              // trims must not leak into the bypass path
            setParam (p, saturn::id::outGain, -9.0f);
        };

        {   // bypass parameter on from the start (the VST3 wrapper calls processBlock, never processBlockBypassed)
            auto p = makeProcessor();
            configure (*p);
            setParam (*p, saturn::id::bypass, 1.0f);
            p->prepareToPlay (kSampleRate, 512);
            const auto out = render (*p, in, 512);
            const double eL = maxDelayedError (out.left, in, p->getLatencySamples(), 0);
            const double eR = maxDelayedError (out.right, in, p->getLatencySamples(), 0);
            check (eL == 0.0 && eR == 0.0, fmt ("bypass parameter: out(n) == in(n - %.0f) bit-exact on both channels", (double) p->getLatencySamples()));
            check (p->getBypassParameter() != nullptr, "the plugin reports a bypass parameter");
        }
        {   // bypass engaged while running, then released again (220 Hz sine, +2 degrees = 261.63 Hz harmony)
            const auto sin220 = sine (220.0, 2.5, 0.5f);
            auto p = makeProcessor();
            configure (*p);
            std::vector<float> out;
            juce::MidiBuffer midi;
            juce::AudioBuffer<float> b (2, 480);
            const size_t blocks = sin220.size() / 480;
            const size_t bypassFrom = 40 * 480, bypassTo = 80 * 480;
            for (size_t k = 0; k < blocks; ++k)
            {
                if (k == 40) setParam (*p, saturn::id::bypass, 1.0f);
                if (k == 80) setParam (*p, saturn::id::bypass, 0.0f);
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < 480; ++i) b.setSample (c, i, sin220[k * 480 + (size_t) i]);
                p->processBlock (b, midi);
                for (int i = 0; i < 480; ++i) out.push_back (b.getSample (0, i));
            }
            const int L = p->getLatencySamples();
            double settled = 0.0, maxStep = 0.0;
            for (size_t n = bypassFrom + 600; n < bypassTo; ++n)         // after the 10 ms crossfade
                settled = std::max (settled, std::abs ((double) out[n] - (double) sin220[n - (size_t) L]));
            for (size_t n = bypassFrom - 100; n < bypassFrom + 900; ++n) maxStep = std::max (maxStep, std::abs ((double) out[n + 1] - (double) out[n]));
            for (size_t n = bypassTo - 100; n < bypassTo + 900; ++n)     maxStep = std::max (maxStep, std::abs ((double) out[n + 1] - (double) out[n]));
            check (settled == 0.0, fmt ("engaged while running: exact pass-through once settled (max error %.2e)", settled));
            check (maxStep < 0.25, fmt ("no click when bypass is engaged / released (largest step %.3f)", maxStep));
            check (allFinite (out, 20.0f), "finite while toggling");

            const double before = tone (out, bypassFrom - 4800, 4800, 261.63);          // processed: harmony present
            const double during = tone (out, bypassFrom + 1200, bypassTo - bypassFrom - 2400, 261.63);
            const double after = tone (out, sec (1.9), sec (0.5), 261.63);              // released: harmony is back
            check (before > 0.1 && during < 0.01 && after > 0.1,
                   fmt ("harmony %.3f before, %.4f while bypassed, %.3f after release", before, during, after));
        }
        {   // host bypass through processBlockBypassed (hosts that do not use the bypass parameter)
            auto p = makeProcessor();
            configure (*p);
            juce::MidiBuffer midi;
            std::vector<float> out;
            juce::AudioBuffer<float> b (2, 480);
            const size_t blocks = in.size() / 480;
            for (size_t k = 0; k < blocks; ++k)
            {
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < 480; ++i) b.setSample (c, i, in[k * 480 + (size_t) i]);
                if (k < 20) p->processBlock (b, midi); else p->processBlockBypassed (b, midi);
                for (int i = 0; i < 480; ++i) out.push_back (b.getSample (0, i));
            }
            const double e = maxDelayedError (out, in, p->getLatencySamples(), 20 * 480 + 600);
            check (e == 0.0, fmt ("processBlockBypassed: exact pass-through (max error %.2e)", e));
        }
        {   // bypass in mono
            auto p = makeProcessor();
            configure (*p);
            setParam (*p, saturn::id::bypass, 1.0f);
            p->prepareToPlay (kSampleRate, 512);
            const auto out = render (*p, in, 512, false);
            check (maxDelayedError (out.left, in, p->getLatencySamples(), 0) == 0.0, "mono bypass: bit-exact");
        }
        {   // the latency must not depend on the bypass state
            auto p = makeProcessor();
            const int before = p->getLatencySamples();
            setParam (*p, saturn::id::bypass, 1.0f);
            check (p->getLatencySamples() == before, "reported latency is the same with bypass on and off");
        }
    }

    //--------------------------------------------------------------------------
    section ("13. Extreme settings: four voices, +/-24 semitones, 0 dB, vocal input");
    {
        auto p = makeProcessor();
        resetGlobal (*p, false);
        setVoice (*p, 0, true, 24, 0, 0.0f, -100.0f, 0.0f);
        setVoice (*p, 1, true, -24, 0, 0.0f, 100.0f, 0.0f);
        setVoice (*p, 2, true, 12, 0, 0.0f, 0.0f, 0.0f);
        setVoice (*p, 3, true, -12, 0, 0.0f, 0.0f, 0.0f);
        const auto out = render (*p, vocalLike (1.5), 512);
        check (allFinite (out.left, 4.0f) && allFinite (out.right, 4.0f), "finite and below |4| at full level");
    }

    //--------------------------------------------------------------------------
    section ("14. Factory presets: 20 presets, English categories, valid values, all render finite");
    {
        const auto list = makeSaturnFactoryPresets();
        check (list.size() == 20, fmt ("%.0f factory presets", (double) list.size()));

        const std::set<juce::String> allowed { "Trap Vocals", "Afrotrap", "Oriental / Rai", "Deep House", "G-Funk", "R&B",
                                               "Spoken Word / Podcast" };
        std::set<juce::String> usedCategories, names;
        bool categoriesOk = true, namesOk = true, asciiOk = true, idsOk = true, rangesOk = true;
        auto p = makeProcessor();
        for (const auto& pr : list)
        {
            usedCategories.insert (pr.category);
            if (allowed.count (pr.category) == 0) categoriesOk = false;
            if (pr.name.isEmpty() || ! names.insert (pr.name).second) namesOk = false;
            if (! isAscii (pr.category) || ! isAscii (pr.name)) asciiOk = false;
            for (const auto& [pid, value] : pr.values)
            {
                auto* prm = p->apvts.getParameter (pid);
                if (prm == nullptr) { idsOk = false; continue; }
                const auto& range = prm->getNormalisableRange();
                if (value < range.start - 1e-4f || value > range.end + 1e-4f) rangesOk = false;
            }
        }
        check (categoriesOk, "every category is one of the 7 English categories");
        check (usedCategories.size() == allowed.size(), fmt ("all %.0f categories are used", (double) allowed.size()));
        check (namesOk, "preset names are non-empty and unique");
        check (asciiOk, "preset names and categories are plain ASCII (English)");
        check (idsOk, "every preset value refers to an existing parameter");
        check (rangesOk, "every preset value is inside the parameter range");

        const auto in = vocalLike (0.8);
        bool ok = true, named = true, applied = true, preserved = true;
        setParam (*p, saturn::id::inGain, 3.0f);
        setParam (*p, saturn::id::outGain, -2.0f);
        for (int i = 0; i < (int) list.size(); ++i)
        {
            p->presets.loadPreset (i);
            if (p->presets.getCurrentName() != list[(size_t) i].name) named = false;
            for (const auto& [pid, value] : list[(size_t) i].values)
            {
                auto* prm = p->apvts.getParameter (pid);
                const float actual = prm->convertFrom0to1 (prm->getValue());
                if (std::abs (actual - value) > 0.06f) applied = false;
            }
            if (std::abs (p->apvts.getParameter (saturn::id::inGain)->convertFrom0to1 (p->apvts.getParameter (saturn::id::inGain)->getValue()) - 3.0f) > 0.06f) preserved = false;
            const auto out = render (*p, in, 512);
            if (! allFinite (out.left, 6.0f) || ! allFinite (out.right, 6.0f))
            {
                std::printf ("    preset %d (%s) rendered badly\n", i, list[(size_t) i].name.toRawUTF8());
                ok = false;
            }
        }
        check (ok, fmt ("all %.0f presets render finite and bounded", (double) list.size()));
        check (named, "the preset manager reports the right name for each preset");
        check (applied, "every preset value is applied to its parameter");
        check (preserved, "loading a preset keeps the Input trim");
    }

    //--------------------------------------------------------------------------
    section ("15. Zero-length block, extra channels and editor construction");
    {
        auto p = makeProcessor();
        juce::AudioBuffer<float> empty (2, 0);
        juce::MidiBuffer m;
        p->processBlock (empty, m);
        check (true, "zero-length block accepted");

        juce::AudioBuffer<float> four (4, 256);
        for (int c = 0; c < 4; ++c)
            for (int i = 0; i < 256; ++i) four.setSample (c, i, 0.25f);
        p->processBlock (four, m);
        bool clear = true;
        for (int c = 2; c < 4; ++c)
            for (int i = 0; i < 256; ++i) if (four.getSample (c, i) != 0.0f) clear = false;
        check (clear, "channels beyond stereo are cleared");

        std::unique_ptr<juce::AudioProcessorEditor> ed (p->createEditor());
        check (ed != nullptr && ed->getWidth() > 0, "editor created headless");
        if (ed != nullptr)
        {
            for (auto size : { juce::Point<int> (480, 300), juce::Point<int> (1920, 1200), juce::Point<int> (960, 600) })
                ed->setSize (size.x, size.y);
            check (true, "editor resized from 50 % to 200 % without trouble");
        }
        ed.reset();
        check (true, "editor destroyed while the processor lives on");
    }

    //--------------------------------------------------------------------------
    section ("16. State save / restore round-trips every parameter and the audio");
    {
        const auto in = vocalLike (0.6);
        for (int variant = 0; variant < 2; ++variant)
        {
            const bool bypassOn = variant == 1;
            auto a = makeProcessor();
            juce::Random rng (variant == 0 ? 20260214 : 77);
            randomiseParameters (*a, rng, { saturn::id::bypass });
            setParam (*a, saturn::id::bypass, bypassOn ? 1.0f : 0.0f);

            int moved = 0, total = 0;
            for (auto* prm : a->getParameters())
                if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
                {
                    ++total;
                    if (std::abs (rp->getValue() - rp->getDefaultValue()) > 1e-6f) ++moved;
                }
            check (moved > total / 2, fmt ("test state is non-trivial: %.0f of %.0f parameters differ from default", (double) moved, (double) total));

            juce::MemoryBlock block;
            a->getStateInformation (block);
            check (block.getSize() > 0, fmt ("state saved (%.0f bytes)", (double) block.getSize()));

            auto b = makeProcessor();                             // fresh instance with default values
            b->setStateInformation (block.getData(), (int) block.getSize());

            double worst = 0.0;
            int mismatches = 0;
            for (auto* prm : a->getParameters())
                if (auto* ra = dynamic_cast<juce::RangedAudioParameter*> (prm))
                {
                    auto* rb = b->apvts.getParameter (ra->paramID);
                    const float va = ra->convertFrom0to1 (ra->getValue());
                    const float vb = rb != nullptr ? rb->convertFrom0to1 (rb->getValue()) : 1e9f;
                    const double tol = 1e-4 * std::max (1.0, (double) std::abs (va));
                    worst = std::max (worst, (double) std::abs (va - vb));
                    if (std::abs (va - vb) > tol) { ++mismatches; std::printf ("    mismatch %s: %f vs %f\n", ra->paramID.toRawUTF8(), va, vb); }
                }
            check (mismatches == 0, fmt ("all %.0f parameters restored (largest difference %.2e)", (double) total, worst));

            // the restored plugin sounds exactly like the original
            a->prepareToPlay (kSampleRate, 512);
            b->prepareToPlay (kSampleRate, 512);
            const auto oa = render (*a, in, 512);
            const auto ob = render (*b, in, 512);
            double maxDiff = 0.0;
            for (size_t i = 0; i < oa.left.size(); ++i)
                maxDiff = std::max ({ maxDiff, std::abs ((double) oa.left[i] - (double) ob.left[i]), std::abs ((double) oa.right[i] - (double) ob.right[i]) });
            check (maxDiff == 0.0, fmt ("restored plugin renders identical audio (bypass %.0f, max diff %.2e)", bypassOn ? 1.0 : 0.0, maxDiff));
        }

        // damaged or foreign state must be ignored, never crash and never change anything
        auto b = makeProcessor();
        setParam (*b, saturn::id::glide, 111.0f);
        const unsigned char junk[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        b->setStateInformation (nullptr, 0);
        b->setStateInformation (junk, 3);
        b->setStateInformation (junk, 16);
        juce::XmlElement foreign ("SOMETHING_ELSE");
        juce::MemoryBlock fb;
        juce::AudioProcessor::copyXmlToBinary (foreign, fb);
        b->setStateInformation (fb.getData(), (int) fb.getSize());
        const auto* pg = b->apvts.getParameter (saturn::id::glide);
        check (std::abs (pg->convertFrom0to1 (pg->getValue()) - 111.0f) < 0.6f, "empty, junk and foreign state data are ignored");

        // an empty (but valid) state of ours keeps the parameters
        juce::XmlElement empty (b->apvts.state.getType());
        juce::MemoryBlock eb;
        juce::AudioProcessor::copyXmlToBinary (empty, eb);
        b->setStateInformation (eb.getData(), (int) eb.getSize());
        check (true, "state without parameters accepted");
    }

    //--------------------------------------------------------------------------
    section ("17. Parameter text: every parameter shows sensible text and parses it back");
    {
        auto p = makeProcessor();
        bool namesOk = true, textOk = true, roundOk = true, intOk = true, unitOk = true;
        int count = 0, floats = 0, ints = 0;
        for (auto* prm : p->getParameters())
        {
            auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm);
            if (rp == nullptr) { check (false, "parameter is not a RangedAudioParameter"); continue; }
            ++count;
            if (rp->getName (64).isEmpty() || ! isAscii (rp->getName (64))) namesOk = false;

            const bool isFloat = dynamic_cast<juce::AudioParameterFloat*> (rp) != nullptr;
            const bool isInt = dynamic_cast<juce::AudioParameterInt*> (rp) != nullptr;
            floats += isFloat ? 1 : 0;
            ints += isInt ? 1 : 0;

            for (float norm : { 0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 1.0f, rp->getDefaultValue() })
            {
                const juce::String t = rp->getText (norm, 64);
                if (t.isEmpty() || ! isAscii (t) || t.containsIgnoreCase ("nan") || t.containsIgnoreCase ("1.#")) { textOk = false; std::printf ("    bad text '%s' for %s\n", t.toRawUTF8(), rp->paramID.toRawUTF8()); }
                if (isInt && t.containsChar ('.')) intOk = false;
                if (isFloat && ! t.containsAnyOf ("msdBct%LRC")) { unitOk = false; std::printf ("    no unit in '%s' for %s\n", t.toRawUTF8(), rp->paramID.toRawUTF8()); }

                const float back = rp->getValueForText (t);
                const float v = rp->convertFrom0to1 (norm), vb = rp->convertFrom0to1 (back);
                if (std::abs (v - vb) > 1e-3f * std::max (1.0f, std::abs (v)))
                {
                    roundOk = false;
                    std::printf ("    round trip of %s: %f -> '%s' -> %f\n", rp->paramID.toRawUTF8(), v, t.toRawUTF8(), vb);
                }
            }
        }
        check (count == 9 + 4 * saturn::kNumVoiceParams, fmt ("%.0f parameters inspected (9 global + 4 voices x 6)", (double) count));
        std::printf ("  (%d float, %d int)\n", floats, ints);
        check (namesOk, "parameter names are English / ASCII");
        check (textOk, "texts are non-empty, ASCII, never NaN");
        check (roundOk, "text -> value -> text round trip on every parameter (min, max, default, quartiles)");
        check (intOk, "integer parameters print no decimals");
        check (unitOk, "every float parameter text carries its unit");

        auto value = [&] (const char* id, const char* text) {
            auto* rp = p->apvts.getParameter (id);
            return rp->convertFrom0to1 (rp->getValueForText (text));
        };
        auto shown = [&] (const char* id, float real) {
            auto* rp = p->apvts.getParameter (id);
            return rp->getText (rp->convertTo0to1 (real), 64);
        };
        check (value ("mix", "50 %") == 50.0f && value ("mix", "25") == 25.0f && value ("mix", "100%") == 100.0f, "Mix: '50 %', '25', '100%' parse as percent");
        check (shown ("mix", 37.0f) == "37 %", "Mix shows '37 %'");
        check (std::abs (value ("glide", "45 ms") - 45.0f) < 0.01f && shown ("glide", 30.0f) == "30 ms", "Glide: '45 ms' parses, 30 shows '30 ms'");
        check (std::abs (value ("inGain", "-3.5 dB") + 3.5f) < 0.01f && shown ("inGain", -3.5f) == "-3.5 dB", "Input: '-3.5 dB'");
        check (std::abs (value ("outGain", "6") - 6.0f) < 0.01f && shown ("outGain", 0.0f) == "0.0 dB", "Output: '6' parses, 0 shows '0.0 dB'");
        check (value ("v1Level", "-inf") == saturn::kMinLevelDb && shown ("v1Level", saturn::kMinLevelDb) == "-inf dB", "Level: minimum is '-inf dB' and parses back");
        check (std::abs (value ("v1Level", "-6 dB") + 6.0f) < 0.01f && shown ("v1Level", -6.0f) == "-6.0 dB", "Level: '-6 dB'");
        check (value ("v2Pan", "L 35") == -35.0f && value ("v2Pan", "R 20") == 20.0f && value ("v2Pan", "C") == 0.0f && value ("v2Pan", "-12") == -12.0f, "Pan: 'L 35', 'R 20', 'C', '-12'");
        check (shown ("v2Pan", -35.0f) == "L 35" && shown ("v2Pan", 20.0f) == "R 20" && shown ("v2Pan", 0.0f) == "C", "Pan shows 'L 35', 'R 20', 'C'");
        check (value ("v1Semi", "+4 st") == 4.0f && value ("v1Semi", "-12") == -12.0f && shown ("v1Semi", 7.0f) == "+7 st" && shown ("v1Semi", -12.0f) == "-12 st", "Semitones: '+4 st', '-12'");
        check (value ("v3Deg", "-7 deg") == -7.0f && shown ("v3Deg", 2.0f) == "+2 deg", "Degrees: '-7 deg', shows '+2 deg'");
        check (std::abs (value ("v1Detune", "+12.5 ct") - 12.5f) < 0.01f && shown ("v1Detune", 12.5f) == "+12.5 ct" && shown ("v1Detune", 0.0f) == "0.0 ct", "Detune: '+12.5 ct'");
        check (shown ("v1On", 1.0f) == "On" && shown ("bypass", 0.0f) == "Off", "Booleans show 'On' / 'Off'");
        check (saturn::text::withUnit (12.34f, 0, "ms") == "12 ms" && saturn::text::withUnit (-0.04f, 1, "dB") == "0.0 dB"
               && saturn::text::withUnit (7.0f, 1, "ct", true) == "+7.0 ct" && saturn::text::withUnit (-7.26f, 1, "ct", true) == "-7.3 ct",
               "text helpers: zero decimals really are integers, no negative zero");
        auto* key = p->apvts.getParameter ("key");
        auto* scale = p->apvts.getParameter ("scale");
        check (key->getText (key->convertTo0to1 (1.0f), 16) == "C#" && scale->getText (scale->convertTo0to1 ((float) saturn::Saba), 32) == "Maqam Saba", "Key and Scale show their names");
    }

    //--------------------------------------------------------------------------
    section ("18. No memory allocation in processBlock (global operator new is counted)");
    {
        auto p = makeProcessor();
        resetGlobal (*p, true);
        for (int v = 0; v < saturn::kNumVoices; ++v) setVoice (*p, v, true, 4 + v, 2, -6.0f, -50.0f + 33.0f * (float) v, 3.0f);
        const auto in = vocalLike (1.2);
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> b (2, 512);
        juce::AudioBuffer<float> mono (1, 777);
        juce::AudioBuffer<float> big (2, 30000);
        for (int i = 0; i < 512; ++i) { b.setSample (0, i, in[(size_t) i]); b.setSample (1, i, in[(size_t) i]); }
        for (int i = 0; i < 777; ++i) mono.setSample (0, i, in[(size_t) i]);
        for (int i = 0; i < 30000; ++i) { big.setSample (0, i, in[(size_t) i]); big.setSample (1, i, in[(size_t) i]); }

        long inside = 0;
        for (int round = 0; round < 40; ++round)
        {
            // parameter changes happen between blocks, as a host or the editor would do
            setParam (*p, saturn::id::key, (float) (round % 12));
            setParam (*p, saturn::id::scale, (float) (round % (int) saturn::NumScales));
            setParam (*p, saturn::id::mode, (float) (round & 1));
            if (round == 10) setParam (*p, saturn::id::bypass, 1.0f);
            if (round == 20) setParam (*p, saturn::id::bypass, 0.0f);
            const long start = alloc::count.load();
            alloc::tracking = true;
            p->processBlock (b, midi);
            if (round % 8 == 0) p->processBlock (mono, midi);
            if (round % 13 == 0) p->processBlock (big, midi);
            p->processBlockBypassed (b, midi);
            alloc::tracking = false;
            inside += alloc::count.load() - start;
        }
        check (inside == 0, fmt ("allocations inside processBlock / processBlockBypassed: %.0f", (double) inside));
    }

    //--------------------------------------------------------------------------
    section ("19. Robustness: random parameters and block sizes, NaN / Inf injection, recovery");
    {
        auto p = makeProcessor();
        juce::Random rng (4242);
        juce::MidiBuffer midi;
        bool finite = true, bounded = true;
        const float inf = std::numeric_limits<float>::infinity();
        const float nan = std::numeric_limits<float>::quiet_NaN();
        size_t total = 0;
        for (int blockIndex = 0; blockIndex < 260; ++blockIndex)
        {
            randomiseParameters (*p, rng);
            const int n = rng.nextInt (7) == 0 ? 0 : 1 + rng.nextInt (3000);
            juce::AudioBuffer<float> b (2, n);
            const int kind = rng.nextInt (4);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < n; ++i)
                {
                    float x = 0.0f;
                    if (kind == 0) x = rng.nextFloat() * 2.0f - 1.0f;                                   // white noise
                    else if (kind == 1) x = 0.8f * std::sin (0.03f * (float) (total + (size_t) i));      // tone
                    else if (kind == 2) x = (rng.nextInt (50) == 0) ? 1.0f : 0.0f;                      // clicks
                    b.setSample (c, i, x);
                }
            if (n > 8 && blockIndex % 9 == 0)
            {
                b.setSample (0, 3, nan);
                b.setSample (1, 5, inf);
                b.setSample (0, 7, -inf);
            }
            p->processBlock (b, midi);
            total += (size_t) n;
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < n; ++i)
                {
                    const float v = b.getSample (c, i);
                    if (! std::isfinite (v)) finite = false;
                    if (std::abs (v) > 1.0e4f) bounded = false;
                }
        }
        check (finite, fmt ("output stayed finite over %.0f random samples with NaN / Inf in the input", (double) total));
        check (bounded, "output stayed below |10000| at the most extreme random settings");

        // after all that abuse a normal signal must still be harmonised properly
        resetGlobal (*p, true);
        setVoice (*p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        std::vector<float> junkIn (4096, nan);
        render (*p, junkIn, 512);                                          // a block made only of NaN
        const auto out = render (*p, sine (220.0, 2.5, 0.5f), 512);
        const double atC4 = tone (out.left, sec (1.5), sec (1.0), 261.63);
        check (allFinite (out.left, 4.0f) && atC4 > 0.2, fmt ("recovers after NaN input: harmony at 261.63 Hz (%.3f)", atC4));
    }

    //--------------------------------------------------------------------------
    section ("20. Sample-rate changes: prepareToPlay again resets everything and keeps the intervals right");
    {
        auto p = makeProcessor (44100.0);
        for (double sr : { 44100.0, 96000.0, 48000.0, 22050.0, 192000.0, 44100.0 })
        {
            p->prepareToPlay (sr, 512);
            resetGlobal (*p, false);
            setVoice (*p, 0, true, 4, 0, 0.0f, 0.0f, 0.0f);
            const auto out = render (*p, sine (220.0, 2.0, 0.5f, sr), 512);
            const double atE = tone (out.left, sec (1.2, sr), sec (0.7, sr), 277.18, sr);
            const double dry = tone (out.left, sec (1.2, sr), sec (0.7, sr), 220.0, sr);
            check (allFinite (out.left, 4.0f) && atE > 0.2 && dry > 0.3, fmt ("%.0f Hz: +4 semitones present (%.3f), dry present (%.3f)", sr, atE, dry));
        }

        // re-preparing must forget the past: the first second after prepare equals a fresh instance
        auto fresh = makeProcessor();
        auto reused = makeProcessor();
        resetGlobal (*fresh, true);
        resetGlobal (*reused, true);
        setVoice (*fresh, 0, true, 4, 2, -3.0f, -30.0f, 0.0f);
        setVoice (*reused, 0, true, 4, 2, -3.0f, -30.0f, 0.0f);
        render (*reused, vocalLike (1.0), 512);                          // dirty every state
        setVoice (*reused, 1, true, 7, 4, -3.0f, 30.0f, 0.0f);            // ... and every setting
        setVoice (*reused, 1, false, 0, 0, -6.0f, 0.0f, 0.0f);
        fresh->prepareToPlay (kSampleRate, 512);                          // the host sequence: parameters, then prepare
        reused->prepareToPlay (kSampleRate, 512);
        const auto in = vocalLike (0.8);
        const auto a = render (*fresh, in, 512);
        const auto b = render (*reused, in, 512);
        double maxDiff = 0.0;
        for (size_t i = 0; i < a.left.size(); ++i) maxDiff = std::max (maxDiff, std::abs ((double) a.left[i] - (double) b.left[i]));
        check (maxDiff == 0.0, fmt ("prepareToPlay resets every filter and delay (max diff to a fresh instance %.2e)", maxDiff));
    }

    //--------------------------------------------------------------------------
    section ("21. Key / scale change while singing re-tracks the degree at once; grain length hysteresis");
    {
        saturn::HarmonyEngine e;
        e.prepare (kSampleRate);
        saturn::EngineSettings s;
        s.degreeMode = true;
        s.key = 0;
        s.scale = saturn::Major;
        s.voice[0].on = true;
        s.voice[0].deg = 2;
        s.voice[0].gain = 1.0f;
        e.setSettings (s);
        float dl, dr, wl, wr;
        const auto x = sine (220.0, 0.6, 0.5f);
        for (float v : x) e.process (v, v, dl, dr, wl, wr);
        check (e.isVoiced() && std::abs (e.getTrackedMidi() - 57.0f) < 0.01f, "A3 tracked in C major");
        s.scale = saturn::Chromatic;
        s.key = 3;
        e.setSettings (s);
        check (std::abs (e.getTrackedMidi() - 57.0f) < 0.01f, fmt ("after a scale/key change the tracked note is available immediately (%.2f)", (double) e.getTrackedMidi()));

        // vibrato across a grain-length threshold (181.8 Hz at 48 kHz) must not flip the grain between 11 and 22 ms
        saturn::GrainWindow w;
        w.prepare (kSampleRate);
        auto settle = [&] (double hz) { w.setPeriod (kSampleRate / hz); for (int i = 0; i < 80000; ++i) w.tick(); return w.length(); };
        double lo = 1e9, hi = 0.0;
        for (int k = 0; k < 20; ++k)
        {
            const double len = settle (k % 2 == 0 ? 178.0 : 186.0);
            lo = std::min (lo, len);
            hi = std::max (hi, len);
        }
        check (hi - lo < 120.0, fmt ("grain length stays within %.0f samples for 178 <-> 186 Hz vibrato (range %.0f)", hi - lo, hi - lo));
        const double len100 = settle (100.0), len400 = settle (400.0), len900 = settle (900.0);
        check (len100 >= 0.85 * 0.011 * kSampleRate && len100 <= 0.022 * kSampleRate + 1e-6
               && len400 >= 0.85 * 0.011 * kSampleRate && len400 <= 0.022 * kSampleRate + 1e-6
               && len900 >= 0.85 * 0.011 * kSampleRate && len900 <= 0.022 * kSampleRate + 1e-6,
               fmt ("grain length stays in bounds at 100, 400, 900 Hz (%.0f, %.0f samples ...)", len100, len400));
        const double doublePeriods = len400 / (2.0 * kSampleRate / 400.0);
        check (std::abs (doublePeriods - std::round (doublePeriods)) < 1e-6,
               fmt ("grain length is a whole number of double periods (coherent crossfade): %.6f", doublePeriods));
    }

    //--------------------------------------------------------------------------
    section ("22. Pitch accuracy of the harmony voices on a vocal-like signal");
    {
        // The wet voice is the output minus the exactly known dry signal (centre pan: gain 0.7071). An independent YIN
        // detector measures the voice and the delayed dry signal over the same frames; the error is the deviation of
        // their ratio from the requested interval.
        auto measure = [] (int semi, Pitch kind, double& median, double& p90, double& gross, size_t& frames)
        {
            auto p = makeProcessor();
            resetGlobal (*p, false);
            setVoice (*p, 0, true, semi, 0, 0.0f, 0.0f, 0.0f);
            const auto in = vocalLike (4.0, kind);
            const auto out = render (*p, in, 512);
            const int L = p->getLatencySamples();
            std::vector<float> wet (in.size(), 0.0f), dryDelayed (in.size(), 0.0f);
            for (size_t n = 0; n < in.size(); ++n)
            {
                dryDelayed[n] = n >= (size_t) L ? in[n - (size_t) L] : 0.0f;
                wet[n] = (out.left[n] - dryDelayed[n]) / 0.70710678f;
            }
            saturn::PitchDetector detWet, detDry;
            detWet.prepare (kSampleRate);
            detDry.prepare (kSampleRate);
            std::vector<double> absErr;
            size_t bad = 0;
            const double expected = std::pow (2.0, (double) semi / 12.0);
            for (size_t n = 0; n < in.size(); ++n)
            {
                const bool a = detWet.pushSample (wet[n]);
                const bool b = detDry.pushSample (dryDelayed[n]);
                if (a && b && n > sec (1.0) && detWet.getF0() > 0.0f && detDry.getF0() > 0.0f)
                {
                    const double e = std::abs (1200.0 * std::log2 ((double) detWet.getF0() / (double) detDry.getF0() / expected));
                    absErr.push_back (e);
                    if (e > 300.0) ++bad;
                }
            }
            std::sort (absErr.begin(), absErr.end());
            frames = absErr.size();
            median = absErr.empty() ? 1e9 : absErr[absErr.size() / 2];
            p90 = absErr.empty() ? 1e9 : absErr[(size_t) ((double) absErr.size() * 0.9)];
            gross = absErr.empty() ? 1.0 : (double) bad / (double) absErr.size();
        };

        for (int semi : { 7, -12, 4, -5, 12 })
        {
            double median, p90, gross;
            size_t frames;
            measure (semi, Pitch::Steady, median, p90, gross, frames);
            std::printf ("  steady 190 Hz, %+d st: %d frames, median error %.1f cents, p90 %.1f cents\n", semi, (int) frames, median, p90);
            check (frames > 150 && median < 10.0 && p90 < 25.0, fmt ("steady pitch, %+.0f semitones: median < 10 cents, p90 < 25 cents", (double) semi));
        }
        for (int semi : { 7, -12, 4, -5, 12 })
        {
            double median, p90, gross;
            size_t frames;
            measure (semi, Pitch::Glide, median, p90, gross, frames);
            std::printf ("  glide 150-250 Hz, %+d st: %d frames, median error %.1f cents, p90 %.1f cents, gross errors %.1f %%\n",
                         semi, (int) frames, median, p90, 100.0 * gross);
            check (frames > 150 && median < 14.0 && p90 < 30.0 && gross < 0.03,
                   fmt ("gliding pitch, %+.0f semitones: median < 14 cents, p90 < 30 cents, < 3 %% gross errors", (double) semi));
        }
        for (int semi : { 7, -12, 4, -5, 12 })
        {
            double median, p90, gross;
            size_t frames;
            measure (semi, Pitch::Vibrato, median, p90, gross, frames);
            std::printf ("  vibrato, %+d st: %d frames, median error %.1f cents, p90 %.1f cents, gross errors %.1f %%\n",
                         semi, (int) frames, median, p90, 100.0 * gross);
            // With vibrato the wet content is up to 11 ms away from the dry reference, which alone is worth 10-30 cents:
            // this case only guards against octave jumps and broken tracking.
            check (frames > 150 && median < 50.0 && p90 < 100.0 && gross < 0.03,
                   fmt ("vibrato, %+.0f semitones: median < 50 cents, p90 < 100 cents, < 3 %% gross errors", (double) semi));
        }
    }

    //--------------------------------------------------------------------------
    section ("23. CPU cost (four voices, stereo, 48 kHz, blocks of 512)");
    {
        auto p = makeProcessor();
        resetGlobal (*p, true);
        for (int v = 0; v < saturn::kNumVoices; ++v) setVoice (*p, v, true, 4, 2 + v, -6.0f, -30.0f + 20.0f * (float) v, 2.0f);
        const auto in = vocalLike (10.0);
        const auto t0 = std::chrono::steady_clock::now();
        const auto out = render (*p, in, 512);
        const double seconds = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
        const double load = 100.0 * seconds / 10.0;
        std::printf ("  10 s of audio in %.3f s = %.1f %% of one core (includes the test's own copying)\n", seconds, load);
        check (out.left.size() == in.size() && load < 25.0, fmt ("CPU load %.1f %% (limit 25 %%)", load));
    }

    std::printf (failures == 0 ? "\nALL TESTS PASSED\n" : "\n%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
