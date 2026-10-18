#include "SfzParser.h"
#include <map>
#include <cctype>

namespace humpire
{
namespace
{
using OpMap = std::map<juce::String, juce::String>;

struct Token { bool header; juce::String name, value; };

bool isIdent (char c) { return std::isalnum ((unsigned char) c) || c == '_' || c == '$'; }

// Strips // and /* */ comments and directive lines (#include / #define are not supported).
std::string stripComments (const std::string& s)
{
    std::string out;
    out.reserve (s.size());
    const size_t n = s.size();
    bool lineStart = true;
    for (size_t i = 0; i < n;)
    {
        if (s[i] == '/' && i + 1 < n && s[i + 1] == '/') { while (i < n && s[i] != '\n') ++i; continue; }
        if (s[i] == '/' && i + 1 < n && s[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < n && ! (s[i] == '*' && s[i + 1] == '/')) ++i;
            i += 2;
            continue;
        }
        if (lineStart && s[i] == '#') { while (i < n && s[i] != '\n') ++i; continue; }
        if (s[i] == '\n') lineStart = true;
        else if (! std::isspace ((unsigned char) s[i])) lineStart = false;
        out.push_back (s[i++]);
    }
    return out;
}

std::vector<Token> tokenize (const std::string& s)
{
    std::vector<Token> toks;
    const size_t n = s.size();
    size_t i = 0;
    while (i < n)
    {
        if (s[i] == '<')
        {
            const auto e = s.find ('>', i);
            if (e == std::string::npos) break;
            toks.push_back ({ true, juce::String (s.substr (i + 1, e - i - 1)).trim().toLowerCase(), {} });
            i = e + 1;
            continue;
        }
        if (isIdent (s[i]) && (i == 0 || ! isIdent (s[i - 1])))
        {
            size_t j = i;
            while (j < n && isIdent (s[j])) ++j;
            if (j < n && s[j] == '=')
            {
                const size_t vs = j + 1;
                size_t k = vs;
                while (k < n)
                {
                    if (s[k] == '<') break;
                    if (std::isspace ((unsigned char) s[k]))
                    {
                        size_t m = k;
                        while (m < n && std::isspace ((unsigned char) s[m])) ++m;
                        size_t q = m;
                        while (q < n && isIdent (s[q])) ++q;
                        if (q > m && q < n && s[q] == '=') break; // start of the next opcode
                    }
                    ++k;
                }
                toks.push_back ({ false, juce::String (s.substr (i, j - i)).toLowerCase(),
                                  juce::String::fromUTF8 (s.substr (vs, k - vs).c_str()).trim() });
                i = k;
                continue;
            }
            i = j;
            continue;
        }
        ++i;
    }
    return toks;
}

void mergeInto (OpMap& dst, const OpMap& src) { for (auto& [k, v] : src) dst[k] = v; }

juce::String get (const OpMap& m, const char* key) { auto it = m.find (key); return it == m.end() ? juce::String() : it->second; }
bool has (const OpMap& m, const char* key) { return m.find (key) != m.end(); }
int getInt (const OpMap& m, const char* key, int def) { return has (m, key) ? parseNoteOrInt (get (m, key), def) : def; }
float getFloat (const OpMap& m, const char* key, float def) { return has (m, key) ? get (m, key).getFloatValue() : def; }
juce::int64 getI64 (const OpMap& m, const char* key, juce::int64 def) { return has (m, key) ? get (m, key).getLargeIntValue() : def; }
} // namespace

int parseNoteOrInt (const juce::String& str, int fallback)
{
    auto s = str.trim();
    if (s.isEmpty()) return fallback;
    if (s.containsOnly ("-0123456789")) return s.getIntValue();

    // Note name: c4 = 60 (sfizz convention), # and b accepted
    static const int base[7] = { 9, 11, 0, 2, 4, 5, 7 }; // a b c d e f g
    const auto letter = std::tolower ((unsigned char) s[0]) - 'a';
    if (letter < 0 || letter > 6) return fallback;
    int semis = base[letter], idx = 1;
    if (s.length() > idx && (s[idx] == '#')) { ++semis; ++idx; }
    else if (s.length() > idx && s[idx] == 'b') { --semis; ++idx; }
    if (idx >= s.length()) return fallback;
    const int oct = s.substring (idx).getIntValue();
    return juce::jlimit (0, 127, (oct + 1) * 12 + semis);
}

SfzFile parseSfz (const juce::File& file)
{
    SfzFile out;
    const auto text = stripComments (file.loadFileAsString().toStdString());
    const auto toks = tokenize (text);

    OpMap control, global, master, group, region;
    enum class Scope { kNone, kControl, kGlobal, kMaster, kGroup, kRegion } scope = Scope::kNone;
    const auto baseDir = file.getParentDirectory();

    auto commit = [&]
    {
        if (scope != Scope::kRegion) return;
        OpMap m;
        mergeInto (m, global); mergeInto (m, master); mergeInto (m, group); mergeInto (m, region);

        auto rel = get (m, "sample").replaceCharacter ('\\', '/');
        if (rel.isEmpty() || rel.startsWithChar ('*'))
        {
            out.warnings.add ("region ignored (missing sample or unsupported generator)");
            region.clear();
            return;
        }

        SfzRegionDef d;
        d.sample = baseDir.getChildFile (get (control, "default_path").replaceCharacter ('\\', '/') + rel);

        if (has (m, "key")) { d.lokey = d.hikey = d.keycenter = getInt (m, "key", 60); }
        d.lokey     = getInt (m, "lokey", d.lokey);
        d.hikey     = getInt (m, "hikey", d.hikey);
        d.keycenter = getInt (m, "pitch_keycenter", d.keycenter);
        d.lovel     = juce::jlimit (0, 127, getInt (m, "lovel", 0));
        d.hivel     = juce::jlimit (0, 127, getInt (m, "hivel", 127));
        d.lorand    = getFloat (m, "lorand", 0.f);
        d.hirand    = getFloat (m, "hirand", 1.f);
        d.seqLength   = juce::jmax (1, getInt (m, "seq_length", 1));
        d.seqPosition = juce::jmax (1, getInt (m, "seq_position", 1));
        d.volumeDb  = getFloat (m, "volume", 0.f);
        d.pan       = juce::jlimit (-100.f, 100.f, getFloat (m, "pan", 0.f));
        d.tuneCents = getFloat (m, "tune", 0.f);
        d.transpose = getInt (m, "transpose", 0);
        d.pitchKeytrack = getFloat (m, "pitch_keytrack", 100.f);

        auto lm = (has (m, "loop_mode") ? get (m, "loop_mode") : get (m, "loopmode")).trim().toLowerCase();
        if (lm == "one_shot") d.loop = SfzRegionDef::Loop::oneShot;
        else if (lm == "loop_continuous") d.loop = SfzRegionDef::Loop::continuous;
        else if (lm == "loop_sustain") d.loop = SfzRegionDef::Loop::sustain;

        d.offset    = getI64 (m, "offset", 0);
        d.end       = getI64 (m, "end", -1);
        d.loopStart = has (m, "loop_start") ? getI64 (m, "loop_start", -1) : getI64 (m, "loopstart", -1);
        d.loopEnd   = has (m, "loop_end")   ? getI64 (m, "loop_end", -1)   : getI64 (m, "loopend", -1);

        d.ampAttack  = juce::jmax (0.f, getFloat (m, "ampeg_attack", 0.f));
        d.ampDecay   = juce::jmax (0.f, getFloat (m, "ampeg_decay", 0.f));
        d.ampSustain = juce::jlimit (0.f, 100.f, getFloat (m, "ampeg_sustain", 100.f));
        d.ampRelease = juce::jmax (0.003f, getFloat (m, "ampeg_release", 0.03f));
        d.ampVeltrack = juce::jlimit (0.f, 100.f, getFloat (m, "amp_veltrack", 100.f));

        d.group = getInt (m, "group", 0);
        d.offBy = getInt (m, "off_by", 0);
        d.triggerRelease = get (m, "trigger").trim().equalsIgnoreCase ("release");
        d.swLast = has (m, "sw_last") ? getInt (m, "sw_last", -1) : -1;

        if (has (m, "sw_lokey")) out.swLo = getInt (m, "sw_lokey", -1);
        if (has (m, "sw_hikey")) out.swHi = getInt (m, "sw_hikey", -1);
        if (has (m, "sw_default")) out.swDefault = getInt (m, "sw_default", -1);

        out.regions.push_back (std::move (d));
        region.clear();
    };

    for (auto& t : toks)
    {
        if (t.header)
        {
            commit();
            if (t.name == "control")      { scope = Scope::kControl; }
            else if (t.name == "global")  { scope = Scope::kGlobal; global.clear(); master.clear(); group.clear(); region.clear(); }
            else if (t.name == "master")  { scope = Scope::kMaster; master.clear(); group.clear(); region.clear(); }
            else if (t.name == "group")   { scope = Scope::kGroup;  group.clear(); region.clear(); }
            else if (t.name == "region")  { scope = Scope::kRegion; region.clear(); }
            else                          { scope = Scope::kNone; }
            continue;
        }
        switch (scope)
        {
            case Scope::kControl: control[t.name] = t.value; break;
            case Scope::kGlobal:  global[t.name]  = t.value; break;
            case Scope::kMaster:  master[t.name]  = t.value; break;
            case Scope::kGroup:   group[t.name]   = t.value; break;
            case Scope::kRegion:  region[t.name]  = t.value; break;
            default: break;
        }
    }
    commit();

    // sw_default / sw_lokey / sw_hikey may be declared in <global>/<master>/<group> without a region: read them again.
    for (auto* m : { &global, &master, &group })
    {
        if (out.swLo < 0 && has (*m, "sw_lokey")) out.swLo = getInt (*m, "sw_lokey", -1);
        if (out.swHi < 0 && has (*m, "sw_hikey")) out.swHi = getInt (*m, "sw_hikey", -1);
        if (out.swDefault < 0 && has (*m, "sw_default")) out.swDefault = getInt (*m, "sw_default", -1);
    }
    return out;
}
} // namespace humpire
