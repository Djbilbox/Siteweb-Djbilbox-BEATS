#include "PluginEditor.h"

#include <cmath>

#include "BinaryData.h"
#include "PluginProcessor.h"

namespace rk
{

namespace
{

// Fichier de reglage facultatif. L'habillage et la disposition sont compiles
// dans le binaire (le plugin doit s'afficher sur une machine ou le dossier de
// developpement n'existe pas) ; ce fichier permet quand meme de retoucher les
// coordonnees sans recompiler, en le deposant a cote des presets.
juce::File getLayoutOverrideFile()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("DJBILBOX")
        .getChildFile ("ROCKPUNK")
        .getChildFile ("layout.json");
}

} // namespace

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (p),
      proc (p),
      apvts (p.apvts)
{
    // L'image porte deja le nom du produit et celui du moteur, en lettrage
    // dessine : les redessiner en Sans par-dessus donnait deux titres empiles.

    // ---- component factories ----
    auto addKnob = [this] (const juce::String& label, const juce::String& paramId,
                           const juce::String& spriteKey) -> AnimatedKnob*
    {
        auto* k = own (new AnimatedKnob (label, lf, spriteKey));
        addAndMakeVisible (k);
        sliderAttachments.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, k->slider));
        glowKnobs.push_back (k);
        return k;
    };

    auto addSwitch = [this] (const juce::String& paramId, std::vector<juce::String> names,
                             const juce::String& spriteKey) -> CyclingSwitch*
    {
        auto* sw = own (new CyclingSwitch (paramId, *apvts.getParameter (paramId), std::move (names),
                                           lf, spriteKey));
        addAndMakeVisible (sw);
        glowSwitches.push_back (sw);
        return sw;
    };

    auto addLevel = [this] (const juce::String& label, const juce::String& paramId,
                            const juce::String& spriteKey, bool vertical) -> LevelSlider*
    {
        auto* s = own (new LevelSlider (label, lf, spriteKey, vertical));
        addAndMakeVisible (s);
        sliderAttachments.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, s->slider));
        glowSliders.push_back (s);
        return s;
    };

    auto addVu = [this] (bool left, const juce::String& spriteKey) -> VuMeter*
    {
        auto* v = own (new VuMeter (left, lf, spriteKey));
        addAndMakeVisible (v);
        glowVus.push_back (v);
        return v;
    };

    // ---- potards ----
    // Chaque controle pilote ce que l'image annonce au-dessus de lui. Avant,
    // REVERB dosait le delay, GLIDE melangeait les deux cordes et PITCH reglait
    // la presence de la corde B : la facade decrivait un autre instrument.
    knobLfoRate   = addKnob ("RATE",   "lfoRate",   "lfoRate");
    knobLfoAmount = addKnob ("AMT",    "lfoAmount", "lfoAmount");
    knobLfoPitch  = addKnob ("PITCH",  "lfoPitch",  "lfoPitch");
    knobGlide     = addKnob ("GLIDE",  "glideMs",   "glide");
    knobFilter    = addKnob ("FILTER", "fltCutoff", "filter");
    knobVol       = addKnob ("VOL",    "masterVol", "vol");
    knobReverb    = addKnob ("REV",    "revMix",    "reverb");
    knobChorus    = addKnob ("CHOR",   "chorAmt",   "chorus");
    knobDist      = addKnob ("DIST",   "driveAmt",  "dist");
    knobTime      = addKnob ("TIME",   "delayTime", "time");
    knobFb        = addKnob ("FB",     "delayFb",   "fb");

    knobLfoRate->setTextFormatter ([] (float v)
    {
        static const char* div[] = { "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2", "1bar", "2bar", "4bar" };
        return juce::String (div[juce::jlimit (0, 8, (int) v)]);
    });

    // ---- ADSR : la vraie enveloppe, celle que l'image nomme ----
    slAttack  = addLevel ("A",   "envA",     "attack",  true);
    slDecay   = addLevel ("D",   "envD",     "decay",   true);
    slSustain = addLevel ("S",   "envS",     "sustain", true);
    slRelease = addLevel ("R",   "envR",     "release", true);

    // ---- faders de droite : l'enveloppe du filtre, puis la velocite ----
    slFA      = addLevel ("A",   "fenvA",    "fA",      true);
    slFS      = addLevel ("S",   "fenvS",    "fS",      true);
    slFD      = addLevel ("D",   "fenvD",    "fD",      true);
    slFR      = addLevel ("R",   "fenvR",    "fR",      true);
    slFVel    = addLevel ("VEL", "velAmt",   "fVel",    true);

    // ---- molettes ----
    wheelPitch = addLevel ("PITCH", "pbend",    "pitchWheel", true);
    wheelMod   = addLevel ("MOD",   "modDepth", "modWheel",   true);

    // ---- speaker grills double as the layer on/off switches ----
    grillA = addSwitch ("laOn", { "OFF", "ON" }, "leftGrill");
    grillB = addSwitch ("lbOn", { "OFF", "ON" }, "rightGrill");
    grillA->setShowStateText (false);
    grillB->setShowStateText (false);

    // ---- selecteurs du bas ----
    // WAVEFORM agit bien sur la forme de l'excitation de la corde, LFO SOURCE
    // sur la forme du LFO et DESTINATION sur ce qu'il module : chaque etiquette
    // decrit enfin son role.
    swWf         = addSwitch ("pickShape", { "DOUX", "ROND", "CLAIR", "TRANCHANT" }, "wf");
    swLfoSrc     = addSwitch ("lfoWave",   { "SINUS", "TRI", "CARRE", "S&H" }, "lfoSrc");
    swDest       = addSwitch ("lfoDest",   { "PITCH", "FILTRE", "VOLUME", "DRIVE" }, "dest");
    swFilterType = addSwitch ("fltType",   { "PASSE-BAS", "BANDE", "PASSE-HAUT", "HORS SERVICE" }, "filterType");
    swMonoPoly   = addSwitch ("monoMode",  { "POLY", "MONO" }, "monoPoly");

    // ---- meters, cassette, piano ----
    vuL = addVu (true, "vuL");
    vuR = addVu (false, "vuR");

    // Un fichier depose dans le dossier utilisateur pendant la session doit
    // apparaitre a la prochaine ouverture de la fenetre, sans recharger le
    // plugin : la relecture precede la construction du selecteur.
    proc.rescanSamples();

    samplePicker = own (new SamplePicker (proc, apvts, lf));
    addAndMakeVisible (samplePicker);

    cassette = own (new ExpansionCassette (proc, lf));
    addAndMakeVisible (cassette);

    presetBar = own (new PresetBar (proc));
    addAndMakeVisible (presetBar);

    piano = own (new VirtualPiano (lf, "piano"));
    addAndMakeVisible (piano);
    piano->setRange (36, 96);
    piano->onNoteChange = [this] (int note, bool down)
    {
        auto& ks = proc.getKeyboardState();
        if (down)
            ks.noteOn (1, note, 1.0f);
        else
            ks.noteOff (1, note, 0.0f);
    };

    // ---- background image + layout ----
    loadLayout();

    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::faceplate_jpg,
                                                       BinaryData::faceplate_jpgSize);

    buildSprites();

    // Fenetre redimensionnable.
    //
    // Elle etait figee a 1120 x 836 : sur un ecran portable, ou dans un hote qui
    // n'offre pas d'ascenseur, une partie de la facade restait hors de vue et
    // rien ne permettait de la reduire. Tout est place en coordonnees de design
    // puis mis a l'echelle (voir place()), donc seul le ratio doit etre tenu -
    // sans lui l'image de fond se deformerait et les controles ne tomberaient
    // plus sur leur dessin.
    setResizable (true, true);
    if (auto* constrainer = getConstrainer())
    {
        constrainer->setFixedAspectRatio ((double) designW / (double) designH);
        constrainer->setSizeLimits ((int) (designW * 0.5f), (int) (designH * 0.5f),
                                    (int) (designW * 1.6f), (int) (designH * 1.6f));
    }

    // La taille choisie est retenue dans l'etat du plugin : la retrouver a la
    // reouverture fait partie du reglage, au meme titre qu'un potard.
    const int savedW = (int) apvts.state.getProperty ("uiWidth", (int) designW);
    const int savedH = (int) apvts.state.getProperty ("uiHeight", (int) designH);
    setSize (juce::jmax (320, savedW), juce::jmax (240, savedH));

    startTimerHz (30);
}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
void PluginEditor::loadLayout()
{
    struct Item
    {
        const char* id;
        float x, y, w, h;
    };

    static const Item def[] =
    {
        // bandeau de presets, cale sur le navigateur dessine dans l'image
        { "presetBar",   348,  42, 420,  52 },

        // cassette (top-left) + vu meters (bottom-right, free corner)
        { "cassette",     84,  48, 256, 212 },
        { "vuL",        1084, 620,  16, 140 },
        { "vuR",        1106, 620,  16, 140 },

        // sample selector strip (top-right, above the faders)
        { "samplePicker", 830, 100, 250,  46 },

        // LFO knobs
        { "lfoRate",    100, 322,  56,  56 },
        { "lfoAmount",  184, 322,  56,  56 },
        { "lfoPitch",   268, 322,  56,  56 },

        // centre GLIDE / FILTER (big knobs)
        { "glide",      450, 458,  72,  72 },
        { "filter",     634, 458,  72,  72 },

        // right knobs row 1
        { "vol",        830, 364,  56,  56 },
        { "reverb",     915, 364,  56,  56 },
        { "chorus",    1000, 364,  56,  56 },

        // right knobs row 2
        { "dist",       830, 462,  56,  56 },
        { "time",       915, 462,  56,  56 },
        { "fb",        1000, 462,  56,  56 },

        // ADSR vertical sliders
        { "attack",     121, 436,  10,  68 },
        { "decay",      179, 436,  10,  68 },
        { "sustain",    237, 436,  10,  68 },
        { "release",    295, 436,  10,  68 },

        // right faders
        { "fA",         843, 170,   8,  72 },
        { "fS",         888, 170,   8,  72 },
        { "fD",         932, 170,   8,  72 },
        { "fR",         977, 170,   8,  72 },
        { "fVel",      1032, 170,   8,  72 },

        // mod wheels
        { "pitchWheel", 121, 636,  29, 124 },
        { "modWheel",   224, 636,  29, 124 },

        // speaker grills (layer on/off)
        { "leftGrill",  384, 140,  64, 264 },
        { "rightGrill", 716, 140,  64, 264 },

        // dropdown combos
        { "wf",         372, 574,  93,  22 },
        { "lfoSrc",     510, 574, 104,  22 },
        { "dest",       644, 574, 102,  22 },
        { "filterType", 810, 574, 102,  22 },
        { "monoPoly",   948, 574,  96,  22 },

        // keyboard
        { "piano",      328, 620, 740, 216 }
    };

    for (auto& it : def)
        positions[it.id] = juce::Rectangle<float> (it.x, it.y, it.w, it.h);

    // La disposition livree est compilee dans le binaire ; un layout.json
    // depose dans le dossier utilisateur la remplace pour un reglage fin.
    {
        const auto override = getLayoutOverrideFile();
        auto parsed = override.existsAsFile()
            ? juce::JSON::parse (override)
            : juce::JSON::parse (juce::String::createStringFromData (BinaryData::layout_json,
                                                                     BinaryData::layout_jsonSize));

        if (auto* root = parsed.getDynamicObject())
        {
            if (auto* comps = root->getProperty ("components").getDynamicObject())
            {
                for (auto& prop : comps->getProperties())
                {
                    if (auto* r = prop.value.getDynamicObject())
                    {
                        positions[prop.name.toString()] = juce::Rectangle<float> (
                            (float) r->getProperty ("x"), (float) r->getProperty ("y"),
                            (float) r->getProperty ("w"), (float) r->getProperty ("h"));
                    }
                }
            }

            // optional sprite tuning: crop size / filmstrip frames / source box
            if (auto* spr = root->getProperty ("sprites").getDynamicObject())
            {
                for (auto& prop : spr->getProperties())
                    spriteOverrides[prop.name.toString()] = prop.value;
            }
        }
    }
}

