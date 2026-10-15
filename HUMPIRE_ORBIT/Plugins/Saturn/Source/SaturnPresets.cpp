#include "SaturnPresets.h"
#include "SaturnParams.h"

using namespace saturn;

namespace
{
    // Une voix : active, demi-tons, degres, niveau dB, pan (-100..100), detune ct, delai ms
    struct V { bool on; int semi, deg; float level, pan, detune, delay; };
    // Un preset : categorie, nom, mode (0 demi-tons / 1 degres), gamme, formant, glide ms, correction %, humanize %, largeur %, mix %
    struct P { const char* cat; const char* name; int mode; int scale; bool formant;
               float glide, corr, human, width, mix; V v[kNumVoices]; };

    const V off { false, 0, 0, -12.0f, 0.0f, 0.0f, 0.0f };

    const P kList[] = {
        // ---------------------------------------------------------------- Voix Trap
        { "Voix Trap", "Trap Hook Stack", ModeDegrees, Minor, true, 20, 0, 15, 80, 50,
          { { true, 4, 2, -5, -45, -3, 0 }, { true, 7, 4, -7, 45, 3, 0 }, { true, -12, -7, -8, 0, 0, 0 }, off } },
        { "Voix Trap", "Trap Octave Sombre", ModeDegrees, Minor, true, 15, 0, 10, 60, 45,
          { { true, -12, -7, -3, -15, -4, 0 }, { true, -12, -7, -6, 15, 6, 12 }, off, off } },
        { "Voix Trap", "Drill Octaves", ModeSemitones, Minor, true, 10, 0, 5, 70, 45,
          { { true, -12, -7, -4, -20, 0, 0 }, { true, 12, 7, -11, 20, 0, 0 }, off, off } },
        // ---------------------------------------------------------------- Afrotrap
        { "Afrotrap", "Afrotrap Chorus", ModeDegrees, Major, true, 35, 0, 40, 90, 50,
          { { true, 4, 2, -6, -40, -5, 14 }, { true, 7, 4, -7, 40, 5, 22 }, { true, -8, -3, -10, 0, 0, 0 }, off } },
        { "Afrotrap", "Afro Call & Response", ModeDegrees, Major, true, 40, 0, 35, 100, 50,
          { { true, 4, 2, -5, -70, 0, 30 }, { true, -3, -2, -6, 70, 0, 45 }, off, off } },
        { "Afrotrap", "Afrotrap Ad-lib Stack", ModeSemitones, Major, false, 15, 0, 25, 100, 45,
          { { true, 7, 4, -6, -60, -6, 18 }, { true, 12, 7, -9, 60, 6, 28 }, off, off } },
        // ---------------------------------------------------------------- Oriental / Rai
        { "Oriental / Rai", "Rai Hijaz Tierce", ModeDegrees, Hijaz, true, 45, 0, 30, 70, 50,
          { { true, 4, 2, -6, -30, 0, 0 }, { true, -12, -7, -8, 30, 0, 0 }, off, off } },
        { "Oriental / Rai", "Maqam Rast Choeur", ModeDegrees, Rast, true, 50, 30, 35, 90, 50,
          { { true, 4, 2, -6, -50, -4, 10 }, { true, 7, 4, -7, 50, 4, 16 }, { true, -12, -7, -9, 0, 0, 0 }, off } },
        { "Oriental / Rai", "Bayati Unisson Large", ModeDegrees, Bayati, true, 30, 0, 40, 100, 50,
          { { true, 0, 0, -5, -80, -11, 16 }, { true, 0, 0, -5, 80, 11, 26 }, { true, -12, -7, -10, 0, 0, 0 }, off } },
        { "Oriental / Rai", "Rai Mawal Quintes", ModeDegrees, HijazKar, true, 60, 0, 30, 70, 50,
          { { true, 7, 4, -6, -35, 0, 0 }, { true, -5, -3, -8, 35, 0, 0 }, off, off } },
        { "Oriental / Rai", "Saba Melancolie", ModeDegrees, Saba, true, 50, 20, 30, 80, 50,
          { { true, 3, 2, -6, -40, 0, 0 }, { true, -3, -2, -8, 40, 0, 0 }, off, off } },
        // ---------------------------------------------------------------- Deep House
        { "Deep House", "Deep House Quintes", ModeSemitones, Minor, true, 0, 0, 10, 90, 45,
          { { true, 7, 4, -7, -50, 0, 0 }, { true, -5, -3, -9, 50, 0, 0 }, { true, -12, -7, -12, 0, 0, 0 }, off } },
        { "Deep House", "House Diva Stack", ModeDegrees, Minor, true, 20, 0, 25, 100, 50,
          { { true, 3, 2, -6, -55, 0, 8 }, { true, 7, 4, -7, 55, 0, 12 }, { true, 12, 7, -12, 0, 0, 0 }, off } },
        // ---------------------------------------------------------------- G-Funk
        { "G-Funk", "G-Funk Talkbox Octaves", ModeSemitones, Mixolydian, false, 30, 0, 5, 40, 55,
          { { true, -12, -7, -3, 0, 0, 0 }, { true, 12, 7, -12, 0, 6, 0 }, off, off } },
        { "G-Funk", "G-Funk Smooth Choir", ModeDegrees, Mixolydian, true, 70, 0, 30, 85, 50,
          { { true, 4, 2, -6, -45, 0, 0 }, { true, 7, 4, -7, 45, 0, 0 }, { true, -8, -3, -9, 0, 0, 0 }, off } },
        // ---------------------------------------------------------------- R&B
        { "R&B", "RnB Neo Soul Stack", ModeDegrees, Dorian, true, 40, 40, 35, 100, 50,
          { { true, 3, 2, -6, -50, -3, 6 }, { true, 7, 4, -7, 50, 3, 9 }, { true, 10, 6, -10, -20, 0, 0 }, { true, -12, -7, -11, 20, 0, 0 } } },
        { "R&B", "RnB Gospel Choir", ModeDegrees, Major, true, 35, 0, 45, 100, 55,
          { { true, 4, 2, -6, -60, 0, 10 }, { true, 7, 4, -7, 60, 0, 15 }, { true, -8, -3, -8, -25, 0, 20 }, { true, -12, -7, -9, 25, 0, 5 } } },
        { "R&B", "RnB Doubler Large", ModeSemitones, Major, true, 10, 0, 30, 100, 50,
          { { true, 0, 0, -4, -85, -9, 18 }, { true, 0, 0, -4, 85, 9, 27 }, off, off } },
        // ---------------------------------------------------------------- Podcast / Voix parlee
        { "Podcast / Voix parlee", "Podcast Epaisseur", ModeSemitones, Chromatic, true, 10, 0, 20, 60, 40,
          { { true, 0, 0, -14, -30, -6, 12 }, { true, 0, 0, -14, 30, 6, 19 }, off, off } },
        { "Podcast / Voix parlee", "Voix Robot Annonce", ModeSemitones, Chromatic, false, 0, 0, 0, 0, 60,
          { { true, -12, -7, -3, 0, 0, 0 }, { true, 7, 4, -6, 0, 0, 0 }, off, off } },
        { "Podcast / Voix parlee", "Voix Monstre", ModeSemitones, Chromatic, false, 0, 0, 0, 30, 75,
          { { true, -12, -7, -1, -30, 0, 0 }, { true, -7, -4, -5, 30, 0, 0 }, off, off } },
        // ---------------------------------------------------------------- Pop / Choeurs
        { "Pop / Choeurs", "Pop Tierce Haute", ModeDegrees, Major, true, 20, 0, 15, 60, 50,
          { { true, 4, 2, -5, -20, 0, 0 }, off, off, off } },
        { "Pop / Choeurs", "Choeur Celeste", ModeDegrees, Major, true, 80, 0, 55, 100, 55,
          { { true, 7, 4, -8, -70, 0, 35 }, { true, 12, 7, -10, 70, 0, 48 }, { true, -12, -7, -10, -30, 0, 22 }, { true, 4, 2, -9, 30, 0, 40 } } },
    };
}

