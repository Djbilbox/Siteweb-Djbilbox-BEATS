// Headless snapshot of a HUMPIRE LOADOUT plugin editor.
// Usage: HumpireSnapshot <Plugin> <out.png> [presetIndex] [--scale=1.5] [--blocks=40] [--settle=ms]
#include <HumpireCore/HumpireProcessor.h>
#include "PluginProcessor.h"   // Glitch
#include <cstdio>
#include <cmath>

#define DECL(n) juce::AudioProcessor* createFilter_##n();
DECL(Skilltree) DECL(Clutch) DECL(Overclock) DECL(Hardcap) DECL(Respawn) DECL(Lag) DECL(Glitch) DECL(Combo)

int main (int argc, char** argv)
{
    juce::String name, out; int preset = 0, blocks = 48, settle = 400; float scale = 1.5f, pos = 0;
    for (int i = 1; i < argc; ++i)
    {
        juce::String a (argv[i]);
        if (a.startsWith ("--scale="))       scale = a.fromFirstOccurrenceOf ("=", false, false).getFloatValue();
        else if (a.startsWith ("--blocks=")) blocks = a.fromFirstOccurrenceOf ("=", false, false).getIntValue();
        else if (a.startsWith ("--settle=")) settle = a.fromFirstOccurrenceOf ("=", false, false).getIntValue();
        else if (name.isEmpty()) name = a; else if (out.isEmpty()) out = a; else preset = a.getIntValue();
    }
    if (name.isEmpty() || out.isEmpty()) { std::fprintf (stderr, "usage: HumpireSnapshot <Plugin> <out.png> [preset]\n"); return 1; }

    juce::ScopedJuceInitialiser_GUI init;
    std::unique_ptr<juce::AudioProcessor> proc;
    #define PICK(n) if (name.equalsIgnoreCase (#n)) proc.reset (createFilter_##n());
    PICK(Skilltree) PICK(Clutch) PICK(Overclock) PICK(Hardcap) PICK(Respawn) PICK(Lag) PICK(Glitch) PICK(Combo)
    if (! proc) { std::fprintf (stderr, "unknown plugin %s\n", name.toRawUTF8()); return 1; }

    humpire::PresetManager* pm = nullptr;
    if (auto* h = dynamic_cast<humpire::HumpireProcessor*> (proc.get())) pm = &h->presets;
    else if (auto* g = dynamic_cast<GlitchAudioProcessor*> (proc.get())) pm = &g->presets;
    if (pm != nullptr)
    {
        if (preset < 0 || preset >= pm->getNumPresets()) { std::fprintf (stderr, "preset out of range (0..%d)\n", pm->getNumPresets() - 1); return 2; }
        pm->load (preset);
    }

    const double sr = 48000.0; const int bs = 512;
    proc->setPlayConfigDetails (2, 2, sr, bs);
    proc->prepareToPlay (sr, bs);
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc->createEditor());
    if (! ed) { std::fprintf (stderr, "no editor\n"); return 3; }

    // test audio: kick-ish thump + bass sine + hat noise, keeps meters/scopes moving
    juce::Random rnd (7); juce::MidiBuffer midi; juce::AudioBuffer<float> buf (2, bs);
    for (int b = 0; b < blocks; ++b)
    {
        for (int i = 0; i < bs; ++i)
        {
            const double t = (b * bs + i) / sr, beat = std::fmod (t, 0.25);
            float s = 0.45f * (float) std::sin (6.2831853 * (50.0 + 90.0 * std::exp (-beat * 30.0)) * t) * (float) std::exp (-beat * 9.0);
            s += 0.25f * (float) std::sin (6.2831853 * 110.0 * t) + 0.12f * (rnd.nextFloat() * 2.0f - 1.0f) * (float) std::exp (-std::fmod (t, 0.125) * 40.0);
            buf.setSample (0, i, s); buf.setSample (1, i, s * 0.95f);
        }
        proc->processBlock (buf, midi);
        if (b % 8 == 7) juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
    }
    juce::MessageManager::getInstance()->runDispatchLoopUntil (settle);
    (void) pos;

    for (auto* c : ed->getChildren())
        if (dynamic_cast<juce::ResizableCornerComponent*> (c)) c->setVisible (false);
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, scale);

    juce::File f (juce::File::getCurrentWorkingDirectory().getChildFile (out));
    f.getParentDirectory().createDirectory(); f.deleteFile();
    bool ok = false;
    { juce::FileOutputStream os (f); if (os.openedOk()) { juce::PNGImageFormat png; ok = png.writeImageToStream (img, os); os.flush(); } }
    if (! ok) { std::fprintf (stderr, "write failed\n"); return 4; }
    std::printf ("wrote %s %dx%d preset %d/%d\n", f.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight(), preset, pm ? pm->getNumPresets() : 0);
    ed.reset();
    return 0;
}
