// Captures / films the HBX-84 TALKBOX interface. --still out.png: one quiet frame at 2x (the picture the faceplate is made from);
// <out dir> [fps] [seconds] [preset]: plays a chord and a riff, saves one PNG per frame and the sound (audio.wav).
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PremiumSkin.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const double sr = 48000.0;
    const int fps = argc > 2 && juce::String (argv[1]) != "--still" ? juce::String (argv[2]).getIntValue() : 30;
    const int block = (int) (sr / fps);
    TkProcessor proc;
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);
    if (argc > 4) proc.applyPreset (juce::String (argv[4]).getIntValue());
    TkPanel panel (proc);
    panel.setBounds (panel.getBounds());
    auto save = [] (const juce::Image& img, const juce::File& f)
    {
        juce::FileOutputStream os (f); os.setPosition (0); os.truncate();
        juce::PNGImageFormat().writeImageToStream (img, os);
    };
    auto step = [&panel] (float dt)
    {
        for (auto* c : panel.getChildren())
            if (auto* o = dynamic_cast<hfx::PremiumOverlay*> (c)) o->advance (dt);
    };
    if (argc > 2 && juce::String (argv[1]) == "--still")
    {
        for (int i = 0; i < 120; ++i) step (1.f / 60.f);
        save (panel.createComponentSnapshot (panel.getLocalBounds(), true, 2.0f), juce::File::getCurrentWorkingDirectory().getChildFile (argv[2]));
        return 0;
    }
    const juce::File out (juce::File::getCurrentWorkingDirectory().getChildFile (argc > 1 ? argv[1] : "frames"));
    out.createDirectory();
    const float seconds = argc > 3 ? juce::String (argv[3]).getFloatValue() : 7.f;
    struct Ev { float t; int note; bool on; };
    std::vector<Ev> evs { { 1.6f, 48, true }, { 1.6f, 55, true }, { 1.6f, 60, true }, { 3.0f, 48, false }, { 3.0f, 55, false }, { 3.0f, 60, false } };
    const int riff[] = { 60, 63, 65, 67, 70, 67, 65, 63 };
    for (int i = 0; i < 8; ++i) { evs.push_back ({ 3.3f + 0.16f * (float) i, riff[i], true }); evs.push_back ({ 3.3f + 0.16f * (float) i + 0.12f, riff[i], false }); }
    evs.push_back ({ 4.8f, 72, true }); evs.push_back ({ 6.0f, 72, false });
    const int frames = (int) (seconds * (float) fps);
    juce::AudioBuffer<float> buf (2, block), audio (2, frames * block);
    juce::MidiBuffer midi;
    for (int f = 0; f < frames; ++f)
    {
        const float t0 = (float) f / (float) fps, t1 = (float) (f + 1) / (float) fps;
        for (auto& e : evs)
            if (e.t >= t0 && e.t < t1) { if (e.on) proc.keyboardState.noteOn (1, e.note, 0.9f); else proc.keyboardState.noteOff (1, e.note, 0.f); }
        midi.clear();
        proc.processBlock (buf, midi);
        for (int ch = 0; ch < 2; ++ch) audio.copyFrom (ch, f * block, buf, ch, 0, block);
        step (1.f / (float) fps);
        save (panel.createComponentSnapshot (panel.getLocalBounds(), true, 1.0f), out.getChildFile ("frame_" + juce::String (f).paddedLeft ('0', 4) + ".png"));
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
