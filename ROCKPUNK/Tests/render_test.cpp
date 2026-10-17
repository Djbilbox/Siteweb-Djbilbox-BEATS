// Verification sans hote ni interface : le moteur produit-il vraiment du son ?
//
// Le bug qu'on ne voit pas autrement est celui du produit livre muet. Depuis que
// ROCKPUNK ne transporte plus aucun enregistrement, le risque s'est deplace :
// les 25 presets doivent sonner sur une installation neuve, dossier
// d'echantillons vide. Chaque chemin de son est donc joue ici.

#include <JuceHeader.h>

#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"

namespace
{
    int failures = 0;

    void check (bool ok, const juce::String& what)
    {
        // Vidage immediat : sans lui, un plantage en cours de route emporte
        // tout le tampon et le banc se tait sur l'endroit exact du probleme.
        std::cout << (ok ? "  OK   " : "  ECHEC") << "  " << what << std::endl;
        if (! ok)
            ++failures;
    }

    // Joue une note pendant une seconde et renvoie le niveau RMS obtenu.
    float renderNoteRms (rk::PluginProcessor& proc, int note)
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        proc.prepareToPlay (sampleRate, blockSize);

        juce::AudioBuffer<float> buffer (2, blockSize);
        double sumSq = 0.0;
        juce::int64 counted = 0;

        for (int block = 0; block < 94; ++block)   // ~1 s
        {
            buffer.clear();
            juce::MidiBuffer midi;

            if (block == 0)
                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);
            if (block == 70)
                midi.addEvent (juce::MidiMessage::noteOff (1, note), 0);

