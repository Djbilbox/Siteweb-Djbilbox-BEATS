// Test DSP console de HUMPIRE SOLAR : verifie la correction de hauteur hors-ligne.
//   cmake --build build --target SolarTest && build/SolarTest_artefacts/Release/SolarTest
#include "PluginProcessor.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond, ...) do { if (cond) std::printf ("  [OK]   " __VA_ARGS__); else { ++failures; std::printf ("  [FAIL] " __VA_ARGS__); } std::printf ("\n"); } while (0)

static void setParam (SolarAudioProcessor& p, const char* id, float realValue)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (realValue));
}

/** Frequence estimee par passages a zero montants (interpolation lineaire) sur la fin du signal. */
static double measureHz (const std::vector<float>& x, double sr, size_t from)
{
    double first = -1.0, last = -1.0; int count = 0;
    for (size_t i = from + 1; i < x.size(); ++i)
        if (x[i - 1] < 0.0f && x[i] >= 0.0f)
        {
            const double pos = (double) i - 1.0 + (double) (-x[i - 1]) / (double) (x[i] - x[i - 1]);
            if (first < 0.0) first = pos; else { last = pos; ++count; }
        }
    return count > 0 ? sr * count / (last - first) : 0.0;
}

/** Fait passer une sinusoide dans le plugin et renvoie la sortie (voie gauche). */
static std::vector<float> run (SolarAudioProcessor& p, double hz, double sr, double seconds, float amp = 0.5f)
{
    const int block = 512, total = (int) (sr * seconds);
    std::vector<float> out; out.reserve ((size_t) total);
    juce::AudioBuffer<float> buf (2, block);
    juce::MidiBuffer midi;
    double ph = 0.0;
    for (int done = 0; done < total; done += block)
    {
        for (int i = 0; i < block; ++i)
        {
            const float s = amp * (float) std::sin (ph);
            buf.setSample (0, i, s); buf.setSample (1, i, s);
            ph += juce::MathConstants<double>::twoPi * hz / sr;
        }
        p.processBlock (buf, midi);
        for (int i = 0; i < block; ++i) out.push_back (buf.getSample (0, i));
    }
    return out;
}

