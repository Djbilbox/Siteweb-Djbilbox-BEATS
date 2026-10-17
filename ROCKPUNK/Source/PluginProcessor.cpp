#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace rk
{

PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "ROCKPUNK_GUITAR_STATE", createGuitarParameterLayout())
{
    fillGuitarParams (params, apvts);

    prepareUserSampleFolder();
    bank.openDefault();
    seedFactoryPresets();
    refreshPresetList();

    for (int i = 0; i < 8; ++i)
        synth.addVoice (new GuitarVoice());
    synth.addSound (new GuitarSound());
    synth.setNoteStealingEnabled (true);

    for (int i = 0; i < 8; ++i)
    {
        auto* sv = new SamplerVoice();
        samplerVoices.push_back (sv);
        sampleSynth.addVoice (sv);
    }
    sampleSynth.addSound (new SamplerSound());
    sampleSynth.setNoteStealingEnabled (true);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    preparedRate = sampleRate > 0.0 ? sampleRate : 48000.0;

    synth.clearVoices();
    for (int i = 0; i < 8; ++i)
    {
        auto* voice = new GuitarVoice();
        voice->setSampleRate (sampleRate);
        synth.addVoice (voice);
    }
    synth.setCurrentPlaybackSampleRate (sampleRate);

    sampleSynth.clearVoices();
    samplerVoices.clear();
    for (int i = 0; i < 8; ++i)
    {
        auto* sv = new SamplerVoice();
        sv->setSampleRate (sampleRate);
        samplerVoices.push_back (sv);
        sampleSynth.addVoice (sv);
    }
    sampleSynth.setCurrentPlaybackSampleRate (sampleRate);

    amp.setSampleRate (sampleRate);
    drive.setSampleRate (sampleRate);
    delay.setSampleRate (sampleRate);
    delay.prepare (2000);

    amp.reset();
    drive.reset();
    delay.reset();

    reverb.setSampleRate (sampleRate);
    reverb.reset();

    lfo.setSampleRate (sampleRate);
    lfo.reset();
    lfoValue = 0.0f;

    // Coupure a ~20 Hz : sous le premier partiel d'une guitare, donc inaudible
    // sur le son utile.
    dcCoeff = (float) std::exp (-2.0 * juce::MathConstants<double>::pi * 20.0 / preparedRate);
    dcInL = dcOutL = dcInR = dcOutR = 0.0f;

    filterL.setSampleRate (sampleRate);
    filterR.setSampleRate (sampleRate);
    filterL.reset();
    filterR.reset();

    filterEnv.setSampleRate (sampleRate);
    filterEnv.reset();
    anyNoteHeld = false;

    // 40 ms de ligne : de quoi loger le retard moyen du chorus et son
    // excursion, quelle que soit la frequence d'echantillonnage.
    const int chorusLen = (int) (sampleRate * 0.04) + 4;
    chorusL.assign ((size_t) chorusLen, 0.0f);
    chorusR.assign ((size_t) chorusLen, 0.0f);
    chorusWrite = 0;
    chorusPhase = 0.0f;

    masterVolSmooth.reset (sampleRate, 0.03);
    widthSmooth.reset (sampleRate, 0.03);
    masterVolSmooth.setCurrentAndTargetValue (dbToGain (params.masterVol.get()));
    widthSmooth.setCurrentAndTargetValue (params.outWidth.get01());

    rmsL = 0.0f;
    rmsR = 0.0f;
    currentBpm = 120.0f;
    delayTimeMs = params.delayTime.get();

    loadSample();
    updateSamplerVoices();
}

void PluginProcessor::releaseResources()
{
}

