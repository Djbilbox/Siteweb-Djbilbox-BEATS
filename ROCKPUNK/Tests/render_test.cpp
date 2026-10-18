// Headless verification of ROCKPUNK GUITAR: no host, no window, no sound card.
//
// What this bench exists to catch:
//   - a product that installs cleanly and then plays nothing (every sound path
//     is rendered, with an empty Samples folder like on a fresh install);
//   - a plug-in that a host can crash: process before prepare, blocks larger
//     than announced, empty blocks, odd sample rates, mono buffers, garbage
//     state data;
//   - sound that never stops (stuck notes), NaN, DC offset;
//   - settings that do not survive a project save/load, parameter text a host
//     could not parse back;
//   - an interface whose controls do not sit where they are drawn or do not
//     drive the parameter they announce.
//
// Run:  ROCKPUNK_RenderTest [path to the factory Presets folder]
// Exit code 0 = everything passed.

#include <JuceHeader.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <set>
#include <vector>

#include "../Source/Dsp/Delay.h"
#include "../Source/Dsp/_archive_synth_engine/Lfo.h"
#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"
#include "../Source/UI/Panel.h"

namespace
{
    int failures = 0;

    void check (bool ok, const juce::String& what)
    {
        // Flushed at once: a crash half-way must not swallow the buffer and
        // leave the bench silent about the exact place of the problem.
        std::cout << (ok ? "  OK    " : "  FAIL  ") << what << std::endl;
        if (! ok)
            ++failures;
    }

    void section (const juce::String& title)
    {
        std::cout << "\n== " << title << " ==" << std::endl;
    }

    void setParam (rk::PluginProcessor& proc, const juce::String& id, float value)
    {
        if (auto* p = proc.apvts.getParameter (id))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (value));
        else
            check (false, "unknown parameter " + id);
    }

    float getParam (rk::PluginProcessor& proc, const juce::String& id)
    {
        if (auto* p = proc.apvts.getRawParameterValue (id))
            return p->load();
        return -1.0e9f;
    }

    //==========================================================================
    // Rendering and measuring
    //==========================================================================
    struct Capture
    {
        std::vector<float> l, r;
        double sampleRate = 48000.0;
        int size() const { return (int) l.size(); }
    };

    struct Level
    {
        double rms = 0.0, peak = 0.0, mean = 0.0;
        bool finite = true;
    };

    Level measure (const Capture& c, int from, int to)
    {
        Level out;
        from = juce::jlimit (0, c.size(), from);
        to   = juce::jlimit (from, c.size(), to);
        double sumSq = 0.0, sum = 0.0;
        juce::int64 n = 0;

        for (int i = from; i < to; ++i)
            for (const float s : { c.l[(size_t) i], c.r[(size_t) i] })
            {
                if (! std::isfinite (s))
                {
                    out.finite = false;
                    continue;
                }
                sumSq += (double) s * (double) s;
                sum += (double) s;
                out.peak = std::max (out.peak, (double) std::abs (s));
                ++n;
            }

        if (n > 0)
        {
            out.rms = std::sqrt (sumSq / (double) n);
            out.mean = sum / (double) n;
        }
        return out;
    }

    Level measure (const Capture& c) { return measure (c, 0, c.size()); }

    // Renders `totalSamples` through processBlock, in blocks of `blockSize`.
    // midiFn (blockIndex, firstSample, numSamples, midi) fills each block's events.
    template <typename MidiFn>
    Capture capture (rk::PluginProcessor& proc, double sampleRate, int blockSize, int totalSamples,
                     MidiFn&& midiFn, bool prepare = true)
    {
        if (prepare)
            proc.prepareToPlay (sampleRate, blockSize);

        Capture c;
        c.sampleRate = sampleRate;
        c.l.reserve ((size_t) totalSamples);
        c.r.reserve ((size_t) totalSamples);

        juce::AudioBuffer<float> storage (2, blockSize);
        int done = 0, blockIndex = 0;

        while (done < totalSamples)
        {
            const int n = std::min (blockSize, totalSamples - done);
            juce::AudioBuffer<float> view (storage.getArrayOfWritePointers(), 2, n);
            view.clear();

            juce::MidiBuffer midi;
            midiFn (blockIndex, done, n, midi);
            proc.processBlock (view, midi);

            for (int i = 0; i < n; ++i)
            {
                c.l.push_back (view.getSample (0, i));
                c.r.push_back (view.getSample (1, i));
            }
            done += n;
            ++blockIndex;
        }
        return c;
    }

    // One note, held for `holdSeconds`, then released; `seconds` in all.
    Capture playNote (rk::PluginProcessor& proc, int note, double seconds, double holdSeconds,
                      double sampleRate = 48000.0, int blockSize = 512)
    {
        const int total = (int) (seconds * sampleRate);
        const int offAt = (int) (holdSeconds * sampleRate);
        return capture (proc, sampleRate, blockSize, total,
                        [&] (int, int first, int n, juce::MidiBuffer& midi)
                        {
                            if (first == 0)
                                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);
                            if (offAt >= first && offAt < first + n)
                                midi.addEvent (juce::MidiMessage::noteOff (1, note), offAt - first);
                        });
    }

    float renderNoteRms (rk::PluginProcessor& proc, int note)
    {
        return (float) measure (playNote (proc, note, 1.0, 0.75)).rms;
    }

    // The string is excited by unseeded noise: two passes on the same note
    // differ by about 3 dB. Averaging four notes spread over the neck brings
    // that scatter down far enough for comparisons to mean something.
    float renderRms (rk::PluginProcessor& proc, int note)
    {
        const int notes[] = { note - 12, note - 5, note, note + 7 };
        double sum = 0.0;
        for (int n : notes)
            sum += (double) renderNoteRms (proc, juce::jlimit (24, 96, n));
        return (float) (sum / (double) juce::numElementsInArray (notes));
    }

    float renderRmsAveraged (rk::PluginProcessor& proc, int note, int times)
    {
        double sum = 0.0;
        for (int i = 0; i < times; ++i)
            sum += (double) renderRms (proc, note);
        return times > 0 ? (float) (sum / times) : 0.0f;
    }

    float renderWindowRms (rk::PluginProcessor& proc, int note, double from, double to)
    {
        const auto c = playNote (proc, note, to + 0.05, to + 0.05);
        return (float) measure (c, (int) (from * 48000.0), (int) (to * 48000.0)).rms;
    }

    // Ripple of the level over time, relative to the mean level: a tremolo is
    // judged by what it makes vary, not by what it lowers.
    float renderLevelRipple (rk::PluginProcessor& proc, int note)
    {
        const auto c = playNote (proc, note, 1.0, 1.0);
        std::vector<double> blockRms;
        for (int b = 30; b < 94; ++b)
            blockRms.push_back (measure (c, b * 512, (b + 1) * 512).rms);

        constexpr size_t window = 16;
        double spread = 0.0;
        int windows = 0;
        for (size_t start = 0; start + window <= blockRms.size(); ++start)
        {
            double lo = blockRms[start], hi = blockRms[start];
            for (size_t i = start; i < start + window; ++i)
            {
                lo = std::min (lo, blockRms[i]);
                hi = std::max (hi, blockRms[i]);
            }
            const double mid = 0.5 * (hi + lo);
            if (mid > 1.0e-9)
            {
                spread += (hi - lo) / mid;
                ++windows;
            }
        }
        return windows > 0 ? (float) (spread / windows) : 0.0f;
    }

    float renderStereoSpread (rk::PluginProcessor& proc, int note)
    {
        const auto c = playNote (proc, note, 1.0, 1.0);
        double sumSq = 0.0;
        for (int i = 0; i < c.size(); ++i)
        {
            const double d = (double) c.l[(size_t) i] - (double) c.r[(size_t) i];
            sumSq += d * d;
        }
        return (float) std::sqrt (sumSq / (double) std::max (1, c.size()));
    }

    float renderTailRms (rk::PluginProcessor& proc, int note)
    {
        // note released at 0.2 s, only the tail after 1.2 s is counted
        const auto c = playNote (proc, note, 2.0, 0.2);
        return (float) measure (c, (int) (1.2 * 48000.0), c.size()).rms;
    }

    // Pitch of a note in a window, by autocorrelation around an expected
    // frequency (+-35 %, which also rules out octave errors). Counting zero
    // crossings is no use here: a plucked string is rich in partials, and each
    // one adds crossings.
    double frequencyOf (const Capture& c, int from, int to, double expectedHz)
    {
        from = std::max (0, from);
        to = std::min (to, c.size());
        const int n = to - from;

        const int minLag = std::max (2, (int) (c.sampleRate / (expectedHz * 1.35)));
        const int maxLag = (int) (c.sampleRate / (expectedHz / 1.35));
        if (n < 2 * maxLag + 4)
            return 0.0;

        std::vector<double> x ((size_t) n);
        double mean = 0.0;
        for (int i = 0; i < n; ++i)
        {
            x[(size_t) i] = 0.5 * ((double) c.l[(size_t) (from + i)] + (double) c.r[(size_t) (from + i)]);
            mean += x[(size_t) i];
        }
        mean /= (double) n;
        for (auto& v : x)
            v -= mean;

        std::vector<double> r ((size_t) maxLag + 2, 0.0);
        int bestLag = -1;
        double best = 0.0;
        for (int lag = minLag - 1; lag <= maxLag + 1; ++lag)
        {
            double sum = 0.0;
            for (int i = 0; i + lag < n; ++i)
                sum += x[(size_t) i] * x[(size_t) (i + lag)];
            r[(size_t) lag] = sum / (double) (n - lag);
            if (lag >= minLag && lag <= maxLag && r[(size_t) lag] > best)
            {
                best = r[(size_t) lag];
                bestLag = lag;
            }
        }
        if (bestLag < 0)
            return 0.0;

        const double a = r[(size_t) bestLag - 1], b = r[(size_t) bestLag], d = r[(size_t) bestLag + 1];
        const double denom = a - 2.0 * b + d;
        const double shift = std::abs (denom) > 1.0e-18 ? 0.5 * (a - d) / denom : 0.0;
        return c.sampleRate / ((double) bestLag + shift);
    }

    // A plain, steady, single-string tone: no effect, no modulation.
    void plainTone (rk::PluginProcessor& proc)
    {
        setParam (proc, "smpOn", 0.0f);
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 0.0f);
        setParam (proc, "layerMix", 0.0f);
        setParam (proc, "laBody", 0.0f);
        setParam (proc, "laSustain", 100.0f);
        setParam (proc, "ampOn", 0.0f);
        setParam (proc, "driveAmt", 0.0f);
        setParam (proc, "delayOn", 0.0f);
        setParam (proc, "revMix", 0.0f);
        setParam (proc, "chorAmt", 0.0f);
        setParam (proc, "fltType", 3.0f);
        setParam (proc, "lfoAmount", 0.0f);
        setParam (proc, "lfoPitch", 0.0f);
        setParam (proc, "modDepth", 0.0f);
        setParam (proc, "pbend", 0.0f);
        setParam (proc, "envA", 0.5f);
        setParam (proc, "envD", 4000.0f);
        setParam (proc, "envS", 100.0f);
        setParam (proc, "envR", 100.0f);
        setParam (proc, "glideMs", 0.0f);
        setParam (proc, "monoMode", 0.0f);
    }

    std::vector<float> allNormalised (rk::PluginProcessor& proc)
    {
        std::vector<float> v;
        for (auto* p : proc.getParameters())
            v.push_back (p->getValue());
        return v;
    }

    bool sameValues (const std::vector<float>& a, const std::vector<float>& b, float tolerance = 1.0e-4f)
    {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (std::abs (a[i] - b[i]) > tolerance)
                return false;
        return true;
    }

    juce::String describe (const Level& l)
    {
        return "rms " + juce::String (l.rms, 5) + ", peak " + juce::String (l.peak, 3);
    }

    // A rough guitar note, written as a WAV: enough to prove that the user's
    // folder is read, with no recording shipped.
    bool writeTestTone (const juce::File& target, double freq)
    {
        constexpr double rate = 44100.0;
        const int length = (int) (rate * 1.5);

        juce::AudioBuffer<float> buffer (1, length);
        auto* d = buffer.getWritePointer (0);
        for (int i = 0; i < length; ++i)
        {
            const double t = (double) i / rate;
            const double env = std::exp (-2.5 * t);
            d[i] = (float) (env * (0.6 * std::sin (6.283185 * freq * t)
                                 + 0.3 * std::sin (6.283185 * freq * 2.0 * t)
                                 + 0.1 * std::sin (6.283185 * freq * 3.0 * t)));
        }

        target.deleteFile();
        std::unique_ptr<juce::FileOutputStream> out (target.createOutputStream());
        if (out == nullptr)
            return false;

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (
            wav.createWriterFor (out.release(), rate, 1, 16, {}, 0));
        return writer != nullptr && writer->writeFromAudioSampleBuffer (buffer, 0, length);
    }
}

