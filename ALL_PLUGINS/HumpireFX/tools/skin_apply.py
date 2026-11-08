"""Step 3 of the premium skin: the synth's editor draws the faceplate (Assets/faceplate.jpg) and puts the live
light on top (hfx::PremiumOverlay), every control sits on its part of the faceplate (Source/FaceplateLayout.h).
Run after skin_prepare.py and skin_layout.py.
usage: python skin_apply.py <synth dir> <theme: electric|spotlights|glitch|police|goldRain|scanner|smoke|film|sun|bubbles|ghostFlame|sunset|rain|bassShake|orientalGold>"""
import os, re, shutil, sys

D = os.path.abspath(sys.argv[1]); theme = sys.argv[2]; name = os.path.basename(D)
HERE = os.path.dirname(os.path.abspath(__file__))
src = lambda f: os.path.join(D, 'Source', f)
read = lambda p: open(p, encoding='utf-8').read()
def write(p, s): open(p, 'w', encoding='utf-8', newline='').write(s)

h = read(src('PluginEditor.h')); c = read(src('PluginEditor.cpp'))
lnf = re.search(r'class (\w+)\s*:\s*public juce::LookAndFeel_V4', h).group(1)
panel = re.search(r'class (\w+Panel)\b', h).group(1)
layout = read(src('FaceplateLayout.h'))
caps = re.findall(r'\{ "(\w+)", ', layout)
has_title = 'titleText' in layout

def body_replace(code, signature_re, new_body):
    m = re.search(signature_re, code)
    assert m, (name, 'not found', signature_re)
    i = code.index('{', m.end() - 1) if code[m.end() - 1] != '{' else m.end() - 1
    depth, j = 0, i
    while True:
        depth += {'{': 1, '}': -1}.get(code[j], 0); j += 1
        if depth == 0: break
    return code[:i] + '{\n' + new_body + '}' + code[j:]

if 'PremiumSkin.h' not in h:
    # ---- header
    h = h.replace('#include "PluginProcessor.h"\n', '#include "PluginProcessor.h"\n#include "PremiumSkin.h"\n', 1)
    h = re.sub(r'(class %s\s*:\s*public juce::LookAndFeel_V4\s*\{\s*public:)' % lnf, r'\1\n    juce::Image faderCap;   // photo-real fader cap (the knob caps and slots are part of the faceplate)', h, count=1)
    h = h.replace('    juce::MidiKeyboardComponent keys;', '    hfx::NeonKeyboard keys;', 1)
    h = re.sub(r'(class %s\b[^{]*\{\s*public:\s*\n(?:.*\n)*?    void resized\(\) override;\n)' % panel, r'\1    void mouseDown (const juce::MouseEvent&) override;   // right click: animations on / off\n', h, count=1)
    h = h.replace('    std::vector<Knob*> all;\n};', '''    std::vector<Knob*> all;

    hfx::FaceplateBackground face;                 // the photo-real front panel (Assets/faceplate.jpg)
    hfx::PremiumOverlay overlay { proc.live, proc.keyboardState, keys, proc.apvts.state };   // everything that lights up
};''', 1)
    for need in ('PremiumSkin.h', 'hfx::NeonKeyboard keys', 'mouseDown', 'hfx::PremiumOverlay overlay', 'juce::Image faderCap'):
        assert need in h, (name, 'header patch failed at', need)
    write(src('PluginEditor.h'), h)

