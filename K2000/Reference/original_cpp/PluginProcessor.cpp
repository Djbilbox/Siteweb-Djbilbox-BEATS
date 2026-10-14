#include "PluginProcessor.h"
#include "PluginEditor.h"

K2000AudioProcessor::K2000AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Add 16 polyphonic voices
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new K2000Audio::Voice());

    synth.addSound(new K2000Audio::SynthSound());

    initPresets();
}

K2000AudioProcessor::~K2000AudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout K2000AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // LFO
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_rate", "LFO Rate", juce::NormalisableRange<float>(0.05f, 25.0f, 0.01f, 0.4f), 4.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_amount", "LFO Amount", 0.0f, 1.0f, 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("lfo_pitch", "LFO Pitch Switch", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_attack", "LFO Attack", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.35f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_decay", "LFO Decay", juce::NormalisableRange<float>(0.005f, 8.0f, 0.001f, 0.35f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_sustain", "LFO Sustain", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lfo_release", "LFO Release", juce::NormalisableRange<float>(0.005f, 8.0f, 0.001f, 0.35f), 0.4f));

    // ADSR + Velocity Bank
    params.push_back(std::make_unique<juce::AudioParameterFloat>("amp_attack", "Amp Attack", juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.35f), 0.02f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("amp_decay", "Amp Decay", juce::NormalisableRange<float>(0.005f, 8.0f, 0.001f, 0.35f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("amp_sustain", "Amp Sustain", 0.0f, 1.0f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("velocity_bank", "Velocity Bank", 0.0f, 1.0f, 0.75f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("velocity_adsr", "Velocity ADSR", 0.0f, 1.0f, 0.4f));

    // FX & Master
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master_volume", "Volume", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fx_reverb", "Reverb", 0.0f, 1.0f, 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fx_chorus", "Chorus", 0.0f, 1.0f, 0.45f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fx_distortion", "Distortion Master", 0.0f, 1.0f, 0.25f));

    // Filter & Delay
    params.push_back(std::make_unique<juce::AudioParameterFloat>("glide_time", "Glide", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f, 0.4f), 0.05f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_cutoff", "Filter", juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f), 3200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filter_drive", "Distortion Drive", 0.0f, 1.0f, 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fx_delay_mix", "Delay Mix", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fx_delay_time", "Delay Time", 0.01f, 1.0f, 0.375f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fx_delay_fb", "Delay Feedback", 0.0f, 0.95f, 0.45f));

    // Dropdown choices
    juce::StringArray waveTypes { "Sawtooth", "Square", "Triangle", "Sine", "SuperSaw", "80s Brass" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("osc_waveform", "Waveform", waveTypes, 0));

    juce::StringArray lfoSources { "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random S&H" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("lfo_source", "LFO Source", lfoSources, 1));

    juce::StringArray lfoDests { "Cutoff", "Pitch", "Pulse Width", "Volume", "Chorus", "Pan" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("lfo_dest", "Destination", lfoDests, 0));

    juce::StringArray filterTypes { "LowPass 24dB", "LowPass 12dB", "BandPass", "HighPass", "Acid LowPass" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("filter_type", "Filter Type", filterTypes, 0));

    juce::StringArray voiceModes { "Poly 8", "Poly 16", "Mono Retrig", "Mono Legato" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("voice_mode", "Mono/Poly", voiceModes, 0));

    return { params.begin(), params.end() };
}

void K2000AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    chorus.setSampleRate(sampleRate);
    delay.setSampleRate(sampleRate);
    reverb.setSampleRate(sampleRate);
}

void K2000AudioProcessor::releaseResources()
{
}

bool K2000AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void K2000AudioProcessor::updateParameters()
{
    auto waveType = static_cast<K2000Audio::WaveformType>(static_cast<int>(*apvts.getRawParameterValue("osc_waveform")));
    auto filtType = static_cast<K2000Audio::FilterType>(static_cast<int>(*apvts.getRawParameterValue("filter_type")));
    auto lfoSrc = static_cast<K2000Audio::LfoWaveform>(static_cast<int>(*apvts.getRawParameterValue("lfo_source")));
    auto lfoDst = static_cast<K2000Audio::LfoDestination>(static_cast<int>(*apvts.getRawParameterValue("lfo_dest")));

    float cutoff = *apvts.getRawParameterValue("filter_cutoff");
    float fDrive = *apvts.getRawParameterValue("filter_drive");
    float ampA = *apvts.getRawParameterValue("amp_attack");
    float ampD = *apvts.getRawParameterValue("amp_decay");
    float ampS = *apvts.getRawParameterValue("amp_sustain");
    float velBank = *apvts.getRawParameterValue("velocity_bank");
    float velAdsr = *apvts.getRawParameterValue("velocity_adsr");
    float glide = *apvts.getRawParameterValue("glide_time");

    float lfoRate = *apvts.getRawParameterValue("lfo_rate");
    float lfoAmt = *apvts.getRawParameterValue("lfo_amount");
    bool lfoPtch = *apvts.getRawParameterValue("lfo_pitch") > 0.5f;
    float lfoA = *apvts.getRawParameterValue("lfo_attack");
    float lfoD = *apvts.getRawParameterValue("lfo_decay");
    float lfoS = *apvts.getRawParameterValue("lfo_sustain");
    float lfoR = *apvts.getRawParameterValue("lfo_release");

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<K2000Audio::Voice*>(synth.getVoice(i)))
        {
            voice->setWaveform(waveType);
            voice->setFilterType(filtType);
            voice->setFilterCutoff(cutoff);
            voice->setFilterDrive(fDrive);
            voice->setAmpAttack(ampA);
            voice->setAmpDecay(ampD);
            voice->setAmpSustain(ampS);
            voice->setVelocityBank(velBank);
            voice->setVelocityAdsr(velAdsr);
            voice->setGlideTime(glide);

            auto& voiceLfo = voice->getLfo();
            voiceLfo.setWaveform(lfoSrc);
            voiceLfo.setDestination(lfoDst);
            voiceLfo.setRate(lfoRate);
            voiceLfo.setAmount(lfoAmt);
            voiceLfo.setPitchSwitch(lfoPtch);
            voiceLfo.getEnvelope().setAttackTime(lfoA);
            voiceLfo.getEnvelope().setDecayTime(lfoD);
            voiceLfo.getEnvelope().setSustainLevel(lfoS);
            voiceLfo.getEnvelope().setReleaseTime(lfoR);
        }
    }

    // Master FX
    masterDistortion.setDrive(*apvts.getRawParameterValue("fx_distortion"));
    chorus.setDepth(*apvts.getRawParameterValue("fx_chorus"));
    delay.setMix(*apvts.getRawParameterValue("fx_delay_mix"));
    delay.setDelayTime(*apvts.getRawParameterValue("fx_delay_time"));
    delay.setFeedback(*apvts.getRawParameterValue("fx_delay_fb"));
    reverb.setMix(*apvts.getRawParameterValue("fx_reverb"));
}

