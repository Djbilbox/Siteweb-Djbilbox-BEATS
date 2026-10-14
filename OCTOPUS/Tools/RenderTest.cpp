// Suite de validation hors-ligne OCTOPUS (voir skill dsp-quality-validation).
//  1) rendu des 70 presets en WAV (accord C3)           2) matrice frequence d'echantillonnage x taille de bloc
//  3) silence, notes rapides, polyphonie max/vol de voix  4) panic (CC123/CC120), pedale sustain, tail
//  5) changement de preset en cours de lecture            6) rappel d'etat (save/restore)
#include "PluginProcessor.h"
#include <cstdio>

namespace
{
int failures = 0;
void check (bool ok, const juce::String& what)
{
    std::printf ("  [%s] %s\n", ok ? "ok" : "FAIL", what.toRawUTF8());
    if (! ok) ++failures;
}

struct Stats { bool finite = true; float peak = 0.f; double rms = 0.0, dc = 0.0; float maxStep = 0.f; double tailRms = 0.0; };

using Script = std::function<void (int blockStartSample, int blockSize, juce::MidiBuffer&)>;

// Rend 'seconds' secondes ; tailSeconds = fenetre finale pour mesurer la queue.
Stats render (OcProcessor& p, double sr, int block, double seconds, double tailSeconds, const Script& script,
              juce::AudioBuffer<float>* keep = nullptr)
{
    p.setPlayConfigDetails (0, 2, sr, block);
    p.prepareToPlay (sr, block);
    const int total = (int) (sr * seconds), tailStart = total - (int) (sr * tailSeconds);
    if (keep) keep->setSize (2, total);
    Stats s; double sq = 0, sum = 0, tsq = 0; int tn = 0; float prev = 0.f;
    for (int pos = 0; pos < total; pos += block)
    {
        juce::MidiBuffer m; juce::AudioBuffer<float> b (2, block);
        if (script) script (pos, block, m);
        p.processBlock (b, m);
        const int cnt = juce::jmin (block, total - pos);
        if (keep) for (int c = 0; c < 2; ++c) keep->copyFrom (c, pos, b, c, 0, cnt);
        for (int i = 0; i < cnt; ++i)
        {
            const float v = b.getSample (0, i);
            if (! std::isfinite (v)) { s.finite = false; continue; }
            s.peak = juce::jmax (s.peak, std::abs (v)); sq += v * v; sum += v;
            if (pos + i > 0) s.maxStep = juce::jmax (s.maxStep, std::abs (v - prev));
            prev = v;
            if (pos + i >= tailStart) { tsq += v * v; ++tn; }
        }
    }
    s.rms = std::sqrt (sq / total); s.dc = sum / total; s.tailRms = tn ? std::sqrt (tsq / tn) : 0.0;
    return s;
}

Script chordThenRelease (double sr, double offAt, std::initializer_list<int> notes)
{
    const std::vector<int> nn (notes);
    return [=] (int pos, int block, juce::MidiBuffer& m)
    {
        if (pos == 0) for (int n : nn) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.8f), 0);
        const int off = (int) (sr * offAt);
        if (pos <= off && off < pos + block) for (int n : nn) m.addEvent (juce::MidiMessage::noteOff (1, n), off - pos);
    };
}
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    OcProcessor proc;
    auto outDir = juce::File::getCurrentWorkingDirectory().getChildFile ("renders"); outDir.createDirectory();
    juce::WavAudioFormat wav;

    std::printf ("== 1. 70 presets @44.1k/512 ==\n");
    std::vector<double> rmsList; std::vector<float> peakList;
    for (int p = 0; p < proc.getNumPrograms(); ++p)
    {
        proc.applyPreset (p);
        juce::AudioBuffer<float> keep;
        auto s = render (proc, 44100.0, 512, 4.0, 1.0, chordThenRelease (44100.0, 2.0, { 48, 55, 60 }), &keep);
        rmsList.push_back (s.rms); peakList.push_back (s.peak);
        std::printf ("  %2d %-22s rms=%.3f peak=%.3f\n", p, proc.getProgramName (p).toRawUTF8(), s.rms, s.peak);
        const bool ok = s.finite && s.peak > 0.02f && s.peak <= 1.0f && std::abs (s.dc) < 0.02;
        if (! ok) { ++failures; std::printf ("  [FAIL] %2d %-20s finite=%d peak=%.3f dc=%.4f\n", p, proc.getProgramName (p).toRawUTF8(), (int) s.finite, s.peak, s.dc); }
        auto f = outDir.getChildFile (juce::String (p + 1).paddedLeft ('0', 2) + "_" + proc.getProgramName (p).replaceCharacters (" &", "__") + ".wav");
        if (auto* os = new juce::FileOutputStream (f)) { os->setPosition (0); os->truncate(); std::unique_ptr<juce::AudioFormatWriter> w (wav.createWriterFor (os, 44100.0, 2, 16, {}, 0)); if (w) w->writeFromAudioSampleBuffer (keep, 0, keep.getNumSamples()); }
    }
    check (failures == 0, "70 presets : finis, non vides, pic <= 1, DC < 0.02");
    { auto r = rmsList; std::sort (r.begin(), r.end()); const float pk = *std::max_element (peakList.begin(), peakList.end());
      std::printf ("  rms median %.3f (min %.3f, max %.3f), pic max %.3f\n", r[r.size() / 2], r.front(), r.back(), pk); }

    std::printf ("== 2. matrice SR x bloc (1 preset par moteur) ==\n");
    const int reps[] = { 0, 10, 20, 30, 40, 50, 60, 65 };
    int matrixFail = 0;
    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0 })
        for (int block : { 32, 480, 2048 })
            for (int p : reps)
            {
                proc.applyPreset (p);
                auto s = render (proc, sr, block, 3.0, 0.3, chordThenRelease (sr, 1.0, { 36, 60, 84 }));
                if (! (s.finite && s.peak > 0.01f && s.peak <= 1.0f && std::abs (s.dc) < 0.02))
                { ++matrixFail; std::printf ("  [FAIL] sr=%.0f block=%d preset=%d finite=%d peak=%.3f dc=%.4f\n", sr, block, p, (int) s.finite, s.peak, s.dc); }
            }
    check (matrixFail == 0, "matrice 4 SR x 3 blocs x 8 presets");
    failures += matrixFail ? 0 : 0;

    std::printf ("== 3. silence / notes rapides / polyphonie max ==\n");
    proc.applyPreset (0);
    auto sil = render (proc, 48000.0, 256, 1.0, 0.5, {});
    check (sil.finite && sil.peak < 1e-6f, "silence en entree -> silence en sortie");
    auto rapid = render (proc, 48000.0, 256, 3.0, 0.2, [] (int pos, int block, juce::MidiBuffer& m)
    { for (int i = 0; i < block; i += 64) { const int n = 40 + ((pos + i) / 64) % 40; m.addEvent (juce::MidiMessage::noteOn (1, n, 0.9f), i); m.addEvent (juce::MidiMessage::noteOff (1, n), juce::jmin (block - 1, i + 32)); } });
    check (rapid.finite && rapid.peak <= 1.0f, juce::String::formatted ("notes tres rapides finies, pic %.3f", rapid.peak));
    proc.applyPreset (10);
    auto poly = render (proc, 48000.0, 512, 3.0, 0.2, [] (int pos, int, juce::MidiBuffer& m)
    { if (pos == 0) for (int n = 36; n < 76; ++n) m.addEvent (juce::MidiMessage::noteOn (1, n, 1.0f), 0); });
    check (poly.finite && poly.peak <= 1.0f, juce::String::formatted ("40 notes simultanees (vol de voix), pic %.3f", poly.peak));
    juce::MidiBuffer garbage;
    for (int i = 0; i < 500; ++i) garbage.addEvent (juce::MidiMessage::controllerEvent (1, i % 128, (i * 7) % 128), i % 64);
    for (int i = 0; i < 100; ++i) garbage.addEvent (juce::MidiMessage::pitchWheel (1, (i * 997) % 16384), i % 64);
    { juce::AudioBuffer<float> b (2, 64); proc.processBlock (b, garbage); check (std::isfinite (b.getSample (0, 5)), "flot MIDI aberrant (CC/pitchwheel) sans NaN"); }

    std::printf ("== 4. panic, sustain, queue ==\n");
    proc.applyPreset (33); // pad long, release 3 s, reverb
    auto panic = render (proc, 48000.0, 256, 4.0, 0.5, [] (int pos, int, juce::MidiBuffer& m)
    { if (pos == 0) for (int n : { 48, 55, 60 }) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.8f), 0);
      if (pos >= 48000 && pos < 48256) { m.addEvent (juce::MidiMessage::allSoundOff (1), 0); } });
    check (panic.finite && panic.tailRms < 0.05, juce::String::formatted ("panic (all sound off) : sortie reste finie, queue rms %.4f", panic.tailRms));
    proc.applyPreset (0);
    auto sus = render (proc, 48000.0, 256, 3.0, 0.3, [] (int pos, int block, juce::MidiBuffer& m)
    { if (pos == 0) { m.addEvent (juce::MidiMessage::controllerEvent (1, 64, 127), 0); m.addEvent (juce::MidiMessage::noteOn (1, 60, 0.8f), 1); m.addEvent (juce::MidiMessage::noteOff (1, 60), 100); }
      if (pos >= 96000 && pos < 96000 + block) m.addEvent (juce::MidiMessage::controllerEvent (1, 64, 0), 0); });
    check (sus.finite && sus.rms > 0.0, "pedale sustain : note tenue puis relachee");
    int tailFail = 0;
    for (int p = 0; p < proc.getNumPrograms(); ++p)
    {
        proc.applyPreset (p);
        auto s = render (proc, 44100.0, 512, 24.0, 1.0, chordThenRelease (44100.0, 1.5, { 48, 55, 60 }));
        if (! (s.finite && s.tailRms < 0.002)) { ++tailFail; std::printf ("  [FAIL] queue non terminee preset %d (%s) tailRms=%.5f\n", p, proc.getProgramName (p).toRawUTF8(), s.tailRms); }
    }
    check (tailFail == 0, "queues terminees (<-54 dB) 22 s apres note-off, 70 presets");

    std::printf ("== 5. changement de preset en lecture ==\n");
    int swapFail = 0; float worstStep = 0.f;
    for (int p = 0; p < proc.getNumPrograms(); p += 7)
    {
        proc.applyPreset (p);
        auto s = render (proc, 48000.0, 256, 3.0, 0.2, [&] (int pos, int block, juce::MidiBuffer& m)
        { if (pos == 0) m.addEvent (juce::MidiMessage::noteOn (1, 52, 0.8f), 0);
          if (pos >= 48000 && pos < 48000 + block) proc.applyPreset ((p + 23) % proc.getNumPrograms()); });
        worstStep = juce::jmax (worstStep, s.maxStep);
        if (! s.finite || s.peak > 1.0f) ++swapFail;
    }
    check (swapFail == 0, juce::String::formatted ("changement de preset en lecture sans NaN/depassement (plus grand saut echantillon a echantillon: %.2f)", worstStep));

    std::printf ("== 6. rappel d'etat ==\n");
    proc.applyPreset (12);
    proc.apvts.getParameter ("filter")->setValueNotifyingHost (0.37f);
    juce::MemoryBlock mb; proc.getStateInformation (mb);
    OcProcessor other;
    other.setStateInformation (mb.getData(), (int) mb.getSize());
    bool same = true;
    for (int i = 0; i < Oc::COUNT; ++i) same = same && std::abs (other.apvts.getParameter (Oc::ids[i])->getValue() - proc.apvts.getParameter (Oc::ids[i])->getValue()) < 1e-5f;
    check (same && other.presetIndex == 12, "tous les parametres + index de preset restaures");
    other.setStateInformation ("garbage", 7);
    check (other.apvts.getParameter ("filter")->getValue() == proc.apvts.getParameter ("filter")->getValue(), "etat invalide ignore sans corrompre");
    check (proc.getNumPrograms() == 70, "70 programmes");

    std::printf ("\n%s : %d echec(s)\n", failures == 0 ? "TOUT OK" : "ECHEC", failures);
    return failures == 0 ? 0 : 1;
}