//==============================================================================
void PluginProcessor::updateVoices()
{
    const float mix  = params.layerMix.get01();
    const float toneA = 0.08f + 0.92f * params.laTone.get01();
    const float toneB = 0.08f + 0.92f * params.lbTone.get01();
    const float gA    = 0.98f + 0.0195f * params.laSustain.get01();
    const float gB    = 0.98f + 0.0195f * params.lbSustain.get01();
    const float pickA = 0.15f + 0.85f * params.laPick.get01();
    const float pickB = 0.15f + 0.85f * params.lbPick.get01();

    const float envA = params.envA.get();
    const float envD = params.envD.get();
    const float envS = params.envS.get01();
    const float envR = params.envR.get();
    const float glide = params.glideMs.get();
    const float velAmt = params.velAmt.get01();
    const int   pick = params.pickShape.getInt();

    // Hauteur modulee : vibrato du LFO (potard PITCH, ouvert davantage par la
    // molette MODULATION) plus la molette PITCH BEND. Deux demi-tons de
    // vibrato au maximum, ce qui reste une guitare et pas une sirene.
    const float vibratoDepth = params.lfoPitch.get01() + params.modDepth.get01();
    const float pitchOffset = params.pbend.get() + 2.0f * vibratoDepth * lfoValue;

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* gv = dynamic_cast<GuitarVoice*> (synth.getVoice (i)))
        {
            gv->setLayerA (params.laOn.getBool(), toneA, gA, params.laBlend.get01(),
                           params.laBody.get01(), params.laLevel.get(), pickA);
            gv->setLayerB (params.lbOn.getBool(), toneB, gB, params.lbBlend.get01(),
                           params.lbBody.get01(), params.lbLevel.get(), pickB);
            gv->setLayerMix (mix);
            gv->setEnvelope (envA, envD, envS, envR);
            gv->setGlideMs (glide);
            gv->setPitchOffset (pitchOffset);
            gv->setVelocityAmount (velAmt);
            gv->setPickShape (pick);
        }
    }
}

//==============================================================================
void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, numSamples);

    if (numSamples == 0)
        return;

    // ---- host tempo for the sync'd delay ----
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())
                currentBpm = (float) juce::jmax (1.0, *bpm);
        }
    }

    // ---- delay time (free ms or tempo-synced) ----
    {
        float target = params.delayTime.get();
        if (params.delaySync.getBool())
            target = divisionToMs (currentBpm, params.delayDiv.getInt());
        delayTimeMs = target;
    }

    // Le LFO avance d'un bloc : une valeur par bloc suffit pour un vibrato
    // (94 mises a jour par seconde en 512 echantillons) et evite de reaccorder
    // la corde a chaque echantillon.
    {
        static const int waveToShape[] = { 0, 1, 3, 4 };   // sinus / triangle / carre / S&H
        lfo.setFrequency (params.lfoRate.get());
        lfo.setShape (waveToShape[juce::jlimit (0, 3, params.lfoWave.getInt())]);

        for (int i = 0; i < numSamples; ++i)
            lfoValue = lfo.process();
    }

    updateVoices();

    // sampled-guitar layer: (re)load the WAV when the selection/root changes,
    // then push the shared buffer + playback settings into every sampler voice.
    if (params.smpSel.getInt() != lastSampleSel || params.smpRoot.getInt() != lastSampleRoot)
        loadSample();
    updateSamplerVoices();

    // merge on-screen keyboard events into the buffer, then track every note
    // (external MIDI + piano clicks) so the UI can light keys/knobs.
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    if (params.monoMode.getBool())
        applyMonoMode (midiMessages);

    trackKeyboardNotes (midiMessages);

    // Les deux moteurs additionnent dans le meme tampon ; renderNextBlock lit
    // le flux MIDI sans le consommer, il peut donc etre passe aux deux.
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);
    sampleSynth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    applyMasterFx (buffer);
}

//==============================================================================
void PluginProcessor::loadSample()
{
    const int sel  = params.smpSel.getInt();
    const int root = params.smpRoot.getInt();
    lastSampleSel  = sel;
    lastSampleRoot = root;

    std::shared_ptr<juce::AudioBuffer<float>> newData;
    double newRate = 44100.0;
    float  newRoot = 60.0f;
    juce::String newName;

    if (! bank.isEmpty())
    {
        const int idx = juce::jlimit (0, bank.getNumEntries() - 1, sel);
        if (const auto* e = bank.getEntry (idx))
            newName = e->name;

        // La note fondamentale a ete mesuree a la construction de la banque ;
        // smpRoot ne fait que la decaler pour rattraper une detection a cote.
        newData = bank.load (idx, newRate, newRoot);
        newRoot += (float) root;
    }

    sampleData        = std::move (newData);
    sampleFileRate    = newRate;
    sampleRootNote    = newRoot;
    currentSampleName = newName;
}

