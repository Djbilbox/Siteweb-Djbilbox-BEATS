// Films the DZPOWER interface while it plays: renders the audio in real time steps, advances the animation and saves
// one PNG per frame (frame_0000.png ...) plus the sound (audio.wav). Assemble them with ffmpeg to watch the lights move.
// usage: DzUiFrames <out dir> [fps=30] [seconds=7] [preset index]
#include "PluginProcessor.h"
#include "PluginEditor.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File out (juce::File::getCurrentWorkingDirectory().getChildFile (argc > 1 ? argv[1] : "frames"));
    const int fps = argc > 2 ? juce::String (argv[2]).getIntValue() : 30;
    const float seconds = argc > 3 ? juce::String (argv[3]).getFloatValue() : 7.f;
    out.createDirectory();

    const double sr = 48000.0;
    const int block = (int) (sr / fps);
    DzProcessor proc;
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);
    if (argc > 4) proc.applyPreset (juce::String (argv[4]).getIntValue());

    DzPanel panel (proc);
    panel.setBounds (0, 0, 1000, 746);

    // the little performance: a chord held, then a fast riff, then a last held note
    struct Ev { float t; int note; bool on; };
    std::vector<Ev> evs { { 1.6f, 48, true }, { 1.6f, 55, true }, { 1.6f, 60, true }, { 3.0f, 48, false }, { 3.0f, 55, false }, { 3.0f, 60, false } };
    const int riff[] = { 60, 63, 65, 67, 70, 67, 65, 63 };
    for (int i = 0; i < 8; ++i) { evs.push_back ({ 3.3f + 0.16f * (float) i, riff[i], true }); evs.push_back ({ 3.3f + 0.16f * (float) i + 0.12f, riff[i], false }); }
    evs.push_back ({ 4.8f, 72, true }); evs.push_back ({ 6.0f, 72, false });

    if (argc > 2 && juce::String (argv[1]) == "--still")   // one quiet frame at 2x: DzUiFrames --still out.png
    {
        for (int i = 0; i < 120; ++i) panel.advance (1.f / 60.f);
        auto img = panel.createComponentSnapshot (panel.getLocalBounds(), true, 2.0f);
        juce::FileOutputStream os (juce::File::getCurrentWorkingDirectory().getChildFile (argv[2]));
        os.setPosition (0); os.truncate();
        juce::PNGImageFormat().writeImageToStream (img, os);
        return 0;
    }
    if (argc > 1 && juce::String (argv[1]) == "--bench")   // milliseconds to repaint each animated area (idle)
    {
        for (int i = 0; i < 90; ++i) panel.advance (1.f / 60.f);
        juce::Image img (juce::Image::RGB, 1000, 746, true);
        auto time = [&] (const char* name, juce::Rectangle<int> area)
        {
            const double t0 = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < 40; ++i)
            {
                juce::Graphics g (img);
                g.reduceClipRegion (area);
                panel.paintEntireComponent (g, false);
            }
            std::printf ("  %-10s %4dx%-4d %.2f ms\n", name, area.getWidth(), area.getHeight(), (juce::Time::getMillisecondCounterHiRes() - t0) / 40.0);
        };
        time ("full", panel.getLocalBounds());
        time ("title", { 306, 268, 388, 154 });
        time ("medallion", { 368, 77, 266, 266 });
        time ("rate", { 85, 279, 86, 86 });
        time ("runner", { 300, 40, 36, 36 });
        time ("spark", { 500, 500, 14, 14 });
        time ("keys", { 291, 553, 616, 154 });
        {   // fixed cost: the panel's own paint() on a 1-pixel area, then the whole component tree on it
            for (int pass = 0; pass < 2; ++pass)
            {
                const double t0 = juce::Time::getMillisecondCounterHiRes();
                for (int i = 0; i < 40; ++i)
                {
                    juce::Graphics g (img);
                    g.reduceClipRegion (juce::Rectangle<int> (2, 740, 1, 1));
                    if (pass == 0) panel.paint (g); else panel.paintEntireComponent (g, false);
                }
                std::printf ("  1 pixel, %s: %.2f ms\n", pass == 0 ? "paint()" : "tree", (juce::Time::getMillisecondCounterHiRes() - t0) / 40.0);
            }
            const double t0 = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < 40; ++i) { juce::Graphics g (img); g.reduceClipRegion (juce::Rectangle<int> (2, 740, 1, 1)); }
            std::printf ("  1 pixel, context only: %.2f ms\n", (juce::Time::getMillisecondCounterHiRes() - t0) / 40.0);
        }
        {   // one quiet frame: the areas a quiet frame repaints, painted together
            juce::RectangleList<int> idle;
            for (auto r : { juce::Rectangle<int> (306, 268, 388, 154), { 368, 77, 266, 266 }, { 85, 279, 86, 86 }, { 172, 279, 86, 86 },
                            { 300, 40, 36, 36 }, { 289, 531, 620, 46 } })
                idle.add (r);
            const double t0 = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < 40; ++i)
            {
                juce::Graphics g (img);
                g.reduceClipRegion (idle);
                panel.paintEntireComponent (g, false);
            }
            std::printf ("  quiet frame %.2f ms\n", (juce::Time::getMillisecondCounterHiRes() - t0) / 40.0);
            const double t1 = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < 200; ++i) panel.advance (1.f / 45.f);
            std::printf ("  advance() %.3f ms (includes the repaint requests)\n", (juce::Time::getMillisecondCounterHiRes() - t1) / 200.0);
        }
        return 0;
    }

    juce::AudioBuffer<float> buf (2, block);
    juce::MidiBuffer midi;
    juce::PNGImageFormat png;
    const int frames = (int) (seconds * (float) fps);
    juce::AudioBuffer<float> audio (2, frames * block);
    for (int f = 0; f < frames; ++f)
    {
        const float t0 = (float) f / (float) fps, t1 = (float) (f + 1) / (float) fps;
        for (auto& e : evs)
            if (e.t >= t0 && e.t < t1)
            {
                if (e.on) proc.keyboardState.noteOn (1, e.note, 0.9f);
                else proc.keyboardState.noteOff (1, e.note, 0.f);
            }
        midi.clear();
        proc.processBlock (buf, midi);
        for (int ch = 0; ch < 2; ++ch) audio.copyFrom (ch, f * block, buf, ch, 0, block);
        panel.advance (1.f / (float) fps);
        auto img = panel.createComponentSnapshot (panel.getLocalBounds(), true, 1.0f);
        juce::FileOutputStream os (out.getChildFile ("frame_" + juce::String (f).paddedLeft ('0', 4) + ".png"));
        os.setPosition (0); os.truncate();
        png.writeImageToStream (img, os);
    }
    juce::WavAudioFormat wav;
    if (auto* os = new juce::FileOutputStream (out.getChildFile ("audio.wav")))
    {
        os->setPosition (0); os->truncate();
        std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (os, sr, 2, 24, {}, 0));
        if (w != nullptr) w->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples());
    }
    std::printf ("%d frames -> %s\n", frames, out.getFullPathName().toRawUTF8());
    return 0;
}
