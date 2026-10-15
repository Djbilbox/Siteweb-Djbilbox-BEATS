#pragma once
#include "HumpireTheme.h"
#include "LcdScreen.h"
#include "PresetManager.h"

namespace humpire
{
/** Base commune des plugins HUMPIRE : APVTS (+ bypass/simple/clean), presets, undo, meters, etat.
    Un plugin derive implemente prepare() et process(). */
class HumpireProcessor : public juce::AudioProcessor
{
public:
    HumpireProcessor (const juce::String& displayName, const juce::String& presetKey,
                      juce::AudioProcessorValueTreeState::ParameterLayout layout,
                      std::vector<FactoryPreset> factory, bool wantsMidi = false);

    // ---- a implementer
    virtual void prepare (double sampleRate, int maxBlock) = 0;
    virtual void process (juce::AudioBuffer<float>&, juce::MidiBuffer&) = 0;

    // ---- AudioProcessor
    void prepareToPlay (double sr, int maxBlock) override { preparedBlock = juce::jmax (1, maxBlock); prepare (sr, maxBlock); isPrepared = true; }
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;
    const juce::String getName() const override { return name; }
    bool acceptsMidi() const override  { return midi; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    /** Valeur brute (unites reelles) d'un parametre : lecture lock-free. */
    float get (const char* id) const noexcept { return apvts.getRawParameterValue (id)->load(); }
    bool  getBool (const char* id) const noexcept { return get (id) > 0.5f; }

    /** Hooks d'etat additionnel (snapshots, MIDI map...). */
    virtual void saveExtraState (juce::ValueTree&) {}
    virtual void loadExtraState (const juce::ValueTree&) {}

    juce::UndoManager undo;                          // avant apvts
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;
    std::atomic<float> inPeak { 0.0f }, outPeak { 0.0f };
    std::array<std::atomic<float>, 6> aux {};        // valeurs DSP -> UI (GR, LUFS, ...)
    ScopeBuffer scope;

    static juce::AudioProcessorValueTreeState::ParameterLayout withCommon (juce::AudioProcessorValueTreeState::ParameterLayout l);

private:
    juce::String name;
    bool midi;
    juce::AudioParameterBool* bypassParam = nullptr;
    bool isPrepared = false;
    int  preparedBlock = 512;
};

// ---- helpers de layout
inline std::unique_ptr<juce::AudioParameterFloat> mkFloat (const char* id, const char* name, float lo, float hi, float def,
                                                           float skew = 1.0f, const char* unit = "", int decimals = 1)
{
    juce::String u (unit);
    return std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name,
        juce::NormalisableRange<float> (lo, hi, 0.0f, skew), def,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([u, decimals] (float v, int)
            { const auto s = decimals > 0 ? juce::String (v, decimals) : juce::String (juce::roundToInt (v)); return u.isEmpty() ? s : s + " " + u; }));
}
inline std::unique_ptr<juce::AudioParameterBool> mkBool (const char* id, const char* name, bool def = false, bool automatable = true)
{
    return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def,
                                                       juce::AudioParameterBoolAttributes().withAutomatable (automatable));
}
inline std::unique_ptr<juce::AudioParameterChoice> mkChoice (const char* id, const char* name, juce::StringArray items, int def = 0)
{
    return std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 }, name, items, def);
}
} // namespace humpire
