#pragma once
// Header-only exporter of a model::Library as WAV files + an SFZ file that the engine's SFZ parser reads back
// to the same regions (std only, no JUCE).  Used by ChaabiRenderLibrary and by the smoke test (round-trip check).
//
//   <outDir>/<Folder>/<Folder>.sfz
//   <outDir>/<Folder>/samples/<articulation>/<zone name>.wav
#include "Model/ModelLibrary.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <locale>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace humpire::model::writer
{
inline std::string num (double v)
{
    std::ostringstream os;
    os.imbue (std::locale::classic());
    os.precision (7);
    os << v;
    return os.str();
}

namespace detail
{
inline void put16 (std::ofstream& f, std::uint32_t v) { const char b[2] = { (char) (v & 255u), (char) ((v >> 8) & 255u) }; f.write (b, 2); }
inline void put32 (std::ofstream& f, std::uint32_t v) { const char b[4] = { (char) (v & 255u), (char) ((v >> 8) & 255u), (char) ((v >> 16) & 255u), (char) ((v >> 24) & 255u)  }; f.write (b, 4); }
} // namespace detail

// bits: 16, 24 (PCM) or 32 (IEEE float, lossless).  loopStart/loopEnd >= 0 add a 'smpl' chunk (loop end inclusive).
inline bool writeWav (const std::filesystem::path& path, const std::vector<float>& data, int channels, double sampleRate, int bits,
                      int loopStart = -1, int loopEnd = -1, int rootKey = 60)
{
    std::ofstream f (path, std::ios::binary);
    if (! f) return false;
    const std::uint32_t ch = (std::uint32_t) channels, sr = (std::uint32_t) std::lround (sampleRate);
    const std::uint32_t bytesPer = (std::uint32_t) bits / 8u;
    const std::uint32_t dataBytes = (std::uint32_t) data.size() * bytesPer;
    const bool loop = loopStart >= 0 && loopEnd > loopStart;
    const std::uint32_t smplBytes = loop ? 36u + 24u : 0u;
    const std::uint32_t riffSize = 4u + (8u + 16u) + (loop ? 8u + smplBytes : 0u) + (8u + dataBytes + (dataBytes & 1u));

    f.write ("RIFF", 4); detail::put32 (f, riffSize); f.write ("WAVE", 4);
    f.write ("fmt ", 4); detail::put32 (f, 16u);
    detail::put16 (f, bits == 32 ? 3u : 1u); detail::put16 (f, ch); detail::put32 (f, sr);
    detail::put32 (f, sr * ch * bytesPer); detail::put16 (f, ch * bytesPer); detail::put16 (f, (std::uint32_t) bits);
    if (loop)
    {
        f.write ("smpl", 4); detail::put32 (f, smplBytes);
        detail::put32 (f, 0u); detail::put32 (f, 0u);                                    // manufacturer, product
        detail::put32 (f, (std::uint32_t) std::lround (1.0e9 / sampleRate));              // sample period (ns)
        detail::put32 (f, (std::uint32_t) rootKey); detail::put32 (f, 0u);                // unity note, pitch fraction
        detail::put32 (f, 0u); detail::put32 (f, 0u);                                    // SMPTE format / offset
        detail::put32 (f, 1u); detail::put32 (f, 0u);                                    // 1 loop, 0 sampler data bytes
        detail::put32 (f, 0u); detail::put32 (f, 0u);                                    // loop id, type (forward)
        detail::put32 (f, (std::uint32_t) loopStart); detail::put32 (f, (std::uint32_t) loopEnd);
        detail::put32 (f, 0u); detail::put32 (f, 0u);                                    // fraction, play count (infinite)
    }
    f.write ("data", 4); detail::put32 (f, dataBytes);
    for (float s : data)
    {
        if (bits == 32)
        {
            std::uint32_t u;
            static_assert (sizeof (u) == sizeof (s), "float must be 32 bit");
            std::memcpy (&u, &s, sizeof (u));
            detail::put32 (f, u);
        }
        else if (bits == 24)
        {
            const double c = s > 1.0f ? 1.0 : (s < -1.0f ? -1.0 : (double) s);
            const long v = std::lround (c * 8388607.0);
            const std::uint32_t u = (std::uint32_t) (v < 0 ? v + 16777216 : v);
            const char b[3] = { (char) (u & 255u), (char) ((u >> 8) & 255u), (char) ((u >> 16) & 255u) };
            f.write (b, 3);
        }
        else
        {
            const double c = s > 1.0f ? 1.0 : (s < -1.0f ? -1.0 : (double) s);
            const long v = std::lround (c * 32767.0);
            detail::put16 (f, (std::uint32_t) (v < 0 ? v + 65536 : v));
        }
    }
    if (dataBytes & 1u) f.put (0);
    return (bool) f;
}

inline const char* loopOpcode (Loop l)
{
    switch (l)
    {
        case Loop::oneShot: return "one_shot";
        case Loop::continuous: return "loop_continuous";
        case Loop::sustain: return "loop_sustain";
        default: return "no_loop";
    }
}

// SFZ text equivalent to what the engine builds from the library (every opcode explicit, one region per line).
inline std::string makeSfz (const Library& lib, const InstrumentSpec& spec)
{
    std::ostringstream o;
    o.imbue (std::locale::classic());
    o << "// HUMPIRE CHAABI ORCHESTRA - built-in modelled " << spec.folder << "\n"
      << "// Generated by ChaabiRenderLibrary from Source/Plugin/Model. Modelled instrument set, not a recording of real instruments.\n"
      << "// Zones: " << lib.zones.size() << "  (alias zones reuse the WAV of another zone)\n\n"
      << "<control>\ndefault_path=samples/\n\n<global>\n";
    if (lib.swLo >= 0) o << "sw_lokey=" << lib.swLo << " sw_hikey=" << lib.swHi << " sw_default=" << lib.swDefault << "\n";
    o << "\n";

    std::string lastArt;
    for (std::size_t i = 0; i < lib.zones.size(); ++i)
    {
        const Zone& z = lib.zones[i];
        const Zone& a = lib.audioZone (i);
        if (z.articulation != lastArt) { o << "\n// ---------- " << z.articulation << (z.triggerRelease ? " (release trigger)" : "") << "\n"; lastArt = z.articulation; }
        o << "<region> sample=" << a.articulation << "/" << a.name << ".wav"
          << " lokey=" << z.lokey << " hikey=" << z.hikey << " pitch_keycenter=" << z.keycenter
          << " lovel=" << z.lovel << " hivel=" << z.hivel
          << " seq_length=" << z.seqLength << " seq_position=" << z.seqPosition;
        if (z.swLast >= 0) o << " sw_last=" << z.swLast;
        o << " volume=" << num (z.volumeDb) << " pan=" << num (z.pan) << " tune=" << num (z.tuneCents)
          << " pitch_keytrack=" << num (z.pitchKeytrack)
          << " ampeg_attack=" << num (z.ampAttack) << " ampeg_decay=" << num (z.ampDecay)
          << " ampeg_sustain=" << num (z.ampSustain) << " ampeg_release=" << num (z.ampRelease)
          << " amp_veltrack=" << num (z.ampVeltrack)
          << " loop_mode=" << loopOpcode (z.loop);
        if (z.loop == Loop::continuous || z.loop == Loop::sustain) o << " loop_start=" << z.loopStart << " loop_end=" << z.loopEnd;
        if (z.group != 0) o << " group=" << z.group;
        if (z.offBy != 0) o << " off_by=" << z.offBy;
        if (z.triggerRelease) o << " trigger=release";
        o << "\n";
    }
    return o.str();
}

struct WriteResult { bool ok = true; std::string error; int wavFiles = 0; std::filesystem::path sfzPath; };

// Writes <outDir>/<Folder>/<Folder>.sfz + WAV files.  wav=false writes the SFZ only.
inline WriteResult writeLibrary (const Library& lib, int instrumentIndex, const std::filesystem::path& outDir, int bits, bool wav = true)
{
    namespace fs = std::filesystem;
    WriteResult r;
    const InstrumentSpec& spec = kSpecs[instrumentIndex];
    const fs::path root = outDir / spec.folder;
    std::error_code ec;
    fs::create_directories (root / "samples", ec);
    if (ec) { r.ok = false; r.error = "cannot create " + root.string() + ": " + ec.message(); return r; }

    if (wav)
        for (std::size_t i = 0; i < lib.zones.size(); ++i)
        {
            const Zone& z = lib.zones[i];
            if (z.sharedWith >= 0) continue;
            const fs::path dir = root / "samples" / z.articulation;
            fs::create_directories (dir, ec);
            if (ec) { r.ok = false; r.error = "cannot create " + dir.string(); return r; }
            const bool looped = z.loop == Loop::continuous || z.loop == Loop::sustain;
            if (! writeWav (dir / (z.name + ".wav"), z.data, z.numChannels, z.sampleRate, bits,
                            looped ? z.loopStart : -1, looped ? z.loopEnd : -1, z.keycenter))
            {
                r.ok = false; r.error = "cannot write " + (dir / (z.name + ".wav")).string(); return r;
            }
            ++r.wavFiles;
        }

    r.sfzPath = root / (std::string (spec.folder) + ".sfz");
    std::ofstream f (r.sfzPath, std::ios::binary);
    if (! f) { r.ok = false; r.error = "cannot write " + r.sfzPath.string(); return r; }
    f << makeSfz (lib, spec);
    return r;
}
} // namespace humpire::model::writer
