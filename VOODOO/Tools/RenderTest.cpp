// Rend chaque preset (note C3 + accord) en WAV et verifie NaN / niveaux.
#include "PluginProcessor.h"
#include <cstdio>

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    VoodooProcessor proc;
    const double sr = 44100.0; const int block = 512;
    proc.setPlayConfigDetails (0, 2, sr, block);
    proc.prepareToPlay (sr, block);
    auto outDir = juce::File::getCurrentWorkingDirectory().getChildFile ("renders");
    outDir.createDirectory();
    juce::WavAudioFormat wav;
    int bad = 0;
    for (int p = 0; p < proc.getNumPrograms(); ++p)
    {
        proc.applyPreset (p);
        proc.prepareToPlay (sr, block);
        const int total = (int) (sr * 4.0);
        juce::AudioBuffer<float> all (2, total);
        int pos = 0; bool nan = false; float peak = 0.f; double sumSq = 0; 
        while (pos < total)
        {
            juce::MidiBuffer m;
            if (pos == 0) { for (int n : { 48, 55, 60 }) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.8f), 0); }
            if (pos >= (int) (sr * 2.0) && pos < (int) (sr * 2.0) + block) { for (int n : { 48, 55, 60 }) m.addEvent (juce::MidiMessage::noteOff (1, n), 0); }
            juce::AudioBuffer<float> b (2, block);
            proc.processBlock (b, m);
            const int cnt = juce::jmin (block, total - pos);
            for (int c = 0; c < 2; ++c) all.copyFrom (c, pos, b, c, 0, cnt);
            for (int i = 0; i < cnt; ++i) { const float v = b.getSample (0, i); if (! std::isfinite (v)) nan = true; peak = juce::jmax (peak, std::abs (v)); sumSq += v * v; }
            pos += cnt;
        }
        const float rms = (float) std::sqrt (sumSq / total);
        const bool ok = ! nan && peak > 0.02f && peak <= 1.0f;
        if (! ok) ++bad;
        std::printf ("%2d %-22s peak=%.3f rms=%.4f %s\n", p, proc.getProgramName (p).toRawUTF8(), peak, rms, nan ? "NaN!" : ok ? "ok" : "CHECK");
        auto f = outDir.getChildFile (juce::String (p + 1).paddedLeft ('0', 2) + "_" + proc.getProgramName (p).replaceCharacters (" &", "__") + ".wav");
        if (auto* os = new juce::FileOutputStream (f)) { os->setPosition (0); os->truncate(); std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (os, sr, 2, 16, {}, 0)); if (w) w->writeFromAudioSampleBuffer (all, 0, total); }
    }
    std::printf ("presets a verifier: %d\n", bad);
    return bad == 0 ? 0 : 1;
}
