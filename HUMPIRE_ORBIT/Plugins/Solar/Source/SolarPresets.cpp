#include "SolarPresets.h"
#include "SolarParams.h"

using namespace solar;

namespace
{
    struct P { const char* cat; const char* name; int scale; float retune, flex, human, amount, vibD, vibR, vibDel, port, mix; };

    //            categorie        nom                    gamme       ret   flex hum  amt  vibD vibR vDel port mix
    const P kList[] = {
        { "Rap Trap",       "Trap Lock",            Minor,      0.0f,  0.0f,  0.0f, 100.0f,  0.0f, 5.5f,  300.0f,   0.0f, 100.0f },
        { "Rap Trap",       "Trap Melody",          PentMinor,  8.0f,  0.0f, 10.0f, 100.0f,  0.0f, 5.5f,  300.0f,   0.0f, 100.0f },
        { "Rap Trap",       "Drill Froid",          Minor,      3.0f,  0.0f,  0.0f, 100.0f,  0.0f, 5.5f,  300.0f,   0.0f, 100.0f },
        { "Rap Trap",       "Melo Glide",           Minor,     15.0f,  0.0f, 20.0f, 100.0f, 10.0f, 6.0f,  250.0f,  45.0f, 100.0f },
        { "Chant Afro",     "Afro Smooth",          PentMajor, 45.0f, 10.0f, 40.0f, 100.0f, 12.0f, 5.2f,  400.0f,  20.0f, 100.0f },
        { "Chant Afro",     "Afrobeat Soie",        Major,     60.0f, 15.0f, 50.0f,  95.0f, 14.0f, 5.0f,  450.0f,  25.0f, 100.0f },
        { "Chant Afro",     "Amapiano Vox",         PentMajor, 30.0f,  8.0f, 30.0f, 100.0f, 10.0f, 5.8f,  300.0f,   0.0f, 100.0f },
        { "Voix Rap Francais", "Rap FR Naturel",    Chromatic,120.0f, 25.0f, 70.0f,  80.0f,  0.0f, 5.5f,  300.0f,   0.0f, 100.0f },
        { "Voix Rap Francais", "Rap FR Melodique",  Minor,     40.0f, 12.0f, 40.0f, 100.0f,  8.0f, 5.5f,  350.0f,  15.0f, 100.0f },
        { "Voix Rap Francais", "Refrain Pop FR",    Major,     22.0f,  6.0f, 25.0f, 100.0f,  6.0f, 5.5f,  300.0f,   0.0f, 100.0f },
        { "Ad-libs",        "Ad-lib Robot",         Chromatic,  0.0f,  0.0f,  0.0f, 100.0f,  0.0f, 5.5f,  300.0f,   0.0f, 100.0f },
        { "Ad-libs",        "Ad-lib Glide Large",   PentMinor, 10.0f,  0.0f, 10.0f, 100.0f,  0.0f, 5.5f,  300.0f,  80.0f, 100.0f },
        { "Ad-libs",        "Ad-lib Sparkle",       Major,      5.0f,  0.0f, 10.0f, 100.0f, 20.0f, 6.5f,  100.0f,   0.0f, 100.0f },
        { "Voix feminine",  "Fem Pop Gloss",        Major,     20.0f, 10.0f, 30.0f, 100.0f,  8.0f, 5.8f,  300.0f,   0.0f, 100.0f },
        { "Voix feminine",  "Fem Soul Douce",       Major,     80.0f, 20.0f, 60.0f,  90.0f, 15.0f, 5.2f,  500.0f,  20.0f, 100.0f },
        { "Voix feminine",  "Fem RnB Trap",         Minor,     14.0f,  4.0f, 20.0f, 100.0f, 10.0f, 6.0f,  250.0f,  30.0f, 100.0f },
        { "Oriental",       "Oriental Hijaz",       Hijaz,     35.0f, 10.0f, 30.0f, 100.0f, 18.0f, 5.5f,  300.0f,  60.0f, 100.0f },
        { "Oriental",       "Maqam Rast",           Rast,      70.0f, 15.0f, 50.0f, 100.0f, 12.0f, 5.0f,  400.0f,  40.0f, 100.0f },
        { "Oriental",       "Maqam Bayati",         Bayati,    70.0f, 15.0f, 50.0f, 100.0f, 22.0f, 5.0f,  400.0f,  90.0f, 100.0f },
        { "Deep House / G-Funk", "Deep House Haze", Minor,     50.0f, 10.0f, 40.0f, 100.0f,  8.0f, 5.0f,  400.0f,   0.0f,  85.0f },
        { "Deep House / G-Funk", "G-Funk Talk",     Chromatic, 100.0f, 20.0f, 60.0f,  60.0f, 15.0f, 5.5f,  300.0f,  30.0f, 100.0f },
        { "Correction",     "Transparente",         Chromatic, 200.0f, 30.0f, 80.0f,  70.0f,  0.0f, 5.5f,  300.0f,   0.0f, 100.0f },
    };
}

std::vector<HumpirePreset> makeSolarFactoryPresets()
{
    std::vector<HumpirePreset> out;
    for (auto& p : kList)
        out.push_back ({ p.cat, p.name, {
            { id::mode, 0.0f }, { id::scale, (float) p.scale }, { id::retune, p.retune }, { id::flex, p.flex },
            { id::humanize, p.human }, { id::amount, p.amount }, { id::vibDepth, p.vibD }, { id::vibRate, p.vibR },
            { id::vibDelay, p.vibDel }, { id::portamento, p.port }, { id::mix, p.mix } } });
    return out;
}
