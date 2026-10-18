#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <memory>
#include <vector>

namespace rk
{

//==============================================================================
// The sampled layer of ROCKPUNK.
//
// ROCKPUNK ships no recording: its instrument is the two modelled strings. This
// layer plays the files that the **user** drops in
// Documents/DJBILBOX/ROCKPUNK/Samples - their own takes, or a library they hold
// the licence for.
//
// The bank format (.rockpunkbank: an index followed by FLAC blocks end to end)
// is still read if one is installed, which is what allows a licensed bank to be
// shipped one day without touching the plug-in. The file is memory-mapped, so a
// large bank costs nothing until a sound is requested.
//
// Thread safety: the entry list is rebuilt on the message thread (rescan) while
// a background thread decodes files, so every access to it goes through `lock`.
// Nothing here is ever called from the audio thread.
//==============================================================================
class SampleBank
{
public:
    struct Entry
    {
        juce::String name, category;
        float        root = 60.0f;      // measured fundamental (bank), or 0 = to be measured
        juce::int64  offset = 0, size = 0;
        juce::File   file;              // set in folder mode only
    };

    SampleBank() { formats.registerBasicFormats(); }

    //==========================================================================
    // The folder where the user drops their guitars.
    static juce::File getUserSampleDirectory()
    {
        return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
            .getChildFile ("DJBILBOX")
            .getChildFile ("ROCKPUNK")
            .getChildFile ("Samples");
    }

    //==========================================================================
    int openDefault()
    {
        const juce::ScopedLock sl (lock);
        const auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);

        // An installed bank wins: it is chosen content, whereas the user folder
        // can contain anything.
        const juce::File bankDirs[] =
        {
            exe.getParentDirectory().getParentDirectory()
               .getChildFile ("Resources").getChildFile ("SampleBank"),
            exe.getParentDirectory().getChildFile ("SampleBank"),
            juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
               .getChildFile ("DJBILBOX").getChildFile ("ROCKPUNK").getChildFile ("SampleBank"),
            juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("DJBILBOX").getChildFile ("ROCKPUNK").getChildFile ("SampleBank"),
        };

        for (const auto& dir : bankDirs)
        {
            const auto bank = dir.getChildFile ("ROCKPUNK.rockpunkbank");
            if (bank.existsAsFile() && open (bank) > 0)
                return (int) entries.size();
        }

        return scanFolder (getUserSampleDirectory());
    }

    //==========================================================================
    // Folder mode: everything the user dropped, sub-folders included. The
    // first-level sub-folder is the category, so everyone can file their takes
    // however they like.
    int scanFolder (const juce::File& root)
    {
        const juce::ScopedLock sl (lock);
        entries.clear();
        map.reset();
        dataStart = 0;
        folderRoot = root;

        if (! root.isDirectory())
            return 0;

        for (const auto& item : juce::RangedDirectoryIterator (root, true, "*.wav;*.aif;*.aiff;*.flac",
                                                               juce::File::findFiles))
        {
            const auto f = item.getFile();

            Entry e;
            e.name = f.getFileNameWithoutExtension();
            e.file = f;
            e.root = 0.0f;              // measured on first load

            const auto relative = f.getParentDirectory().getRelativePathFrom (root);
            e.category = relative.upToFirstOccurrenceOf (juce::File::getSeparatorString(), false, false);
            if (e.category.isEmpty() || e.category == ".")
                e.category = "My samples";

            entries.push_back (std::move (e));
        }

        std::sort (entries.begin(), entries.end(), [] (const Entry& a, const Entry& b)
        {
            if (a.category != b.category)
                return a.category < b.category;
            return a.name < b.name;
        });

        return (int) entries.size();
    }

    //==========================================================================
    int open (const juce::File& bankFile)
    {
        const juce::ScopedLock sl (lock);
        entries.clear();
        map.reset();
        dataStart = 0;
        folderRoot = juce::File();

        juce::FileInputStream in (bankFile);
        if (! in.openedOk())
            return 0;

        if (in.readInt() != 0x4B425052)   // 'RPBK'
            return 0;
        if (in.readInt() != 1)            // refuse a format we cannot read
            return 0;

        const int count = in.readInt();
        if (count <= 0 || count > 100000)
            return 0;

        std::vector<Entry> parsed;
        parsed.reserve ((size_t) count);

        for (int i = 0; i < count; ++i)
        {
            Entry e;
            e.name     = in.readString();
            e.category = in.readString();
            e.root     = in.readFloat();
            e.offset   = in.readInt64();
            e.size     = in.readInt64();

            if (e.name.isEmpty() || e.size <= 0)
                return 0;

            parsed.push_back (std::move (e));
        }

        dataStart = in.readInt64();
        if (dataStart <= 0 || dataStart >= bankFile.getSize())
            return 0;

        map = std::make_shared<juce::MemoryMappedFile> (bankFile, juce::MemoryMappedFile::readOnly);
        if (map->getData() == nullptr)
        {
            map.reset();
            return 0;
        }

        entries = std::move (parsed);
        return (int) entries.size();
    }

