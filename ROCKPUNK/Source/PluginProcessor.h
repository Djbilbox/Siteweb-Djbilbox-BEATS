#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>
#include <vector>

#include "Dsp/Amp.h"
#include "Dsp/Delay.h"
#include "Dsp/Drive.h"
#include "Dsp/GuitarVoice.h"
#include "Dsp/SampleBank.h"
#include "Dsp/SamplerVoice.h"
#include "Dsp/_archive_synth_engine/Filter.h"
#include "Dsp/_archive_synth_engine/Lfo.h"
#include "Params.h"

namespace rk
{

class PluginProcessor final : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ROCKPUNK GUITAR"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    GuitarParams params;

    juce::File getPresetDirectory() const;
    void seedFactoryPresets() const;

    // ---- bibliotheque de presets ----
    // Partagee par la cassette et par le bandeau du haut : les deux montraient
    // sinon des etats differents, et le bandeau dessine sur l'image n'etait
    // relie a rien du tout.
    void refreshPresetList();
    const juce::Array<juce::File>& getPresetFiles() const noexcept { return presetFiles; }
    juce::String getCurrentPresetName() const;
    int getCurrentPresetIndex() const noexcept { return currentPreset; }
    bool loadPreset (int index);
    bool loadPreset (const juce::File&);
    void stepPreset (int delta);
    bool savePreset (const juce::File&);
    std::function<void()> onPresetListChanged;
    void prepareUserSampleFolder() const;

    // Relit le dossier d'echantillons de l'utilisateur : l'editeur l'appelle a
    // son ouverture, pour qu'un fichier depose pendant la session apparaisse
    // sans avoir a recharger le plugin.
    void rescanSamples();
    juce::File getUserSampleDirectory() const { return SampleBank::getUserSampleDirectory(); }
    bool isUsingUserFolder() const { return bank.isUserFolder(); }

    // sampled-guitar layer (banque interne)
    juce::StringArray getSampleFiles() const { return bank.getNames(); }
    int getNumSampleFiles() const { return bank.getNumEntries(); }
    juce::String getCurrentSampleName() const { return currentSampleName; }
    float getSampleRootNote() const { return sampleRootNote; }
    juce::String getSampleCategory (int index) const
    {
        const auto* e = bank.getEntry (index);
        return e != nullptr ? e->category : juce::String();
    }

    float getOutputLevel (int channel) const noexcept { return channel == 1 ? rmsR : rmsL; }

    // UI keyboard / note tracking
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }
    std::set<int> getActiveNotesSnapshot() const;

private:
    void updateVoices();
    void trackKeyboardNotes (const juce::MidiBuffer&);
    void applyMonoMode (juce::MidiBuffer&);
    void applyMasterFx (juce::AudioBuffer<float>&);

    void loadSample();
    void updateSamplerVoices();

    // Deux Synthesiser separes, et non un seul portant deux sons.
    //
    // juce::Synthesiser::noteOn boucle sur les sons ; pour chacun, il commence
    // par arreter toute voix jouant deja cette note - sans verifier qu'elle
    // appartient au meme son. Avec les cordes et l'echantillonneur dans le meme
    // Synthesiser, le passage du second son coupait donc la corde a l'instant
    // meme ou le premier venait de la lancer. Le defaut restait invisible tant
    // que la voix s'eteignait sur un fondu fixe de 120 ms ; avec une vraie
    // enveloppe, la corde devenait franchement muette.
    juce::Synthesiser synth;         // cordes modelisees
    juce::Synthesiser sampleSynth;   // couche echantillonnee
    Amp   amp;
    Drive drive;
    Delay delay;

    // REVERB et CHORUS de la facade : deux vrais effets. Le chorus est une
    // ligne a retard courte modulee, decalee entre les deux canaux, ce qui
    // elargit sans deriver comme le ferait un simple gain de largeur.
    juce::Reverb reverb;
    std::vector<float> chorusL, chorusR;
    int   chorusWrite = 0;
    float chorusPhase = 0.0f;

    // La frequence retenue par prepareToPlay. AudioProcessor::getSampleRate()
    // n'est renseignee que par setRateAndBufferSizeDetails, appelee par l'hote :
    // s'en servir dans processBlock rendait 0 hors hote, et le chorus calculait
    // un retard NaN qui indexait le tampon n'importe ou.
    double preparedRate = 48000.0;

    // Le LFO et le filtre que la facade annonce depuis toujours. Le filtre a sa
    // propre enveloppe, celle des faders A S D R de droite : elle est declenchee
    // par la premiere note tenue, comme sur un synthe monotimbral.
    // Bloqueur de continu, apres l'ampli et le drive.
    //
    // L'etage "Urban Slayer" pousse plus fort l'alternance positive que la
    // negative et ajoute un terme en x carre : les deux produisent une
    // composante continue. Elle mangeait de la marge avant le limiteur, faussait
    // le VU-metre, et surtout traversait le filtre sans etre touchee - un
    // passe-bas ferme a 60 Hz ne retirait alors que 10 dB au lieu de 30.
    float dcInL = 0.0f, dcOutL = 0.0f, dcInR = 0.0f, dcOutR = 0.0f;
    float dcCoeff = 0.9974f;

    Lfo lfo;
    SvfFilter filterL, filterR;
    juce::ADSR filterEnv;
    bool anyNoteHeld = false;
    float lfoValue = 0.0f;

    // sampled-guitar layer
    SampleBank bank;
    std::shared_ptr<juce::AudioBuffer<float>> sampleData;
    double sampleFileRate = 44100.0;
    float  sampleRootNote = 60.0f;
    juce::String currentSampleName;
    int lastSampleSel  = -1;
    int lastSampleRoot = -1;
    std::vector<SamplerVoice*> samplerVoices;

    juce::Array<juce::File> presetFiles;
    int currentPreset = -1;

    juce::MidiKeyboardState keyboardState;
    std::set<int> activeNotes;
    std::set<int> monoHeld;
    mutable juce::CriticalSection notesLock;

    juce::SmoothedValue<float> masterVolSmooth;
    juce::SmoothedValue<float> widthSmooth;

    float rmsL = 0.0f;
    float rmsR = 0.0f;
    float currentBpm = 120.0f;
    float delayTimeMs = 350.0f;

    juce::UndoManager undoManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

} // namespace rk