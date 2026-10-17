// Construit la banque interne de ROCKPUNK a partir des WAV sources.
//
// Les sources sont des phrases de guitare de 3 a 7 secondes (Slayer Electric,
// Urban Slayer Acoustic, bibliotheque Player). Chaque fichier est rogne au
// premier son, limite a la duree utile, normalise puis encode en FLAC : sans
// perte, mais environ deux fois plus compact, et tout est reuni dans un seul
// conteneur pour que le plugin ouvre un descripteur au lieu de parcourir un
// dossier a chaque demarrage.
//
// La note fondamentale est mesuree ici, une fois pour toutes, et gravee dans
// l'index : le plugin n'a plus a lancer d'autocorrelation au chargement.
//
//   convert_samples <dossier-source> [<dossier-source> ...] <fichier-banque>

#include <JuceHeader.h>

namespace
{
    constexpr double kMaxSeconds   = 6.0;    // assez pour une phrase complete
    constexpr float  kSilenceFloor = 0.0015f;
    constexpr float  kTargetPeak   = 0.98f;

    int findFirstSound (const juce::AudioBuffer<float>& b)
    {
        const int n = b.getNumSamples();
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < b.getNumChannels(); ++c)
                if (std::abs (b.getSample (c, i)) > kSilenceFloor)
                    return juce::jmax (0, i - 64);   // un peu d'elan avant l'attaque
        return 0;
    }

    // Autocorrelation sur les ~250 premieres ms : meme mesure que celle que le
    // plugin faisait au chargement, faite une seule fois a la construction.
    float detectRootNote (const juce::AudioBuffer<float>& buf, double sampleRate)
    {
        const int numSrc = buf.getNumSamples();
        const int winLen = juce::jmin (numSrc, (int) (sampleRate * 0.25));
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

    struct Entry
    {
        juce::String name, category;
        float        root = 60.0f;
        juce::int64  offset = 0, size = 0;
    };
}

int main (int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "usage: convert_samples <dossier-source> [...] <fichier-banque>\n";
        return 1;
    }

    juce::Array<juce::File> sourceDirs;
    for (int i = 1; i < argc - 1; ++i)
    {
        const juce::String p (argv[i]);
        const juce::File d (p);
        if (! d.isDirectory())
        {
            std::cout << "dossier introuvable : " << p << "\n";
            return 1;
        }
        sourceDirs.add (d);
    }

    const juce::String bankPath (argv[argc - 1]);
    const juce::File bankFile (bankPath);
    bankFile.getParentDirectory().createDirectory();

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    juce::FlacAudioFormat flac;

    // La categorie est le nom du dossier source : c'est ce que le selecteur du
    // plugin affiche pour regrouper les sons.
    juce::Array<juce::File> wavs;
    juce::StringArray wavCategories;
    for (const auto& dir : sourceDirs)
    {
        juce::Array<juce::File> found;
        for (const auto& item : juce::RangedDirectoryIterator (dir, true, "*.wav", juce::File::findFiles))
            found.add (item.getFile());

        found.sort();
        for (const auto& f : found)
        {
            wavs.add (f);
            wavCategories.add (dir.getFileName());
        }
    }

    std::vector<Entry> entries;
    juce::MemoryBlock blobs;
    int written = 0, failed = 0;
    juce::int64 sourceBytes = 0;

    for (int i = 0; i < wavs.size(); ++i)
    {
        const auto& file = wavs.getReference (i);

        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
        if (reader == nullptr || reader->lengthInSamples <= 0)
        {
            ++failed;
            std::cout << "ILLISIBLE " << file.getFileName() << "\n";
            continue;
        }

        const int channels  = (int) juce::jmin<juce::uint32> (2, reader->numChannels);
        const int available = (int) juce::jmin<juce::int64> (reader->lengthInSamples,
                                                             (juce::int64) (reader->sampleRate * 15.0));

        juce::AudioBuffer<float> buffer (channels, available);
        reader->read (&buffer, 0, available, 0, true, channels > 1);

        const int start = findFirstSound (buffer);
        const int keep  = juce::jmin (available - start, (int) (reader->sampleRate * kMaxSeconds));
        if (keep < 1024)
        {
            ++failed;
            std::cout << "TROP COURT " << file.getFileName() << "\n";
            continue;
        }

        juce::AudioBuffer<float> trimmed (channels, keep);
        for (int c = 0; c < channels; ++c)
            trimmed.copyFrom (c, 0, buffer, c, start, keep);

        // Les trois bibliotheques sources n'ont pas le meme niveau de gravure :
        // sans cette normalisation le changement de sample fait un saut de volume.
        const float peak = trimmed.getMagnitude (0, keep);
        if (peak > 0.0001f)
            trimmed.applyGain (kTargetPeak / peak);

        // Fondu de sortie court : une queue coupee net claque.
        const int fade = juce::jmin (keep / 8, (int) (reader->sampleRate * 0.05));
        if (fade > 0)
            trimmed.applyGainRamp (keep - fade, fade, 1.0f, 0.0f);

        const float root = detectRootNote (trimmed, reader->sampleRate);

        juce::MemoryBlock encoded;
        {
            auto* mem = new juce::MemoryOutputStream (encoded, false);
            std::unique_ptr<juce::AudioFormatWriter> writer (
                flac.createWriterFor (mem, reader->sampleRate, (unsigned int) channels, 16, {}, 5));
            if (writer == nullptr)
            {
                delete mem;
                ++failed;
                continue;
            }
            writer->writeFromAudioSampleBuffer (trimmed, 0, keep);
        }

        if (encoded.getSize() == 0)
        {
            ++failed;
            continue;
        }

        Entry e;
        e.name     = file.getFileNameWithoutExtension();
        e.category = wavCategories[i];
        e.root     = root;
        e.offset   = (juce::int64) blobs.getSize();
        e.size     = (juce::int64) encoded.getSize();
        entries.push_back (e);

        blobs.append (encoded.getData(), encoded.getSize());
        sourceBytes += file.getSize();
        ++written;

        if (written % 25 == 0)
            std::cout << "  ... " << written << " convertis\n";
    }

    if (entries.empty())
    {
        std::cout << "aucun echantillon converti\n";
        return 1;
    }

    bankFile.deleteFile();
    std::unique_ptr<juce::FileOutputStream> out (bankFile.createOutputStream());
    if (out == nullptr)
    {
        std::cout << "ecriture impossible : " << bankFile.getFullPathName() << "\n";
        return 1;
    }

    out->writeInt (0x4B425052);                  // 'RPBK'
    out->writeInt (1);                           // version de format
    out->writeInt ((int) entries.size());

    // L'index d'abord : le plugin liste le contenu sans toucher a l'audio.
    for (const auto& e : entries)
    {
        out->writeString (e.name);
        out->writeString (e.category);
        out->writeFloat (e.root);
        out->writeInt64 (e.offset);
        out->writeInt64 (e.size);
    }

    const juce::int64 dataStart = out->getPosition() + 8;
    out->writeInt64 (dataStart);
    out->write (blobs.getData(), blobs.getSize());
    out->flush();
    const auto bankSize = bankFile.getSize();
    out.reset();

    std::cout << "\nconvertis : " << written << "\n"
              << "echecs    : " << failed  << "\n"
              << "source    : " << (sourceBytes / 1024 / 1024) << " Mo\n"
              << "banque    : " << (bankSize    / 1024 / 1024) << " Mo\n"
              << "ecrite    : " << bankFile.getFullPathName() << "\n";

    return failed == 0 ? 0 : 1;
}
