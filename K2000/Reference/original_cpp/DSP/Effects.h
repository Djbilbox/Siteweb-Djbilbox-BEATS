#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace K2000Audio
{
    // ==========================================
    // 1. Warm Analog Distortion
    // ==========================================
    class DistortionEffect
    {
    public:
        DistortionEffect() = default;
        void setDrive(float driveAmount); // 0.0 to 1.0
        void process(float& left, float& right);

    private:
        float drive = 0.0f;
    };

    // ==========================================
    // 2. 80s Stereo Dimension Chorus
    // ==========================================
    class DimensionChorus
    {
    public:
        DimensionChorus();
        void setSampleRate(double sampleRate);
        void setDepth(float depth); // 0.0 to 1.0
        void process(float& left, float& right);

    private:
        double sampleRate = 44100.0;
        float depth = 0.0f;
        std::vector<float> bufferL;
        std::vector<float> bufferR;
        size_t writePos = 0;
        double lfoPhase = 0.0;
    };

    // ==========================================
    // 3. Stereo Tape / Analog Delay
    // ==========================================
    class TapeDelay
    {
    public:
        TapeDelay();
        void setSampleRate(double sampleRate);
        void setMix(float mix);           // 0.0 to 1.0
        void setDelayTime(float seconds); // 0.01 to 1.0 s
        void setFeedback(float fb);       // 0.0 to 0.95
        void process(float& left, float& right);

    private:
        double sampleRate = 44100.0;
        float mix = 0.3f;
        float delayTime = 0.375f;
        float feedback = 0.45f;

        std::vector<float> bufferL;
        std::vector<float> bufferR;
        size_t writePos = 0;
        float lowpassL = 0.0f;
        float lowpassR = 0.0f;
    };

    // ==========================================
    // 4. Algorithmic 80s Ambient Reverb
    // ==========================================
    class AlgorithmicReverb
    {
    public:
        AlgorithmicReverb();
        void setSampleRate(double sampleRate);
        void setMix(float mix); // 0.0 to 1.0
        void process(float& left, float& right);

    private:
        double sampleRate = 44100.0;
        float mix = 0.35f;

        // Comb filters
        std::vector<float> combL[4];
        std::vector<float> combR[4];
        size_t combPosL[4] { 0, 0, 0, 0 };
        size_t combPosR[4] { 0, 0, 0, 0 };
        float combFilterL[4] { 0, 0, 0, 0 };
        float combFilterR[4] { 0, 0, 0, 0 };

        // Allpass filters
        std::vector<float> allpassL[2];
        std::vector<float> allpassR[2];
        size_t allpassPosL[2] { 0, 0 };
        size_t allpassPosR[2] { 0, 0 };

        void initBuffers();
    };

    // ==========================================
    // 5. Master Output Protection & Limiter
    // ==========================================
    class MasterLimiter
    {
    public:
        MasterLimiter() = default;
        void process(float& left, float& right, float masterVolume);

    private:
        float dcBlockerL = 0.0f;
        float dcBlockerR = 0.0f;
        float prevInL = 0.0f;
        float prevInR = 0.0f;
    };
}