//==============================================================================
void PluginProcessor::updateSamplerVoices()
{
    const bool  on   = params.smpOn.getBool();
    const bool  loop = params.smpLoop.getBool();
    const float atk  = params.smpAttack.get();
    const float rel  = params.smpRelease.get();
    const float lvl  = params.smpLevel.get();

    std::shared_ptr<juce::AudioBuffer<float>> data = on ? sampleData : nullptr;

    for (auto* sv : samplerVoices)
    {
        sv->setSharedSample (data, sampleFileRate, sampleRootNote, loop, atk, rel);
        sv->setLevel (lvl);
    }
}

//==============================================================================
// [MONO/POLY] de la facade. juce::Synthesiser est polyphonique par nature : le
// mode mono se rend en coupant les notes tenues juste avant chaque nouvelle
// attaque, ce qui laisse aussi le glissando relier les deux hauteurs.
void PluginProcessor::applyMonoMode (juce::MidiBuffer& midi)
{
    juce::MidiBuffer out;

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int  pos = metadata.samplePosition;

        if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            for (int held : monoHeld)
                if (held != msg.getNoteNumber())
                    out.addEvent (juce::MidiMessage::noteOff (msg.getChannel(), held), pos);

            monoHeld.clear();
            monoHeld.insert (msg.getNoteNumber());
        }
        else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
        {
            monoHeld.erase (msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            monoHeld.clear();
        }

        out.addEvent (msg, pos);
    }

    midi.swapWith (out);
}

//==============================================================================
void PluginProcessor::trackKeyboardNotes (const juce::MidiBuffer& midi)
{
    juce::ScopedLock sl (notesLock);
    for (const auto metadata : midi)
    {
        const auto& msg = metadata.getMessage();
        if (msg.isNoteOnOrOff())
        {
            const int note = msg.getNoteNumber();
            if (msg.isNoteOn() && msg.getVelocity() > 0)
                activeNotes.insert (note);
            else
                activeNotes.erase (note);
        }
    }
}

std::set<int> PluginProcessor::getActiveNotesSnapshot() const
{
    juce::ScopedLock sl (notesLock);
    return activeNotes;
}

