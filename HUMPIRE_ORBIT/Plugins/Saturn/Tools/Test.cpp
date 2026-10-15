// Headless DSP test of HUMPIRE SATURN.
//   cmake --build /tmp/claude-0/orbit_Saturn --target HumpireSaturnTest
//   HumpireSaturnTest                    -> runs all checks, returns 0 when everything passes
//   HumpireSaturnTest --snapshot <dir>   -> writes editor PNG captures into <dir>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SaturnPresets.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    constexpr double kSampleRate = 48000.0;
    int failures = 0;

    void check (bool ok, const std::string& what)
    {
        std::printf ("  [%s] %s\n", ok ? "OK  " : "FAIL", what.c_str());
        if (! ok) ++failures;
    }

    std::string fmt (const char* pattern, double a = 0.0, double b = 0.0)
    {
        char buf[256];
        std::snprintf (buf, sizeof buf, pattern, a, b);
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

    /** Neutral starting point: 0 dB in/out, full mix, degree mode, C major, 30 ms glide. */
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

    std::vector<float> sine (double hz, double seconds, float amp)
    {
        std::vector<float> x ((size_t) (kSampleRate * seconds));
        for (size_t i = 0; i < x.size(); ++i)
            x[i] = amp * (float) std::sin (2.0 * saturn::kPi * hz * (double) i / kSampleRate);
        return x;
    }

    /** Glottal-like sawtooth at about 190 Hz with vibrato, a formant-like low-pass and breath noise. */
    std::vector<float> vocalLike (double seconds)
    {
        std::vector<float> x ((size_t) (kSampleRate * seconds));
        double phase = 0.0, lp1 = 0.0, lp2 = 0.0, lp3 = 0.0;
        unsigned int rng = 0x2545F491u;
        for (size_t i = 0; i < x.size(); ++i)
        {
            const double t = (double) i / kSampleRate;
            const double f0 = 190.0 * (1.0 + 0.025 * std::sin (2.0 * saturn::kPi * 5.2 * t))
                                     * (1.0 + 0.04 * std::sin (2.0 * saturn::kPi * 0.7 * t));
            phase += f0 / kSampleRate;
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
    double tone (const std::vector<float>& x, size_t from, size_t count, double hz)
    {
        const double w = 2.0 * saturn::kPi * hz / kSampleRate, coeff = 2.0 * std::cos (w);
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

    size_t sec (double s) { return (size_t) (s * kSampleRate); }

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
}

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

    std::printf ("== 1. processBlock before prepareToPlay ==\n");
    {
        SaturnAudioProcessor p;
        juce::AudioBuffer<float> b (2, 256);
        const auto x = sine (220.0, 0.05, 0.5f);
        for (int i = 0; i < 256; ++i) { b.setSample (0, i, x[(size_t) i]); b.setSample (1, i, x[(size_t) i]); }
        juce::MidiBuffer m;
        p.processBlock (b, m);
        bool ok = true;
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < 256; ++i) if (! std::isfinite (b.getSample (c, i))) ok = false;
        check (ok, "no crash, finite output");
    }

    std::printf ("== 2. prepareToPlay(48000, 512): latency ==\n");
    SaturnAudioProcessor probe;
    probe.prepareToPlay (kSampleRate, 512);
    const int latency = probe.getLatencySamples();
    std::printf ("  announced latency: %d samples (%.2f ms)\n", latency, 1000.0 * latency / kSampleRate);
    check (latency > 300 && latency < 700, fmt ("latency is %.0f samples (expected 300-700)", (double) latency));

    std::printf ("== 3. Mix 0 % is bit-exact dry, aligned on the latency ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, true);
        setParam (p, saturn::id::mix, 0.0f);
        setVoice (p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const auto in = sine (220.0, 1.0, 0.5f);
        const auto out = render (p, in, 512);
        const int L = p.getLatencySamples();
        double maxErr = 0.0;
        for (size_t n = (size_t) L; n < in.size(); ++n)
            maxErr = std::max (maxErr, std::abs ((double) out.left[n] - (double) in[n - (size_t) L]));
        check (maxErr < 1e-5, fmt ("max |out(n) - in(n - latency)| = %.2e", maxErr));
    }

    std::printf ("== 4. Unison voice lines up with the dry signal ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, false);                                  // semitone mode
        setVoice (p, 0, true, 0, 0, 0.0f, 0.0f, 0.0f);           // unison, 0 dB, centre pan
        const auto in = sine (220.0, 2.0, 0.5f);
        const auto out = render (p, in, 512);
        const int L = p.getLatencySamples();
        // dry (1.0) + centre voice (cos 45 deg = 0.7071) -> 1.7071 * delayed input
        double maxErr = 0.0;
        for (size_t n = sec (1.5); n < sec (1.6); ++n)
            maxErr = std::max (maxErr, std::abs ((double) out.left[n] - 1.70711 * (double) in[n - (size_t) L]));
        check (maxErr < 3e-3, fmt ("unison voice aligned with dry (max error %.2e)", maxErr));
    }

    std::printf ("== 5. Scale degrees: A3 (220 Hz) + 2 degrees in C major = C4 ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, true);
        setVoice (p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const auto out = render (p, sine (220.0, 2.5, 0.5f), 512);
        const size_t from = sec (1.5), count = sec (1.0);
        const double atC4 = tone (out.left, from, count, 261.63);
        const double offTone = tone (out.left, from, count, 240.0);
        std::printf ("  amplitude at 261.63 Hz = %.3f, at 240 Hz = %.5f, tracked note = %.2f (A3 = 57)\n",
                     atC4, offTone, (double) p.uiTrackedMidi.load());
        check (atC4 > 0.2, fmt ("harmony present at 261.63 Hz (%.3f)", atC4));
        check (atC4 > 20.0 * offTone, "no energy off the expected interval");
        check (std::abs (p.uiTrackedMidi.load() - 57.0f) < 0.5f, "input note tracked as A3");
    }

    std::printf ("== 6. Semitone mode: +4 semitones on 220 Hz = 277.18 Hz ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, false);
        setVoice (p, 0, true, 4, 0, 0.0f, 0.0f, 0.0f);
        const auto out = render (p, sine (220.0, 2.5, 0.5f), 512);
        const double atE = tone (out.left, sec (1.5), sec (1.0), 277.18);
        const double near = tone (out.left, sec (1.5), sec (1.0), 262.0);
        check (atE > 0.2 && near < 0.2 * atE, fmt ("277.18 Hz present (%.3f), 262 Hz absent (%.3f)", atE, near));
    }

    std::printf ("== 7. Detune: +50 cents on 220 Hz = 226.27 Hz ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, false);
        setVoice (p, 0, true, 0, 0, 0.0f, 0.0f, 50.0f);
        const auto out = render (p, sine (220.0, 2.5, 0.5f), 512);
        const double amp = tone (out.left, sec (1.5), sec (1.0), 226.27);
        check (amp > 0.2, fmt ("detuned voice at 226.27 Hz (%.3f)", amp));
    }

    std::printf ("== 8. Pan: voice hard left leaves the right channel clean ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, false);
        setVoice (p, 0, true, 4, 0, 0.0f, -100.0f, 0.0f);
        const auto out = render (p, sine (220.0, 2.5, 0.5f), 512);
        const double left = tone (out.left, sec (1.5), sec (1.0), 277.18);
        const double right = tone (out.right, sec (1.5), sec (1.0), 277.18);
        check (left > 0.2 && right < 0.01, fmt ("left %.3f, right %.4f at 277.18 Hz", left, right));
    }

    std::printf ("== 9. Mono host: harmony still present ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, true);
        setVoice (p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const auto out = render (p, sine (220.0, 2.5, 0.5f), 512, false);
        const double atC4 = tone (out.left, sec (1.5), sec (1.0), 261.63);
        check (atC4 > 0.15 && allFinite (out.left, 4.0f), fmt ("mono harmony at 261.63 Hz (%.3f)", atC4));
    }

    std::printf ("== 10. Block size invariance (1, 37, 512, 2048) ==\n");
    {
        const auto in = vocalLike (1.0);
        auto configure = [] (SaturnAudioProcessor& p) {
            resetGlobal (p, true);
            setVoice (p, 0, true, 4, 2, -6.0f, -35.0f, 0.0f);
            setVoice (p, 1, true, 7, 4, -8.0f, 35.0f, 4.0f);
        };
        SaturnAudioProcessor ref;
        ref.prepareToPlay (kSampleRate, 512);
        configure (ref);
        const auto reference = render (ref, in, 512);
        for (int block : { 1, 37, 2048 })
        {
            SaturnAudioProcessor p;
            p.prepareToPlay (kSampleRate, 512);
            configure (p);
            const auto out = render (p, in, block);
            double maxDiff = 0.0;
            for (size_t i = 0; i < reference.left.size(); ++i)
                maxDiff = std::max (maxDiff, std::abs ((double) out.left[i] - (double) reference.left[i]));
            check (maxDiff < 1e-6, fmt ("block %.0f matches block 512 (max diff %.2e)", (double) block, maxDiff));
        }
    }

    std::printf ("== 11. Silence in, silence out ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, true);
        for (int v = 0; v < saturn::kNumVoices; ++v) setVoice (p, v, true, 4, 2, 0.0f, 0.0f, 0.0f);
        const std::vector<float> zeros ((size_t) sec (0.5), 0.0f);
        const auto out = render (p, zeros, 512);
        bool ok = true;
        for (float v : out.left) if (std::abs (v) > 1e-7f) ok = false;
        check (ok, "silence -> silence");
    }

    std::printf ("== 12. Bypass (host or parameter) passes the dry signal, aligned ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, true);
        setVoice (p, 0, true, 4, 2, 0.0f, 0.0f, 0.0f);
        setParam (p, saturn::id::bypass, 1.0f);
        const auto in = sine (220.0, 1.0, 0.5f);
        const auto out = render (p, in, 512);
        const int L = p.getLatencySamples();
        double maxErr = 0.0;
        for (size_t n = sec (0.5) + (size_t) L; n < in.size(); ++n)
            maxErr = std::max (maxErr, std::abs ((double) out.left[n] - (double) in[n - (size_t) L]));
        check (maxErr < 1e-4, fmt ("bypassed output = delayed dry (max error %.2e)", maxErr));
    }

    std::printf ("== 13. Extreme settings: four voices, +/-24 semitones, 0 dB, vocal input ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        resetGlobal (p, false);
        setVoice (p, 0, true, 24, 0, 0.0f, -100.0f, 0.0f);
        setVoice (p, 1, true, -24, 0, 0.0f, 100.0f, 0.0f);
        setVoice (p, 2, true, 12, 0, 0.0f, 0.0f, 0.0f);
        setVoice (p, 3, true, -12, 0, 0.0f, 0.0f, 0.0f);
        const auto out = render (p, vocalLike (1.5), 512);
        check (allFinite (out.left, 4.0f) && allFinite (out.right, 4.0f), "finite and below |4| at full level");
    }

    std::printf ("== 14. Every factory preset renders without NaN or explosion ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        const int count = p.presets.getNumPresets();
        const auto in = vocalLike (0.8);
        bool ok = true, named = true;
        for (int i = 0; i < count; ++i)
        {
            p.presets.loadPreset (i);
            if (p.presets.getCurrentName().isEmpty()) named = false;
            const auto out = render (p, in, 512);
            if (! allFinite (out.left, 4.0f) || ! allFinite (out.right, 4.0f))
            {
                std::printf ("    preset %d (%s) rendered badly\n", i, p.presets.getCurrentName().toRawUTF8());
                ok = false;
            }
        }
        check (count == 20, fmt ("%.0f factory presets", (double) count));
        check (ok, fmt ("all %.0f presets finite and bounded", (double) count));
        check (named, "every preset has a name");
    }

    std::printf ("== 15. Zero-length block and editor construction ==\n");
    {
        SaturnAudioProcessor p;
        p.prepareToPlay (kSampleRate, 512);
        juce::AudioBuffer<float> empty (2, 0);
        juce::MidiBuffer m;
        p.processBlock (empty, m);
        check (true, "zero-length block accepted");

        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        check (ed != nullptr && ed->getWidth() > 0, "editor created headless");
    }

    std::printf (failures == 0 ? "\nALL TESTS PASSED\n" : "\n%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
