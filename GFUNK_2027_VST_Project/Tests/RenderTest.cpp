// Offline sanity test: renders every factory preset with a short legato phrase and checks
// for NaNs, silence, clipping and CPU load. Optionally writes WAVs: RenderTest <outDir> <idx...>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <chrono>
#include <cstdio>

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    if (argc > 2 && juce::String (argv[1]) == "snap")
    {
        GFunkProcessor proc;
        proc.setRateAndBufferSizeDetails (44100.0, 512);
        proc.prepareToPlay (44100.0, 512);
        proc.setCurrentProgram (argc > 3 ? std::atoi (argv[3]) : 0);
        proc.skin.store (argc > 4 ? std::atoi (argv[4]) : 0);
        for (int b = 0; b < 40; ++b)
        {
            juce::MidiBuffer m;
            if (b == 0) { for (int n : { 48, 55, 60, 64 }) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.95f), 0); }
            if (b == 12) { m.addEvent (juce::MidiMessage::noteOn (1, 67, 0.9f), 0); m.addEvent (juce::MidiMessage::noteOn (1, 72, 1.0f), 100); }
            juce::AudioBuffer<float> buf (2, 512);
            proc.processBlock (buf, m);
        }
        std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
        juce::MessageManager::getInstance()->runDispatchLoopUntil (argc > 5 ? std::atoi (argv[5]) : 5800);
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        juce::File f (argv[2]);
        f.deleteFile();
        juce::FileOutputStream os (f);
        juce::PNGImageFormat png;
        png.writeImageToStream (img, os);
        std::printf ("snapshot %dx%d\n", img.getWidth(), img.getHeight());
        for (int level : { 1, 0 })
        {
            proc.blingLevel.store (level);
            const auto t0 = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < 15; ++i)
                ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
            std::printf ("full paint, bling=%d: %.1f ms/frame\n", level, (juce::Time::getMillisecondCounterHiRes() - t0) / 15.0);
        }
        ed.reset();
        return 0;
    }


    // "hot" mode: switch presets on a live processor (no prepareToPlay), like a DAW does,
    // and check that a freshly played chord is audible and finite after every switch.
    if (argc > 1 && juce::String (argv[1]) == "hot")
    {
        GFunkProcessor hp;
        hp.setRateAndBufferSizeDetails (44100.0, 512);
        hp.prepareToPlay (44100.0, 512);
        juce::Random rr (7);
        int dead = 0, bad = 0;
        for (int k = 0; k < 3000; ++k)
        {
            const int idx = k < 1000 ? k : rr.nextInt (GFunkSpec::numPresets);
            // preset switched while a chord is still being held (note-off arrives after the switch)
            if (k % 2 == 0)
            {
                juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, 52, 0.9f), 0);
                juce::AudioBuffer<float> b (2, 512); hp.processBlock (b, m);
            }
            hp.setCurrentProgram (idx);
            float peak = 0.0f; bool nonFinite = false;
            for (int blk = 0; blk < 60; ++blk)
            {
                juce::MidiBuffer m;
                if (blk == 0)  { m.addEvent (juce::MidiMessage::noteOff (1, 52), 0);
                                 m.addEvent (juce::MidiMessage::noteOn (1, 48, 0.9f), 1);
                                 m.addEvent (juce::MidiMessage::noteOn (1, 60, 0.9f), 1); }
                if (blk == 40) { m.addEvent (juce::MidiMessage::noteOff (1, 48), 0);
                                 m.addEvent (juce::MidiMessage::noteOff (1, 60), 0); }
                juce::AudioBuffer<float> b (2, 512); b.clear();
                hp.processBlock (b, m);
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < 512; ++i)
                    {
                        const float v = b.getSample (c, i);
                        if (! std::isfinite (v)) nonFinite = true;
                        else if (blk < 40) peak = juce::jmax (peak, std::fabs (v));
                    }
            }
            if (nonFinite) { ++bad; std::printf ("NaN/inf after switch to #%d (step %d)\n", idx, k); }
            if (peak < 1.0e-3f) { ++dead; if (dead < 20) std::printf ("SILENT after switch to #%d (step %d) %s\n", idx, k, GFunkSpec::presetInfo[idx].name); }
        }
        std::printf ("hot switch: nonfinite=%d silent=%d / 3000\n", bad, dead);
        return bad + dead;
    }

    constexpr double sr = 44100.0;
    constexpr int block = 512;

    GFunkProcessor proc;
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);

    juce::File outDir;
    std::vector<int> wanted;
    if (argc > 2)
    {
        outDir = juce::File (argv[1]);
        outDir.createDirectory();
        for (int i = 2; i < argc; ++i) wanted.push_back (std::atoi (argv[i]));
    }

    int nan = 0, silent = 0, loud = 0, quiet = 0;
    double totalSec = 0, totalAudio = 0;
    float minRms = 1e9f, maxRms = 0.0f;
    std::vector<float> peaks;
    juce::String csv;

    for (int idx = 0; idx < GFunkSpec::numPresets; ++idx)
    {
        proc.setCurrentProgram (idx);
        proc.prepareToPlay (sr, block);

        const int total = (int) (sr * 2.0);
        juce::AudioBuffer<float> out (2, total);
        out.clear();

        auto t0 = std::chrono::steady_clock::now();
        for (int pos = 0; pos < total; pos += block)
        {
            juce::MidiBuffer midi;
            auto at = [&] (double sec) { return (int) (sec * sr) - pos; };
            auto add = [&] (juce::MidiMessage m, double sec)
            {
                const int s = at (sec);
                if (s >= 0 && s < block) midi.addEvent (m, s);
            };
            add (juce::MidiMessage::noteOn (1, 48, 0.9f), 0.0);
            add (juce::MidiMessage::noteOn (1, 55, 0.8f), 0.0);
            add (juce::MidiMessage::noteOn (1, 60, 0.9f), 0.0);
            add (juce::MidiMessage::noteOn (1, 64, 0.8f), 0.4);
            add (juce::MidiMessage::noteOff (1, 48), 0.9);
            add (juce::MidiMessage::noteOff (1, 55), 0.9);
            add (juce::MidiMessage::noteOff (1, 60), 0.9);
            add (juce::MidiMessage::noteOff (1, 64), 1.0);

            const int n = juce::jmin (block, total - pos);
            juce::AudioBuffer<float> b (2, block);
            b.clear();
            proc.processBlock (b, midi);
            for (int c = 0; c < 2; ++c) out.copyFrom (c, pos, b, c, 0, n);
        }
        totalSec += std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
        totalAudio += 2.0;

        float peak = 0.0f; double sum = 0.0; bool bad = false;
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < total; ++i)
            {
                const float v = out.getSample (c, i);
                if (! std::isfinite (v)) bad = true;
                peak = juce::jmax (peak, std::fabs (v));
                if (i < (int) sr) sum += (double) v * v;
            }
        const float rms = (float) std::sqrt (sum / (2.0 * sr));
        if (bad) ++nan;
        if (peak < 1.0e-3f) ++silent;
        if (peak > 1.05f) ++loud;
        if (rms < 0.02f) { ++quiet; if (quiet < 15) std::printf ("quiet: #%d %s rms=%.4f\n", idx, GFunkSpec::presetInfo[idx].name, rms); }
        minRms = juce::jmin (minRms, rms);
        maxRms = juce::jmax (maxRms, rms);
        peaks.push_back (peak);
        csv << idx << "," << rms << "," << peak << "\n";

        if (std::find (wanted.begin(), wanted.end(), idx) != wanted.end())
        {
            juce::WavAudioFormat wav;
            auto f = outDir.getChildFile (juce::String (idx) + ".wav");
            f.deleteFile();
            std::unique_ptr<juce::FileOutputStream> os (f.createOutputStream());
            if (os)
                if (std::unique_ptr<juce::AudioFormatWriter> w { wav.createWriterFor (os.get(), sr, 2, 16, {}, 0) })
                {
                    os.release();
                    w->writeFromAudioSampleBuffer (out, 0, total);
                }
        }
    }

    if (outDir != juce::File()) outDir.getChildFile ("rms.csv").replaceWithText (csv);
    std::sort (peaks.begin(), peaks.end());
    std::printf ("presets=%d nan=%d silent=%d clipped(>1.05)=%d quiet(rms<0.02)=%d\n",
                 GFunkSpec::numPresets, nan, silent, loud, quiet);
    std::printf ("rms min=%.4f max=%.4f  peak median=%.3f max=%.3f\n", minRms, maxRms, peaks[peaks.size() / 2], peaks.back());
    std::printf ("CPU: %.1fx realtime (all presets)\n", totalAudio / totalSec);
    return nan + silent;
}
