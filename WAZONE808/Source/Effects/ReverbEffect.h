#pragma once
#include <JuceHeader.h>

// Reverb sur la mid-voie. L'ancienne version (2 combs + 2 allpass a longueurs fixes,
// reglees pour 44.1 kHz) sonnait metallique et changeait de taille avec la
// frequence d'echantillonnage ; elle s'appuie maintenant sur juce::Reverb (Freeverb
// complet 8 combs + 4 allpass, longueurs mises a l'echelle du sample rate).
class ReverbEffect
{
public:
    ReverbEffect();
    ~ReverbEffect() = default;

    void setSampleRate(double sr);
    void setRoomSize(double room);
    void setDamping(double damping);
    void setWidth(double width);
    void setMix(double mix);
    void process(double* buffer, int numSamples);

private:
    void update();
    juce::Reverb reverb;
    double sampleRate = 44100.0;
    double roomSize = 0.5, damping = 0.5, width = 1.0, mix = 0.25;
};
