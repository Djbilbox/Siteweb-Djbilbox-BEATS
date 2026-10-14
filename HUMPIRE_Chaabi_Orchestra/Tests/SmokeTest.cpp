// Test de fumee : lance avec  HumpireSmokeTest <dossier Samples avec WAV>
#include "PluginProcessor.h"
#include <cmath>
#include <cstdio>

using namespace humpire;

static int failures = 0;
#define CHECK(cond, msg) do { if (! (cond)) { std::printf ("  FAIL: %s\n", msg); ++failures; } else std::printf ("  ok  : %s\n", msg); } while (0)

static void pump (int ms) { juce::MessageManager::getInstance()->runDispatchLoopUntil (ms); }

static bool waitReady (ChaabiProcessor& p, int slot)
{
    for (int i = 0; i < 400; ++i)
    {
        pump (25);
        auto st = p.getSlotStatus (slot);
        if (st.state == 2) { pump (25); return true; }
        if (st.state == 3) { std::printf ("  status: %s\n", st.message.toRawUTF8()); return false; }
    }
    return false;
}

static void setP (ChaabiProcessor& p, const juce::String& id, float v)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (v));
}

struct Run { float peak = 0.f; bool bad = false; };

static Run render (ChaabiProcessor& p, juce::MidiBuffer first, int blocks, int bs = 512)
{
    Run r;
    juce::AudioBuffer<float> buf (2, bs);
    for (int b = 0; b < blocks; ++b)
    {
        juce::MidiBuffer m;
        if (b == 0) m = first;
        buf.clear();
        p.processBlock (buf, m);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < bs; ++i)
            {
                const float v = buf.getSample (c, i);
                if (! std::isfinite (v)) r.bad = true;
                r.peak = juce::jmax (r.peak, std::abs (v));
            }
    }
    return r;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    if (argc < 2) { std::printf ("usage: HumpireSmokeTest <Samples>\n"); return 2; }

    ChaabiProcessor p;
    p.setPlayConfigDetails (0, 2, 48000.0, 512);
    p.setLibraryRoot (juce::File (argv[1]));
    p.prepareToPlay (48000.0, 512);

    std::printf ("[Mandole] chargement SFZ\n");
    CHECK (waitReady (p, 0), "banque mandole chargee");
    std::printf ("  %s\n", p.getSlotStatus (0).message.toRawUTF8());

    {
        juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, 60, 0.9f), 0);
        auto r = render (p, m, 40);
        CHECK (r.peak > 0.02f && ! r.bad, "mandole note 60 (mediator) sonne");

        juce::MidiBuffer off; off.addEvent (juce::MidiMessage::noteOff (1, 60), 0);
        render (p, off, 200);

        juce::MidiBuffer ks; ks.addEvent (juce::MidiMessage::noteOn (1, 13, 1.0f), 0); // keyswitch tremolo
        render (p, ks, 1);
        juce::MidiBuffer m2; m2.addEvent (juce::MidiMessage::noteOn (1, 64, 0.9f), 0);
        auto r2 = render (p, m2, 400);                // ~4 s : la tenue en boucle doit encore sonner
        juce::AudioBuffer<float> tail (2, 512); juce::MidiBuffer none; tail.clear(); p.processBlock (tail, none);
        CHECK (tail.getMagnitude (0, 512) > 0.01f && ! r2.bad, "tremolo en boucle tient > 4 s");
        juce::MidiBuffer off2; off2.addEvent (juce::MidiMessage::noteOff (1, 64), 0);
        render (p, off2, 300);
        juce::MidiBuffer ks12; ks12.addEvent (juce::MidiMessage::noteOn (1, 12, 1.0f), 0);
        render (p, ks12, 1);
    }

    std::printf ("[Derbouka] selection + chargement\n");
    setP (p, pid::selected, 5.f);
    CHECK (waitReady (p, 5), "banque derbouka chargee");
    for (int note : { 36, 38, 40, 41, 43 })
    {
        juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, note, 0.8f), 0);
        auto r = render (p, m, 30);
        CHECK (r.peak > 0.005f && ! r.bad, ("derbouka note " + juce::String (note)).toRawUTF8());
    }

    std::printf ("[Phrases] touche 24 (maqsum)\n");
    {
        juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, 24, 0.9f), 0);
        auto r = render (p, m, 200);
        CHECK (r.peak > 0.01f && ! r.bad, "phrase derbouka declenchee");
        juce::MidiBuffer off; off.addEvent (juce::MidiMessage::noteOff (1, 24), 0);
        render (p, off, 100);
    }

    std::printf ("[Sequenceur] pattern derbouka\n");
    {
        setP (p, pid::seqOn, 1.f);
        auto r = render (p, {}, 200);
        CHECK (r.peak > 0.01f && ! r.bad, "sequenceur 16 pas sonne");
        setP (p, pid::seqOn, 0.f);
        render (p, {}, 100);
    }

    std::printf ("[Multi] mandole + derbouka + tar\n");
    {
        p.presets->loadIndex (16);   // premier preset "Mix chaabi"
        std::printf ("  preset: %s\n", p.presets->getCurrentName().toRawUTF8());
        waitReady (p, 0);
        CHECK (waitReady (p, 5), "derbouka prete en multi");
        juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, 62, 0.9f), 0); m.addEvent (juce::MidiMessage::noteOn (1, 36, 0.9f), 10);
        auto r = render (p, m, 40);
        CHECK (r.peak > 0.02f && ! r.bad, "mix multi sonne");
    }

    std::printf ("[Etat] aller-retour\n");
    {
        juce::MemoryBlock mb; p.getStateInformation (mb);
        ChaabiProcessor q; q.setStateInformation (mb.getData(), (int) mb.getSize());
        CHECK (q.apvts.getRawParameterValue (pid::multi)->load() > 0.5f, "mode multi restaure");
        CHECK (q.getLibraryRoot() == juce::File (argv[1]), "racine de la bibliotheque restauree");
    }

    std::printf ("[CPU] 16 voix\n");
    {
        setP (p, pid::multi, 0.f); setP (p, pid::selected, 0.f); pump (50);
        juce::MidiBuffer m;
        for (int n = 52; n < 68; ++n) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.9f), 0);
        juce::AudioBuffer<float> buf (2, 512);
        auto t0 = juce::Time::getMillisecondCounterHiRes();
        for (int b = 0; b < 200; ++b) { juce::MidiBuffer mm; if (b == 0) mm = m; p.processBlock (buf, mm); }
        const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
        const double budget = 200.0 * 512.0 / 48000.0 * 1000.0;
        std::printf ("  %.1f ms pour %.1f ms d'audio (%.1f %% CPU, build %s)\n", ms, budget, 100.0 * ms / budget,
                     JUCE_DEBUG ? "Debug" : "Release");
    }

    if (argc > 2)   // capture de l'interface : HumpireSmokeTest <Samples> <sortie.png>
    {
        setP (p, pid::multi, 0.f); setP (p, pid::selected, 5.f); pump (300);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1280, 760);
        pump (400);
        auto clickTab = [&] (const char* label)
        {
            std::function<void (juce::Component&)> rec = [&] (juce::Component& c)
            {
                if (auto* b = dynamic_cast<juce::TextButton*> (&c)) if (b->getButtonText() == label) b->triggerClick();
                for (auto* ch : c.getChildren()) rec (*ch);
            };
            rec (*ed);
        };
        if (argc > 3) { clickTab (argv[3]); pump (300); }
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        juce::File out (argv[2]);
        out.deleteFile();
        juce::PNGImageFormat png;
        if (auto os = out.createOutputStream()) png.writeImageToStream (img, *os);
        std::printf ("capture UI : %s\n", out.getFullPathName().toRawUTF8());
    }

    std::printf ("\n%s (%d echec(s))\n", failures == 0 ? "TOUS LES TESTS OK" : "ECHEC", failures);
    return failures == 0 ? 0 : 1;
}