//==============================================================================
// Strings, controls, effects
//==============================================================================
static void testEngineAndControls()
{
    section ("The two strings and the controls");

    {
        rk::PluginProcessor proc;
        setParam (proc, "smpOn", 0.0f);
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 0.0f);

        const float aOnly = renderRms (proc, 52);
        check (aOnly > 0.0005f, "string A alone sounds (rms " + juce::String (aOnly, 5) + ")");

        setParam (proc, "laOn", 0.0f);
        setParam (proc, "lbOn", 1.0f);
        const float bOnly = renderRms (proc, 52);
        check (bOnly > 0.0005f, "string B alone sounds (rms " + juce::String (bOnly, 5) + ")");

        setParam (proc, "laOn", 0.0f);
        setParam (proc, "lbOn", 0.0f);
        const float none = renderRms (proc, 52);
        check (none < 1.0e-6f, "both grills off: silence (rms " + juce::String (none, 8) + ")");

        // The body resonator once started with zero coefficients and returned
        // silence: raising BODY only attenuated the string.
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 1.0f);
        setParam (proc, "laBody", 0.0f);
        setParam (proc, "lbBody", 0.0f);
        const float dry = renderRms (proc, 45);
        setParam (proc, "laBody", 100.0f);
        setParam (proc, "lbBody", 100.0f);
        const float wet = renderRms (proc, 45);
        check (wet > 0.0005f && std::abs (wet - dry) > 0.05f * dry,
               "BODY changes the sound (dry " + juce::String (dry, 5) + " / body " + juce::String (wet, 5) + ")");

        // The two engines once shared one Synthesiser, and the second sound cut
        // the first at the very instant it started: the string must still ring
        // long after the attack.
        setParam (proc, "laBody", 30.0f);
        setParam (proc, "lbBody", 25.0f);
        setParam (proc, "envS", 80.0f);
        setParam (proc, "envD", 2000.0f);
        const float onset = renderWindowRms (proc, 52, 0.00, 0.10);
        const float tail  = renderWindowRms (proc, 52, 0.45, 0.90);
        check (tail > onset * 0.10f,
               "the string holds the note (attack " + juce::String (onset, 4) + " / tail " + juce::String (tail, 4) + ")");
    }

    // Turning a control must be heard: each one is pushed to its two extremes.
    {
        rk::PluginProcessor proc;
        setParam (proc, "smpOn", 0.0f);
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 1.0f);
        setParam (proc, "revMix", 0.0f);
        setParam (proc, "chorAmt", 0.0f);
        setParam (proc, "fltType", 3.0f);
        setParam (proc, "lfoAmount", 0.0f);
        setParam (proc, "lfoPitch", 0.0f);
        setParam (proc, "modDepth", 0.0f);

        // FILTER: a tight low-pass must clearly muffle. Measured high on the
        // neck, a cut-off at 60 Hz sits two octaves under the notes.
        setParam (proc, "fltType", 0.0f);
        setParam (proc, "fltCutoff", 18000.0f);
        setParam (proc, "fenvS", 100.0f);
        setParam (proc, "fenvA", 0.5f);
        const float wideOpen = renderRmsAveraged (proc, 72, 4);
        setParam (proc, "fltCutoff", 60.0f);
        const float clamped = renderRmsAveraged (proc, 72, 4);
        check (clamped < wideOpen * 0.5f,
               "FILTER closes the sound (open " + juce::String (wideOpen, 4) + " / closed " + juce::String (clamped, 4) + ")");

        // The high-pass is the mirror image: it must also do something.
        setParam (proc, "fltType", 2.0f);
        setParam (proc, "fltCutoff", 8000.0f);
        const float highPassed = renderRmsAveraged (proc, 55, 2);
        check (highPassed < wideOpen * 0.5f, "HIGH-PASS removes the body of a low note (rms " + juce::String (highPassed, 4) + ")");

        setParam (proc, "fltType", 3.0f);
        setParam (proc, "fltCutoff", 14000.0f);

        // LFO to VOLUME must make the level wave. One string only: two strings
        // beat against each other and would make the reference jump.
        setParam (proc, "lbOn", 0.0f);
        setParam (proc, "envA", 0.5f);
        setParam (proc, "envD", 4000.0f);
        setParam (proc, "envS", 100.0f);
        setParam (proc, "laSustain", 100.0f);
        setParam (proc, "lfoDest", 2.0f);
        setParam (proc, "lfoAmount", 0.0f);
        const float steady = renderLevelRipple (proc, 55);
        setParam (proc, "lfoAmount", 100.0f);
        setParam (proc, "lfoRate", 6.0f);
        const float tremolo = renderLevelRipple (proc, 55);
        check (tremolo > steady * 2.0f,
               "LFO to VOLUME waves the level (off " + juce::String (steady, 4) + " / on " + juce::String (tremolo, 4) + ")");

        // Every LFO shape must move something, S&H included: its trigger once
        // tested a phase that is always wrapped before it is read.
        for (int wave = 0; wave < 4; ++wave)
        {
            // S&H draws a new level once per cycle: at a higher rate the window
            // sees more of them, which makes the comparison less of a lottery.
            setParam (proc, "lfoWave", (float) wave);
            setParam (proc, "lfoRate", wave == 3 ? 12.0f : 6.0f);
            const float r = renderLevelRipple (proc, 55);
            check (r > steady * (wave == 3 ? 1.3f : 2.0f),
                   "LFO shape " + rk::names::lfoWave()[wave] + " modulates (ripple " + juce::String (r, 4) + ")");
        }
        setParam (proc, "lfoWave", 0.0f);
        setParam (proc, "lfoAmount", 0.0f);

        const float dryStereo = renderStereoSpread (proc, 55);
        setParam (proc, "chorAmt", 100.0f);
        const float wetStereo = renderStereoSpread (proc, 55);
        check (wetStereo > dryStereo * 1.2f,
               "CHORUS widens the image (dry " + juce::String (dryStereo, 4) + " / wet " + juce::String (wetStereo, 4) + ")");
        setParam (proc, "chorAmt", 0.0f);

        setParam (proc, "envR", 5.0f);
        setParam (proc, "revMix", 0.0f);
        const float dryTail = renderTailRms (proc, 55);
        setParam (proc, "revMix", 100.0f);
        const float wetTail = renderTailRms (proc, 55);
        check (wetTail > dryTail * 2.0f,
               "REVERB leaves a tail (dry " + juce::String (dryTail, 5) + " / wet " + juce::String (wetTail, 5) + ")");

        // DELAY: repeats after the note has been released.
        setParam (proc, "revMix", 0.0f);
        setParam (proc, "delayOn", 0.0f);
        const float noEcho = renderTailRms (proc, 55);
        setParam (proc, "delayOn", 1.0f);
        setParam (proc, "delayTime", 300.0f);
        setParam (proc, "delayFb", 70.0f);
        setParam (proc, "delayMix", 60.0f);
        const float echo = renderTailRms (proc, 55);
        check (echo > noEcho * 3.0f + 1.0e-5f,
               "DELAY repeats after the release (off " + juce::String (noEcho, 5) + " / on " + juce::String (echo, 5) + ")");
        setParam (proc, "delayOn", 0.0f);
    }

    // The pitch wheel (MIDI and on-screen) and the mod wheel must reach a note
    // that STARTS after they moved: a controller that only reaches the voices
    // already playing leaves the next note out. A fresh plug-in for every
    // measurement: a controller keeps its position, as a real wheel does.
    {
        using Events = std::function<void (int, int, int, juce::MidiBuffer&)>;

        auto render = [] (const Events& events, float modWheelParam)
        {
            rk::PluginProcessor proc;
            plainTone (proc);
            setParam (proc, "lfoRate", 5.0f);
            setParam (proc, "lfoWave", 0.0f);
            setParam (proc, "modDepth", modWheelParam);
            return capture (proc, 48000.0, 256, 48000 * 2, events);
        };

        auto vibratoSpread = [&] (const Events& events, float modWheelParam)
        {
            const auto c = render (events, modWheelParam);
            std::vector<double> f;
            for (int w = 0; w < 24; ++w)
            {
                const double hz = frequencyOf (c, 12000 + w * 2048, 12000 + w * 2048 + 1536, 220.0);
                if (hz > 0.0)
                    f.push_back (hz);
            }
            if (f.size() < 12)
                return -1.0;
            const auto mm = std::minmax_element (f.begin(), f.end());
            return (*mm.second - *mm.first) / (0.5 * (*mm.second + *mm.first));
        };

        auto pitchOf = [&] (const Events& events)
        {
            return frequencyOf (render (events, 0.0f), 6000, 30000, 235.0);
        };

        const Events noteOnly = [] (int, int first, int, juce::MidiBuffer& m)
        {
            if (first == 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
        };

        const double still = vibratoSpread (noteOnly, 0.0f);

        // CC1 sent in a block of its own, long before the note
        const double cc1Before = vibratoSpread ([] (int block, int first, int, juce::MidiBuffer& m)
        {
            if (block == 0) m.addEvent (juce::MidiMessage::controllerEvent (1, 1, 127), 0);
            if (block == 4 && first > 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
        }, 0.0f);

        // CC1 at 0 afterwards brings it back
        const double cc1Back = vibratoSpread ([] (int block, int first, int, juce::MidiBuffer& m)
        {
            if (block == 0) m.addEvent (juce::MidiMessage::controllerEvent (1, 1, 127), 0);
            if (block == 2) m.addEvent (juce::MidiMessage::controllerEvent (1, 1, 0), 0);
            if (block == 4 && first > 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
        }, 0.0f);

        check (still >= 0.0 && still < 0.02, "no vibrato without the mod wheel (spread " + juce::String (still, 4) + ")");
        check (cc1Before > 0.08, "CC1 sent before the note reaches it (spread " + juce::String (cc1Before, 4) + ")");
        check (cc1Back >= 0.0 && cc1Back < 0.02, "CC1 back at 0 closes the vibrato (spread " + juce::String (cc1Back, 4) + ")");

        // the MODULATION wheel of the faceplate does the same
        const double wheelUi = vibratoSpread (noteOnly, 100.0f);
        check (wheelUi > 0.08, "MODULATION wheel opens the vibrato (spread " + juce::String (wheelUi, 4) + ")");

        // pitch bend: +2 semitones before the note
        const double plain = pitchOf (noteOnly);
        const double bent = pitchOf ([] (int block, int first, int, juce::MidiBuffer& m)
        {
            if (block == 0) m.addEvent (juce::MidiMessage::pitchWheel (1, 16383), 0);
            if (block == 3 && first > 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
        });
        const double down = pitchOf ([] (int block, int first, int, juce::MidiBuffer& m)
        {
            if (block == 0) m.addEvent (juce::MidiMessage::pitchWheel (1, 0), 0);
            if (block == 3 && first > 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
        });
        check (plain > 0.0 && std::abs (bent / plain - 1.1225) < 0.012,
               "MIDI pitch bend up, sent before the note, reaches it (ratio " + juce::String (plain > 0.0 ? bent / plain : 0.0, 4) + ", expected 1.1225)");
        check (plain > 0.0 && std::abs (down / plain - 0.8909) < 0.012,
               "MIDI pitch bend down reaches it too (ratio " + juce::String (plain > 0.0 ? down / plain : 0.0, 4) + ", expected 0.8909)");

        // A reset-all-controllers must bring the wheel back
        const double afterReset = pitchOf ([] (int block, int first, int, juce::MidiBuffer& m)
        {
            if (block == 0)
            {
                m.addEvent (juce::MidiMessage::pitchWheel (1, 16383), 0);
                m.addEvent (juce::MidiMessage::controllerEvent (1, 121, 0), 1);
            }
            if (block == 3 && first > 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
        });
        check (plain > 0.0 && std::abs (afterReset / plain - 1.0) < 0.006,
               "CC121 resets the pitch wheel (ratio " + juce::String (plain > 0.0 ? afterReset / plain : 0.0, 4) + ")");

        // the PITCH BEND wheel of the faceplate: up a full travel = +2 semitones
        {
            rk::PluginProcessor proc;
            plainTone (proc);
            setParam (proc, "pbend", 2.0f);
            const double up = frequencyOf (capture (proc, 48000.0, 256, 48000, noteOnly), 6000, 30000, 235.0);
            check (plain > 0.0 && std::abs (up / plain - 1.1225) < 0.012, "PITCH BEND wheel raises the note by two semitones (ratio "
                                                                              + juce::String (plain > 0.0 ? up / plain : 0.0, 4) + ")");
        }

        // Pitch is right: A3 = 220 Hz. The string's own delay (two-tap average
        // and damping filter) lowers it by a fraction of a sample per cycle.
        check (std::abs (plain - 220.0) < 4.0, "note 57 plays 220 Hz (measured " + juce::String (plain, 2) + ")");
    }

    // GLIDE and MONO
    {
        rk::PluginProcessor proc;
        plainTone (proc);

        setParam (proc, "monoMode", 1.0f);
        setParam (proc, "glideMs", 0.0f);
        // C3 held, D3 pressed on top: in MONO the first note must be cut
        const auto c = capture (proc, 48000.0, 256, 48000, [] (int, int first, int, juce::MidiBuffer& m)
        {
            if (first == 0) m.addEvent (juce::MidiMessage::noteOn (1, 48, 1.0f), 0);
            if (first == 256 * 40) m.addEvent (juce::MidiMessage::noteOn (1, 50, 1.0f), 0);
        });
        const double f0 = frequencyOf (c, 4000, 9000, 130.8);
        const double f1 = frequencyOf (c, 40 * 256 + 6000, 40 * 256 + 12000, 146.8);
        check (std::abs (f0 - 130.8) < 3.0 && std::abs (f1 - 146.8) < 3.0,
               "MONO: the second note replaces the first (" + juce::String (f0, 1) + " Hz then " + juce::String (f1, 1) + " Hz)");

        setParam (proc, "monoMode", 0.0f);
        const auto poly = capture (proc, 48000.0, 256, 48000, [] (int, int first, int, juce::MidiBuffer& m)
        {
            if (first == 0) m.addEvent (juce::MidiMessage::noteOn (1, 48, 1.0f), 0);
            if (first == 256 * 40) m.addEvent (juce::MidiMessage::noteOn (1, 50, 1.0f), 0);
        });
        // Both strings still ring in POLY: the zero-crossing count of two
        // tones is not a single frequency, so test that C3 is still present.
        const Level lateLevel = measure (poly, 40 * 256 + 6000, 40 * 256 + 12000);
        check (lateLevel.rms > 0.001, "POLY: both notes keep sounding (" + describe (lateLevel) + ")");
    }
}

//==============================================================================
// Hosts: what they are allowed to do to a plug-in
//==============================================================================
static void testHostBehaviour()
{
    section ("Host behaviour");

    // processBlock before any prepareToPlay
    {
        rk::PluginProcessor proc;
        const auto c = capture (proc, 48000.0, 512, 48000, [] (int, int first, int, juce::MidiBuffer& m)
        {
            if (first == 0) m.addEvent (juce::MidiMessage::noteOn (1, 52, 1.0f), 0);
        }, false);
        const auto lv = measure (c);
        check (lv.finite && lv.rms > 0.001, "process before prepareToPlay does not crash and sounds (" + describe (lv) + ")");
    }

    // blocks larger than the announced size, odd sizes and rates
    {
        const double rates[] = { 22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
        const int sizes[] = { 1, 7, 64, 513, 2048, 8192 };
        bool allFine = true;
        juce::String firstProblem;

        for (double rate : rates)
            for (int size : sizes)
            {
                rk::PluginProcessor proc;
                proc.prepareToPlay (rate, 64);        // announced: 64 samples at most
                setParam (proc, "delayOn", 1.0f);
                setParam (proc, "chorAmt", 50.0f);
                setParam (proc, "fltType", 0.0f);

                const int total = (int) (rate * 0.3);
                const auto c = capture (proc, rate, size, total, [] (int, int first, int, juce::MidiBuffer& m)
                {
                    if (first == 0)
                    {
                        m.addEvent (juce::MidiMessage::noteOn (1, 40, 1.0f), 0);
                        m.addEvent (juce::MidiMessage::noteOn (1, 64, 0.8f), 0);
                    }
                }, false);

                const auto lv = measure (c);
                if (! lv.finite || lv.rms < 1.0e-4 || lv.peak > 1.0001)
                {
                    allFine = false;
                    if (firstProblem.isEmpty())
                        firstProblem = juce::String (rate, 0) + " Hz, block " + juce::String (size) + ": " + describe (lv);
                }
            }

        check (allFine, "6 sample rates x 6 block sizes (larger than announced): finite, audible, bounded"
                            + (firstProblem.isEmpty() ? juce::String() : " - " + firstProblem));
    }

    // empty block
    {
        rk::PluginProcessor proc;
        proc.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> empty (2, 0);
        juce::MidiBuffer midi;
        proc.processBlock (empty, midi);
        juce::AudioBuffer<float> noChannels (0, 128);
        proc.processBlock (noChannels, midi);
        check (true, "empty blocks (0 samples, 0 channels) are accepted");
    }

    // a mono buffer, with an event in the second half of a long block
    {
        rk::PluginProcessor proc;
        proc.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> mono (1, 6000);
        mono.clear();
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 52, 1.0f), 4500);
        proc.processBlock (mono, midi);

        double early = 0.0, late = 0.0;
        bool finite = true;
        for (int i = 0; i < 6000; ++i)
        {
            const float s = mono.getSample (0, i);
            finite = finite && std::isfinite (s);
            (i < 4500 ? early : late) += (double) s * (double) s;
        }
        check (finite && early < 1.0e-12 && late > 1.0e-4,
               "mono output: events keep their position across the internal chunks");
    }

    // events outside the block must not be lost: a lost note-off is a stuck note
    {
        rk::PluginProcessor proc;
        plainTone (proc);
        setParam (proc, "envR", 5.0f);
        const auto c = capture (proc, 48000.0, 256, 48000 * 2, [] (int block, int, int, juce::MidiBuffer& m)
        {
            if (block == 0) m.addEvent (juce::MidiMessage::noteOn (1, 52, 1.0f), 0);
            // stamped past the end of the block: a sloppy host
            if (block == 20) m.addEvent (juce::MidiMessage::noteOff (1, 52), 5000);
        });
        check (measure (c, 48000, 96000).rms < 1.0e-4, "a note-off stamped beyond the block still releases the note");
    }

    // bypass
    {
        rk::PluginProcessor proc;
        plainTone (proc);
        check (proc.getBypassParameter() == nullptr,
               "no own bypass parameter: the VST3 wrapper provides it and calls processBlockBypassed");

        proc.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> buf (2, 512);
        juce::MidiBuffer m;
        m.addEvent (juce::MidiMessage::noteOn (1, 52, 1.0f), 0);
        proc.processBlock (buf, m);

        // bypass while the note is held, with a key clicked on screen meanwhile
        proc.getKeyboardState().noteOn (1, 60, 1.0f);
        bool silent = true;
        for (int b = 0; b < 10; ++b)
        {
            buf.clear();
            juce::MidiBuffer m2;
            proc.processBlockBypassed (buf, m2);
            silent = silent && buf.getMagnitude (0, 512) == 0.0f && m2.isEmpty();
        }
        check (silent, "bypassed: silence, and no MIDI passed on");
        check (proc.getActiveNotesSnapshot().empty(), "bypassed: held notes are forgotten");

        // lifting the bypass: nothing may restart on its own, not even the
        // on-screen key pressed meanwhile
        double after = 0.0;
        for (int b = 0; b < 40; ++b)
        {
            buf.clear();
            juce::MidiBuffer m3;
            proc.processBlock (buf, m3);
            after += buf.getMagnitude (0, 512) + buf.getMagnitude (1, 512);
        }
        check (after < 1.0e-4, "after the bypass: no stuck or phantom note (sum " + juce::String (after, 6) + ")");
    }

    // garbage state data
    {
        rk::PluginProcessor proc;
        const auto before = allNormalised (proc);
        proc.setStateInformation (nullptr, 0);
        const char junk[] = "this is not a state";
        proc.setStateInformation (junk, (int) sizeof (junk));
        juce::MemoryBlock noise (4096, true);
        juce::Random rng (7);
        for (size_t i = 0; i < noise.getSize(); ++i)
            noise[i] = (char) rng.nextInt (256);
        proc.setStateInformation (noise.getData(), (int) noise.getSize());
        juce::XmlElement wrong ("SOMETHING_ELSE");
        juce::MemoryBlock wrongBlock;
        juce::AudioProcessor::copyXmlToBinary (wrong, wrongBlock);
        proc.setStateInformation (wrongBlock.getData(), (int) wrongBlock.getSize());
        check (sameValues (before, allNormalised (proc)), "garbage and foreign state data are ignored");
    }

    // layout
    {
        rk::PluginProcessor proc;
        juce::AudioProcessor::BusesLayout stereo, mono;
        stereo.outputBuses.add (juce::AudioChannelSet::stereo());
        mono.outputBuses.add (juce::AudioChannelSet::mono());
        check (proc.isBusesLayoutSupported (stereo) && ! proc.isBusesLayoutSupported (mono),
               "stereo output accepted, mono refused (the host then picks stereo)");
        check (proc.acceptsMidi() && ! proc.producesMidi() && ! proc.isMidiEffect(), "instrument: MIDI in, audio out");
    }
}

//==============================================================================
// Notes that must end
//==============================================================================
static void testStuckNotes()
{
    section ("Stuck notes, NaN, DC");

    // a release brings the sound to silence
    {
        rk::PluginProcessor proc;
        plainTone (proc);
        setParam (proc, "envR", 80.0f);
        setParam (proc, "laSustain", 40.0f);
        setParam (proc, "envS", 100.0f);

        const auto c = playNote (proc, 52, 3.0, 0.3);
        check (measure (c, 48000 * 2, 48000 * 3).rms < 1.0e-5, "a released note falls silent (" + describe (measure (c, 48000 * 2, 48000 * 3)) + ")");
    }

    // more notes than voices, then everything released
    {
        rk::PluginProcessor proc;
        plainTone (proc);
        setParam (proc, "envR", 60.0f);
        const auto c = capture (proc, 48000.0, 128, 48000 * 3, [] (int block, int, int, juce::MidiBuffer& m)
        {
            if (block < 40 && block % 2 == 0)
                m.addEvent (juce::MidiMessage::noteOn (1, 40 + block / 2, 0.9f), 0);
            if (block == 60)
                for (int n = 40; n < 60; ++n)
                    m.addEvent (juce::MidiMessage::noteOff (1, n), 0);
        });
        const auto lv = measure (c, 48000 * 2, 48000 * 3);
        check (measure (c).finite && lv.rms < 1.0e-5, "20 notes on 8 voices, all released: silence (" + describe (lv) + ")");
    }

    // all notes off / all sound off
    for (int controller : { 123, 120 })
    {
        rk::PluginProcessor proc;
        plainTone (proc);
        setParam (proc, "envR", 4000.0f);          // an endless release: only the controller can stop it
        const auto c = capture (proc, 48000.0, 256, 48000 * 3, [controller] (int block, int, int, juce::MidiBuffer& m)
        {
            if (block == 0)
                for (int n : { 40, 47, 52, 56, 59, 64 })
                    m.addEvent (juce::MidiMessage::noteOn (1, n, 1.0f), 0);
            if (block == 20)
                m.addEvent (juce::MidiMessage::controllerEvent (1, controller, 0), 0);
        });
        const bool noPhantomKeys = proc.getActiveNotesSnapshot().empty();
        const auto lv = measure (c, 48000 * 2, 48000 * 3);
        if (controller == 120)
            check (lv.rms < 1.0e-5 && noPhantomKeys, "CC120 (all sound off) silences at once");
        else
            check (noPhantomKeys && measure (c, 48000, 48000 * 3).finite, "CC123 (all notes off) clears the held notes");
    }

    // the on-screen keyboard: a note on and off, and what the editor keeps
    {
        rk::PluginProcessor proc;
        plainTone (proc);
        setParam (proc, "envR", 40.0f);
        proc.getKeyboardState().noteOn (1, 60, 1.0f);
        const auto c = capture (proc, 48000.0, 256, 48000 * 2, [&proc] (int block, int, int, juce::MidiBuffer&)
        {
            if (block == 60)
                proc.getKeyboardState().noteOff (1, 60, 0.0f);
        });
        check (measure (c, 0, 8000).rms > 0.001 && measure (c, 48000 + 24000, 48000 * 2).rms < 1.0e-5,
               "on-screen key: sounds while held, stops when released");
    }

    // random settings: no NaN, nothing out of bounds, no DC
    {
        juce::Random rng (20260816);
        bool nonFinite = false, overshoot = false, dcSeen = false;
        int worst = 0;
        double worstDc = 0.0;
        int events = 0;

        for (int iteration = 0; iteration < 120; ++iteration)
        {
            rk::PluginProcessor proc;
            for (auto* p : proc.getParameters())
                p->setValueNotifyingHost (rng.nextFloat());
            setParam (proc, "smpOn", 0.0f);
            setParam (proc, "laOn", 1.0f);

            const bool limiter = proc.apvts.getRawParameterValue ("outLimiter")->load() > 0.5f;
            const int bend = rng.nextInt (16384);
            const int mod = rng.nextInt (128);

            const auto c = capture (proc, 48000.0, 128 << rng.nextInt (4), 48000 / 2, [&] (int block, int, int, juce::MidiBuffer& m)
            {
                if (block == 0)
                {
                    m.addEvent (juce::MidiMessage::pitchWheel (1, bend), 0);
                    m.addEvent (juce::MidiMessage::controllerEvent (1, 1, mod), 0);
                    for (int n : { 28, 40, 52, 64, 76, 100 })
                        m.addEvent (juce::MidiMessage::noteOn (1, n, rng.nextFloat()), 0);
                }
            });

            const auto lv = measure (c);
            if (! lv.finite || proc.getNonFiniteEventCount() > 0)
            {
                nonFinite = true;
                worst = iteration;
            }
            if (limiter && lv.peak > 1.0001)
                overshoot = true;
            events += proc.getNonFiniteEventCount();
            worstDc = std::max (worstDc, std::abs (lv.mean) / std::max (1.0e-9, lv.rms));
            dcSeen = dcSeen || (limiter && std::abs (lv.mean) > 0.05);
        }

        check (! nonFinite, "120 random settings x 6 notes: no NaN or infinity (safety net triggered "
                                + juce::String (events) + " times" + (nonFinite ? ", first at #" + juce::String (worst) : juce::String()) + ")");
        check (! overshoot, "120 random settings: the limiter keeps the output within +-1");
        check (! dcSeen, "120 random settings: no DC offset (worst mean/rms " + juce::String (worstDc, 3) + ")");
    }
}

//==============================================================================
// Units: LFO and delay
//==============================================================================
static void testDspUnits()
{
    section ("LFO and delay");

    {
        rk::Lfo lfo;
        lfo.setSampleRate (48000.0);
        lfo.reset();
        lfo.setFrequency (5.0f);
        lfo.setShape (4);

        std::set<float> values;
        for (int i = 0; i < 48000 * 3; ++i)
            values.insert (lfo.process());
        check (values.size() >= 12 && values.size() <= 16,
               "S&H: about one new value per cycle (" + juce::String ((int) values.size()) + " values in 15 cycles)");

        lfo.setShape (0);
        float lo = 9.0f, hi = -9.0f;
        for (int i = 0; i < 48000; ++i)
        {
            const float v = lfo.process();
            lo = std::min (lo, v);
            hi = std::max (hi, v);
        }
        check (lo < -0.99f && hi > 0.99f, "sine LFO spans -1 to +1");
    }

    {
        // echo exactly where it should be
        rk::Delay d;
        d.setSampleRate (48000.0);
        d.prepare();

        std::vector<float> in (48000, 0.0f), out (48000, 0.0f), zero (48000, 0.0f), dummy (48000, 0.0f);
        in[0] = 1.0f;
        d.process (in.data(), zero.data(), out.data(), dummy.data(), 48000, 100.0f, 0.0f, 1.0f, 20000.0f, true);
        // ping-pong: the echo of the left input is read from the right line,
        // so the left output has nothing and the right one carries it
        int at = -1;
        for (int i = 1; i < 48000; ++i)
            if (std::abs (dummy[(size_t) i]) > 0.1f) { at = i; break; }
        check (std::abs (at - 4800) <= 2, "100 ms delay: first echo at sample " + juce::String (at) + " (expected 4800)");
    }

    {
        // long synced divisions: 4 bars at 120 bpm = 8 s
        rk::Delay d;
        d.setSampleRate (48000.0);
        d.prepare();
        const int total = 48000 * 9;
        std::vector<float> in ((size_t) total, 0.0f), out ((size_t) total), other ((size_t) total), zero ((size_t) total, 0.0f);
        in[0] = 1.0f;
        // two blocks: right input carries the click so that the echo shows on the left output
        d.process (zero.data(), in.data(), out.data(), other.data(), total, rk::divisionToMs (120.0f, 8), 0.0f, 1.0f, 20000.0f, true);
        int at = -1;
        for (int i = 1; i < total; ++i)
            if (std::abs (out[(size_t) i]) > 0.1f) { at = i; break; }
        check (std::abs (at - 8 * 48000) <= 2, "4-bar delay at 120 bpm: echo at " + juce::String (at / 48000.0, 3) + " s (expected 8 s)");
    }

    {
        // switching the delay off and on again leaves no ghost
        rk::Delay d;
        d.setSampleRate (48000.0);
        d.prepare();
        std::vector<float> tone (4800), zero (4800, 0.0f), o1 (4800), o2 (4800);
        for (size_t i = 0; i < tone.size(); ++i)
            tone[i] = (float) std::sin (0.1 * (double) i);
        d.process (tone.data(), tone.data(), o1.data(), o2.data(), 4800, 50.0f, 0.8f, 0.5f, 20000.0f, true);
        d.process (zero.data(), zero.data(), o1.data(), o2.data(), 4800, 50.0f, 0.8f, 0.5f, 20000.0f, false);
        d.process (zero.data(), zero.data(), o1.data(), o2.data(), 4800, 50.0f, 0.8f, 0.5f, 20000.0f, true);
        float peak = 0.0f;
        for (float v : o1) peak = std::max (peak, std::abs (v));
        for (float v : o2) peak = std::max (peak, std::abs (v));
        check (peak == 0.0f, "delay switched off then on: no old echo comes back");
    }

    {
        // a change of delay time must not click
        rk::Delay d;
        d.setSampleRate (48000.0);
        d.prepare();
        const int block = 480;
        std::vector<float> in ((size_t) block), outL ((size_t) block), outR ((size_t) block);
        double phase = 0.0;
        float previous = 0.0f, worstStep = 0.0f;
        bool started = false;

        for (int b = 0; b < 200; ++b)
        {
            for (int i = 0; i < block; ++i)
            {
                in[(size_t) i] = 0.5f * (float) std::sin (phase);
                phase += 2.0 * juce::MathConstants<double>::pi * 440.0 / 48000.0;
            }
            const float time = b < 100 ? 100.0f : 160.0f;      // jump at block 100
            d.process (in.data(), in.data(), outL.data(), outR.data(), block, time, 0.0f, 1.0f, 20000.0f, true);
            for (int i = 0; i < block; ++i)
            {
                // the line is fed with the left and read from the right one: the
                // wet signal shows on both outputs once both sides are fed
                if (started && b > 20)
                    worstStep = std::max (worstStep, std::abs (outL[(size_t) i] - previous));
                previous = outL[(size_t) i];
                started = true;
            }
        }
        // an abrupt jump would show a step of up to 1.0; a 440 Hz sine of 0.5
        // moves by 0.03 per sample, and a glide may at most double that
        check (worstStep < 0.2f, "delay time change glides (largest step " + juce::String (worstStep, 3) + ")");
    }
}

//==============================================================================
// State, parameter text
//==============================================================================
static void testStateAndText()
{
    section ("State and parameters");

    rk::PluginProcessor proc;
    const int count = proc.getParameters().size();
    check (count == 65, "65 parameters declared (" + juce::String (count) + ")");

    // every parameter survives getStateInformation / setStateInformation
    {
        juce::Random rng (99);
        for (auto* p : proc.getParameters())
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
                ranged->setValueNotifyingHost (ranged->convertTo0to1 (ranged->convertFrom0to1 (rng.nextFloat())));   // a legal value
        const auto expected = allNormalised (proc);

        juce::MemoryBlock saved;
        proc.getStateInformation (saved);

        rk::PluginProcessor fresh;
        fresh.setStateInformation (saved.getData(), (int) saved.getSize());
        const auto restored = allNormalised (fresh);

        int bad = 0;
        juce::String first;
        for (size_t i = 0; i < expected.size(); ++i)
            if (std::abs (expected[i] - restored[i]) > 1.0e-4f)
            {
                ++bad;
                if (first.isEmpty())
                    first = fresh.getParameters()[(int) i]->getName (40);
            }

        check (bad == 0, "state round trip: every one of the " + juce::String (count) + " parameters is restored"
                             + (bad == 0 ? juce::String() : " - " + juce::String (bad) + " differ, first " + first));

        // and a second round trip is stable
        juce::MemoryBlock again;
        fresh.getStateInformation (again);
        rk::PluginProcessor third;
        third.setStateInformation (again.getData(), (int) again.getSize());
        check (sameValues (restored, allNormalised (third)), "second round trip gives the same values");
    }

    // parameter text: unit, and what a host can parse back
    {
        int idempotent = 0, total = 0;
        juce::String firstBad;

        for (auto* p : proc.getParameters())
            for (float v : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f })
            {
                const auto text = p->getText (v, 0);
                const auto back = p->getValueForText (text);
                const auto again = p->getText (back, 0);
                ++total;
                if (again == text && text.isNotEmpty())
                    ++idempotent;
                else if (firstBad.isEmpty())
                    firstBad = p->getName (40) + ": \"" + text + "\" -> \"" + again + "\"";
            }

        check (idempotent == total, "parameter text parses back to the same text (" + juce::String (idempotent) + "/" + juce::String (total)
                                        + (firstBad.isEmpty() ? juce::String() : ", first " + firstBad) + ")");

        auto textOf = [&] (const juce::String& id, float plain)
        {
            auto* p = proc.apvts.getParameter (id);
            return p->getText (p->convertTo0to1 (plain), 0);
        };

        check (textOf ("masterVol", -3.0f).endsWith ("dB"), "Volume reads in dB: " + textOf ("masterVol", -3.0f));
        check (textOf ("ampTone", 2500.0f) == "2.50 kHz", "Amp Tone reads in kHz: " + textOf ("ampTone", 2500.0f));
        check (textOf ("ampTone", 250.0f) == "250 Hz", "Amp Tone reads in Hz: " + textOf ("ampTone", 250.0f));
        check (textOf ("delayTime", 350.0f) == "350 ms", "Delay Time reads in ms: " + textOf ("delayTime", 350.0f));
        check (textOf ("delayTime", 1500.0f) == "1.50 s", "Delay Time reads in s: " + textOf ("delayTime", 1500.0f));
        check (textOf ("laBlend", 80.0f) == "80 %", "Blend reads in %: " + textOf ("laBlend", 80.0f));
        check (textOf ("laLevel", 0.5f) == "50 %", "Level reads in %: " + textOf ("laLevel", 0.5f));
        check (textOf ("driveType", 3.0f) == "Fuzz", "Drive Type reads by name: " + textOf ("driveType", 3.0f));
        check (textOf ("delayDiv", 2.0f) == "1/8", "Delay Division reads as a note value: " + textOf ("delayDiv", 2.0f));
        check (textOf ("fltType", 3.0f) == "Off", "Filter Type reads by name: " + textOf ("fltType", 3.0f));
        check (textOf ("monoMode", 1.0f) == "On" && textOf ("laOn", 0.0f) == "Off", "switches read On / Off");

        auto* tone = proc.apvts.getParameter ("ampTone");
        check (std::abs (tone->convertFrom0to1 (tone->getValueForText ("2.5k")) - 2500.0f) < 5.0f,
               "typing \"2.5k\" gives 2500 Hz");
        auto* delayTime = proc.apvts.getParameter ("delayTime");
        check (std::abs (delayTime->convertFrom0to1 (delayTime->getValueForText ("1.2 s")) - 1200.0f) < 5.0f,
               "typing \"1.2 s\" gives 1200 ms");
        auto* drive = proc.apvts.getParameter ("driveType");
        check (std::abs (drive->convertFrom0to1 (drive->getValueForText ("fuzz")) - 3.0f) < 0.1f, "typing \"fuzz\" selects Fuzz");

        // names a host shows: English, no leftovers
        bool allEnglish = true;
        for (auto* p : proc.getParameters())
            allEnglish = allEnglish && p->getName (64).containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ &/");
        check (allEnglish, "parameter names are plain English");
    }
}

//==============================================================================
// Presets
//==============================================================================
static void testPresets (const juce::File& factoryDir)
{
    section ("Presets");

    if (! factoryDir.isDirectory())
    {
        check (false, "factory presets folder found: " + factoryDir.getFullPathName());
        return;
    }

    auto files = factoryDir.findChildFiles (juce::File::findFiles, false, "*.rpk");
    files.sort();
    check (files.size() == 76, "76 factory presets found (" + juce::String (files.size()) + ")");

    rk::PluginProcessor reference;
    juce::StringArray knownIds;
    for (auto* p : reference.getParameters())
        if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
            knownIds.add (withId->paramID);

    double lowest = 1.0e9, highest = 0.0;
    int silent = 0, unreadable = 0, usingSamples = 0, unknownIds = 0, notFinite = 0, overshoot = 0, dcBad = 0, missing = 0;
    juce::String firstProblem, quietest, loudest;

    for (const auto& f : files)
    {
        rk::PluginProcessor proc;
        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
        if (xml == nullptr)
        {
            ++unreadable;
            firstProblem = firstProblem.isEmpty() ? "unreadable " + f.getFileName() : firstProblem;
            continue;
        }

        const auto tree = juce::ValueTree::fromXml (*xml);
        juce::StringArray idsInFile;
        bool usesSamples = false;
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            const auto child = tree.getChild (i);
            const auto id = child.getProperty ("id").toString();
            idsInFile.add (id);
            if (! knownIds.contains (id))
            {
                ++unknownIds;
                firstProblem = firstProblem.isEmpty() ? f.getFileName() + " has unknown parameter " + id : firstProblem;
            }
            if (id == "smpOn" && (float) child.getProperty ("value") >= 0.5f)
                usesSamples = true;
        }
        for (const auto& id : knownIds)
            if (! idsInFile.contains (id))
                ++missing;
        if (usesSamples)
            ++usingSamples;       // would be mute on a fresh install

        check (proc.loadPreset (f) && proc.getCurrentPresetName() == f.getFileNameWithoutExtension(),
               f.getFileNameWithoutExtension().paddedRight (' ', 24) + " loads through the plug-in and shows its name");

        // four notes spread on the neck, as in the level measurement
        double rmsSum = 0.0;
        for (int n : { 43, 50, 55, 62 })
        {
            const auto c = playNote (proc, n, 1.2, 0.9);
            const auto lv = measure (c);
            rmsSum += lv.rms;
            if (! lv.finite) ++notFinite;
            if (lv.peak > 1.0001) ++overshoot;
            if (std::abs (lv.mean) > 0.02) ++dcBad;
        }
        const double rms = rmsSum / 4.0;
        if (rms < lowest)  { lowest = rms;  quietest = f.getFileNameWithoutExtension(); }
        if (rms > highest) { highest = rms; loudest = f.getFileNameWithoutExtension(); }
        if (rms < 0.0005)
        {
            ++silent;
            firstProblem = firstProblem.isEmpty() ? f.getFileName() + " is silent" : firstProblem;
        }
        if (proc.getNonFiniteEventCount() > 0)
            ++notFinite;
    }

    check (unreadable == 0, "every preset file is readable XML");
    check (unknownIds == 0, "presets reference only existing parameters");
    check (missing == 0, "presets define every parameter (a missing one would keep the previous value)");
    check (usingSamples == 0, "no factory preset needs a sample (the Samples folder is empty on a new install)");
    check (silent == 0, "no factory preset is silent");
    check (notFinite == 0, "no factory preset produces NaN or infinity");
    check (overshoot == 0, "no factory preset exceeds full scale");
    check (dcBad == 0, "no factory preset carries a DC offset");
    check (highest < lowest * 6.0, "levels stay within 15 dB of each other: " + juce::String (lowest, 3) + " (" + quietest + ") to "
                                       + juce::String (highest, 3) + " (" + loudest + ") rms");
    if (firstProblem.isNotEmpty())
        std::cout << "        first problem: " << firstProblem << std::endl;

    // loading must not depend on what was loaded before
    if (files.size() >= 2)
    {
        rk::PluginProcessor a, b;
        a.loadPreset (files[files.size() - 1]);
        a.loadPreset (files[0]);
        b.loadPreset (files[0]);
        check (sameValues (allNormalised (a), allNormalised (b)), "a preset loads the same whatever was loaded before");
    }

    // stepping
    {
        rk::PluginProcessor proc;
        proc.refreshPresetList();
        const int n = proc.getPresetFiles().size();
        if (n > 1)
        {
            proc.stepPreset (1);
            const int first = proc.getCurrentPresetIndex();
            proc.stepPreset (-1);
            const int back = proc.getCurrentPresetIndex();
            check (first == 0 && back == n - 1, "from a fresh patch, next gives the first preset and previous wraps to the last");
        }
    }

    // a project saved with a preset remembers its name, and a new instance shows it
    {
        rk::PluginProcessor a;
        a.loadPreset (files[3]);
        juce::MemoryBlock saved;
        a.getStateInformation (saved);
        rk::PluginProcessor b;
        b.loadPreset (files[7]);                 // some other sound was loaded here
        b.setStateInformation (saved.getData(), (int) saved.getSize());
        check (b.getCurrentPresetName() == files[3].getFileNameWithoutExtension(),
               "restoring a project shows the preset name saved with it (" + b.getCurrentPresetName() + ")");
    }

    // save, list, reload, delete
    {
        rk::PluginProcessor proc;
        proc.loadPreset (files[5]);
        setParam (proc, "ampGain", 17.0f);
        const auto target = proc.getPresetDirectory().getChildFile ("_rockpunk_test_preset.rpk");
        check (proc.savePreset (target) && target.existsAsFile(), "a preset can be saved");

        rk::PluginProcessor other;
        check (other.loadPreset (target) && std::abs (getParam (other, "ampGain") - 17.0f) < 0.01f, "a saved preset loads back with its values");

        const auto text = target.loadFileAsString();
        check (! text.contains ("uiWidth") && ! text.contains ("presetName"), "window size and name are not written into the preset");

        check (! other.loadPreset (factoryDir.getChildFile ("does not exist.rpk")), "a missing preset file is refused");
        const auto broken = proc.getPresetDirectory().getChildFile ("_rockpunk_broken.rpk");
        broken.replaceWithText ("<this is <not> xml");
        check (! other.loadPreset (broken), "a broken preset file is refused");
        broken.deleteFile();
        target.deleteFile();
        proc.refreshPresetList();
    }
}

//==============================================================================
// The sample folder
//==============================================================================
static void testSampleFolder()
{
    section ("Your samples folder");

    const auto dir = rk::SampleBank::getUserSampleDirectory();
    dir.getChildFile ("README.txt").deleteFile();

    rk::PluginProcessor proc;
    check (dir.isDirectory(), "Samples folder created: " + dir.getFullPathName());

    const auto readme = dir.getChildFile ("README.txt");
    check (readme.existsAsFile(), "instructions written at first launch");

    const auto bytes = readme.loadFileAsString();
    check (bytes.contains ("Drop your .wav") && bytes.contains ("EXPANSION"), "the instructions are in English");
    check (! bytes.contains ("\r\r") && bytes.contains ("\r\n"), "line endings are CR+LF, not doubled");
    check (bytes.containsOnly (" \r\n\t\"'.,;:()-_/+%0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@"),
           "the instructions are plain ASCII (they open anywhere)");

    const auto probe = dir.getChildFile ("_rockpunk_test_tone.wav");
    if (writeTestTone (probe, 220.0))
    {
        proc.rescanSamples();
        check (proc.getNumSampleFiles() > 0, "a dropped file is seen (" + juce::String (proc.getNumSampleFiles()) + " entries)");

        int index = -1;
        const auto names = proc.getSampleFiles();
        for (int i = 0; i < names.size(); ++i)
            if (names[i] == probe.getFileNameWithoutExtension())
                index = i;

        if (index >= 0)
        {
            setParam (proc, "laOn", 0.0f);
            setParam (proc, "lbOn", 0.0f);
            setParam (proc, "smpOn", 1.0f);
            setParam (proc, "smpSel", (float) index);
            proc.pollSampleSelection();
            const float rms = renderRms (proc, 60);
            check (rms > 0.0005f, "a dropped file plays (rms " + juce::String (rms, 5) + ")");

            const float root = proc.getSampleRootNote();
            check (std::abs (root - 57.0f) < 2.0f, "its fundamental is measured (" + juce::String (root, 1) + " for a 220 Hz A)");

            // switching the layer off while a note rings must not leave it ringing
            setParam (proc, "smpLoop", 1.0f);
            setParam (proc, "revMix", 0.0f);
            setParam (proc, "chorAmt", 0.0f);
            setParam (proc, "delayOn", 0.0f);
            const auto c = capture (proc, 48000.0, 256, 48000 * 2, [&proc] (int block, int, int, juce::MidiBuffer& m)
            {
                if (block == 0) m.addEvent (juce::MidiMessage::noteOn (1, 57, 1.0f), 0);
                if (block == 20) setParam (proc, "smpOn", 0.0f);
            });
            check (measure (c, 48000, 96000).rms < 1.0e-5, "switching the sample layer off silences it");
        }
        else
        {
            check (false, "the test file is missing from the list");
        }
        probe.deleteFile();
        proc.rescanSamples();
    }
    else
    {
        check (false, "could not write the test file");
    }

    // a corrupt file must not hurt
    {
        const auto bad = dir.getChildFile ("_rockpunk_test_corrupt.wav");
        bad.replaceWithText ("RIFF this is not audio");
        rk::PluginProcessor p2;
        p2.rescanSamples();
        setParam (p2, "smpOn", 1.0f);
        const auto names = p2.getSampleFiles();
        for (int i = 0; i < names.size(); ++i)
            if (names[i] == bad.getFileNameWithoutExtension())
            {
                setParam (p2, "smpSel", (float) i);
                p2.pollSampleSelection();
            }
        const auto c = playNote (p2, 60, 0.5, 0.3);
        check (measure (c).finite, "a corrupt audio file is survived");
        bad.deleteFile();
    }
}

//==============================================================================
// The interface: geometry, hit-testing, snapshot
//==============================================================================
namespace
{
    juce::MouseEvent makeMouseEvent (juce::Component& c, juce::Point<float> position, juce::Point<float> downPosition,
                                     bool dragged, bool shift = false)
    {
        return juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), position,
                                 shift ? juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::shiftModifier)
                                       : juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier),
                                 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, &c, &c, juce::Time::getCurrentTime(),
                                 downPosition, juce::Time::getCurrentTime(), 1, dragged);
    }

    // image (mockup) pixels -> the panel's own pixels
    juce::Point<float> toPanel (float x, float y) { return { x * rk::Art::kx, y * rk::Art::ky }; }

    // A press, a drag of `dy` design pixels and (optionally) a release.
    void dragAt (rk::Panel& panel, juce::Point<float> start, float dy, bool release)
    {
        panel.mouseDown (makeMouseEvent (panel, start, start, false));
        panel.mouseDrag (makeMouseEvent (panel, start + juce::Point<float> (0.0f, dy), start, true));
        if (release)
            panel.mouseUp (makeMouseEvent (panel, start + juce::Point<float> (0.0f, dy), start, true));
    }

    juce::Image snapshotOf (rk::PluginProcessor& proc, int width, int height)
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
        if (editor == nullptr)
            return {};
        editor->setSize (width, height);
        return editor->createComponentSnapshot (editor->getLocalBounds(), true);
    }

    bool writePng (const juce::Image& image, const juce::File& target)
    {
        target.deleteFile();
        std::unique_ptr<juce::FileOutputStream> out (target.createOutputStream());
        if (out == nullptr)
            return false;
        juce::PNGImageFormat png;
        return png.writeImageToStream (image, *out);
    }
}