    //==========================================================================
    int getNumEntries() const noexcept { const juce::ScopedLock sl (lock); return (int) entries.size(); }
    bool isEmpty() const noexcept { const juce::ScopedLock sl (lock); return entries.empty(); }
    bool isUserFolder() const noexcept { const juce::ScopedLock sl (lock); return folderRoot != juce::File(); }
    juce::File getFolderRoot() const { const juce::ScopedLock sl (lock); return folderRoot; }

    // A copy, never a pointer: the list can be rebuilt by another thread.
    bool getEntryInfo (int index, Entry& out) const
    {
        const juce::ScopedLock sl (lock);
        if (index < 0 || index >= (int) entries.size())
            return false;
        out = entries[(size_t) index];
        return true;
    }

    juce::StringArray getNames() const
    {
        const juce::ScopedLock sl (lock);
        juce::StringArray names;
        for (const auto& e : entries)
            names.add (e.name);
        return names;
    }

    //==========================================================================
    // Decodes a sample into memory. Runs on the background loader thread (or on
    // the message thread while preparing), never on the audio thread; the
    // result is then shared by every voice through a shared_ptr.
    std::shared_ptr<juce::AudioBuffer<float>> load (int index, double& fileRate, float& rootNote)
    {
        Entry e;
        std::shared_ptr<juce::MemoryMappedFile> mapped;
        juce::int64 start = 0;
        {
            const juce::ScopedLock sl (lock);
            if (! getEntryInfo (index, e))
                return nullptr;
            mapped = map;
            start = dataStart;
        }

        std::unique_ptr<juce::AudioFormatReader> reader;

        if (e.file != juce::File())
        {
            reader.reset (formats.createReaderFor (e.file));
        }
        else if (mapped != nullptr)
        {
            const auto* base = static_cast<const char*> (mapped->getData());
            if (base == nullptr || start + e.offset + e.size > (juce::int64) mapped->getSize())
                return nullptr;

            auto stream = std::make_unique<juce::MemoryInputStream> (base + start + e.offset,
                                                                     (size_t) e.size, false);
            reader.reset (formats.createReaderFor (std::move (stream)));
        }

        if (reader == nullptr || reader->lengthInSamples <= 0)
            return nullptr;

        // A file dropped by the user can last any time: the read is capped so a
        // ten-minute loop does not fill the host's memory.
        const int numSamples = (int) juce::jmin<juce::int64> (reader->lengthInSamples,
                                                              (juce::int64) (reader->sampleRate * 30.0));

        auto buffer = std::make_shared<juce::AudioBuffer<float>> ((int) juce::jmin<juce::uint32> (2, reader->numChannels),
                                                                  numSamples);
        reader->read (buffer.get(), 0, numSamples, 0, true, true);

        fileRate = reader->sampleRate > 0.0 ? reader->sampleRate : 44100.0;

        // The bank carries the fundamental in its index; a loose file must be
        // measured, once, and the result kept.
        if (e.root > 0.0f)
        {
            rootNote = e.root;
        }
        else
        {
            rootNote = detectRootNote (*buffer, fileRate);
            const juce::ScopedLock sl (lock);
            if (index < (int) entries.size() && entries[(size_t) index].name == e.name)
                entries[(size_t) index].root = rootNote;
        }

        return buffer;
    }

    //==========================================================================
    // Autocorrelation over the first ~250 ms.
    static float detectRootNote (const juce::AudioBuffer<float>& buf, double sampleRate)
    {
        const int winLen = juce::jmin (buf.getNumSamples(), (int) (sampleRate * 0.25));
        const int loLag  = juce::jmax (1, (int) (sampleRate / 900.0));
        const int hiLag  = juce::jmax (loLag + 1, (int) (sampleRate / 55.0));

        if (winLen < hiLag + 8)
            return 60.0f;

        juce::HeapBlock<float> seg (winLen);
        float sum = 0.0f, sumSq = 0.0f;
        for (int i = 0; i < winLen; ++i)
        {
            const float s = buf.getSample (0, i);
            seg[i] = s;
            sum   += s;
            sumSq += s * s;
        }

        const float mean = sum / (float) winLen;
        const float var  = (sumSq / (float) winLen) - mean * mean;
        if (var < 1.0e-7f)
            return 60.0f;

        for (int i = 0; i < winLen; ++i)
            seg[i] -= mean;

        float bestVal = 0.0f;
        int   bestLag = 0;
        for (int lag = loLag; lag < hiLag; ++lag)
        {
            float v = 0.0f;
            for (int i = 0; i < winLen - lag; ++i)
                v += seg[i] * seg[i + lag];
            if (v > bestVal)
            {
                bestVal = v;
                bestLag = lag;
            }
        }

        if (bestLag <= 0)
            return 60.0f;

        const float freq = (float) (sampleRate / (double) bestLag);
        return 69.0f + 12.0f * std::log2 (freq / 440.0f);
    }

private:
    mutable juce::CriticalSection lock;
    juce::AudioFormatManager formats;
    std::vector<Entry> entries;
    std::shared_ptr<juce::MemoryMappedFile> map;
    juce::int64 dataStart = 0;
    juce::File  folderRoot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleBank)
};

} // namespace rk