//==============================================================================
// Sprite definitions: every control in the UI gets a crop out of the artwork,
// with optional overrides coming from layout.json ("sprites" section).
//==============================================================================
void PluginEditor::buildSprites()
{
    if (! backgroundImage.isValid())
        return;

    std::vector<RockpunkLookAndFeel::SpriteSpec> specs;

    auto addSpec = [this, &specs] (const juce::String& key, const juce::String& kind,
                                   float size, int frames, const juce::String& box)
    {
        RockpunkLookAndFeel::SpriteSpec s;
        s.key = key;
        s.kind = kind;
        s.size = size;
        s.frames = frames;
        s.box = box;

        // layout.json overrides
        auto it = spriteOverrides.find (key);
        if (it != spriteOverrides.end())
        {
            if (auto* o = it->second.getDynamicObject())
            {
                if (o->hasProperty ("kind"))   s.kind   = o->getProperty ("kind").toString();
                if (o->hasProperty ("size"))   s.size   = (float) o->getProperty ("size");
                if (o->hasProperty ("frames")) s.frames = (int) o->getProperty ("frames");
                if (o->hasProperty ("box"))    s.box    = o->getProperty ("box").toString();
                if (o->hasProperty ("cx"))     s.cx     = (float) o->getProperty ("cx");
                if (o->hasProperty ("cy"))     s.cy     = (float) o->getProperty ("cy");
            }
        }

        specs.push_back (s);
    };

    // knobs (rotating filmstrips, 16 frames; size/centre from layout.json)
    const juce::String knobKeys[] =
    {
        "lfoRate", "lfoAmount", "lfoPitch",
        "glide", "filter",
        "vol", "reverb", "chorus", "dist", "time", "fb"
    };
    for (auto& k : knobKeys)
        addSpec (k, "knob", 0.0f, 16, k);

    // vertical sliders + wheels (track crops)
    const juce::String sliderKeys[] =
    {
        "attack", "decay", "sustain", "release",
        "fA", "fS", "fD", "fR", "fVel",
        "pitchWheel", "modWheel"
    };
    for (auto& k : sliderKeys)
        addSpec (k, "slider", 0.0f, 1, k);

    // speaker grills (static crops, discrete on/off states)
    addSpec ("leftGrill",  "crop", 0.0f, 1, "leftGrill");
    addSpec ("rightGrill", "crop", 0.0f, 1, "rightGrill");

    // dropdown combos (static crops, discrete states)
    addSpec ("wf",         "crop", 0.0f, 1, "wf");
    addSpec ("lfoSrc",     "crop", 0.0f, 1, "lfoSrc");
    addSpec ("dest",       "crop", 0.0f, 1, "dest");
    addSpec ("filterType", "crop", 0.0f, 1, "filterType");
    addSpec ("monoPoly",   "crop", 0.0f, 1, "monoPoly");

    // straight crops: cassette panel + keyboard
    addSpec ("cassette", "crop", 0.0f, 1, "cassette");
    addSpec ("piano",    "crop", 0.0f, 1, "piano");

    lf.buildSprites (backgroundImage, positions, designW, designH, specs);
}