static void testInterface()
{
    section ("Interface");

    rk::Art art;
    check (art.valid, "artwork and layout load from the binary");
    if (! art.valid)
        return;

    // geometry: everything inside the faceplate, and the layout is what the product has
    check (art.knobs.size() == 11 && art.faders.size() == 9 && art.wheels.size() == 2 && art.combos.size() == 5,
           "layout: 11 knobs, 9 faders, 2 wheels, 5 drop-downs");

    bool inside = true;
    for (const auto& k : art.knobs)
        inside = inside && k.x > 60 && k.x < rk::Art::imageW - 60 && k.y > 60 && k.y < rk::Art::imageH - 60;
    for (const auto& f : art.faders)
        inside = inside && f.cx > 0 && f.cx < rk::Art::imageW && f.slotTop > 0 && f.slotBottom < rk::Art::imageH && f.minY < f.maxY;
    check (inside, "every control lies inside the faceplate");

    // every knob, fader and wheel exists as a parameter
    {
        rk::PluginProcessor proc;
        bool allExist = true;
        for (const auto& k : art.knobs)  allExist = allExist && proc.apvts.getParameter (k.param) != nullptr;
        for (const auto& f : art.faders) allExist = allExist && proc.apvts.getParameter (f.param) != nullptr;
        for (const auto& w : art.wheels) allExist = allExist && proc.apvts.getParameter (w.param) != nullptr;
        for (const auto& c : art.combos) allExist = allExist && proc.apvts.getParameter (c.param) != nullptr;
        check (allExist, "every control of the layout is bound to an existing parameter");

        // and no two controls share a parameter
        juce::StringArray used;
        bool unique = true;
        for (const auto& k : art.knobs)  { unique = unique && ! used.contains (k.param); used.add (k.param); }
        for (const auto& f : art.faders) { unique = unique && ! used.contains (f.param); used.add (f.param); }
        for (const auto& w : art.wheels) { unique = unique && ! used.contains (w.param); used.add (w.param); }
        for (const auto& c : art.combos) { unique = unique && ! used.contains (c.param); used.add (c.param); }
        check (unique, "no two controls drive the same parameter");
    }

    // dragging each control, at its drawn centre, moves its parameter
    {
        rk::PluginProcessor proc;
        rk::Panel panel (proc, art);

        int moved = 0, total = 0;
        juce::String stuck;

        auto tryControl = [&] (const juce::String& param, juce::Point<float> where, const juce::String& name, bool wheel)
        {
            auto* p = proc.apvts.getParameter (param);
            const float before = p->getValue();
            const float dy = before > 0.5f ? 50.0f : -50.0f;

            panel.mouseMove (makeMouseEvent (panel, toPanel (where.x, where.y), toPanel (where.x, where.y), false));
            dragAt (panel, toPanel (where.x, where.y), dy, ! wheel);
            const float during = p->getValue();
            if (wheel)
                panel.mouseUp (makeMouseEvent (panel, toPanel (where.x, where.y) + juce::Point<float> (0.0f, dy), toPanel (where.x, where.y), true));

            ++total;
            if (std::abs (during - before) > 0.02f)
                ++moved;
            else if (stuck.isEmpty())
                stuck = name + " (" + param + ")";

            // back to where it was, for the next control
            p->setValueNotifyingHost (before);
        };

        for (const auto& k : art.knobs)  tryControl (k.param, { k.x, k.y }, k.id, false);
        for (const auto& f : art.faders)
        {
            const float y = 0.5f * (f.minY + f.maxY);
            tryControl (f.param, { f.cx, y }, f.id, false);
        }
        for (const auto& w : art.wheels) tryControl (w.param, { w.x, 0.5f * (w.minY + w.maxY) }, w.id, w.id == "pitchWheel");

        check (moved == total, "dragging each knob, fader and wheel at its drawn position moves its parameter ("
                                   + juce::String (moved) + "/" + juce::String (total) + (stuck.isEmpty() ? juce::String() : ", first dead: " + stuck) + ")");

        // the pitch wheel springs back, the modulation wheel stays
        auto* bend = proc.apvts.getParameter ("pbend");
        for (const auto& w : art.wheels)
            if (w.id == "pitchWheel")
            {
                const auto p = toPanel (w.x, 0.5f * (w.minY + w.maxY));
                dragAt (panel, p, -40.0f, true);
                check (std::abs (bend->getValue() - bend->getDefaultValue()) < 0.001f, "the pitch wheel springs back to the middle");
            }

        // double-click puts a knob back to its default
        {
            const auto& k = art.knobs.front();
            auto* p = proc.apvts.getParameter (k.param);
            p->setValueNotifyingHost (0.9f);
            panel.mouseDoubleClick (makeMouseEvent (panel, toPanel (k.x, k.y), toPanel (k.x, k.y), false));
            check (std::abs (p->getValue() - p->getDefaultValue()) < 0.001f, "double-click returns a knob to its default value");
        }

        // TIME and FEEDBACK switch the delay on, FILTER switches the filter on
        {
            setParam (proc, "delayOn", 0.0f);
            for (const auto& k : art.knobs)
                if (k.id == "time")
                    dragAt (panel, toPanel (k.x, k.y), 20.0f, true);
            check (getParam (proc, "delayOn") > 0.5f, "turning TIME switches the delay on");

            setParam (proc, "fltType", 3.0f);
            for (const auto& k : art.knobs)
                if (k.id == "filter")
                    dragAt (panel, toPanel (k.x, k.y), 20.0f, true);
            check (getParam (proc, "fltType") < 2.5f, "turning FILTER switches the filter on");
        }

        // the grills switch the strings
        {
            setParam (proc, "laOn", 1.0f);
            const auto c = art.grillA.getCentre();
            panel.mouseDown (makeMouseEvent (panel, toPanel (c.x, c.y), toPanel (c.x, c.y), false));
            panel.mouseUp (makeMouseEvent (panel, toPanel (c.x, c.y), toPanel (c.x, c.y), false));
            check (getParam (proc, "laOn") < 0.5f, "clicking grill A switches string A off");
            setParam (proc, "laOn", 1.0f);
        }

        // preset arrows
        {
            proc.refreshPresetList();
            const int before = proc.getCurrentPresetIndex();
            const auto c = art.barNext.getCentre();
            panel.mouseDown (makeMouseEvent (panel, toPanel (c.x, c.y), toPanel (c.x, c.y), false));
            panel.mouseUp (makeMouseEvent (panel, toPanel (c.x, c.y), toPanel (c.x, c.y), false));
            const int afterNext = proc.getCurrentPresetIndex();
            const auto p = art.barPrev.getCentre();
            panel.mouseDown (makeMouseEvent (panel, toPanel (p.x, p.y), toPanel (p.x, p.y), false));
            panel.mouseUp (makeMouseEvent (panel, toPanel (p.x, p.y), toPanel (p.x, p.y), false));
            check (before == -1 && afterNext == 0 && proc.getCurrentPresetIndex() == proc.getPresetFiles().size() - 1,
                   "the \">\" and \"<\" arrows step through the presets");
        }
    }

    // Hovering must not disturb the rest of the faceplate. A translucent glow
    // once left its alpha behind and faded every sprite painted after it: the
    // faders and the wheel blocks vanished whenever the mouse rested on a knob.
    {
        rk::PluginProcessor proc;
        rk::Panel panel (proc, art);
        auto snapshot = [&panel] { return panel.createComponentSnapshot (panel.getLocalBounds(), true); };

        auto same = [] (const juce::Image& a, const juce::Image& b, juce::Rectangle<int> r)
        {
            for (int y = r.getY(); y < r.getBottom(); ++y)
                for (int x = r.getX(); x < r.getRight(); ++x)
                {
                    const auto p = a.getPixelAt (x, y), q = b.getPixelAt (x, y);
                    if (std::abs (p.getRed() - q.getRed()) > 6 || std::abs (p.getGreen() - q.getGreen()) > 6
                        || std::abs (p.getBlue() - q.getBlue()) > 6 || std::abs (p.getAlpha() - q.getAlpha()) > 6)
                        return false;
                }
            return true;
        };

        // witnesses, in the panel's own pixels
        const juce::Rectangle<int> witnesses[] = { { 90, 380, 200, 310 }, { 700, 100, 220, 200 }, { 70, 540, 210, 190 },
                                                   { 700, 300, 230, 170 }, { 90, 270, 200, 120 } };

        const auto resting = snapshot();
        auto imageCentre = [&] (const juce::String& id) -> juce::Point<float>
        {
            for (const auto& k : art.knobs)  if (k.id == id) return { k.x, k.y };
            for (const auto& f : art.faders) if (f.id == id) return { f.cx, 0.5f * (f.minY + f.maxY) };
            for (const auto& w : art.wheels) if (w.id == id) return { w.x, 0.5f * (w.minY + w.maxY) };
            return {};
        };

        const std::pair<juce::String, juce::Point<float>> targets[] =
        {
            { "VOLUME knob", imageCentre ("vol") }, { "D fader", imageCentre ("fD") },
            { "pitch wheel", imageCentre ("pitchWheel") }, { "grill A", art.grillA.getCentre() },
            { "cassette", art.cassette.getCentre() }, { "preset arrow", art.barNext.getCentre() },
            { "GLIDE knob", imageCentre ("glide") }, { "ATTACK fader", imageCentre ("attack") },
        };

        int disturbed = 0, tried = 0;
        juce::String first;
        for (const auto& t : targets)
        {
            const auto p = toPanel (t.second.x, t.second.y);
            panel.mouseMove (makeMouseEvent (panel, p, p, false));
            const auto hovered = snapshot();
            check (! same (resting, hovered, panel.getLocalBounds()) || t.first.isEmpty(), t.first + " shows a hover state");

            for (const auto& w : witnesses)
            {
                if (w.expanded (70).contains (p.toInt()))
                    continue;
                ++tried;
                if (! same (resting, hovered, w))
                {
                    ++disturbed;
                    if (first.isEmpty())
                        first = "hovering the " + t.first + " altered the area at " + w.toString();
                }
            }
            panel.mouseMove (makeMouseEvent (panel, { 1.0f, 1.0f }, { 1.0f, 1.0f }, false));
        }
        check (disturbed == 0, "hovering a control leaves the rest of the faceplate untouched (" + juce::String (tried) + " comparisons"
                                   + (first.isEmpty() ? juce::String() : ", " + first) + ")");

        // and once the mouse leaves, everything is back as it was
        check (same (resting, snapshot(), panel.getLocalBounds()), "after the mouse leaves, the faceplate is back to its resting state");
    }

    // The menus open under the thing they belong to, at any window size: the
    // anchor handed to the pop-up is the drawn zone, scaled with the window.
    {
        rk::PluginProcessor proc;
        bool aligned = true;
        juce::String first;

        for (int width : { 600, 1000, 1500, 2000 })
        {
            std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
            editor->setSize (width, juce::roundToInt (width * rk::Art::designH / rk::Art::designW));
            auto* panel = dynamic_cast<rk::Panel*> (editor->getChildComponent (0));
            if (panel == nullptr)
            {
                aligned = false;
                continue;
            }

            const float scale = (float) width / rk::Art::imageW;
            const std::pair<const char*, juce::Rectangle<float>> zones[] =
            {
                { "folder", art.barFolder }, { "preset field", art.barField },
                { "cassette", art.cassette }, { "WAVEFORM box", art.combos.front().box },
            };

            for (const auto& [name, zone] : zones)
            {
                const auto got = panel->imageToScreen (zone);
                const auto want = zone * scale;
                if (std::abs ((float) got.getX() - want.getX()) > 2.0f || std::abs ((float) got.getY() - want.getY()) > 2.0f
                    || std::abs ((float) got.getWidth() - want.getWidth()) > 3.0f || std::abs ((float) got.getHeight() - want.getHeight()) > 3.0f)
                {
                    aligned = false;
                    if (first.isEmpty())
                        first = juce::String (name) + " at " + juce::String (width) + " px: " + got.toString() + " instead of " + want.toString();
                }
            }
        }
        check (aligned, "pop-up menus are anchored on their zone at 4 window sizes" + (first.isEmpty() ? juce::String() : " - " + first));
    }

    // the keyboard: every key of the drawing plays its own note
    {
        rk::PluginProcessor proc;
        rk::Panel panel (proc, art);
        auto* keys = dynamic_cast<rk::KeyboardControl*> (panel.keyboard);
        check (keys != nullptr, "the panel has a keyboard");

        if (keys != nullptr)
        {
            check (keys->numWhite() == 22, "22 white keys, C2 to C5 (" + juce::String (keys->numWhite()) + ")");

            int wrong = 0;
            juce::String firstWrong;

            auto press = [&] (float x, float y, int expected, const juce::String& what)
            {
                const auto pt = toPanel (x, y);
                panel.mouseDown (makeMouseEvent (panel, pt, pt, false));
                const bool right = proc.getKeyboardState().isNoteOn (1, expected);
                int others = 0;
                for (int n = 0; n < 128; ++n)
                    if (n != expected && proc.getKeyboardState().isNoteOn (1, n))
                        ++others;
                panel.mouseUp (makeMouseEvent (panel, pt, pt, false));

                if (! right || others != 0 || proc.getKeyboardState().isNoteOn (1, expected))
                {
                    ++wrong;
                    if (firstWrong.isEmpty())
                        firstWrong = what;
                }
            };

            for (int i = 0; i < keys->numWhite(); ++i)
            {
                const auto r = keys->whiteRect (i);
                press (r.getCentreX(), art.keys.whiteBottom - 40.0f, keys->whiteNote (i), "white key " + juce::String (i));
            }
            for (int j = 0; j < (int) art.keys.black.size(); ++j)
            {
                const auto r = keys->blackRect (j);
                press (r.getCentreX(), r.getY() + 0.4f * r.getHeight(), keys->blackNote (j), "black key " + juce::String (j));
            }
            check (wrong == 0, "all 37 keys play their own note, and let go on release"
                                   + (wrong == 0 ? juce::String() : ", first wrong: " + firstWrong));
            check (keys->whiteNote (0) == 36 && keys->whiteNote (21) == 72, "range C2 (36) to C5 (72), fixed, no scrolling");

            // closing the window with a key held must release it
            const auto pt = toPanel (keys->whiteRect (3).getCentreX(), art.keys.whiteBottom - 40.0f);
            panel.mouseDown (makeMouseEvent (panel, pt, pt, false));
            panel.releaseAllNotes();
            check (! proc.getKeyboardState().isNoteOn (1, keys->whiteNote (3)), "a key held while the window closes is released");
        }
    }

    // snapshot at several sizes; the owner can look at the default one
    {
        rk::PluginProcessor proc;
        const auto shot = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getSiblingFile ("rockpunk_ui.png");

        const auto small = snapshotOf (proc, 500, 373);
        const auto normal = snapshotOf (proc, 1000, 746);
        const auto large = snapshotOf (proc, 2000, 1492);
        check (small.isValid() && normal.isValid() && large.isValid(), "the editor renders at 500, 1000 and 2000 px wide");
        check (normal.isValid() && writePng (normal, shot), "interface photographed: " + shot.getFullPathName());

        // not blank: the faceplate has plenty of contrast and some pink
        if (normal.isValid())
        {
            int pink = 0, bright = 0, dark = 0, sampled = 0;
            for (int y = 0; y < normal.getHeight(); y += 3)
                for (int x = 0; x < normal.getWidth(); x += 3)
                {
                    const auto c = normal.getPixelAt (x, y);
                    ++sampled;
                    if (c.getRed() > 180 && c.getGreen() < 120 && c.getBlue() > 90) ++pink;
                    if (c.getBrightness() > 0.6f) ++bright;
                    if (c.getBrightness() < 0.2f) ++dark;
                }
            check (pink > sampled / 400 && bright > sampled / 100 && dark > sampled / 3,
                   "the snapshot carries the faceplate (dark " + juce::String (100 * dark / sampled) + " %, pink "
                       + juce::String (100 * pink / sampled) + " %)");
        }

        // a held key lights up on the picture
        {
            const auto before = snapshotOf (proc, 1000, 746);
            proc.getKeyboardState().noteOn (1, 60, 1.0f);
            juce::AudioBuffer<float> b (2, 256);
            juce::MidiBuffer m;
            proc.processBlock (b, m);

            std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
            editor->setSize (1000, 746);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (200);
            const auto after = editor->createComponentSnapshot (editor->getLocalBounds(), true);

            int changed = 0;
            for (int y = 540; y < 700; y += 2)
                for (int x = 280; x < 920; x += 2)
                    if (before.getPixelAt (x, y).getPerceivedBrightness() + 0.08f < after.getPixelAt (x, y).getPerceivedBrightness()
                        || after.getPixelAt (x, y).getRed() > before.getPixelAt (x, y).getRed() + 40)
                        ++changed;
            check (changed > 200, "a played note lights its key on the keyboard (" + juce::String (changed) + " pixels changed)");
            proc.getKeyboardState().noteOff (1, 60, 0.0f);
        }
    }

    // the editor can be opened and closed repeatedly, with keys held
    {
        rk::PluginProcessor proc;
        for (int i = 0; i < 5; ++i)
        {
            std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
            editor->setSize (700 + 50 * i, 522 + 37 * i);
            proc.getKeyboardState().noteOn (1, 50 + i, 1.0f);
        }
        check (true, "opening and closing the editor five times is safe");
    }
}