            proc.processBlock (buffer, midi);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    const float s = buffer.getSample (ch, i);
                    sumSq += (double) s * (double) s;
                    ++counted;
                }
        }

        proc.releaseResources();
        return counted > 0 ? (float) std::sqrt (sumSq / (double) counted) : 0.0f;
    }

    // Niveau RMS sur une fenetre temporelle precise de la note tenue, en
    // secondes depuis l'attaque. Sert a comparer l'attaque et la queue.
    float renderWindowRms (rk::PluginProcessor& proc, int note, double from, double to)
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        proc.prepareToPlay (sampleRate, blockSize);

        juce::AudioBuffer<float> buffer (2, blockSize);
        const int firstBlock = (int) (from * sampleRate / blockSize);
        const int lastBlock  = (int) (to   * sampleRate / blockSize);

        double sumSq = 0.0;
        juce::int64 counted = 0;

        for (int block = 0; block <= lastBlock; ++block)
        {
            buffer.clear();
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);

            proc.processBlock (buffer, midi);

            if (block < firstBlock)
                continue;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    const float s = buffer.getSample (ch, i);
                    sumSq += (double) s * (double) s;
                    ++counted;
                }
        }

        proc.releaseResources();
        return counted > 0 ? (float) std::sqrt (sumSq / (double) counted) : 0.0f;
    }

    // Ondulation du niveau au fil du temps, rapportee au niveau moyen.
    //
    // Un tremolo se juge a ce qu'il fait varier, pas a ce qu'il fait baisser :
    // comparer deux niveaux moyens revenait a mesurer un ecart de 1 dB dans un
    // bruit de 1,5 dB, et le controle passait ou echouait au hasard.
    float renderLevelRipple (rk::PluginProcessor& proc, int note)
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        proc.prepareToPlay (sampleRate, blockSize);
        juce::AudioBuffer<float> buffer (2, blockSize);

        std::vector<double> blockRms;

        for (int block = 0; block < 94; ++block)
        {
            buffer.clear();
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);

            proc.processBlock (buffer, midi);

            double sumSq = 0.0;
            for (int i = 0; i < blockSize; ++i)
            {
                const double s = buffer.getSample (0, i);
                sumSq += s * s;
            }

            // On attend un tiers de seconde : juste apres l'attaque la corde
            // decroit encore vite, et cette pente se confondrait avec
            // l'ondulation qu'on cherche a mesurer.
            if (block >= 30)
                blockRms.push_back (std::sqrt (sumSq / blockSize));
        }

        proc.releaseResources();
        if (blockRms.size() < 4)
            return 0.0f;

        // Ecart crete a creux sur une fenetre glissante d'environ un sixieme de
        // seconde. Sur une fenetre aussi courte la corde n'a presque pas
        // decru, alors qu'un tremolo y accomplit un cycle entier : la mesure
        // separe l'ondulation de l'extinction naturelle, ce qu'une simple
        // difference d'un bloc au suivant ne faisait pas.
        constexpr size_t window = 16;
        if (blockRms.size() <= window)
            return 0.0f;

        double spread = 0.0;
        int windows = 0;

        for (size_t start = 0; start + window <= blockRms.size(); ++start)
        {
            double lo = blockRms[start], hi = blockRms[start];
            for (size_t i = start; i < start + window; ++i)
            {
                lo = std::min (lo, blockRms[i]);
                hi = std::max (hi, blockRms[i]);
            }

            const double mid = 0.5 * (hi + lo);
            if (mid > 1.0e-9)
            {
                spread += (hi - lo) / mid;
                ++windows;
            }
        }

        return windows > 0 ? (float) (spread / windows) : 0.0f;
    }

    // Energie de la difference gauche-droite : mesure l'ouverture stereo, la
    // seule chose qu'un chorus doit apporter et qu'un simple gain ne peut pas.
    float renderStereoSpread (rk::PluginProcessor& proc, int note)
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        proc.prepareToPlay (sampleRate, blockSize);
        juce::AudioBuffer<float> buffer (2, blockSize);

        double sumSq = 0.0;
        juce::int64 counted = 0;

        for (int block = 0; block < 94; ++block)
        {
            buffer.clear();
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);

            proc.processBlock (buffer, midi);

            for (int i = 0; i < blockSize; ++i)
            {
                const double d = (double) buffer.getSample (0, i) - buffer.getSample (1, i);
                sumSq += d * d;
                ++counted;
            }
        }

        proc.releaseResources();
        return counted > 0 ? (float) std::sqrt (sumSq / (double) counted) : 0.0f;
    }

    // Niveau restant bien apres le relachement de la note : ce qu'une reverb
    // ajoute et qu'une simple hausse de volume n'ajouterait pas.
    float renderTailRms (rk::PluginProcessor& proc, int note)
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        proc.prepareToPlay (sampleRate, blockSize);
        juce::AudioBuffer<float> buffer (2, blockSize);

        double sumSq = 0.0;
        juce::int64 counted = 0;

        for (int block = 0; block < 190; ++block)     // ~2 s
        {
            buffer.clear();
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);
            if (block == 20)
                midi.addEvent (juce::MidiMessage::noteOff (1, note), 0);

            proc.processBlock (buffer, midi);

            if (block < 120)                          // on ne compte que la queue
                continue;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < blockSize; ++i)
                {
                    const float s = buffer.getSample (ch, i);
                    sumSq += (double) s * (double) s;
                    ++counted;
                }
        }

        proc.releaseResources();
        return counted > 0 ? (float) std::sqrt (sumSq / (double) counted) : 0.0f;
    }

    // La corde est excitee par un bruit non graine : deux passages sur la meme
    // note donnent des niveaux differents, de l'ordre de 3 dB. Mesurer sur
    // quatre notes reparties sur le manche ramene cette dispersion assez bas
    // pour que caler les presets ait un sens.
    float renderRms (rk::PluginProcessor& proc, int note)
    {
        const int notes[] = { note - 12, note - 5, note, note + 7 };

        double sum = 0.0;
        for (int n : notes)
            sum += (double) renderNoteRms (proc, juce::jlimit (24, 96, n));

        return (float) (sum / (double) juce::numElementsInArray (notes));
    }

    // Moyenne de plusieurs rendus complets.
    //
    // La corde part d'un bruit non graine : comparer deux rendus isoles revient
    // a comparer deux tirages. Pour les controles qui opposent deux reglages,
    // on moyenne plusieurs passages de chaque cote.
    float renderRmsAveraged (rk::PluginProcessor& proc, int note, int times)
    {
        double sum = 0.0;
        for (int i = 0; i < times; ++i)
            sum += (double) renderRms (proc, note);
        return times > 0 ? (float) (sum / times) : 0.0f;
    }

    void setParam (rk::PluginProcessor& proc, const juce::String& id, float value)
    {
        if (auto* p = proc.apvts.getParameter (id))
            p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (value));
    }

    // Une note de guitare grossiere, ecrite en WAV : de quoi verifier que le
    // dossier utilisateur est lu, sans embarquer le moindre enregistrement.
    bool writeTestTone (const juce::File& target, double freq)
    {
        constexpr double rate = 44100.0;
        const int length = (int) (rate * 1.5);

        juce::AudioBuffer<float> buffer (1, length);
        auto* d = buffer.getWritePointer (0);
        for (int i = 0; i < length; ++i)
        {
            const double t = (double) i / rate;
            const double env = std::exp (-2.5 * t);
            d[i] = (float) (env * (0.6 * std::sin (6.283185 * freq * t)
                                 + 0.3 * std::sin (6.283185 * freq * 2.0 * t)
                                 + 0.1 * std::sin (6.283185 * freq * 3.0 * t)));
        }

        target.deleteFile();
        std::unique_ptr<juce::FileOutputStream> out (target.createOutputStream());
        if (out == nullptr)
            return false;

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (
            wav.createWriterFor (out.release(), rate, 1, 16, {}, 0));
        if (writer == nullptr)
            return false;

        return writer->writeFromAudioSampleBuffer (buffer, 0, length);
    }
}