std::vector<HumpirePreset> makeSaturnFactoryPresets()
{
    std::vector<HumpirePreset> out;
    for (auto& p : kList)
    {
        HumpirePreset hp;
        hp.category = p.cat;
        hp.name = p.name;
        hp.values = { { id::mode, (float) p.mode }, { id::scale, (float) p.scale }, { id::formant, p.formant ? 1.0f : 0.0f },
                      { id::glide, p.glide }, { id::correction, p.corr }, { id::humanize, p.human },
                      { id::width, p.width }, { id::mix, p.mix } };
        for (int v = 0; v < kNumVoices; ++v)
        {
            const auto& vv = p.v[v];
            hp.values.push_back ({ voiceId (v, vOn),     vv.on ? 1.0f : 0.0f });
            hp.values.push_back ({ voiceId (v, vSemi),   (float) vv.semi });
            hp.values.push_back ({ voiceId (v, vDeg),    (float) vv.deg });
            hp.values.push_back ({ voiceId (v, vLevel),  vv.level });
            hp.values.push_back ({ voiceId (v, vPan),    vv.pan });
            hp.values.push_back ({ voiceId (v, vDetune), vv.detune });
            hp.values.push_back ({ voiceId (v, vDelay),  vv.delay });
        }
        out.push_back (std::move (hp));
    }
    return out;
}
