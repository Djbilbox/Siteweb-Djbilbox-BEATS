#include "Effects.h"

namespace K2000Audio
{
    static constexpr double TWO_PI = 6.28318530717958647692;

    // ==========================================
    // 1. Distortion Effect
    // ==========================================
    void DistortionEffect::setDrive(float driveAmount)
    {
        drive = std::max(0.0f, std::min(driveAmount, 1.0f));
    }

    void DistortionEffect::process(float& left, float& right)
    {
        if (drive < 0.001f) return;

        float preGain = 1.0f + drive * 5.0f;
        float postGain = 1.0f / (1.0f + drive * 1.5f);

        auto saturate = [](float x) {
            float in = x * 1.2f;
            return (in + 0.15f * in * in) / (1.0f + std::abs(in) + 0.15f * in * in);
        };

        left = saturate(left * preGain) * postGain;
        right = saturate(right * preGain) * postGain;
    }

    // ==========================================
    // 2. Dimension Chorus
    // ==========================================
    DimensionChorus::DimensionChorus()
    {
        bufferL.resize(4096, 0.0f);
        bufferR.resize(4096, 0.0f);
    }

    void DimensionChorus::setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0)
        {
            sampleRate = newSampleRate;
            bufferL.assign(static_cast<size_t>(sampleRate * 0.1), 0.0f);
            bufferR.assign(static_cast<size_t>(sampleRate * 0.1), 0.0f);
            writePos = 0;
        }
    }

    void DimensionChorus::setDepth(float newDepth)
    {
        depth = std::max(0.0f, std::min(newDepth, 1.0f));
    }

    void DimensionChorus::process(float& left, float& right)
    {
        if (depth < 0.001f || bufferL.empty()) return;

        bufferL[writePos] = left;
        bufferR[writePos] = right;

        // Quadrature LFOs (0.8 Hz)
        double lfoL = std::sin(lfoPhase);
        double lfoR = std::cos(lfoPhase);
        lfoPhase += TWO_PI * 0.8 / sampleRate;
        if (lfoPhase >= TWO_PI) lfoPhase -= TWO_PI;

        double baseDelayMs = 7.0;
        double modDepthMs = 3.5 * depth;

        double delaySamplesL = (baseDelayMs + modDepthMs * lfoL) * sampleRate * 0.001;
        double delaySamplesR = (baseDelayMs + modDepthMs * lfoR) * sampleRate * 0.001;

        auto readInterp = [](const std::vector<float>& buf, double readPos) {
            size_t size = buf.size();
            while (readPos < 0.0) readPos += size;
            size_t idx0 = static_cast<size_t>(readPos) % size;
            size_t idx1 = (idx0 + 1) % size;
            double frac = readPos - static_cast<size_t>(readPos);
            return static_cast<float>(buf[idx0] * (1.0 - frac) + buf[idx1] * frac);
        };

        float wetL = readInterp(bufferL, static_cast<double>(writePos) - delaySamplesL);
        float wetR = readInterp(bufferR, static_cast<double>(writePos) - delaySamplesR);

        writePos = (writePos + 1) % bufferL.size();

        float wetGain = depth * 0.7f;
        left = left * (1.0f - wetGain * 0.3f) + wetL * wetGain;
        right = right * (1.0f - wetGain * 0.3f) + wetR * wetGain;
    }

    // ==========================================
    // 3. Stereo Tape / Analog Delay
    // ==========================================
    TapeDelay::TapeDelay()
    {
        bufferL.resize(96000, 0.0f);
        bufferR.resize(96000, 0.0f);
    }

    void TapeDelay::setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0)
        {
            sampleRate = newSampleRate;
            bufferL.assign(static_cast<size_t>(sampleRate * 2.0), 0.0f);
            bufferR.assign(static_cast<size_t>(sampleRate * 2.0), 0.0f);
            writePos = 0;
        }
    }

    void TapeDelay::setMix(float newMix)
    {
        mix = std::max(0.0f, std::min(newMix, 1.0f));
    }

    void TapeDelay::setDelayTime(float seconds)
    {
        delayTime = std::max(0.01f, std::min(seconds, 1.5f));
    }

    void TapeDelay::setFeedback(float fb)
    {
        feedback = std::max(0.0f, std::min(fb, 0.95f));
    }

    void TapeDelay::process(float& left, float& right)
    {
        if (mix < 0.001f || bufferL.empty()) return;

        size_t bufSize = bufferL.size();
        size_t delaySamplesL = static_cast<size_t>(delayTime * sampleRate);
        size_t delaySamplesR = static_cast<size_t>(delayTime * 1.15f * sampleRate); // Ping-pong offset

        size_t readPosL = (writePos + bufSize - (delaySamplesL % bufSize)) % bufSize;
        size_t readPosR = (writePos + bufSize - (delaySamplesR % bufSize)) % bufSize;

        float delayedL = bufferL[readPosL];
        float delayedR = bufferR[readPosR];

        // Analog tape lowpass damping & soft saturation in feedback path
        lowpassL += (delayedL - lowpassL) * 0.45f;
        lowpassR += (delayedR - lowpassR) * 0.45f;

        float fbL = std::tanh((lowpassR * 0.8f + lowpassL * 0.2f) * feedback);
        float fbR = std::tanh((lowpassL * 0.8f + lowpassR * 0.2f) * feedback);

        bufferL[writePos] = left + fbL;
        bufferR[writePos] = right + fbR;

        writePos = (writePos + 1) % bufSize;

        left = left * (1.0f - mix * 0.5f) + delayedL * mix;
        right = right * (1.0f - mix * 0.5f) + delayedR * mix;
    }

    // ==========================================
    // 4. Algorithmic Reverb
    // ==========================================
    AlgorithmicReverb::AlgorithmicReverb()
    {
        initBuffers();
    }

    void AlgorithmicReverb::setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0)
        {
            sampleRate = newSampleRate;
            initBuffers();
        }
    }

    void AlgorithmicReverb::initBuffers()
    {
        // 4 comb delays (tuned prime sample lengths)
        static constexpr int combLengths[4] = { 1116, 1188, 1277, 1356 };
        static constexpr int allpassLengths[2] = { 225, 556 };

        double scale = sampleRate / 44100.0;
        for (int i = 0; i < 4; ++i)
        {
            size_t len = static_cast<size_t>(combLengths[i] * scale);
            combL[i].assign(len, 0.0f);
            combR[i].assign(len + 23, 0.0f);
            combPosL[i] = 0;
            combPosR[i] = 0;
        }

        for (int i = 0; i < 2; ++i)
        {
            size_t len = static_cast<size_t>(allpassLengths[i] * scale);
            allpassL[i].assign(len, 0.0f);
            allpassR[i].assign(len + 17, 0.0f);
            allpassPosL[i] = 0;
            allpassPosR[i] = 0;
        }
    }

    void AlgorithmicReverb::setMix(float newMix)
    {
        mix = std::max(0.0f, std::min(newMix, 1.0f));
    }

    void AlgorithmicReverb::process(float& left, float& right)
    {
        if (mix < 0.001f) return;

        float monoIn = (left + right) * 0.5f;

        float outL = 0.0f;
        float outR = 0.0f;

        // Comb banks
        static constexpr float combFeedback = 0.82f;
        static constexpr float damp = 0.25f;

        for (int i = 0; i < 4; ++i)
        {
            size_t posL = combPosL[i];
            size_t sizeL = combL[i].size();
            float valL = combL[i][posL];
            combFilterL[i] = valL * (1.0f - damp) + combFilterL[i] * damp;
            combL[i][posL] = monoIn + combFilterL[i] * combFeedback;
            combPosL[i] = (posL + 1) % sizeL;
            outL += valL;

            size_t posR = combPosR[i];
            size_t sizeR = combR[i].size();
            float valR = combR[i][posR];
            combFilterR[i] = valR * (1.0f - damp) + combFilterR[i] * damp;
            combR[i][posR] = monoIn + combFilterR[i] * combFeedback;
            combPosR[i] = (posR + 1) % sizeR;
            outR += valR;
        }

        // Allpass diffusers
        static constexpr float allpassGain = 0.5f;
        for (int i = 0; i < 2; ++i)
        {
            size_t posL = allpassPosL[i];
            size_t sizeL = allpassL[i].size();
            float bufValL = allpassL[i][posL];
            float newInL = outL + bufValL * allpassGain;
            allpassL[i][posL] = newInL;
            outL = -newInL * allpassGain + bufValL;
            allpassPosL[i] = (posL + 1) % sizeL;

            size_t posR = allpassPosR[i];
            size_t sizeR = allpassR[i].size();
            float bufValR = allpassR[i][posR];
            float newInR = outR + bufValR * allpassGain;
            allpassR[i][posR] = newInR;
            outR = -newInR * allpassGain + bufValR;
            allpassPosR[i] = (posR + 1) % sizeR;
        }

        left = left * (1.0f - mix * 0.4f) + outL * mix * 0.25f;
        right = right * (1.0f - mix * 0.4f) + outR * mix * 0.25f;
    }

    // ==========================================
    // 5. Master Limiter & DC Blocker
    // ==========================================
    void MasterLimiter::process(float& left, float& right, float masterVolume)
    {
        // Master volume scaling
        left *= masterVolume;
        right *= masterVolume;

        // DC Blocker (R = 0.995)
        dcBlockerL = left - prevInL + 0.995f * dcBlockerL;
        prevInL = left;
        left = dcBlockerL;

        dcBlockerR = right - prevInR + 0.995f * dcBlockerR;
        prevInR = right;
        right = dcBlockerR;

        // Transparent soft-knee tanh limiter
        left = std::tanh(left);
        right = std::tanh(right);
    }
}
