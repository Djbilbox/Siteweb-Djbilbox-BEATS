#include "ReverbEffect.h"

ReverbEffect::ReverbEffect() { setSampleRate(44100.0); update(); }

void ReverbEffect::setSampleRate(double sr)
{
    sampleRate = sr;
    reverb.setSampleRate(sr);
    reverb.reset();      // vide les queues (prepareToPlay / changement de sample rate)
}

void ReverbEffect::update()
{
    juce::Reverb::Parameters p;
    p.roomSize = (float) juce::jlimit(0.0, 1.0, 0.35 + 0.6 * roomSize);
    p.damping  = (float) juce::jlimit(0.0, 1.0, damping);
    p.width    = (float) juce::jlimit(0.0, 1.0, width);
    p.wetLevel = (float) (mix * 0.45);
    p.dryLevel = 1.0f;
    p.freezeMode = 0.0f;
    reverb.setParameters(p);
}

void ReverbEffect::setRoomSize(double r) { roomSize = r; update(); }
void ReverbEffect::setDamping(double d)  { damping = d;  update(); }
void ReverbEffect::setWidth(double w)    { width = w;    update(); }
void ReverbEffect::setMix(double m)      { mix = juce::jlimit(0.0, 1.0, m); update(); }

void ReverbEffect::process(double* buffer, int numSamples)
{
    constexpr int kChunk = 256;
    float tmp[kChunk];
    for (int pos = 0; pos < numSamples; pos += kChunk)
    {
        const int n = juce::jmin(kChunk, numSamples - pos);
        for (int i = 0; i < n; ++i) tmp[i] = (float) buffer[pos + i];
        reverb.processMono(tmp, n);
        for (int i = 0; i < n; ++i) buffer[pos + i] = (double) tmp[i];
    }
}