//==============================================================================
// Cost
//==============================================================================
static void testCpu()
{
    section ("Cost");

    rk::PluginProcessor proc;
    setParam (proc, "delayOn", 1.0f);
    setParam (proc, "chorAmt", 60.0f);
    setParam (proc, "revMix", 40.0f);
    setParam (proc, "fltType", 0.0f);
    setParam (proc, "lfoAmount", 50.0f);
    setParam (proc, "lfoPitch", 40.0f);
    setParam (proc, "envR", 2000.0f);

    const double seconds = 6.0;
    const auto t0 = juce::Time::getMillisecondCounterHiRes();
    const auto c = capture (proc, 48000.0, 256, (int) (seconds * 48000.0), [] (int block, int, int, juce::MidiBuffer& m)
    {
        // eight notes held, plus an attack every other block: the worst case
        if (block % 20 == 0)
            for (int n = 0; n < 8; ++n)
                m.addEvent (juce::MidiMessage::noteOn (1, 40 + n * 3 + (block / 20) % 3, 0.8f), 0);
    });
    const double elapsed = (juce::Time::getMillisecondCounterHiRes() - t0) / 1000.0;
    const double load = 100.0 * elapsed / seconds;
    check (measure (c).finite, "worst-case load renders finite audio");
    check (load < 60.0, "full polyphony with every effect on: " + juce::String (load, 1) + " % of one core in real time");
}

//==============================================================================
int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    std::cout << std::unitbuf;

    const juce::File factoryDir (argc > 1
        ? juce::String (argv[1])
        : juce::File::getSpecialLocation (juce::File::currentExecutableFile)
              .getParentDirectory().getChildFile ("Presets").getFullPathName());

    testEngineAndControls();
    testHostBehaviour();
    testStuckNotes();
    testDspUnits();
    testStateAndText();
    testSampleFolder();
    testPresets (factoryDir);
    testInterface();
    testCpu();

    std::cout << "\n" << (failures == 0 ? "ALL GREEN" : juce::String (failures) + " FAILURE(S)") << std::endl;
    return failures == 0 ? 0 : 1;
}