//==============================================================================
void PluginProcessor::applyMasterFx (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    auto* dataL = buffer.getWritePointer (0);
    auto* dataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : dataL;

    const float ampGain  = params.ampGain.get();
    const float ampDist  = params.ampDist.get01();
    const float ampTone  = params.ampTone.get();
    const float ampLevel = params.ampLevel.get();
    const bool  ampOn    = params.ampOn.getBool();

    const float driveAmt  = params.driveAmt.get01();
    const int   driveType = params.driveType.getInt();
    const float driveTone = params.driveTone.get();

    const bool  delayActive = params.delayOn.getBool();
    const float delayFb     = params.delayFb.get01();
    const float delayMix    = params.delayMix.get01();
    const float delayLP     = params.delayLP.get();

    const float masterVolTarget = dbToGain (params.masterVol.get());
    masterVolSmooth.setTargetValue (masterVolTarget);
    widthSmooth.setTargetValue (params.outWidth.get01());

    const bool  limiterOn = params.outLimiter.getBool();
    const float width     = params.outWidth.get01();

    // ---- LFO vers les destinations autres que la hauteur ----
    const int   lfoDest = params.lfoDest.getInt();
    const float lfoAmt  = params.lfoAmount.get01();

    float driveAmtMod = driveAmt;
    float lfoGain = 1.0f;
    float cutoffMod = 1.0f;

    switch (lfoDest)
    {
        case 1:  cutoffMod   = std::pow (2.0f, 2.5f * lfoAmt * lfoValue); break;  // filtre
        case 2:  lfoGain     = 1.0f - 0.5f * lfoAmt * (0.5f + 0.5f * lfoValue); break; // volume
        case 3:  driveAmtMod = juce::jlimit (0.0f, 1.0f, driveAmt + lfoAmt * lfoValue); break;
        default: break;   // 0 = hauteur, deja applique dans updateVoices
    }

    // ---- amp + drive (per sample, in place) ----
    for (int i = 0; i < numSamples; ++i)
    {
        amp.processStereo (dataL[i], dataR[i], ampGain, ampDist, ampTone, ampLevel, ampOn);
        drive.processStereo (dataL[i], dataR[i], driveAmtMod, driveType, driveTone);

        // Retrait du continu produit par les deux etages de saturation.
        dcOutL = dataL[i] - dcInL + dcCoeff * dcOutL;
        dcInL  = dataL[i];
        dataL[i] = dcOutL;

        dcOutR = dataR[i] - dcInR + dcCoeff * dcOutR;
        dcInR  = dataR[i];
        dataR[i] = dcOutR;
    }

    // ---- filtre (potard FILTER + selecteur [FILTER TYPE]) ----
    // Le type 3 met le filtre hors service : sans cette sortie, un passe-bas a
    // 14 kHz resterait dans le chemin sans rien apporter.
    const int filterType = params.fltType.getInt();
    if (filterType < 3)
    {
        juce::ADSR::Parameters fp;
        fp.attack  = params.fenvA.get() * 0.001f;
        fp.decay   = params.fenvD.get() * 0.001f;
        fp.sustain = params.fenvS.get01();
        fp.release = params.fenvR.get() * 0.001f;
        filterEnv.setParameters (fp);

        // L'enveloppe suit les notes tenues : elle attaque a la premiere et se
        // relache quand le clavier est relache.
        const bool held = ! getActiveNotesSnapshot().empty();
        if (held && ! anyNoteHeld)
            filterEnv.noteOn();
        else if (! held && anyNoteHeld)
            filterEnv.noteOff();
        anyNoteHeld = held;

        const float base = params.fltCutoff.get();
        const float reso = params.fltReso.get();

        filterL.setType (filterType);
        filterR.setType (filterType);
        filterL.setResonance (reso);
        filterR.setResonance (reso);

        const float nyquist = 0.45f * (float) preparedRate;


        for (int i = 0; i < numSamples; ++i)
        {
            // L'enveloppe ouvre le filtre depuis un quart de la frequence de
            // coupure jusqu'a elle : c'est le balayage que les faders dessinent.
            const float e = filterEnv.getNextSample();
            const float fc = juce::jlimit (40.0f, nyquist,
                                           base * cutoffMod * (0.25f + 0.75f * e));
            filterL.setCutoff (fc);
            filterR.setCutoff (fc);

            dataL[i] = filterL.process (dataL[i]);
            dataR[i] = filterR.process (dataR[i]);
        }

    }

    // ---- delay (block, in place) ----
    delay.process (dataL, dataR, dataL, dataR, numSamples,
                   delayTimeMs, delayFb, delayMix, delayLP, delayActive);

    // ---- chorus (le potard CHOR de la facade) ----
    const float chorusAmount = params.chorAmt.get01();
    if (chorusAmount > 0.001f && ! chorusL.empty())
    {
        const int   len = (int) chorusL.size();
        const float rate = 0.55f;                       // Hz, lent et epais
        const float baseDelay = 0.011f * (float) preparedRate;      // 11 ms
        const float depth = 0.004f * (float) preparedRate * chorusAmount;
        const float phaseInc = rate / (float) preparedRate;

        for (int i = 0; i < numSamples; ++i)
        {
            chorusL[(size_t) chorusWrite] = dataL[i];
            chorusR[(size_t) chorusWrite] = dataR[i];

            // Les deux canaux lisent en quadrature : c'est ce dephasage qui
            // ouvre l'image stereo, pas un simple elargissement mid/side.
            auto tap = [&] (const std::vector<float>& buf, float phase)
            {
                const float d = baseDelay + depth * (0.5f + 0.5f * std::sin (6.2831853f * phase));
                float pos = (float) chorusWrite - d;
                if (! std::isfinite (pos))
                    return 0.0f;
                while (pos < 0.0f)
                    pos += (float) len;

                const int   i0 = juce::jlimit (0, len - 1, (int) pos);
                const float fr = pos - (float) i0;
                const int   i1 = (i0 + 1) % len;
                return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
            };

            // Modulation en opposition de phase entre les deux canaux : quand
            // un cote s'allonge l'autre se raccourcit, ce qui ouvre vraiment
            // l'image. En quadrature, les deux voies bougeaient trop ensemble et
            // le chorus s'entendait a peine plus large que le signal sec.
            const float wetL = tap (chorusL, chorusPhase);
            const float wetR = tap (chorusR, chorusPhase + 0.5f);

            const float blend = 0.7f * chorusAmount;
            dataL[i] += (wetL - dataL[i]) * blend;
            dataR[i] += (wetR - dataR[i]) * blend;

            chorusWrite = (chorusWrite + 1) % len;
            chorusPhase += phaseInc;
            if (chorusPhase >= 1.0f)
                chorusPhase -= 1.0f;
        }
    }

    // ---- reverb (le potard REV de la facade) ----
    {
        juce::Reverb::Parameters rp;
        rp.roomSize   = juce::jlimit (0.0f, 1.0f, params.revSize.get01());
        rp.damping    = 0.45f;
        rp.width      = 1.0f;
        rp.wetLevel   = params.revMix.get01();
        rp.dryLevel   = 1.0f - 0.35f * params.revMix.get01();
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        if (rp.wetLevel > 0.001f)
            reverb.processStereo (dataL, dataR, numSamples);
    }

    // ---- width + master volume + limiter (per sample) ----
    double rmsSumL = 0.0;
    double rmsSumR = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = dataL[i];
        float r = dataR[i];

        // mid/side width
        const float m = (l + r) * 0.5f;
        const float s = (l - r) * 0.5f * width;
        l = m + s;
        r = m - s;

        l *= masterVolSmooth.getNextValue() * lfoGain;
        r *= masterVolSmooth.getNextValue() * lfoGain;

        if (limiterOn)
        {
            l = std::tanh (l);
            r = std::tanh (r);
        }

        dataL[i] = l;
        dataR[i] = r;

        rmsSumL += (double) (l * l);
        rmsSumR += (double) (r * r);
    }

    const float blockRmsL = (float) std::sqrt (rmsSumL / (double) numSamples);
    const float blockRmsR = (float) std::sqrt (rmsSumR / (double) numSamples);
    rmsL += (blockRmsL - rmsL) * 0.5f;
    rmsR += (blockRmsR - rmsR) * 0.5f;
}