//==============================================================================
void PluginEditor::place (const juce::String& id, juce::Component& c)
{
    auto it = positions.find (id);
    if (it == positions.end())
        return;

    const auto r = it->second;
    const float sx = (float) getWidth() / designW;
    const float sy = (float) getHeight() / designH;

    c.setBounds (juce::Rectangle<int> ((int) (r.getX() * sx), (int) (r.getY() * sy),
                                       (int) (r.getWidth() * sx), (int) (r.getHeight() * sy)));
}

//==============================================================================
void PluginEditor::resized()
{
    apvts.state.setProperty ("uiWidth", getWidth(), nullptr);
    apvts.state.setProperty ("uiHeight", getHeight(), nullptr);

    place ("presetBar", *presetBar);
    place ("cassette", *cassette);
    place ("vuL", *vuL);
    place ("vuR", *vuR);
    place ("samplePicker", *samplePicker);

    place ("lfoRate", *knobLfoRate);
    place ("lfoAmount", *knobLfoAmount);
    place ("lfoPitch", *knobLfoPitch);
    place ("glide", *knobGlide);
    place ("filter", *knobFilter);
    place ("vol", *knobVol);
    place ("reverb", *knobReverb);
    place ("chorus", *knobChorus);
    place ("dist", *knobDist);
    place ("time", *knobTime);
    place ("fb", *knobFb);

    place ("attack", *slAttack);
    place ("decay", *slDecay);
    place ("sustain", *slSustain);
    place ("release", *slRelease);
    place ("fA", *slFA);
    place ("fS", *slFS);
    place ("fD", *slFD);
    place ("fR", *slFR);
    place ("fVel", *slFVel);
    place ("pitchWheel", *wheelPitch);
    place ("modWheel", *wheelMod);

    place ("leftGrill", *grillA);
    place ("rightGrill", *grillB);

    place ("wf", *swWf);
    place ("lfoSrc", *swLfoSrc);
    place ("dest", *swDest);
    place ("filterType", *swFilterType);
    place ("monoPoly", *swMonoPoly);

    place ("piano", *piano);
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
        g.drawImage (backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll (Palette::bg());
}

//==============================================================================
void PluginEditor::applyGlow (float g)
{
    for (auto* k : glowKnobs)
        k->setGlow01 (g);
    for (auto* s : glowSliders)
        s->setGlow01 (g);
    for (auto* sw : glowSwitches)
        sw->setGlow01 (g);
    for (auto* v : glowVus)
        v->setGlow01 (g);
    cassette->setGlow01 (g);
}

//==============================================================================
void PluginEditor::timerCallback()
{
    auto notes = proc.getActiveNotesSnapshot();

    bool newNote = false;
    for (int n : notes)
    {
        if (lastNotes.find (n) == lastNotes.end())
        {
            newNote = true;
            break;
        }
    }
    lastNotes = notes;

    if (newNote)
        flash = 1.0f;
    flash *= 0.82f;

    const float target = notes.empty() ? 0.0f : 1.0f;
    glow += (target - glow) * (target > glow ? 0.35f : 0.06f);
    const float g = juce::jlimit (0.0f, 1.0f, juce::jmax (glow, flash));

    applyGlow (g);

    piano->setNotesDown (notes);
    piano->setGlow01 (g);

    samplePicker->syncFromParam();

    for (auto* sw : glowSwitches)
    {
        sw->syncFromParam();
        sw->timerTick();
    }
    for (auto* s : glowSliders)
        s->timerTick();
    for (auto* v : glowVus)
        v->timerTick();
    cassette->timerTick();
    presetBar->setGlow01 (g);
    presetBar->timerTick();

    vuL->setLevel (proc.getOutputLevel (0));
    vuR->setLevel (proc.getOutputLevel (1));
}

} // namespace rk