/** Capture PNG de l'editeur (headless) : SolarTest --snapshot dossier */
static void snapshots (const juce::File& dir)
{
    SolarAudioProcessor p;
    p.prepareToPlay (48000.0, 512);
    setParam (p, "scale", (float) solar::Minor);
    setParam (p, "vibDepth", 30.0f);
    // chant glissant : remplit l'historique de pitch
    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    double ph = 0.0;
    for (int b = 0; b < 400; ++b)
    {
        for (int i = 0; i < 512; ++i)
        {
            const double t = (b * 512 + i) / 48000.0;
            const double hz = 196.0 * std::pow (2.0, (std::sin (t * 1.7) * 5.0 + 0.3 * std::sin (t * 31.0)) / 12.0);
            ph += juce::MathConstants<double>::twoPi * hz / 48000.0;
            const float s = 0.4f * (float) (std::sin (ph) + 0.4 * std::sin (2 * ph) + 0.2 * std::sin (3 * ph));
            buf.setSample (0, i, s); buf.setSample (1, i, s);
        }
        p.processBlock (buf, midi);
    }
    for (int mode = 0; mode < 2; ++mode)
    {
        setParam (p, "mode", (float) mode);
        if (mode == 1) p.snapGraphToScale();
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (960, 600);
        ed->resized();
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        juce::PNGImageFormat png;
        auto f = dir.getChildFile (mode == 0 ? "solar_auto.png" : "solar_graph.png");
        f.deleteFile();
        juce::FileOutputStream os (f);
        png.writeImageToStream (img, os);
        std::printf ("  snapshot -> %s\n", f.getFullPathName().toRawUTF8());
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const double sr = 48000.0;
    if (argc >= 3 && juce::String (argv[1]) == "--snapshot") { snapshots (juce::File (argv[2])); return 0; }

    std::printf ("== Correction chromatique, retune 0 ==\n");
    {
        SolarAudioProcessor p;
        p.prepareToPlay (sr, 512);
        setParam (p, "scale", (float) solar::Chromatic);
        setParam (p, "retune", 0.0f); setParam (p, "humanize", 0.0f); setParam (p, "flex", 0.0f);
        const double in = 224.0;                       // +31 cents au-dessus de A3 (220 Hz)
        auto out = run (p, in, sr, 2.5);
        const double f = measureHz (out, sr, (size_t) (sr * 1.5));
        std::printf ("  entree %.2f Hz -> sortie %.2f Hz (cible 220.00)\n", in, f);
        CHECK (std::abs (f - 220.0) < 1.5, "224 Hz ramene a 220 Hz");
        CHECK (p.getLatencySamples() > 300 && p.getLatencySamples() < 700, "latence annoncee = %d echantillons", p.getLatencySamples());
    }

    std::printf ("== Gamme Mineur La : 233 Hz (La#) doit aller vers La ou Si ==\n");
    {
        SolarAudioProcessor p;
        p.prepareToPlay (sr, 512);
        setParam (p, "key", 9.0f); setParam (p, "scale", (float) solar::Minor);
        setParam (p, "retune", 0.0f); setParam (p, "humanize", 0.0f);
        auto out = run (p, 236.0, sr, 2.5);          // La# + ~20 ct -> plus proche de B (246.94) ? (La#=233.08, B=246.94)
        const double f = measureHz (out, sr, (size_t) (sr * 1.5));
        std::printf ("  236.00 Hz -> %.2f Hz\n", f);
        CHECK (std::abs (f - 220.0) < 2.0 || std::abs (f - 246.94) < 2.0, "La# hors gamme remonte/descend sur une note de la gamme");
    }

    std::printf ("== Amount 0 / Bypass = transparent ==\n");
    {
        SolarAudioProcessor p;
        p.prepareToPlay (sr, 512);
        setParam (p, "amount", 0.0f);
        auto out = run (p, 224.0, sr, 2.0);
        const double f = measureHz (out, sr, (size_t) (sr * 1.0));
        CHECK (std::abs (f - 224.0) < 0.8, "Amount=0 conserve 224 Hz (mesure %.2f)", f);
    }

    std::printf ("== Mode Graph : courbe snap sur gamme chromatique ==\n");
    {
        SolarAudioProcessor p;
        p.prepareToPlay (sr, 512);
        setParam (p, "mode", 1.0f); setParam (p, "scale", (float) solar::Chromatic);
        setParam (p, "retune", 0.0f); setParam (p, "humanize", 0.0f);
        p.snapGraphToScale();
        auto out = run (p, 224.0, sr, 2.5);
        const double f = measureHz (out, sr, (size_t) (sr * 1.5));
        std::printf ("  224.00 Hz -> %.2f Hz\n", f);
        CHECK (std::abs (f - 220.0) < 2.0, "courbe graphique : 224 Hz -> 220 Hz");
    }

    std::printf ("== Presets d'usine : rendu sans NaN / explosion ==\n");
    {
        SolarAudioProcessor p;
        p.prepareToPlay (sr, 512);
        bool ok = true;
        for (int i = 0; i < p.presets.getNumPresets(); ++i)
        {
            p.presets.loadPreset (i);
            auto out = run (p, 180.0 + 5.0 * i, sr, 0.5);
            for (float v : out) if (! std::isfinite (v) || std::abs (v) > 4.0f) { ok = false; break; }
        }
        CHECK (ok, "%d presets rendus proprement", p.presets.getNumPresets());
    }

    std::printf ("== Silence et bruit : aucune explosion ==\n");
    {
        SolarAudioProcessor p;
        p.prepareToPlay (sr, 512);
        auto out = run (p, 220.0, sr, 0.3, 0.0f);
        bool ok = true; for (float v : out) if (! std::isfinite (v) || std::abs (v) > 1e-6f) ok = false;
        CHECK (ok, "silence -> silence");
    }

    std::printf (failures == 0 ? "\nTOUS LES TESTS PASSENT\n" : "\n%d ECHEC(S)\n", failures);
    return failures == 0 ? 0 : 1;
}