//==============================================================================
juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::File PluginProcessor::getPresetDirectory() const
{
    // L'utilisateur enregistre ses cartouches ici, a cote des presets d'usine :
    // un seul dossier, dans les Documents, inscriptible sans elevation
    // contrairement au bundle lui-meme.
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("DJBILBOX")
        .getChildFile ("ROCKPUNK")
        .getChildFile ("Presets");

    if (! dir.isDirectory())
        dir.createDirectory();

    return dir;
}

//==============================================================================
void PluginProcessor::refreshPresetList()
{
    const auto previous = currentPreset >= 0 && currentPreset < presetFiles.size()
        ? presetFiles[currentPreset] : juce::File();

    presetFiles.clear();
    for (const auto& f : getPresetDirectory().findChildFiles (juce::File::findFiles, false, "*.rpk"))
        presetFiles.add (f);

    // Tri par nom : les presets d'usine sont numerotes, l'ordre affiche doit
    // etre celui-la et non celui du systeme de fichiers.
    presetFiles.sort();

    currentPreset = previous != juce::File() ? presetFiles.indexOf (previous) : -1;

    if (onPresetListChanged)
        onPresetListChanged();
}

juce::String PluginProcessor::getCurrentPresetName() const
{
    if (currentPreset >= 0 && currentPreset < presetFiles.size())
        return presetFiles[currentPreset].getFileNameWithoutExtension();
    return "ROCKPUNK - Init Patch";
}

bool PluginProcessor::loadPreset (const juce::File& f)
{
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (f));
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;

    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    currentPreset = presetFiles.indexOf (f);

    if (onPresetListChanged)
        onPresetListChanged();
    return true;
}

bool PluginProcessor::loadPreset (int index)
{
    if (index < 0 || index >= presetFiles.size())
        return false;
    return loadPreset (presetFiles[index]);
}

