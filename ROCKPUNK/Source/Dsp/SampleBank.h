#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <memory>
#include <vector>

namespace rk

{

//==============================================================================
// La couche echantillonnee de ROCKPUNK.
//
// ROCKPUNK ne livre aucun enregistrement : son instrument, ce sont les deux
// cordes modelisees. Cette couche joue les fichiers que **l'utilisateur** depose
// dans Documents/DJBILBOX/ROCKPUNK/Samples - ses propres prises, ou une
// bibliotheque dont il a la licence.
//
// Le format de banque (.rockpunkbank, un index puis des blocs FLAC bout a bout)
// reste lu s'il en existe une : c'est ce qui permet de livrer un jour une
// banque signee sans retoucher le plugin. Le fichier est alors projete en
// memoire plutot que lu, donc une grosse banque ne coute rien tant qu'aucun son
// n'est demande.
//==============================================================================
class SampleBank
{
public:
    struct Entry
    {
        juce::String name, category;
        float        root = 60.0f;      // fondamentale mesuree (banque) ou 0 = a mesurer
        juce::int64  offset = 0, size = 0;
        juce::File   file;              // renseigne en mode dossier uniquement
    };

    SampleBank() { formats.registerBasicFormats(); }

    //==========================================================================
    // Le dossier ou l'utilisateur depose ses guitares.
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
        const auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);

        // Une banque installee l'emporte : c'est du contenu choisi, alors que le
        // dossier utilisateur peut contenir n'importe quoi.
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
    // Mode dossier : tout ce que l'utilisateur a depose, sous-dossiers compris.
    // Le sous-dossier de premier niveau sert de categorie, ce qui laisse chacun
    // ranger ses prises comme il veut.
    int scanFolder (const juce::File& root)
    {
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
            e.root = 0.0f;              // mesuree au premier chargement

            const auto relative = f.getParentDirectory().getRelativePathFrom (root);
            e.category = relative.upToFirstOccurrenceOf (juce::File::getSeparatorString(), false, false);
            if (e.category.isEmpty() || e.category == ".")
                e.category = "Mes samples";

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
        entries.clear();
        map.reset();
        dataStart = 0;
        folderRoot = juce::File();

        juce::FileInputStream in (bankFile);
        if (! in.openedOk())
            return 0;

        if (in.readInt() != 0x4B425052)   // 'RPBK'
            return 0;
        if (in.readInt() != 1)            // refuse un format qu'on ne sait pas lire
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

        map = std::make_unique<juce::MemoryMappedFile> (bankFile, juce::MemoryMappedFile::readOnly);
        if (map->getData() == nullptr)
        {
            map.reset();
            return 0;
        }

        entries = std::move (parsed);
        return (int) entries.size();
    }

    //==========================================================================
    int getNumEntries() const noexcept { return (int) entries.size(); }
    bool isEmpty() const noexcept { return entries.empty(); }
    bool isUserFolder() const noexcept { return folderRoot != juce::File(); }
    juce::File getFolderRoot() const { return folderRoot; }

    const Entry* getEntry (int index) const noexcept
    {
        if (index < 0 || index >= (int) entries.size())
            return nullptr;
        return &entries[(size_t) index];
    }

    juce::StringArray getNames() const
    {
        juce::StringArray names;
        for (const auto& e : entries)
            names.add (e.name);
        return names;
    }

    //==========================================================================
    // Decode un echantillon en memoire. Appele hors du thread audio (depuis
    // processBlock la premiere fois qu'une selection change), le resultat etant
    // ensuite partage par toutes les voix via un shared_ptr.
    std::shared_ptr<juce::AudioBuffer<float>> load (int index, double& fileRate, float& rootNote)
    {
        const auto* e = getEntry (index);
        if (e == nullptr)
            return nullptr;

        std::unique_ptr<juce::AudioFormatReader> reader;

        if (e->file != juce::File())
        {
            reader.reset (formats.createReaderFor (e->file));
        }
        else if (map != nullptr)
        {
            const auto* base = static_cast<const char*> (map->getData());
            if (base == nullptr || dataStart + e->offset + e->size > (juce::int64) map->getSize())
                return nullptr;

            auto stream = std::make_unique<juce::MemoryInputStream> (base + dataStart + e->offset,
                                                                     (size_t) e->size, false);
            reader.reset (formats.createReaderFor (std::move (stream)));
        }

        if (reader == nullptr || reader->lengthInSamples <= 0)
            return nullptr;

        // Un fichier depose par l'utilisateur peut durer n'importe quoi : on
        // borne la lecture pour qu'une boucle de dix minutes ne remplisse pas la
        // memoire de l'hote.
        const int numSamples = (int) juce::jmin<juce::int64> (reader->lengthInSamples,
                                                              (juce::int64) (reader->sampleRate * 30.0));

        auto buffer = std::make_shared<juce::AudioBuffer<float>> ((int) juce::jmin<juce::uint32> (2, reader->numChannels),
                                                                  numSamples);
        reader->read (buffer.get(), 0, numSamples, 0, true, true);

        fileRate = reader->sampleRate > 0.0 ? reader->sampleRate : 44100.0;

        // La banque porte la fondamentale dans son index ; un fichier libre doit
        // etre mesure, une seule fois, et le resultat retenu.
        if (e->root > 0.0f)
        {
            rootNote = e->root;
        }
        else
        {
            rootNote = detectRootNote (*buffer, fileRate);
            entries[(size_t) index].root = rootNote;
        }

        return buffer;
    }

    //==========================================================================
    // Autocorrelation sur les ~250 premieres ms.
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
    juce::AudioFormatManager formats;
    std::vector<Entry> entries;
    std::unique_ptr<juce::MemoryMappedFile> map;
    juce::int64 dataStart = 0;
    juce::File  folderRoot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleBank)
};

} // namespace rk