if 'FaceplateLayout.h' not in c:
    c = c.replace('#include "BinaryData.h"\n', '#include "BinaryData.h"\n#include "FaceplateLayout.h"\n', 1)
    # ---- look and feel: only what sits on top of the faceplate
    c = body_replace(c, r'void %s::drawRotarySlider \(juce::Graphics& (\w+), int (\w+), int (\w+), int (\w+), int (\w+), float (\w+), float (\w+), float (\w+), juce::Slider& (\w+)\)\s*\{' % lnf,
                     '    hfx::plateRotary (g, x, y, w, h, pos, a0, a1, s, ramp (pos));\n')
    c = re.sub(r'void %s::drawRotarySlider \(juce::Graphics& \w+, int \w+, int \w+, int \w+, int \w+, float \w+, float \w+, float \w+, juce::Slider& \w+\)' % lnf,
               'void %s::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)' % lnf, c, count=1)
    c = body_replace(c, r'void %s::drawLinearSlider \([^)]*\)\s*\{' % lnf,
                     '''    if ((bool) s.getProperties()["wheel"]) { hfx::plateWheel (g, x, w, pos); return; }
    hfx::plateLinear (g, x, y, w, h, pos, s, faderCap, ramp ((pos - (float) y) / (float) juce::jmax (1, h)));
''')
    c = re.sub(r'void %s::drawLinearSlider \([^)]*\)' % lnf,
               'void %s::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)' % lnf, c, count=1)
    c = body_replace(c, r'void %s::drawComboBox \([^)]*\)\s*\{' % lnf, '    // the box and its arrow are part of the faceplate: only the text (the combo\'s label) is drawn\n')
    c = re.sub(r'void %s::drawComboBox \([^)]*\)' % lnf, 'void %s::drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&)' % lnf, c, count=1)

    # ---- the panel: faceplate + overlay
    roles = {'rate': 'lfoRate', 'amount': 'lfoAmount', 'filter': 'cutoff', 'fenvK': 'filterEnv', 'resoK': 'resonance', 'glide': 'glide',
             'volume': 'volume', 'reverb': 'effect', 'chorus': 'effect', 'dist': 'effect', 'delay': 'delayMix', 'dtime': 'delayTime', 'feedback': 'delayFeedback'}
    setup = ['', '    // premium skin: the photo-real faceplate, the live light on top (see ALL_PLUGINS/HumpireFX/PremiumSkin.h)',
             '    face.setImage (juce::ImageFileFormat::loadFrom (BinaryData::faceplate_jpg, BinaryData::faceplate_jpgSize));',
             '    face.setMascot (camel, { Face::mascotX, Face::mascotY }, Face::mascotR);   // the original artwork in the medallion',
             '    lnf.faderCap = juce::ImageFileFormat::loadFrom (BinaryData::fader_png, BinaryData::fader_pngSize);',
             '    keys.neon = ember; keys.neonHot = amber;',
             '    keys.setOverlayOnly();   // the keys are part of the faceplate',
             '    overlay.a = amber; overlay.b = ember; overlay.theme = hfx::Theme::%s;' % theme,
             '    overlay.mascot = { Face::mascotX, Face::mascotY }; overlay.mascotR = Face::mascotR;']
    if has_title:
        setup += ['    overlay.setTitleFromFace (juce::ImageFileFormat::loadFrom (BinaryData::faceplate_jpg, BinaryData::faceplate_jpgSize),',
                  '                              { Face::titleX, Face::titleY, Face::titleW, Face::titleH });   // light from the real letters']
    for k in caps:
        setup.append('    overlay.addKnob (%s.s, hfx::PremiumOverlay::%s);' % (k, roles.get(k, 'plain')))
    # faders: the labels say which envelope stage each one is (ATTACK, DECAY, SUSTAIN, RELEASE) or VELOCITY
    stages = ['ATTACK', 'DECAY', 'SUSTAIN', 'RELEASE']
    def labels(var):
        m = re.search(r'const char\* %s\[\d+\] = \{ ([^}]*) \};' % var, c)
        alias = {'ATK': 'ATTACK', 'DEC': 'DECAY', 'SUS': 'SUSTAIN', 'REL': 'RELEASE'}
        return [alias.get(x.strip().strip('"').upper(), x.strip().strip('"').upper()) for x in m.group(1).split(',')]
    en, tn = labels('en'), labels('tn')
    for i, l in enumerate(en):
        setup.append('    overlay.addFader (env[%d].s, true, %d, hfx::PremiumOverlay::ledColumns (Face::envSlots[%d].x, Face::envSlots[%d].top + 3.f, Face::envSlots[%d].bottom - 3.f));' % (i, stages.index(l), i, i, i))
    for i, l in enumerate(tn):
        cols = 'hfx::PremiumOverlay::ledColumns (Face::rslSlots[%d].x, Face::rslSlots[%d].top + 3.f, Face::rslSlots[%d].bottom - 3.f)' % (i, i, i)
        setup.append('    overlay.addFader (rsl[%d].s, false, %d, %s);' % (i, stages.index(l), cols) if l in stages else
                     '    overlay.addVelocityFader (rsl[%d].s, %s);' % (i, cols) if l == 'VELOCITY' else
                     '    overlay.addPlainFader (rsl[%d].s, %s);   // a timbre fader (%s)' % (i, cols, l))
    setup += ['    addAndMakeVisible (overlay);', '']
    n_env, n_rsl = len(en), len(tn)
    m = re.search(r'\n(    setSize \(1000, 746\);)', c[c.index('%s::%s (' % (panel, panel)):])
    at = c.index('%s::%s (' % (panel, panel)) + m.start()
    c = c[:at] + '\n'.join(setup) + c[at:]
    c = body_replace(c, r'void %s::paint \(juce::Graphics& g\)\s*\{' % panel,
                     '    face.draw (g, getLocalBounds());   // the whole front panel is one photo-real render (see Assets/faceplate.jpg)\n')
    # resized: every control on its part of the faceplate
    m = re.search(r'void %s::resized\(\)\s*\{' % panel, c); i = m.end(); depth = 1
    while depth: depth += {'{': 1, '}': -1}.get(c[i], 0); i += 1
    end = i - 1
    c = c[:end] + '''
    // on the faceplate: knobs centred on their caps (value ring just outside), faders on their slots
    for (const auto& cap : Face::caps)
        for (auto* k : { %(knobs)s })
            if (&k->s == &cap_knob (cap.name))
            {
                const float ring = cap.r + (cap.r > 14.f ? 4.5f : 2.6f), half = ring + 3.f;
                k->s.setBounds (juce::Rectangle<float> (half * 2.f, half * 2.f).withCentre ({ cap.x, cap.y }).toNearestInt());
            }
    for (int i = 0; i < %(ne)d; ++i) env[i].s.setBounds (juce::Rectangle<float> (28.f, Face::envSlots[i].bottom - Face::envSlots[i].top + 8.f).withCentre ({ Face::envSlots[i].x, (Face::envSlots[i].top + Face::envSlots[i].bottom) * 0.5f }).toNearestInt());
    for (int i = 0; i < %(nr)d; ++i) rsl[i].s.setBounds (juce::Rectangle<float> (28.f, Face::rslSlots[i].bottom - Face::rslSlots[i].top + 8.f).withCentre ({ Face::rslSlots[i].x, (Face::rslSlots[i].top + Face::rslSlots[i].bottom) * 0.5f }).toNearestInt());
    overlay.setBounds (getLocalBounds());
''' % {'knobs': ', '.join('&' + k for k in caps), 'ne': n_env, 'nr': n_rsl} + c[end:]
    c = c.replace('&k->s == &cap_knob (cap.name)', 'juce::String (cap.name) == knobName (k)')
    # helpers: the knob's member name, the right-click menu
    names = '\n'.join('    if (k == &%s) return "%s";' % (k, k) for k in caps)
    c += '''
juce::String %(p)s::knobName (const Knob* k) const
{
%(names)s
    return {};
}

void %(p)s::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    juce::PopupMenu m;
    m.addItem (1, "Animations", true, overlay.animationsOn());
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(), [this] (int r) { if (r == 1) overlay.setAnimations (! overlay.animationsOn()); });
}
''' % {'p': panel, 'names': names}
    write(src('PluginEditor.cpp'), c)
    h = read(src('PluginEditor.h'))
    h = h.replace('    void mouseDown (const juce::MouseEvent&) override;   // right click: animations on / off\n',
                  '    void mouseDown (const juce::MouseEvent&) override;   // right click: animations on / off\n', 1)
    h = h.replace('    std::vector<Knob*> all;\n', '    std::vector<Knob*> all;\n    juce::String knobName (const Knob*) const;   // member name of a knob (FaceplateLayout.h names them)\n', 1)
    write(src('PluginEditor.h'), h)

# ---- assets
shutil.copyfile(os.path.join(HERE, '..', '..', '..', 'DZPOWER', 'Assets', 'fader.png'), os.path.join(D, 'Assets', 'fader.png'))
cm = read(os.path.join(D, 'CMakeLists.txt'))
if 'faceplate.jpg' not in cm:
    cm = re.sub(r'(juce_add_binary_data\(\w+ SOURCES[^)]*)\)', r'\1 Assets/fader.png Assets/faceplate.jpg)', cm, count=1)
    write(os.path.join(D, 'CMakeLists.txt'), cm)
print('applied', name, theme, len(caps), 'knobs')
