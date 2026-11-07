"""Step 1 of the premium skin for one DJBILBOX BEATS synth (same template as DZPOWER):
  - the voices report envelopes / LFO / cutoff to an hfx::LiveTap, the processor publishes levels and note-ons,
  - CMake: HumpireFX include dir, MinGW static runtime + Common Controls manifest, a UiFrames tool,
  - Tools/UiFrames.cpp: captures the interface (--still out.png = one quiet frame at 2x).
Run before the faceplate exists; skin_apply.py does the editor afterwards.
usage: python skin_prepare.py <synth dir>"""
import os, re, sys

D = os.path.abspath(sys.argv[1]); name = os.path.basename(D)
src = lambda f: os.path.join(D, 'Source', f)
read = lambda p: open(p, encoding='utf-8').read()
def write(p, s): open(p, 'w', encoding='utf-8', newline='').write(s)

ed_h = read(src('PluginEditor.h'))
panel = re.search(r'class (\w+Panel)\b', ed_h).group(1)
proc_h = read(src('PluginProcessor.h'))
proc = re.search(r'class (\w+)\s*:\s*public juce::AudioProcessor\b', proc_h).group(1)
voice_f = [f for f in os.listdir(os.path.join(D, 'Source')) if re.search(r'Voice\.h$', f)][0]
target = re.search(r'juce_add_plugin\((\w+)', read(os.path.join(D, 'CMakeLists.txt'))).group(1)
print(name, panel, proc, voice_f, target)

# ---- voice: report the loudest voice to the interface
v = read(src(voice_f))
if '"NeonFX.h"' not in v:
    v = re.sub(r'(#include "Params.h"\n)', r'\1#include "NeonFX.h"\n', v, count=1)
    gs = v.index('struct GlideState'); end = v.index('\n};', gs)   # (its members hold braces: find the struct's own end)
    v = v[:end] + '\n    hfx::LiveTap* live = nullptr;          // what the interface shows (envelopes, LFO, cutoff of the loudest voice)' + v[end:]
    v = v.replace('freshNote = true;', 'freshNote = true; released = false;', 1)
    v = re.sub(r'if \(allowTailOff\) \{ adsr\.noteOff\(\); fadsr\.noteOff\(\); \}', 'if (allowTailOff) { adsr.noteOff(); fadsr.noteOff(); released = true; }', v, count=1)
    v = re.sub(r'(voiceFilter\.set \((\w+), [^;]*;)', r'\1\n                lastFc = \2;', v, count=1)
    v = re.sub(r'(const float env = adsr\.getNextSample\(\);)', r'\1\n            lastEnv = env; lastLfo = lfo;', v, count=1)
    # end of renderNextBlock: the loop ends with the clearCurrentNote / break line
    v = re.sub(r'(if \(! adsr\.isActive\(\)\) \{ clearCurrentNote\(\); break; \}\n        \}\n)(    \})',
               r'''\1        if (auto* t = glideState.live; t != nullptr && isVoiceActive())
        {
            const float secs = (float) age / (float) sr;
            t->voice (lastEnv, hfx::envStage (released, secs, P (atk), P (dec)), fenvLevel, hfx::envStage (released, secs, P (fatk), P (fdec)),
                      lastLfo, (float) lfoPh, lastFc);
        }
\2''', v, count=1)
    v = re.sub(r'(bool freshNote = true)(;)', r'\1, released = false\2', v, count=1)
    v = re.sub(r'(float fenvLevel = 0\.f)(;)', r'\1, lastEnv = 0.f, lastLfo = 0.f, lastFc = 9000.f\2   // for the interface', v, count=1)
    for need in ('released = true', 'lastFc =', 'lastEnv = env', 't->voice', 'released = false', 'lastLfo = 0.f', 'hfx::LiveTap* live', '"NeonFX.h"'):
        assert need in v, (name, 'voice patch failed at', need)
    write(src(voice_f), v)

# ---- processor
ph = read(src('PluginProcessor.h'))
if 'hfx::LiveTap live' not in ph:
    ph = ph.replace('    juce::MidiKeyboardState keyboardState;\n',
                    '    juce::MidiKeyboardState keyboardState;\n    hfx::LiveTap live;   // audio thread -> interface: levels, envelopes, LFO, cutoff, note-ons\n', 1)
    assert 'hfx::LiveTap live' in ph
    write(src('PluginProcessor.h'), ph)
pc = read(src('PluginProcessor.cpp'))
if 'live.beginBlock' not in pc:
    pc = pc.replace('keyboardState.processNextMidiBuffer (midi, 0, n, true);\n', 'keyboardState.processNextMidiBuffer (midi, 0, n, true);\n    live.beginBlock (midi);\n', 1)
    pc = re.sub(r'(\n    ptrs\.bind \(apvts\);\n)', r'\1    glideState.live = &live;\n', pc, count=1)
    # the last line of processBlock: publish the output
    m = re.search(r'void \w+::processBlock[^{]*\{', pc); start = m.end()
    depth, i = 1, start
    while depth: depth += {'{': 1, '}': -1}.get(pc[i], 0); i += 1
    body_end = i - 1
    L = re.findall(r'float\* (\w+) = buffer\.getWritePointer \(0\);', pc[start:body_end]); R = re.findall(r'float\* (\w+) = buffer\.getNumChannels\(\) > 1 \? buffer\.getWritePointer \(1\) : \w+;', pc[start:body_end])
    assert L and R, (name, 'no L/R pointers')
    pc = pc[:body_end] + '    live.publish (%s, %s, n);\n' % (L[0], R[0]) + pc[body_end:]
    for need in ('live.beginBlock', 'glideState.live = &live', 'live.publish'):
        assert need in pc, (name, 'processor patch failed at', need)
    write(src('PluginProcessor.cpp'), pc)

# ---- CMake
cm = read(os.path.join(D, 'CMakeLists.txt'))
if 'HumpireFX' not in cm:
    m = re.search(r'target_sources\(%s PRIVATE[^)]*\)' % target, cm)
    cm = cm[:m.end()] + '''
target_include_directories(%(t)s PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(%(t)s PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET %(t)s_${fmt})
            target_link_options(%(t)s_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(%(t)s_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(%(t)sUiFrames PRODUCT_NAME "%(t)sUiFrames")
    target_sources(%(t)sUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(%(t)sUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(%(t)sUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(%(t)sUiFrames PRIVATE %(assets)s juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(%(t)sUiFrames PRIVATE -static)
    endif()
endif()''' % {'t': target, 'assets': re.search(r'juce_add_binary_data\((\w+)', cm).group(1)} + cm[m.end():]
    write(os.path.join(D, 'CMakeLists.txt'), cm)

# ---- capture tool
os.makedirs(os.path.join(D, 'Tools'), exist_ok=True)
write(os.path.join(D, 'Tools', 'UiFrames.cpp'), '''// Captures / films the %(n)s interface. --still out.png: one quiet frame at 2x (the picture the faceplate is made from);
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
    %(p)s proc;
    proc.setRateAndBufferSizeDetails (sr, block);
    proc.prepareToPlay (sr, block);
    if (argc > 4) proc.applyPreset (juce::String (argv[4]).getIntValue());
    %(panel)s panel (proc);
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
    std::printf ("%%d frames -> %%s\\n", frames, out.getFullPathName().toRawUTF8());
    return 0;
}
''' % {'n': name, 'p': proc, 'panel': panel})
print('prepared', name)
