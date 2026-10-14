#include "PhrasePlayer.h"
#include <cmath>

namespace humpire
{
void PhrasePlayer::prepare (double sampleRate)
{
    sr = sampleRate;
    reset();
}

void PhrasePlayer::reset()
{
    for (auto& a : actives) a.on = false;
    for (auto& o : offs) o.used = false;
    clock = 0;
    seqBeat = 0.0;
}

void PhrasePlayer::trigger (int key, const std::array<bool, kNumInstruments>& activeSlots, int sampleOffset)
{
    const int idx = key - kPhraseLo;
    for (int s = 0; s < kNumInstruments; ++s)
    {
        if (! activeSlots[(size_t) s]) continue;
        const auto& list = phrasesFor (s);
        if (list.empty()) continue;

        // emplacement libre (ou recyclage de la meme touche/slot)
        Active* target = nullptr;
        for (auto& a : actives)
            if (a.on && a.key == key && a.slot == s) { target = &a; break; }
        if (target == nullptr)
            for (auto& a : actives)
                if (! a.on) { target = &a; break; }
        if (target == nullptr) continue;

        target->on = true;
        target->slot = s;
        target->key = key;
        target->phrase = &list[(size_t) (idx % (int) list.size())];
        target->beat = 0.0;
        target->startOffset = sampleOffset;
        target->pendingSync = true;
    }
}

void PhrasePlayer::release (int key)
{
    for (auto& a : actives)
        if (a.on && a.key == key) a.on = false;
}

void PhrasePlayer::emit (SlotMidi& out, int slot, int note, int vel, int offset, double lenSamples)
{
    out[(size_t) slot].addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) juce::jlimit (1, 127, vel)), offset);
    for (auto& o : offs)
        if (! o.used)
        {
            o = { true, slot, note, clock + offset + (juce::int64) juce::jmax (64.0, lenSamples) };
            return;
        }
}

void PhrasePlayer::process (SlotMidi& out, int n, double bpm, bool hostPlaying, double hostPpq,
                            bool seqRunning, double swing, int stepsPerBeat, const std::array<bool, kNumInstruments>& activeSlots)
{
    const double bps = bpm / 60.0 / sr; // beats par echantillon

    // --- fins de notes programmees
    for (auto& o : offs)
        if (o.used && o.at < clock + n)
        {
            out[(size_t) o.slot].addEvent (juce::MidiMessage::noteOff (1, o.note), (int) juce::jmax<juce::int64> (0, o.at - clock));
            o.used = false;
        }

    // --- phrases
    for (auto& a : actives)
    {
        if (! a.on || a.phrase == nullptr) continue;
        const double len = (double) a.phrase->lengthBeats;
        const int s0 = juce::jlimit (0, n, a.startOffset);
        a.startOffset = 0;

        if (a.pendingSync)
        {
            // Synchronise la phrase sur la grille de l'hote (si l'hote joue), sinon part du debut.
            a.beat = hostPlaying ? std::fmod (hostPpq + (double) s0 * bps, len) : 0.0;
            a.pendingSync = false;
        }

        const double b0 = a.beat, b1 = b0 + (double) (n - s0) * bps;
        const int k0 = (int) std::floor (b0 / len), k1 = (int) std::floor (b1 / len);
        for (const auto& e : a.phrase->events)
            for (int k = k0; k <= k1; ++k)
            {
                const double t = (double) k * len + (double) e.beat;
                if (t >= b0 && t < b1)
                    emit (out, a.slot, e.note, e.vel, s0 + (int) ((t - b0) / bps), (double) e.len / bps);
            }
        a.beat = std::fmod (b1, len);
        if (a.beat < 0.0) a.beat += len;
    }

    // --- sequenceur 16 pas
    if (seqRunning)
    {
        const double stepLen = 1.0 / (double) juce::jmax (1, stepsPerBeat);
        const double b0 = hostPlaying ? hostPpq : seqBeat;
        const double b1 = b0 + (double) n * bps;
        const int kFirst = (int) std::floor (b0 / stepLen) - 1, kLast = (int) std::ceil (b1 / stepLen);

        for (int k = kFirst; k <= kLast; ++k)
        {
            const double t = (double) k * stepLen + ((k & 1) != 0 ? swing * stepLen : 0.0);
            if (t < b0 || t >= b1) continue;
            const int step = ((k % kSeqSteps) + kSeqSteps) % kSeqSteps;
            const int off = juce::jlimit (0, n - 1, (int) ((t - b0) / bps));
            seqDisplayStep.store (step);

            for (int inst = 0; inst < kSeqInstruments; ++inst)
            {
                const int slot = kSeqSlot[inst];
                if (! activeSlots[(size_t) slot]) continue;
                for (int lane = 0; lane < kSeqLanes; ++lane)
                {
                    const int v = grid[(size_t) inst][(size_t) lane][(size_t) step].load();
                    if (v > 0) emit (out, slot, kSeqNotes[inst][lane], v == 2 ? 118 : 88, off, stepLen * 0.9 / bps);
                }
            }
        }
        if (! hostPlaying) seqBeat = b1;
    }
    else
    {
        seqBeat = 0.0;
        seqDisplayStep.store (-1);
    }

    clock += n;
}
} // namespace humpire
