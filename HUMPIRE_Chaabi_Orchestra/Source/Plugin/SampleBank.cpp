#include "SampleBank.h"
#include "Instruments.h"
#include <array>
#include <cmath>
#include <exception>
#include <map>
#include <mutex>

namespace humpire
{
std::shared_ptr<Bank> Bank::load (const juce::File& sfz, std::atomic<float>& progress,
                                  const std::function<bool()>& cancelled, juce::String& error)
{
    if (! sfz.existsAsFile())
    {
        error = "SFZ file not found: " + sfz.getFullPathName();
        return nullptr;
    }

    auto parsed = parseSfz (sfz);
    if (parsed.regions.empty())
    {
        error = "No valid region in " + sfz.getFileName();
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

    // distinct files
    std::map<juce::String, std::shared_ptr<SampleData>> cache;
    for (auto& r : parsed.regions) cache.emplace (r.sample.getFullPathName(), nullptr);

    const float total = (float) cache.size();
    float done = 0.f;
    size_t bytes = 0;

    for (auto& [path, sd] : cache)
    {
        if (cancelled()) { error = "Cancelled"; return nullptr; }

        juce::File f (path);
        std::unique_ptr<juce::AudioFormatReader> reader;
        if (f.existsAsFile()) reader.reset (fm.createReaderFor (f));

        if (reader != nullptr && reader->lengthInSamples > 0 && reader->numChannels > 0 && reader->sampleRate > 0.0)
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
            bank->warnings.add ("Unreadable sample: " + f.getFileName());

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
        error = "No readable sample (check the samples folder)";
        return nullptr;
    }

    bank->memoryMB = (double) bytes / (1024.0 * 1024.0);
    return bank;
}

// ------------------------------------------------------------------------------ built-in modelled library
std::shared_ptr<Bank> Bank::fromLibrary (const model::Library& lib, juce::String& error)
{
    auto bank = std::make_shared<Bank>();
    bank->builtIn = true;
    bank->name = "Built-in modelled " + juce::String (lib.instrumentId);
    bank->swLo = lib.swLo;
    bank->swHi = lib.swHi;
    bank->swDefault = lib.swDefault;

    std::vector<std::shared_ptr<SampleData>> audio (lib.zones.size());
    size_t bytes = 0;
    int skipped = 0;

    for (size_t i = 0; i < lib.zones.size(); ++i)
    {
        const auto& z = lib.zones[i];
        const auto& owner = lib.audioZone (i);
        const size_t nch = (size_t) owner.numChannels;
        const bool ownerOk = (nch == 1 || nch == 2) && ! owner.data.empty() && owner.data.size() % nch == 0 && owner.sampleRate > 1000.0;

        if (! ownerOk || z.lokey > z.hikey) { ++skipped; continue; }

        if (z.sharedWith < 0)
        {
            const int frames = (int) (owner.data.size() / nch);
            auto sd = std::make_shared<SampleData>();
            sd->audio.setSize ((int) nch, frames);
            for (size_t c = 0; c < nch; ++c)
            {
                float* dst = sd->audio.getWritePointer ((int) c);
                for (int n = 0; n < frames; ++n)
                {
                    const float v = owner.data[(size_t) n * nch + c];
                    dst[n] = std::isfinite (v) ? v : 0.f;
                }
            }
            sd->sampleRate = owner.sampleRate;
            sd->numFrames = frames;
            bytes += (size_t) frames * nch * sizeof (float);
            audio[i] = std::move (sd);
        }
        else
        {
            audio[i] = audio[(size_t) z.sharedWith];       // alias: shares the data of a lower zone
            if (audio[i] == nullptr) { ++skipped; continue; }
        }

        SfzRegionDef d;
        d.lokey = juce::jlimit (0, 127, z.lokey);
        d.hikey = juce::jlimit (0, 127, z.hikey);
        d.keycenter = juce::jlimit (0, 127, z.keycenter);
        d.lovel = juce::jlimit (0, 127, z.lovel);
        d.hivel = juce::jlimit (0, 127, z.hivel);
        d.seqLength = juce::jmax (1, z.seqLength);
        d.seqPosition = juce::jmax (1, z.seqPosition);
        d.volumeDb = z.volumeDb;
        d.pan = juce::jlimit (-100.f, 100.f, z.pan);
        d.tuneCents = z.tuneCents;
        d.pitchKeytrack = z.pitchKeytrack;
        switch (z.loop)
        {
            case model::Loop::oneShot:    d.loop = SfzRegionDef::Loop::oneShot; break;
            case model::Loop::continuous: d.loop = SfzRegionDef::Loop::continuous; break;
            case model::Loop::sustain:    d.loop = SfzRegionDef::Loop::sustain; break;
            default:                      d.loop = SfzRegionDef::Loop::noLoop; break;
        }
        d.loopStart = z.loopStart;
        d.loopEnd = z.loopEnd;
        d.ampAttack = juce::jmax (0.f, z.ampAttack);
        d.ampDecay = juce::jmax (0.f, z.ampDecay);
        d.ampSustain = juce::jlimit (0.f, 100.f, z.ampSustain);
        d.ampRelease = juce::jmax (0.003f, z.ampRelease);
        d.ampVeltrack = juce::jlimit (0.f, 100.f, z.ampVeltrack);
        d.group = z.group;
        d.offBy = z.offBy;
        d.triggerRelease = z.triggerRelease;
        d.swLast = z.swLast;

        if (d.triggerRelease) bank->hasReleaseRegions = true;
        bank->regions.add (new Region (std::move (d), audio[i]));
    }

    if (skipped > 0) bank->warnings.add (juce::String (skipped) + " invalid zone(s) of the built-in library were skipped");
    if (bank->regions.isEmpty())
    {
        error = "The built-in library of '" + juce::String (lib.instrumentId) + "' produced no playable zone";
        return nullptr;
    }
    bank->memoryMB = (double) bytes / (1024.0 * 1024.0);
    return bank;
}

namespace
{
struct ModelCache
{
    std::mutex mutex;
    std::array<std::weak_ptr<Bank>, kNumInstruments> banks;
    std::array<double, kNumInstruments> rates {};
};

ModelCache& modelCache()
{
    static ModelCache cache;       // thread-safe initialisation (C++11 magic statics)
    return cache;
}
} // namespace

std::shared_ptr<Bank> Bank::buildFromModel (int instrumentIndex, double sampleRate, std::atomic<float>& progress,
                                            const std::function<bool()>& cancelled, juce::String& error)
{
    if (instrumentIndex < 0 || instrumentIndex >= kNumInstruments)
    {
        error = "Unknown instrument";
        return nullptr;
    }

    auto& cache = modelCache();
    {
        const std::lock_guard<std::mutex> lock (cache.mutex);
        auto& slot = cache.banks[(size_t) instrumentIndex];
        if (auto hit = slot.lock(); hit != nullptr && cache.rates[(size_t) instrumentIndex] == sampleRate)
        {
            progress.store (1.0f);
            return hit;
        }
    }

    try
    {
        bool wasCancelled = false;
        auto library = model::build (instrumentIndex, sampleRate, [&] (float f)
        {
            progress.store (0.9f * f);
            if (cancelled()) { wasCancelled = true; return false; }
            return true;
        });
        if (wasCancelled || library.cancelled || cancelled())
        {
            error = "Cancelled";
            return nullptr;
        }

        // Safety net: a renderer bug must never crash or silently produce an unplayable bank.
        const auto report = model::validate (library, instrumentIndex);

        auto bank = fromLibrary (library, error);
        if (bank == nullptr) return nullptr;
        for (auto& e : report.errors) bank->warnings.add ("Built-in library check: " + juce::String (e));
        bank->name = "Built-in modelled " + instrumentName (instrumentIndex);
        progress.store (1.0f);

        const std::lock_guard<std::mutex> lock (cache.mutex);
        auto& slot = cache.banks[(size_t) instrumentIndex];
        if (auto existing = slot.lock(); existing != nullptr && cache.rates[(size_t) instrumentIndex] == sampleRate)
            return existing;                                  // another instance finished first: share its bank
        slot = bank;
        cache.rates[(size_t) instrumentIndex] = sampleRate;
        return bank;
    }
    catch (const std::bad_alloc&)
    {
        error = "Not enough memory to build the built-in instrument";
    }
    catch (const std::exception& e)
    {
        error = "Could not build the built-in instrument: " + juce::String (e.what());
    }
    catch (...)
    {
        error = "Could not build the built-in instrument";
    }
    return nullptr;
}
} // namespace humpire
