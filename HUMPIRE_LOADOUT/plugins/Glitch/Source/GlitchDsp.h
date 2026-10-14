#pragma once
#include <juce_dsp/juce_dsp.h>
#include <HumpireCore/LcdScreen.h>   // ScopeBuffer

// ============================================================================
// DSP HUMPIRE GLITCH (ecrit pour ce projet) :
//   in -> drive (saturation douce) -> reducteur de sample-rate (sample & hold,
//   aliasing volontaire) -> dither -> quantification N bits (fractionnaire)
//   -> [filtre haut-parleur mono : HP/LP + saturation] -> mix -> gain de sortie.
// Zero allocation dans process(). Latence : 0 echantillon.
// ============================================================================
namespace glitch
{
struct DspParams
{
    float bits = 8.0f, rate = 48000.0f, drive = 0.0f, tone = 0.4f, mix = 1.0f, outDb = 0.0f;
    bool  speaker = false;
    int   dither = 0;            // 0 Off, 1 RPDF, 2 TPDF
};

class GlitchDsp
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 1 };
        hp.prepare (spec);  hp.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        lp.prepare (spec);  lp.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        constexpr double ramp = 0.03;   // 30 ms anti-zipper
        bitsSm.reset (sampleRate, ramp);      rateSm.reset (sampleRate, ramp);
        driveSm.reset (sampleRate, ramp);     toneSm.reset (sampleRate, ramp);
        mixSm.reset (sampleRate, ramp);       outSm.reset (sampleRate, ramp);
        spkSm.reset (sampleRate, 0.02);
        reset();
    }

    void reset()
    {
        hp.reset();  lp.reset();
        phase = 1.0f;  held[0] = held[1] = 0.0f;
        first = true;
    }

    /** Appele depuis processBlock (thread audio) : met a jour les cibles. */
    void setParams (const DspParams& p)
    {
        const float outGain = juce::Decibels::decibelsToGain (p.outDb);
        if (first)    // 1er bloc : pas de glissando depuis des valeurs par defaut
        {
            bitsSm.setCurrentAndTargetValue (p.bits);   rateSm.setCurrentAndTargetValue (p.rate);
            driveSm.setCurrentAndTargetValue (p.drive); toneSm.setCurrentAndTargetValue (p.tone);
            mixSm.setCurrentAndTargetValue (p.mix);     outSm.setCurrentAndTargetValue (outGain);
            spkSm.setCurrentAndTargetValue (p.speaker ? 1.0f : 0.0f);
            first = false;
        }
        bitsSm.setTargetValue (p.bits);   rateSm.setTargetValue (p.rate);
        driveSm.setTargetValue (p.drive); toneSm.setTargetValue (p.tone);
        mixSm.setTargetValue (p.mix);     outSm.setTargetValue (outGain);
        spkSm.setTargetValue (p.speaker ? 1.0f : 0.0f);
        dither = p.dither;
    }

    /** Traite 1 ou 2 canaux en place. Pousse l'amplitude de sortie dans `scope` (colonne / `scopeStride` echantillons). */
    void process (juce::AudioBuffer<float>& buf, humpire::ScopeBuffer* scope, int scopeStride) noexcept
    {
        const int n = buf.getNumSamples();
        const int nch = juce::jmin (2, buf.getNumChannels());
        if (nch == 0) return;
        float* ch[2] = { buf.getWritePointer (0), nch > 1 ? buf.getWritePointer (1) : nullptr };

        for (int i = 0; i < n; ++i)
        {
            const float bits  = bitsSm.getNextValue();
            const float rate  = rateSm.getNextValue();
            const float drive = driveSm.getNextValue();
            const float tone  = toneSm.getNextValue();
            const float mix   = mixSm.getNextValue();
            const float outG  = outSm.getNextValue();
            const float spk   = spkSm.getNextValue();

            if ((i & 15) == 0)   // cutoffs mis a jour tous les 16 echantillons
            {
                hp.setCutoffFrequency (500.0f * std::pow (120.0f / 500.0f, tone));     // 500 -> 120 Hz
                lp.setCutoffFrequency (juce::jmin (3500.0f * std::pow (8000.0f / 3500.0f, tone),
                                                   (float) sr * 0.45f));               // 3.5 -> 8 kHz
            }

            // sample & hold : phase accumulee, declenche quand >= 1
            phase += juce::jmin (1.0f, rate / (float) sr);
            const bool trig = phase >= 1.0f;
            if (trig) phase -= std::floor (phase);

            const float levels = std::exp2 (bits - 1.0f);    // bits=1 -> 1 niveau (-1,0,1) ; 16 -> 32768
            const float lsb = 1.0f / levels;
            const float g = 1.0f + 9.0f * drive, invSqrtG = 1.0f / std::sqrt (g);

            float dry[2] { 0.0f, 0.0f }, q[2] { 0.0f, 0.0f };
            for (int c = 0; c < nch; ++c)
            {
                float x = ch[c][i];
                dry[c] = x;
                x += drive * (std::tanh (g * x) * invSqrtG - x);       // drive: transparent a 0
                if (trig) held[c] = x;
                float y = held[c];

                if (dither == 1)      y += (rand01() - 0.5f) * lsb;                // RPDF +-0.5 LSB
                else if (dither == 2) y += (rand01() - rand01()) * lsb;            // TPDF +-1 LSB
                q[c] = juce::jlimit (-1.0f, 1.0f, std::floor (y * levels + 0.5f) * lsb);
            }

            // haut-parleur mono : somme -> HP -> LP -> saturation douce + compensation
            const float mono = nch == 2 ? 0.5f * (q[0] + q[1]) : q[0];
            float s = lp.processSample (0, hp.processSample (0, mono));
            s = std::tanh (1.6f * s) * 0.85f;

            float last = 0.0f;
            for (int c = 0; c < nch; ++c)
            {
                const float wet = q[c] + spk * (s - q[c]);
                const float o = (dry[c] + mix * (wet - dry[c])) * outG;
                ch[c][i] = o;
                last = juce::jmax (last, std::abs (o));
            }

            if (scope != nullptr)
            {
                scopeAcc = juce::jmax (scopeAcc, last);
                if (++scopeCount >= scopeStride) { scope->push (juce::jmin (1.0f, scopeAcc)); scopeAcc = 0.0f; scopeCount = 0; }
            }
        }
    }

private:
    float rand01() noexcept            // xorshift32 -> [0,1)
    {
        rng ^= rng << 13;  rng ^= rng >> 17;  rng ^= rng << 5;
        return (float) (rng >> 8) * (1.0f / 16777216.0f);
    }

    double sr = 44100.0;
    juce::dsp::StateVariableTPTFilter<float> hp, lp;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bitsSm, driveSm, toneSm, mixSm, spkSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> rateSm, outSm;
    float phase = 1.0f, held[2] { 0.0f, 0.0f }, scopeAcc = 0.0f;
    int   dither = 0, scopeCount = 0;
    bool  first = true;
    uint32_t rng = 0x9E3779B9u;
};
} // namespace glitch
