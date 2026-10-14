#pragma once
#include "HumpireCore/HumpirePresetManager.h"
#include "Params.h"

namespace humpire
{
// Presets d'usine : 2 par instrument + 3 "mix chaabi".
inline std::vector<FactoryPreset> makeFactoryPresets()
{
    std::vector<FactoryPreset> out;

    for (int i = 0; i < kNumInstruments; ++i)
    {
        const auto name = instrumentName (i);
        const bool perc = kInstruments[i].percussive;

        FactoryPreset natural { name + juce::String::fromUTF8 (" \xE2\x80\x94 Naturel"), name, {}, {} };
        natural.values = { { pid::selected, (float) i }, { pid::multi, 0.f }, { pid::authent, 0.25f },
                           { pid::inst (i, "rev"), perc ? 0.08f : 0.15f } };
        out.push_back (natural);

        FactoryPreset hall { name + juce::String::fromUTF8 (" \xE2\x80\x94 Salle & Delay"), name, {}, {} };
        hall.values = { { pid::selected, (float) i }, { pid::multi, 0.f }, { pid::authent, 0.45f },
                        { pid::inst (i, "rev"), perc ? 0.30f : 0.42f }, { pid::inst (i, "dly"), 0.18f },
                        { pid::inst (i, "filter"), perc ? 14000.f : 9000.f }, { pid::revSize, 0.75f } };
        out.push_back (hall);
    }

    auto mix = [&] (const char* nm, std::initializer_list<std::tuple<int, int, int>> parts, float auth)
    {
        FactoryPreset p { juce::String::fromUTF8 (nm), "Mix chaabi", {}, {} };
        p.values = { { pid::selected, (float) std::get<0> (*parts.begin()) }, { pid::multi, 1.f }, { pid::authent, auth } };
        for (int k = 0; k < kNumInstruments; ++k) p.values.push_back ({ pid::inst (k, "on"), 0.f });
        for (auto& [slot, lo, hi] : parts)
        {
            p.values.push_back ({ pid::inst (slot, "on"), 1.f });
            p.values.push_back ({ pid::inst (slot, "lo"), (float) lo });
            p.values.push_back ({ pid::inst (slot, "hi"), (float) hi });
        }
        out.push_back (p);
    };

    mix ("Mix Chaabi \xE2\x80\x94 Classique (mandole + derbouka + tar)", { { 0, 60, 96 }, { 5, 36, 47 }, { 6, 48, 59 } }, 0.40f);
    mix ("Mix Chaabi \xE2\x80\x94 Salon (mandole + banjo + derbouka + bendir)", { { 0, 60, 96 }, { 1, 60, 96 }, { 5, 36, 47 }, { 7, 48, 59 } }, 0.50f);
    mix ("Mix Chaabi \xE2\x80\x94 Orchestre (violon + qan\xC3\xBBn + ney + derbouka)", { { 2, 60, 96 }, { 4, 48, 59 }, { 3, 60, 96 }, { 5, 36, 47 } }, 0.55f);
    return out;
}
} // namespace humpire
