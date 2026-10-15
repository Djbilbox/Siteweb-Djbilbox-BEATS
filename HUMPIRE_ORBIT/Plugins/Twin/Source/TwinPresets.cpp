#include "TwinPresets.h"
#include "TwinParams.h"

using namespace twin;

namespace
{
    struct P { const char* cat; const char* name; int voices; float detune, delay, spread, hum, drift, width, hpf, lpf, mix; };

    //   categorie             nom                      voix  det    del   spr   hum   drift  width  hpf     lpf      mix
    const P kList[] = {
        { "Voix Trap",         "Trap Double Serre",       1,  6.0f, 12.0f, 30.0f, 25.0f, 0.30f,  40.0f, 150.0f, 12000.0f, 45.0f },
        { "Voix Trap",         "Trap Stack Large",        4, 14.0f, 22.0f, 70.0f, 45.0f, 0.40f, 100.0f, 200.0f, 11000.0f, 50.0f },
        { "Voix Trap",         "Drill Sombre",            2,  9.0f, 16.0f, 40.0f, 20.0f, 0.25f,  85.0f, 180.0f,  8000.0f, 40.0f },
        { "Afrotrap",          "Afrotrap Refrain",        2, 10.0f, 20.0f, 50.0f, 40.0f, 0.45f,  90.0f, 140.0f, 13000.0f, 50.0f },
        { "Afrotrap",          "Afro Chorus Soleil",      3, 16.0f, 26.0f, 60.0f, 55.0f, 0.50f,  95.0f, 120.0f, 14000.0f, 55.0f },
        { "Afrotrap",          "Afro Ad-libs Wide",       2, 22.0f, 30.0f, 80.0f, 60.0f, 0.70f, 100.0f, 300.0f,  9000.0f, 45.0f },
        { "Oriental / Rai",    "Rai Double Chaud",        1,  8.0f, 24.0f, 30.0f, 50.0f, 0.35f,  60.0f, 100.0f, 10000.0f, 45.0f },
        { "Oriental / Rai",    "Chorale Orientale",       4, 18.0f, 32.0f, 80.0f, 60.0f, 0.40f, 100.0f, 160.0f, 12000.0f, 55.0f },
        { "Oriental / Rai",    "Mawal Ample",             2, 12.0f, 35.0f, 60.0f, 65.0f, 0.25f,  90.0f,  90.0f,  9500.0f, 40.0f },
        { "Deep House",        "Deep House Halo",         2, 15.0f, 28.0f, 60.0f, 45.0f, 0.30f, 100.0f, 250.0f,  9000.0f, 45.0f },
        { "Deep House",        "Deep House Choir",        4, 20.0f, 34.0f, 90.0f, 55.0f, 0.35f, 100.0f, 300.0f,  8000.0f, 60.0f },
        { "G-Funk",            "G-Funk Talk Twin",        2, 25.0f, 14.0f, 40.0f, 30.0f, 0.60f,  80.0f, 200.0f,  6500.0f, 45.0f },
        { "G-Funk",            "West Coast Smooth",       3, 12.0f, 22.0f, 50.0f, 40.0f, 0.30f,  90.0f, 160.0f,  9000.0f, 45.0f },
        { "R&B",               "R&B Silk Double",         1,  7.0f, 18.0f, 20.0f, 35.0f, 0.30f,  50.0f, 120.0f, 15000.0f, 40.0f },
        { "R&B",               "R&B Stack Soyeux",        4, 11.0f, 20.0f, 60.0f, 50.0f, 0.35f, 100.0f, 150.0f, 14000.0f, 50.0f },
        { "R&B",               "Neo Soul Velours",        2,  9.0f, 26.0f, 50.0f, 55.0f, 0.25f,  80.0f, 110.0f, 10000.0f, 45.0f },
        { "Podcast / Voix parlee", "Podcast Epaisseur",   1,  4.0f,  8.0f,  0.0f, 15.0f, 0.20f,  20.0f, 120.0f, 12000.0f, 25.0f },
        { "Podcast / Voix parlee", "Voix Off Cinema",     2,  6.0f, 12.0f, 30.0f, 20.0f, 0.20f,  50.0f,  90.0f, 10000.0f, 30.0f },
        { "Pop / Refrain",     "Refrain Pop Large",       4, 13.0f, 24.0f, 70.0f, 45.0f, 0.40f, 100.0f, 140.0f, 16000.0f, 50.0f },
        { "Pop / Refrain",     "Mono Compatible",         2,  8.0f, 15.0f, 40.0f, 30.0f, 0.30f,  35.0f, 150.0f, 13000.0f, 40.0f },
        { "Vintage / Creatif", "ADT Vintage 66",          1,  3.0f, 20.0f,  0.0f, 70.0f, 0.60f,   0.0f, 100.0f,  9000.0f, 50.0f },
        { "Vintage / Creatif", "Detune Extreme",          4, 45.0f, 38.0f,100.0f, 80.0f, 1.00f, 100.0f, 200.0f, 10000.0f, 60.0f },
    };
}

std::vector<HumpirePreset> makeTwinFactoryPresets()
{
    std::vector<HumpirePreset> out;
    for (auto& p : kList)
        out.push_back ({ p.cat, p.name, {
            { id::voices, (float) p.voices }, { id::detune, p.detune }, { id::delay, p.delay }, { id::spread, p.spread },
            { id::humanize, p.hum }, { id::drift, p.drift }, { id::width, p.width }, { id::hpf, p.hpf },
            { id::lpf, p.lpf }, { id::mix, p.mix } } });
    return out;
}
