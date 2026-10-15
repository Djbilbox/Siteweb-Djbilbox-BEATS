#include "TwinPresets.h"
#include "TwinParams.h"

using namespace twin;

namespace
{
    struct P
    {
        const char* cat;
        const char* name;
        int doubles;
        float detune, delay, humanize, drift, width, hp, lp, mix;
    };

    //            category                 name                    dbl  detune delay humanize drift width   hp      lp       mix
    const P kList[] = {
        { "Trap Vocals",           "Trap Stack",             2,   10.0f, 14.0f, 35.0f, 0.40f,  90.0f, 150.0f, 11000.0f, 45.0f },
        { "Trap Vocals",           "Hard Trap Triple",       3,   14.0f, 11.0f, 20.0f, 0.60f, 100.0f, 180.0f,  9000.0f, 50.0f },
        { "Trap Vocals",           "Ad-lib Ghost",           1,    6.0f, 28.0f, 50.0f, 0.25f,  70.0f, 300.0f,  7000.0f, 35.0f },
        { "Afrotrap",              "Afrotrap Glow",          2,    8.0f, 22.0f, 45.0f, 0.30f,  85.0f, 140.0f, 12000.0f, 42.0f },
        { "Afrotrap",              "Amapiano Sway",          3,   12.0f, 30.0f, 55.0f, 0.22f,  95.0f, 120.0f, 10000.0f, 40.0f },
        { "Oriental / Rai",        "Rai Lead Double",        2,   18.0f, 26.0f, 40.0f, 0.50f,  75.0f, 200.0f,  9500.0f, 40.0f },
        { "Oriental / Rai",        "Oud Shimmer",            4,   22.0f, 34.0f, 50.0f, 0.35f, 100.0f, 260.0f,  8000.0f, 38.0f },
        { "Deep House",            "Deep Vocal Chorus",      4,    9.0f, 33.0f, 60.0f, 0.18f, 100.0f, 180.0f,  9000.0f, 55.0f },
        { "Deep House",            "Club Air",               2,    7.0f, 21.0f, 30.0f, 0.20f, 100.0f, 220.0f, 14000.0f, 45.0f },
        { "G-Funk",                "West Coast Talk",        2,   16.0f, 24.0f, 35.0f, 0.45f,  90.0f, 160.0f,  7500.0f, 45.0f },
        { "G-Funk",                "Synth Double Glide",     3,   24.0f, 19.0f, 40.0f, 0.80f, 100.0f, 140.0f,  6500.0f, 50.0f },
        { "R&B",                   "Silk Double",            2,   11.0f, 18.0f, 50.0f, 0.28f,  85.0f, 130.0f, 13000.0f, 40.0f },
        { "R&B",                   "Velvet Stack",           3,    9.0f, 27.0f, 62.0f, 0.20f,  90.0f, 120.0f, 12500.0f, 42.0f },
        { "R&B",                   "Falsetto Choir",         4,    6.0f, 16.0f, 45.0f, 0.33f, 100.0f, 200.0f, 15000.0f, 48.0f },
        { "Pop Vocals",            "Pop Chorus Wide",        3,   13.0f, 25.0f, 30.0f, 0.40f, 100.0f, 150.0f, 16000.0f, 50.0f },
        { "Pop Vocals",            "Pop Lead Tight",         2,    5.0f, 12.0f, 15.0f, 0.30f,  60.0f, 120.0f, 18000.0f, 35.0f },
        { "Spoken Word / Podcast", "Narrator Thicken",       1,    4.0f,  9.0f,  8.0f, 0.12f,  40.0f,  90.0f, 14000.0f, 25.0f },
        { "Spoken Word / Podcast", "Warm Voiceover",         2,    3.0f, 12.0f, 10.0f, 0.10f,  30.0f, 100.0f, 12000.0f, 22.0f },
        { "Ambient / Cinematic",   "Cinematic Halo",         4,   30.0f, 38.0f, 70.0f, 0.10f, 100.0f, 250.0f, 10000.0f, 60.0f },
        { "Utility",               "Transparent Thickener",  1,    2.0f,  5.0f,  0.0f, 0.10f,   0.0f,  60.0f, 20000.0f, 25.0f },
    };
}

std::vector<HumpirePreset> makeTwinFactoryPresets()
{
    std::vector<HumpirePreset> out;
    for (auto& p : kList)
        out.push_back ({ p.cat, p.name, {
            { id::doubles,  (float) p.doubles },
            { id::detune,   p.detune },
            { id::delay,    p.delay },
            { id::humanize, p.humanize },
            { id::drift,    p.drift },
            { id::width,    p.width },
            { id::hpFreq,   p.hp },
            { id::lpFreq,   p.lp },
            { id::mix,      p.mix } } });
    return out;
}
