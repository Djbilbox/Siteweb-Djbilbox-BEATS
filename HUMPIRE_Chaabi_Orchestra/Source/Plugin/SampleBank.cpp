#include "SampleBank.h"
#include <map>

namespace humpire
{
std::shared_ptr<Bank> Bank::load (const juce::File& sfz, std::atomic<float>& progress,
                                  const std::function<bool()>& cancelled, juce::String& error)
{
    if (! sfz.existsAsFile())
    {
        error = juce::String::fromUTF8 ("SFZ introuvable : ") + sfz.getFullPathName();
        return nullptr;
    }

    auto parsed = parseSfz (sfz);
    if (parsed.regions.empty())
    {
        error = juce::String::fromUTF8 ("Aucune r\xC3\xA9gion valide dans ") + sfz.getFileName();
        return nullptr;
    }

    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    auto bank = std::make_shared<Bank>();
    bank->name = sfz.getFileNameWithoutExtension();
    bank->swLo = parsed.swLo;
    bank->swHi = parsed.swHi;
    bank->swDefault = parsed.swDefault;
    bank->warnings = parsed.warnings;

    // fichiers distincts
    std::map<juce::String, std::shared_ptr<SampleData>> cache;
    for (auto& r : parsed.regions) cache.emplace (r.sample.getFullPathName(), nullptr);

    const float total = (float) cache.size();
    float done = 0.f;
    size_t bytes = 0;

    for (auto& [path, sd] : cache)
    {
        if (cancelled()) { error = "Annule"; return nullptr; }

        juce::File f (path);
        if (std::unique_ptr<juce::AudioFormatReader> reader { fm.createReaderFor (f) })
        {
            const int len = (int) juce::jmin<juce::int64> (reader->lengthInSamples, 0x7fffffff);
            const int nch = juce::jlimit (1, 2, (int) reader->numChannels);
            auto data = std::make_shared<SampleData>();
            data->audio.setSize (nch, len);
            reader->read (&data->audio, 0, len, 0, true, nch > 1);
            data->sampleRate = reader->sampleRate;
            data->numFrames = len;
            bytes += (size_t) len * (size_t) nch * sizeof (float);
            sd = std::move (data);
        }
        else
            bank->warnings.add ("Sample illisible : " + f.getFileName());

        progress.store (++done / total);
    }

    for (auto& d : parsed.regions)
    {
        auto it = cache.find (d.sample.getFullPathName());
        if (it == cache.end() || it->second == nullptr) continue;
        if (d.triggerRelease) bank->hasReleaseRegions = true;
        bank->regions.add (new Region (d, it->second));
    }

    if (bank->regions.isEmpty())
    {
        error = juce::String::fromUTF8 ("Aucun sample lisible (v\xC3\xA9rifiez le dossier de samples)");
        return nullptr;
    }

    bank->memoryMB = (double) bytes / (1024.0 * 1024.0);
    return bank;
}
} // namespace humpire