void K2000AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    updateParameters();

    // Render Synth voices
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    float masterVol = *apvts.getRawParameterValue("master_volume");
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : leftChannel;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float l = leftChannel[sample];
        float r = rightChannel[sample];

        masterDistortion.process(l, r);
        chorus.process(l, r);
        delay.process(l, r);
        reverb.process(l, r);
        limiter.process(l, r, masterVol);

        leftChannel[sample] = l;
        if (buffer.getNumChannels() > 1)
            rightChannel[sample] = r;
    }
}

const juce::String K2000AudioProcessor::getName() const { return "K2000"; }
bool K2000AudioProcessor::acceptsMidi() const { return true; }
bool K2000AudioProcessor::producesMidi() const { return false; }
bool K2000AudioProcessor::isMidiEffect() const { return false; }
double K2000AudioProcessor::getTailLengthSeconds() const { return 2.5; }

int K2000AudioProcessor::getNumPrograms() { return static_cast<int>(factoryPresets.size()); }
int K2000AudioProcessor::getCurrentProgram() { return currentProgramIndex; }
void K2000AudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < static_cast<int>(factoryPresets.size()))
    {
        currentProgramIndex = index;
        loadPreset(index);
    }
}
const juce::String K2000AudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < static_cast<int>(factoryPresets.size()))
        return factoryPresets[static_cast<size_t>(index)].name;
    return {};
}
void K2000AudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void K2000AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void K2000AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void K2000AudioProcessor::initPresets()
{
    factoryPresets = {
        { "K2000 - Init Patch", { {"osc_waveform", 0}, {"filter_cutoff", 3200.0f}, {"fx_chorus", 0.45f}, {"fx_reverb", 0.35f}, {"fx_delay_mix", 0.3f}, {"master_volume", 0.8f}} },
        { "01 - Neon Night Drive", { {"osc_waveform", 4}, {"filter_cutoff", 4200.0f}, {"fx_chorus", 0.65f}, {"fx_reverb", 0.5f}, {"fx_delay_mix", 0.35f}, {"glide_time", 0.08f}} },
        { "02 - Outrun Bassline", { {"osc_waveform", 1}, {"filter_cutoff", 1200.0f}, {"amp_decay", 0.35f}, {"amp_sustain", 0.1f}, {"filter_drive", 0.4f}, {"fx_chorus", 0.2f}} },
        { "03 - Blade Runner Lead", { {"osc_waveform", 5}, {"filter_cutoff", 2800.0f}, {"lfo_amount", 0.4f}, {"lfo_pitch", 1.0f}, {"fx_reverb", 0.7f}, {"glide_time", 0.15f}} },
        { "04 - Miami Vice Pad", { {"osc_waveform", 4}, {"filter_cutoff", 2200.0f}, {"amp_attack", 0.6f}, {"amp_decay", 2.0f}, {"amp_sustain", 0.9f}, {"fx_chorus", 0.8f}} },
        { "05 - Turbo Arp Pluck", { {"osc_waveform", 0}, {"filter_cutoff", 1800.0f}, {"amp_attack", 0.002f}, {"amp_decay", 0.25f}, {"amp_sustain", 0.0f}, {"fx_delay_mix", 0.45f}} },
        { "06 - VHS Tape Dreams", { {"osc_waveform", 2}, {"filter_cutoff", 1600.0f}, {"fx_chorus", 0.75f}, {"fx_distortion", 0.3f}, {"fx_reverb", 0.6f}} },
        { "07 - Cyber Chaser Bass", { {"osc_waveform", 0}, {"filter_cutoff", 950.0f}, {"filter_type", 4.0f}, {"filter_drive", 0.55f}, {"amp_decay", 0.28f}} },
        { "08 - Sunset Grid Pluck", { {"osc_waveform", 4}, {"filter_cutoff", 3500.0f}, {"amp_decay", 0.4f}, {"amp_sustain", 0.2f}, {"fx_delay_mix", 0.4f}, {"fx_delay_fb", 0.55f}} },
        { "09 - Stranger Horror Pad", { {"osc_waveform", 0}, {"filter_cutoff", 1100.0f}, {"lfo_rate", 0.8f}, {"lfo_amount", 0.6f}, {"fx_reverb", 0.85f}} },
        { "10 - Megadrive Solo Lead", { {"osc_waveform", 1}, {"filter_cutoff", 4800.0f}, {"glide_time", 0.12f}, {"fx_distortion", 0.45f}, {"fx_delay_mix", 0.35f}} },
        { "11 - Chrome Overdrive", { {"osc_waveform", 0}, {"filter_cutoff", 2600.0f}, {"fx_distortion", 0.75f}, {"filter_drive", 0.6f}, {"amp_sustain", 0.8f}} },
        { "12 - Digital Rain Pluck", { {"osc_waveform", 3}, {"filter_cutoff", 5500.0f}, {"amp_decay", 0.18f}, {"amp_sustain", 0.0f}, {"fx_delay_mix", 0.5f}} },
        { "13 - Retrowave Brass Stabs", { {"osc_waveform", 5}, {"filter_cutoff", 3800.0f}, {"amp_attack", 0.05f}, {"amp_decay", 0.6f}, {"amp_sustain", 0.5f}} },
        { "14 - Cyborg Camel Cry", { {"osc_waveform", 4}, {"filter_cutoff", 2900.0f}, {"lfo_source", 3.0f}, {"lfo_dest", 0.0f}, {"lfo_amount", 0.7f}, {"fx_reverb", 0.65f}} },
        { "15 - Cassette LoFi Keys", { {"osc_waveform", 2}, {"filter_cutoff", 1400.0f}, {"fx_chorus", 0.6f}, {"fx_reverb", 0.4f}, {"lfo_amount", 0.25f}, {"lfo_pitch", 1.0f}} },
        { "16 - Future Hero Solo", { {"osc_waveform", 4}, {"filter_cutoff", 6000.0f}, {"glide_time", 0.1f}, {"fx_chorus", 0.5f}, {"fx_delay_mix", 0.4f}, {"fx_reverb", 0.6f}} }
    };
}

void K2000AudioProcessor::loadPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(factoryPresets.size())) return;
    const auto& preset = factoryPresets[static_cast<size_t>(index)];

    for (const auto& [paramId, value] : preset.params)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    }
}

juce::AudioProcessorEditor* K2000AudioProcessor::createEditor()
{
    return new K2000AudioProcessorEditor(*this);
}

bool K2000AudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new K2000AudioProcessor();
}
