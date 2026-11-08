"""Plugs real instruments (Source/SampleMap.h, made by make_samples.py) into the engines of a DJBILBOX BEATS synth
(same voice template as DZPOWER): the chosen engines play recorded notes instead of being synthesised, coloured by
the engine's TONE / BODY / BRIGHT / TEXTURE knobs; the other engines are unchanged.
usage: python sample_apply.py <synth dir> <engine>=<instrument>[,<engine>=<instrument>...] [drums=<engine>,...] [layer=<engine>:<0..1 mix>,...]
example: python sample_apply.py MARABOUT 0=kora,1=balafon,2=oud,3=ney,4=zurna
layer: keeps the synthesised engine under the recording at that level (e.g. a sub under a drum)"""
import os, re, sys

D = os.path.abspath(sys.argv[1]); name = os.path.basename(D)
pairs = [p.split('=') for p in sys.argv[2].split(',')]
drums = set(sys.argv[3].split('=')[1].split(',')) if len(sys.argv) > 3 and sys.argv[3].startswith('drums=') else set()
layers = {}
for a in sys.argv[3:]:
    if a.startswith('layer='):
        for l in a.split('=')[1].split(','): e, v = l.split(':'); layers[e] = v
src = lambda f: os.path.join(D, 'Source', f)
read = lambda p: open(p, encoding='utf-8').read()
def write(p, s): open(p, 'w', encoding='utf-8', newline='').write(s)

vf = [f for f in os.listdir(os.path.join(D, 'Source')) if f.endswith('Voice.h')][0]
v = read(src(vf))
ns = re.search(r'namespace (\w+)', read(src('SampleMap.h'))).group(1)
voice = re.search(r'class (\w+)\s*:\s*public juce::SynthesiserVoice', v).group(1)
insts = [i for _, i in pairs]
if 'RealInstruments' not in v:
    v = v.replace('#include "NeonFX.h"\n', '#include "NeonFX.h"\n#include "BinaryData.h"\n#include "SampleMap.h"\n', 1)
    cases = ' '.join('case %s: return &%s;' % (e, i) for e, i in pairs)
    loads = ' '.join('%s.load (%sNotes, BinaryData::getNamedResource);' % (i, i) for i in insts)
    v = v.replace('class %s : public juce::SynthesiserVoice' % voice, '''// The real instruments of %(n)s: recorded notes (see Assets/samples/CREDITS.txt) played by some engines instead of
// synthesis. Decoded once, on the message thread (processor constructor).
struct RealInstruments
{
    hfx::SampleSet %(members)s;
    static const RealInstruments& get() { static const RealInstruments r; return r; }
    // engine -> its recordings (nullptr: the engine stays synthesised)
    const hfx::SampleSet* forEngine (int e) const
    {
        switch (e) { %(cases)s default: return nullptr; }
    }
private:
    RealInstruments() { %(loads)s }
};

class %(voice)s : public juce::SynthesiserVoice''' % {'n': name, 'members': ', '.join(insts), 'cases': cases, 'loads': loads, 'voice': voice}, 1)
    v = re.sub(r'\n        ((?:fadsr\.noteOn\(\); )?adsr\.noteOn\(\);)', lambda m: '\n        real.note = nullptr; realEngine = -1; noteKey = midiNote;   // the recording is picked when the note first sounds\n        ' + m.group(1), v, count=1)   # no filter envelope: adsr.noteOn() alone
    m = re.search(r'            float y = renderEngine \(freq, t, \(float\) sr\)( \* trim\[eng\])?;', v)
    assert m, (name, 'no renderEngine line')
    trim = m.group(1) or ''   # some synths have no per-engine trim
    lay = ''.join(' + (eng == %s ? renderEngine (freq, t, (float) sr)%s * %sf : 0.f)' % (e, trim, x) for e, x in layers.items())
    v = v[:m.start()] + '''            float y;
            if (const auto* set = RealInstruments::get().forEngine (eng))
                y = playReal (*set, freq, t, (float) sr)%s;
            else
                y = renderEngine (freq, t, (float) sr)%s;''' % (lay, trim) + v[m.end():]
    v = re.sub(r'(    SVF [^;]*voiceFilter)(;)', r'\1, realTone\2', v, count=1)
    drum_test = ' || '.join('eng == %s' % e for e in sorted(drums)) or 'false'
    m = re.search(r'    double ph\[\d+\] \{\}, lfoPh = 0\.0;', v)
    assert m, (name, 'no phase members')
    v = v[:m.end()] + '''
    hfx::SamplePlayer real;      // the real instrument's recording this note plays
    int realEngine = -1, noteKey = 60;
    float realGain[8] { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };   // levels of the recordings, matched to the synthesised engines (set_levels)

    // a recorded note, coloured by the engine's knobs: TONE darkens, BRIGHT adds air, BODY warms, TEXTURE adds breath
    float playReal (const hfx::SampleSet& set, double f, const float* t, float sr)
    {
        if (realEngine != eng || ! real.active())
        {
            int key = noteKey;
            if (%(drum)s) key = 36 + ((noteKey - 36) %% 24 + 24) %% 24;   // drums: every octave plays the strokes
            real.start (set.pick (key, vel)); realEngine = eng;
        }
        float x = real.next (f, (double) sr);
        const float tone = t[0], body = t[1], bright = t[2], tex = t[4];
        realTone.set (juce::jlimit (200.f, sr * 0.45f, real.isDrum() ? 2500.f + 14000.f * (1.f - tone) : (float) f * (4.f + 14.f * (1.f - tone))), 0.7f, sr);
        const auto o = realTone.process (x);
        x = o.lp * (0.65f + 0.35f * (1.f - bright)) + o.hp * (0.1f + 0.7f * bright) + o.bp * 0.3f * body;
        return (x * 1.7f + (real.isDrum() ? 0.f : (rng.nextFloat() * 2.f - 1.f) * 0.01f * tex)) * realGain[eng];
    }''' % {'drum': drum_test} + v[m.end():]
    for need in ('struct RealInstruments', 'playReal (*set', 'realTone', 'hfx::SamplePlayer real', 'noteKey = midiNote'):
        assert need in v, (name, 'voice patch failed at', need)
    write(src(vf), v)

pc = read(src('PluginProcessor.cpp'))
if 'RealInstruments::get()' not in pc:
    pc = pc.replace('    glideState.live = &live;\n', '    glideState.live = &live;\n    %s::RealInstruments::get();   // decode the real instruments now, not in the audio thread\n' % ns, 1)
    assert 'RealInstruments::get()' in pc
    write(src('PluginProcessor.cpp'), pc)

cm = read(os.path.join(D, 'CMakeLists.txt'))
if 'Assets/samples' not in cm:
    m = re.search(r'juce_add_binary_data\((\w+) SOURCES([^)]*)\)', cm)
    cm = cm[:m.start()] + '''# Assets/samples: the real instruments (see Assets/samples/CREDITS.txt), made by
# ALL_PLUGINS/HumpireFX/tools/make_samples.py from Tools/samples.json
file(GLOB REAL_SAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/Assets/samples/*.flac)
juce_add_binary_data(%s SOURCES%s ${REAL_SAMPLES})''' % (m.group(1), m.group(2)) + cm[m.end():]
    write(os.path.join(D, 'CMakeLists.txt'), cm)
print('real instruments in', name, pairs, 'drums', drums, 'layers', layers)