void PluginProcessor::stepPreset (int delta)
{
    if (presetFiles.isEmpty())
        return;

    // Depuis "aucun preset charge", un pas en avant doit donner le premier et un
    // pas en arriere le dernier - pas l'index -1 ou 1.
    int next = currentPreset < 0 ? (delta > 0 ? 0 : presetFiles.size() - 1)
                                 : currentPreset + delta;

    while (next < 0)
        next += presetFiles.size();
    next %= presetFiles.size();

    loadPreset (next);
}

bool PluginProcessor::savePreset (const juce::File& target)
{
    auto f = target.hasFileExtension (".rpk") ? target : target.withFileExtension (".rpk");
    if (! f.replaceWithText (apvts.state.toXmlString()))
        return false;

    refreshPresetList();
    currentPreset = presetFiles.indexOf (f);

    if (onPresetListChanged)
        onPresetListChanged();
    return true;
}

//==============================================================================
void PluginProcessor::rescanSamples()
{
    bank.openDefault();
    lastSampleSel = -1;          // force le rechargement : les index ont bouge
    loadSample();
    updateSamplerVoices();
}

void PluginProcessor::prepareUserSampleFolder() const
{
    // ROCKPUNK ne livre aucun enregistrement : cette couche joue ce que
    // l'utilisateur y depose. Le dossier est cree vide, avec la marche a suivre
    // dedans - sinon le selecteur SAMPLE reste inexplicablement vide.
    auto dir = SampleBank::getUserSampleDirectory();
    if (! dir.isDirectory() && ! dir.createDirectory().wasOk())
        return;

    auto readme = dir.getChildFile ("LISEZ-MOI.txt");
    if (readme.existsAsFile())
        return;

    readme.replaceWithText (
        "ROCKPUNK GUITAR - vos propres guitares\r\n"
        "\r\n"
        "Deposez ici vos fichiers .wav, .aif ou .flac : ils apparaissent dans le\r\n"
        "selecteur SAMPLE, en haut a droite de l'interface, et se jouent sur tout\r\n"
        "le clavier.\r\n"
        "\r\n"
        "ROCKPUNK mesure tout seul la note de chaque fichier pour accorder le\r\n"
        "clavier ; le reglage ROOT permet de la corriger d'un demi-ton si la\r\n"
        "mesure tombe a cote (accords et riffs sont ambigus par nature).\r\n"
        "\r\n"
        "Un sous-dossier devient une categorie dans la liste. Par exemple :\r\n"
        "  Samples/Rythmiques/palm-mute-E.wav\r\n"
        "  Samples/Solos/bend-A.wav\r\n"
        "\r\n"
        "Les deux cordes de ROCKPUNK, elles, ne demandent aucun fichier : elles\r\n"
        "sont entierement calculees. Les 25 presets d'usine ne reposent que sur\r\n"
        "elles et fonctionnent avec ce dossier vide.\r\n"
        "\r\n"
        "DJBILBOX BEATS - support@djbilboxbeats.com\r\n");
}

void PluginProcessor::seedFactoryPresets() const
{
    auto dest = getPresetDirectory();
    if (! dest.isDirectory())
        return;

    // L'installeur Windows tourne en mode eleve : les Documents qu'il voit sont
    // ceux de l'administrateur, pas ceux de l'utilisateur. Les presets d'usine
    // voyagent donc dans le bundle et sont recopies ici au premier lancement.
    const auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    const juce::File sources[] =
    {
        exe.getParentDirectory().getParentDirectory()
           .getChildFile ("Resources").getChildFile ("Presets"),
        exe.getParentDirectory().getChildFile ("Presets"),
        juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
           .getChildFile ("DJBILBOX").getChildFile ("ROCKPUNK").getChildFile ("Presets"),
    };

    for (const auto& src : sources)
    {
        if (! src.isDirectory())
            continue;

        // Copie seulement ce qui manque : un preset d'usine que l'utilisateur a
        // retouche et reenregistre sous le meme nom ne doit pas revenir.
        for (const auto& f : src.findChildFiles (juce::File::findFiles, false, "*.rpk"))
        {
            const auto target = dest.getChildFile (f.getFileName());
            if (! target.existsAsFile())
                f.copyFileTo (target);
        }
        return;
    }
}

} // namespace rk

//==============================================================================
// Entry point shared by every JUCE plugin wrapper (VST3, standalone, ...).
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new rk::PluginProcessor();
}