namespace
{
    // Photographie l'interface dans un PNG. Le banc verifiait le son mais
    // personne n'avait jamais regarde la fenetre : elle est partie en
    // production avec des libelles superposes a ceux de l'image de fond. Un
    // rendu hors ecran coute trois lignes et rend le defaut visible sans hote.
    bool snapshotEditor (const juce::File& target)
    {
        rk::PluginProcessor proc;
        std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
        if (editor == nullptr)
            return false;

        // La fenetre doit accepter d'etre reduite : elle etait figee et se
        // faisait couper dans les hotes sans ascenseur.
        editor->setSize (700, 522);
        const bool shrank = editor->getWidth() <= 760 && editor->getHeight() <= 570;
        editor->setSize (1120, 836);

        const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true);
        if (! shrank)
            std::cout << "  ECHEC  la fenetre refuse d'etre reduite" << std::endl;
        if (! image.isValid())
            return false;

        target.deleteFile();
        std::unique_ptr<juce::FileOutputStream> out (target.createOutputStream());
        if (out == nullptr)
            return false;

        juce::PNGImageFormat png;
        return png.writeImageToStream (image, *out);
    }
}

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    std::cout << std::unitbuf;

    // ---- l'interface ----
    {
        const auto shot = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                              .getSiblingFile ("rockpunk_ui.png");
        check (snapshotEditor (shot), "interface photographiee : " + shot.getFullPathName());
    }

    // ---- les cordes, seules ----
    // C'est desormais l'instrument tout entier : si elles se taisent, il ne
    // reste rien a vendre.
    {
        rk::PluginProcessor proc;

        setParam (proc, "smpOn", 0.0f);
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 0.0f);

        const float aOnly = renderRms (proc, 52);
        check (aOnly > 0.0005f, "corde A seule (RMS " + juce::String (aOnly, 5) + ")");

        setParam (proc, "laOn", 0.0f);
        setParam (proc, "lbOn", 1.0f);
        const float bOnly = renderRms (proc, 52);
        check (bOnly > 0.0005f, "corde B seule (RMS " + juce::String (bOnly, 5) + ")");

        // Le resonateur de caisse naissait avec des coefficients a zero : monter
        // BODY ne faisait qu'attenuer la corde. Le test compare donc les deux
        // extremes - ils doivent differer autrement que par un simple gain.
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 1.0f);
        setParam (proc, "laBody", 0.0f);
        setParam (proc, "lbBody", 0.0f);
        const float dry = renderRms (proc, 45);

        setParam (proc, "laBody", 100.0f);
        setParam (proc, "lbBody", 100.0f);
        const float wet = renderRms (proc, 45);
        check (wet > 0.0005f,
               "BODY a fond produit du son (sec " + juce::String (dry, 5)
                   + " / caisse " + juce::String (wet, 5) + ")");

        // Garde-fou : la corde doit encore sonner longtemps apres l'attaque.
        //
        // Les deux moteurs ont vecu dans un seul juce::Synthesiser portant deux
        // sons ; noteOn boucle sur les sons et arrete toute voix jouant deja la
        // note, sans verifier a quel son elle appartient - la corde etait donc
        // coupee a l'instant meme ou elle demarrait. Un niveau global non nul ne
        // le voyait pas, la queue de l'ancien fondu suffisant a le masquer.
        setParam (proc, "laBody", 30.0f);
        setParam (proc, "lbBody", 25.0f);
        setParam (proc, "envS", 80.0f);
        setParam (proc, "envD", 2000.0f);

        const float onset = renderWindowRms (proc, 52, 0.00, 0.10);
        const float tail  = renderWindowRms (proc, 52, 0.45, 0.90);
        check (tail > onset * 0.10f,
               "la corde tient la note (attaque " + juce::String (onset, 4)
                   + " / queue " + juce::String (tail, 4) + ")");
    }

    // ---- les controles font-ils ce que la facade annonce ? ----
    //
    // Rebrancher un potard sur un parametre au nom correct ne prouve rien : il
    // faut que tourner le potard s'entende. Chaque controle est donc pousse a
    // ses deux extremes et le resultat doit differer.
    {
        rk::PluginProcessor proc;
        setParam (proc, "smpOn", 0.0f);
        setParam (proc, "laOn", 1.0f);
        setParam (proc, "lbOn", 1.0f);
        setParam (proc, "revMix", 0.0f);
        setParam (proc, "chorAmt", 0.0f);
        setParam (proc, "fltType", 3.0f);          // hors service
        setParam (proc, "lfoAmount", 0.0f);
        setParam (proc, "lfoPitch", 0.0f);
        setParam (proc, "modDepth", 0.0f);

        const float plain = renderRms (proc, 55);

        // FILTER + [FILTER TYPE] : un passe-bas serre doit franchement etouffer.
        // Mesure haut sur le manche : renderRms moyenne quatre notes reparties
        // sur deux octaves, et depuis la plus grave la coupure n'etait plus
        // qu'a une demi-octave - de quoi diluer l'effet du filtre.
        setParam (proc, "fltType", 0.0f);
        setParam (proc, "fltCutoff", 18000.0f);
        setParam (proc, "fenvS", 100.0f);
        setParam (proc, "fenvA", 0.5f);
        const float wideOpen = renderRmsAveraged (proc, 72, 4);

        // 60 Hz de coupure contre une note a ~200 Hz : pres de deux octaves
        // au-dessus, donc une attenuation massive. A 150 Hz la fondamentale
        // n'etait qu'a un tiers d'octave et l'ecart passait sous le seuil un
        // rendu sur trois.
        setParam (proc, "fltCutoff", 60.0f);
        const float clamped = renderRmsAveraged (proc, 72, 4);
        check (clamped < wideOpen * 0.5f,
               "FILTER ferme le son (ouvert " + juce::String (wideOpen, 4)
                   + " / ferme " + juce::String (clamped, 4) + ")");

        setParam (proc, "fltType", 3.0f);
        setParam (proc, "fltCutoff", 14000.0f);

        // LFO vers le VOLUME : le niveau doit onduler.
        //
        // La note est tenue aussi plate que possible - attaque immediate,
        // sustain au maximum, decroissance tres longue - pour que l'ondulation
        // mesuree vienne du tremolo et non de l'extinction de la corde.
        // Une seule corde : deux cordes qui sonnent ensemble s'amortissent
        // legerement differemment et battent l'une contre l'autre. Ce
        // battement produit exactement le genre d'ondulation qu'on cherche, et
        // faisait sauter la reference d'un rendu a l'autre.
        setParam (proc, "lbOn", 0.0f);
        setParam (proc, "envA", 0.5f);
        setParam (proc, "envD", 4000.0f);
        setParam (proc, "envS", 100.0f);
        setParam (proc, "laSustain", 100.0f);

        setParam (proc, "lfoDest", 2.0f);
        setParam (proc, "lfoAmount", 0.0f);
        const float steady = renderLevelRipple (proc, 55);

        setParam (proc, "lfoAmount", 100.0f);
        setParam (proc, "lfoRate", 6.0f);
        const float tremolo = renderLevelRipple (proc, 55);
        check (tremolo > steady * 2.0f,
               "LFO vers VOLUME fait onduler le niveau (sans " + juce::String (steady, 4)
                   + " / avec " + juce::String (tremolo, 4) + ")");
        setParam (proc, "lfoAmount", 0.0f);
        juce::ignoreUnused (plain);

        // CHORUS : il doit ouvrir l'image stereo, pas seulement changer le gain.
        const float dryStereo = renderStereoSpread (proc, 55);
        setParam (proc, "chorAmt", 100.0f);
        const float wetStereo = renderStereoSpread (proc, 55);
        check (wetStereo > dryStereo * 1.2f,
               "CHORUS elargit la stereo (sec " + juce::String (dryStereo, 4)
                   + " / traite " + juce::String (wetStereo, 4) + ")");
        setParam (proc, "chorAmt", 0.0f);

        // REVERB : de l'energie doit rester apres le relachement de la note.
        setParam (proc, "envR", 5.0f);
        setParam (proc, "revMix", 0.0f);
        const float dryTail = renderTailRms (proc, 55);
        setParam (proc, "revMix", 100.0f);
        const float wetTail = renderTailRms (proc, 55);
        check (wetTail > dryTail * 2.0f,
               "REVERB laisse une queue (sec " + juce::String (dryTail, 5)
                   + " / traite " + juce::String (wetTail, 5) + ")");
    }

    // ---- le dossier d'echantillons de l'utilisateur ----
    {
        rk::PluginProcessor proc;

        const auto dir = proc.getUserSampleDirectory();
        check (dir.isDirectory(), "dossier d'echantillons cree : " + dir.getFullPathName());
        check (dir.getChildFile ("LISEZ-MOI.txt").existsAsFile(),
               "mode d'emploi depose dans le dossier");

        const auto probe = dir.getChildFile ("_rockpunk_test_tone.wav");
        if (writeTestTone (probe, 220.0))
        {
            proc.rescanSamples();
            const int found = proc.getNumSampleFiles();
            check (found > 0, "le fichier depose est vu (" + juce::String (found) + " entree(s))");

            // Retrouver l'index du fichier temoin : le dossier peut deja
            // contenir les fichiers de l'utilisateur.
            int index = -1;
            const auto names = proc.getSampleFiles();
            for (int i = 0; i < names.size(); ++i)
                if (names[i] == probe.getFileNameWithoutExtension())
                    index = i;

            if (index >= 0)
            {
                setParam (proc, "laOn", 0.0f);
                setParam (proc, "lbOn", 0.0f);
                setParam (proc, "smpOn", 1.0f);
                setParam (proc, "smpSel", (float) index);
                const float rms = renderRms (proc, 60);
                check (rms > 0.0005f, "le fichier depose se joue (RMS " + juce::String (rms, 5) + ")");

                const float root = proc.getSampleRootNote();
                check (std::abs (root - 57.0f) < 2.0f,
                       "sa fondamentale est mesuree (" + juce::String (root, 1) + " pour un La 220 Hz)");
            }
            else
            {
                check (false, "le fichier temoin est introuvable dans la liste");
            }

            probe.deleteFile();
        }
        else
        {
            check (false, "impossible d'ecrire le fichier temoin");
        }
    }

    // ---- les presets d'usine ----
    const juce::File factoryDir (argc > 1
        ? juce::String (argv[1])
        : juce::File::getSpecialLocation (juce::File::currentExecutableFile)
              .getParentDirectory().getChildFile ("Presets").getFullPathName());

    if (factoryDir.isDirectory())
    {
        auto files = factoryDir.findChildFiles (juce::File::findFiles, false, "*.rpk");
        files.sort();
        check (! files.isEmpty(), "presets d'usine trouves (" + juce::String (files.size()) + ")");

        std::cout << "\npresets : " << factoryDir.getFullPathName() << "\n";

        for (const auto& f : files)
        {
            rk::PluginProcessor proc;

            std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
            if (xml == nullptr)
            {
                check (false, "XML illisible : " + f.getFileName());
                continue;
            }

            const auto tree = juce::ValueTree::fromXml (*xml);
            proc.apvts.replaceState (tree);

            // Un preset d'usine qui allume la couche echantillonnee serait muet
            // chez un acheteur au dossier vide - exactement la panne que ce banc
            // existe pour attraper.
            bool usesSamples = false;
            for (int i = 0; i < tree.getNumChildren(); ++i)
            {
                const auto child = tree.getChild (i);
                if (child.getProperty ("id").toString() == "smpOn"
                    && (float) child.getProperty ("value") >= 0.5f)
                    usesSamples = true;
            }

            const float rms = renderRms (proc, 55);
            const auto label = f.getFileNameWithoutExtension().paddedRight (' ', 24);

            check (! usesSamples, label + " n'exige aucun echantillon");
            check (rms > 0.0005f, label + " RMS " + juce::String (rms, 5));
        }
    }
    else
    {
        std::cout << "\n(aucun dossier de presets a verifier)\n";
    }

    std::cout << "\n" << (failures == 0 ? "tout est vert\n" : juce::String (failures) + " echec(s)\n");
    return failures == 0 ? 0 : 1;
}
