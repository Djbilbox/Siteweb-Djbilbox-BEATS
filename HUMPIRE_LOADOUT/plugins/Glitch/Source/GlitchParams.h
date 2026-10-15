#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <HumpireCore/PresetManager.h>

// ============================================================================
// HUMPIRE GLITCH : identifiants, layout APVTS et presets d'usine.
// Ne JAMAIS renommer un ID apres release (compat. projets DAW / automation).
// ============================================================================
namespace glitch
{
namespace id
{
    inline constexpr const char* bits    = "bits";      // 1..16 (reel, fractionnaire autorise)
    inline constexpr const char* rate    = "rate";      // frequence d'echantillonnage cible (Hz)
    inline constexpr const char* drive   = "drive";     // saturation pre-crush 0..1
    inline constexpr const char* speaker = "speaker";   // filtre haut-parleur mono ON/OFF
    inline constexpr const char* tone    = "tone";      // taille du haut-parleur 0 (mini) .. 1 (large)
    inline constexpr const char* dither  = "dither";    // Off / RPDF / TPDF
    inline constexpr const char* mix     = "mix";       // dry/wet 0..1
    inline constexpr const char* out     = "out";       // gain de sortie dB
    inline constexpr const char* bypass  = "bypass";    // bypass hote (getBypassParameter)
    inline constexpr const char* simple  = "simple";    // UI : mode Simple (3 controles) / Avance
    inline constexpr const char* clean   = "clean";     // UI : mode Clean (plat) / Skeuo
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    using P = AudioProcessorValueTreeState::ParameterLayout;
    P layout;

    auto ver = [] (const char* s) { return ParameterID { s, 1 }; };

    layout.add (std::make_unique<AudioParameterFloat> (ver (id::bits), "Bits",
        NormalisableRange<float> (1.0f, 16.0f, 0.01f), 8.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " bit"; })));

    layout.add (std::make_unique<AudioParameterFloat> (ver (id::rate), "Sample Rate",
        NormalisableRange<float> (500.0f, 48000.0f, 1.0f, 0.3f), 48000.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
            { return v >= 1000.0f ? String (v / 1000.0f, 1) + " kHz" : String ((int) v) + " Hz"; })
            .withValueFromStringFunction ([] (const String& t) { const float x = t.getFloatValue(); return t.containsIgnoreCase ("k") ? x * 1000.0f : x; })));

    layout.add (std::make_unique<AudioParameterFloat> (ver (id::drive), "Drive",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; })
        .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue() / 100.0f; })));

    layout.add (std::make_unique<AudioParameterBool> (ver (id::speaker), "Speaker", false));

    layout.add (std::make_unique<AudioParameterFloat> (ver (id::tone), "Speaker Size",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.4f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; })
        .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue() / 100.0f; })));

    layout.add (std::make_unique<AudioParameterChoice> (ver (id::dither), "Dither",
        StringArray { "Off", "RPDF", "TPDF" }, 0));

    layout.add (std::make_unique<AudioParameterFloat> (ver (id::mix), "Mix",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; })
        .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue() / 100.0f; })));

    layout.add (std::make_unique<AudioParameterFloat> (ver (id::out), "Output",
        NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dB"; })));

    layout.add (std::make_unique<AudioParameterBool> (ver (id::bypass), "Bypass", false));

    // Preferences d'interface : sauvegardees dans l'etat, non automatisables.
    layout.add (std::make_unique<AudioParameterBool> (ver (id::simple), "UI Simple", true,
        AudioParameterBoolAttributes().withAutomatable (false)));
    layout.add (std::make_unique<AudioParameterBool> (ver (id::clean), "UI Clean", false,
        AudioParameterBoolAttributes().withAutomatable (false)));

    return layout;
}

// ---------------------------------------------------------------- presets d'usine (20)
inline std::vector<humpire::FactoryPreset> factoryPresets()
{
    using namespace id;
    auto P = [] (const char* name, const char* cat, float b, float r, float drv, bool spk, float tn, int dth, float mx, float o)
    {
        return humpire::FactoryPreset { name, cat, {
            { bits, b }, { rate, r }, { drive, drv }, { speaker, spk ? 1.0f : 0.0f },
            { tone, tn }, { dither, (float) dth }, { mix, mx }, { out, o } } };
    };

    return {
        //   nom                         categorie       bits  rate     drv   spk   tone  dth  mix   out
        P ("Radio Crush",               "Trap Vocals",    10.f, 16000.f, 0.20f, true,  0.35f, 2, 1.00f,  0.f),
        P ("Telephone Ad-Lib",          "Trap Vocals",    12.f, 8000.f,  0.10f, true,  0.05f, 1, 1.00f,  0.f),
        P ("Dusty Vocal Chop",          "Trap Vocals",    9.f,  22050.f, 0.15f, false, 0.40f, 2, 0.70f, -1.f),
        P ("Megaphone Hook",            "Trap Vocals",    11.f, 12000.f, 0.45f, true,  0.15f, 0, 1.00f, -2.f),
        P ("Dirty Sub 12bit",           "808",          12.f, 32000.f, 0.25f, false, 0.40f, 2, 1.00f, -1.f),
        P ("808 Grit",                  "808",          8.f,  44100.f, 0.40f, false, 0.40f, 0, 0.60f, -2.f),
        P ("Sampler Crunch 808",        "808",          10.f, 26000.f, 0.30f, false, 0.40f, 1, 0.80f, -1.f),
        P ("8-Bit Punch",               "Kick",         8.f,  24000.f, 0.20f, false, 0.40f, 0, 0.65f,  0.f),
        P ("Boom Bap Dust",             "Kick",         12.f, 20000.f, 0.10f, false, 0.40f, 2, 0.50f,  0.f),
        P ("Hard Clip Kick",            "Kick",         6.f,  48000.f, 0.60f, false, 0.40f, 0, 0.40f, -3.f),
        P ("Cassette Oud",              "Oud",          13.f, 18000.f, 0.05f, true,  0.55f, 2, 0.55f,  0.f),
        P ("Old Souk Radio",            "Oud",          11.f, 11025.f, 0.10f, true,  0.25f, 2, 0.85f, -1.f),
        P ("Ghost Strings",             "Oud",          14.f, 14000.f, 0.00f, false, 0.40f, 2, 0.35f,  0.f),
        P ("Darbouka Alley",            "Darbouka",     9.f,  16000.f, 0.25f, true,  0.45f, 1, 0.60f,  0.f),
        P ("Tin Can Tabla",             "Darbouka",     7.f,  12000.f, 0.15f, true,  0.20f, 0, 0.70f, -1.f),
        P ("Crushed Drum Bus",          "Bus Drums",    10.f, 30000.f, 0.35f, false, 0.40f, 2, 0.30f,  0.f),
        P ("Parallel Dirt",             "Bus Drums",    8.f,  22050.f, 0.50f, false, 0.40f, 1, 0.25f, -1.f),
        P ("Lo-Fi Loop Glue",           "Bus Drums",    12.f, 26000.f, 0.10f, true,  0.60f, 2, 0.45f,  0.f),
        P ("Master Tape Hint",          "Master",       15.f, 44100.f, 0.05f, false, 0.40f, 2, 0.15f,  0.f),
        P ("Old Console Preview",       "Master",       4.f,  8000.f,  0.30f, true,  0.00f, 0, 1.00f, -4.f),
    };
}
} // namespace glitch
