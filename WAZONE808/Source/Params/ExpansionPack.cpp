#include "ExpansionPack.h"

namespace
{
    double num(const juce::var& v, const char* key, double fallback)
    {
        auto x = v[key];
        return x.isVoid() ? fallback : (double) x;
    }

    int inum(const juce::var& v, const char* key, int fallback)
    {
        auto x = v[key];
        return x.isVoid() ? fallback : (int) x;
    }
}

static bool parsePack(const juce::var& parsed, ExpansionPack& outPack);

bool loadExpansionPackFromFile(const juce::File& file, ExpansionPack& outPack)
{
    if (!file.existsAsFile())
        return false;
    return parsePack(juce::JSON::parse(file), outPack);
}

bool loadExpansionPackFromString(const juce::String& json, ExpansionPack& outPack)
{
    return parsePack(juce::JSON::parse(json), outPack);
}

static bool parsePack(const juce::var& parsed, ExpansionPack& outPack)
{
    if (!parsed.isObject())
        return false;

    if (parsed.getProperty("format", "").toString() != "warzone808-expansion")
        return false;

    auto* presetsArray = parsed.getProperty("presets", juce::var()).getArray();
    if (presetsArray == nullptr || presetsArray->isEmpty())
        return false;

    ExpansionPack pack;
    pack.name = parsed.getProperty("name", "Expansion").toString();
    pack.author = parsed.getProperty("author", "").toString();

    for (const auto& item : *presetsArray)
    {
        if (!item.isObject())
            continue;

        RuntimePreset p;
        p.name = item.getProperty("name", "Untitled").toString();
        p.category = item.getProperty("category", "Lead").toString();

        p.oscWaveform      = inum(item, "oscWaveform", 0);
        p.filterCutoffHz   = num(item, "filterCutoffHz", 8000.0);
        p.filterResonance  = num(item, "filterResonance", 1.0);
        p.filterType       = inum(item, "filterType", 0);
        p.filterEnvDepthOct = num(item, "filterEnvDepthOct", 0.0);

        p.attackMs    = num(item, "attackMs", 10.0);
        p.decayMs     = num(item, "decayMs", 200.0);
        p.sustainLevel = num(item, "sustainLevel", 0.7);
        p.releaseMs   = num(item, "releaseMs", 300.0);
        p.glideMs     = num(item, "glideMs", 0.0);

        p.lfoRateHz   = num(item, "lfoRateHz", 2.0);
        p.lfoAmount   = num(item, "lfoAmount", 0.0);
        p.lfoDestination = inum(item, "lfoDestination", 0);

        p.distortionDriveDb = num(item, "distortionDriveDb", 0.0);
        p.distortionToneHz  = num(item, "distortionToneHz", 5000.0);
        p.distortionMix     = num(item, "distortionMix", 0.0);

        p.chorusRateHz = num(item, "chorusRateHz", 1.5);
        p.chorusAmount = num(item, "chorusAmount", 0.0);
        p.chorusMix    = num(item, "chorusMix", 0.0);

        p.delayTimeMs  = num(item, "delayTimeMs", 250.0);
        p.delayFeedback = num(item, "delayFeedback", 0.0);
        p.delayMix     = num(item, "delayMix", 0.0);

        p.reverbRoom    = num(item, "reverbRoom", 0.3);
        p.reverbDamping = num(item, "reverbDamping", 0.5);
        p.reverbWidth   = num(item, "reverbWidth", 1.0);
        p.reverbMix     = num(item, "reverbMix", 0.2);

        p.masterVolume  = num(item, "masterVolume", 1.0);

        p.filterAttackMs = num(item, "filterAttackMs", 1.0);
        p.filterDecayMs  = num(item, "filterDecayMs", -1.0);
        p.filterSustain  = num(item, "filterSustain", -1.0);
        p.velocity       = num(item, "velocity", 0.4);

        p.unisonVoices  = inum(item, "unisonVoices", 1);
        p.unisonDetune  = num(item, "unisonDetune", 0.0);
        p.unisonWidth   = num(item, "unisonWidth", 0.0);

        pack.presets.push_back(std::move(p));
    }

    if (pack.presets.empty())
        return false;

    outPack = std::move(pack);
    return true;
}
