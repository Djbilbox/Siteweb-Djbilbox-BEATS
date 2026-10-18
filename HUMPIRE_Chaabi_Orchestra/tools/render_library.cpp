// ChaabiRenderLibrary - renders the built-in modelled library of HUMPIRE CHAABI ORCHESTRA to disk so that it can
// be inspected by ear and analysed (tools/analyze_library.py).  Pure C++17, std only, links the Model sources only.
//
//   ChaabiRenderLibrary --out DIR [--bits 16|24|32] [--instrument id[,id...]|all] [--rate 44100] [--no-wav] [--list] [--progress] [--validate-only]
//
// Output per instrument:  DIR/<Folder>/<Folder>.sfz  +  DIR/<Folder>/samples/<articulation>/<zone>.wav
// The SFZ is equivalent to what the plug-in builds in memory (the engine's own SFZ parser can load it, which also
// makes it usable as a user library).  Exit code: 0 ok, 1 validation errors, 2 usage / I/O error.
#include "LibraryWriter.h"
#include "Model/ModelCommon.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace humpire::model;

static void usage()
{
    std::cout << "usage: ChaabiRenderLibrary --out DIR [--bits 16|24|32] [--instrument id[,id...]|all] [--rate HZ] [--no-wav] [--list] [--progress] [--validate-only]\n"
                 "  --validate-only: render + validate + time only, write nothing (OUT not needed)\n"
                 "  instruments:";
    for (int i = 0; i < kNumModelInstruments; ++i) std::cout << ' ' << kSpecs[i].id;
    std::cout << "\n";
}

int main (int argc, char** argv)
{
    std::string outDir;
    std::string which = "all";
    int bits = 24;
    double rate = 44100.0;
    bool writeWavs = true, listOnly = false, showProgress = false, validateOnly = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];
        auto next = [&] () -> std::string { return i + 1 < argc ? std::string (argv[++i]) : std::string(); };
        if (a == "--out") outDir = next();
        else if (a == "--bits") bits = std::atoi (next().c_str());
        else if (a == "--instrument") which = next();
        else if (a == "--rate") rate = std::atof (next().c_str());
        else if (a == "--no-wav") writeWavs = false;
        else if (a == "--list") listOnly = true;
        else if (a == "--progress") showProgress = true;
        else if (a == "--validate-only") validateOnly = true;
        else if (a == "-h" || a == "--help") { usage(); return 0; }
        else { std::cerr << "unknown option: " << a << "\n"; usage(); return 2; }
    }
    if (listOnly)
    {
        for (int i = 0; i < kNumModelInstruments; ++i)
            std::cout << kSpecs[i].id << "  folder=" << kSpecs[i].folder << "  keys=" << kSpecs[i].lo << ".." << kSpecs[i].hi
                      << (kSpecs[i].percussive ? "  percussive" : "  pitched") << "\n";
        return 0;
    }
    if ((outDir.empty() && ! validateOnly) || (bits != 16 && bits != 24 && bits != 32) || rate < 8000.0 || rate > 192000.0) { usage(); return 2; }

    std::vector<int> todo;
    if (which == "all") for (int i = 0; i < kNumModelInstruments; ++i) todo.push_back (i);
    else
    {
        std::stringstream ss (which);
        std::string id;
        while (std::getline (ss, id, ','))
        {
            const int idx = findInstrument (id);
            if (idx < 0) { std::cerr << "unknown instrument: " << id << "\n"; return 2; }
            todo.push_back (idx);
        }
    }

    int exitCode = 0;
    double totalMs = 0.0;
    for (int idx : todo)
    {
        const auto t0 = std::chrono::steady_clock::now();
        int lastPct = -1;
        Library lib = build (idx, rate, [&] (float f)
        {
            const int pct = (int) (f * 100.f);
            if (showProgress && pct != lastPct && pct % 10 == 0) { lastPct = pct; std::cerr << "\r  " << kSpecs[idx].id << " " << pct << "%   " << std::flush; }
            return true;
        });
        if (showProgress) std::cerr << "\r                                  \r";
        const double ms = std::chrono::duration<double, std::milli> (std::chrono::steady_clock::now() - t0).count();
        totalMs += ms;

        const auto rep = validate (lib, idx);
        int aliases = 0;
        for (auto& z : lib.zones) if (z.sharedWith >= 0) ++aliases;
        std::printf ("%-9s %3zu zones (%d aliases)  %6.2f MB  rendered in %7.1f ms  %s\n", kSpecs[idx].id, lib.zones.size(), aliases,
                     lib.megabytes(), ms, rep.ok() ? "validation OK" : "VALIDATION FAILED");
        if (ms > 3000.0) std::printf ("  warning: render time above the 3 s budget\n");
        for (auto& e : rep.errors) std::printf ("  ERROR   %s\n", e.c_str());
        for (std::size_t k = 0; k < rep.warnings.size(); ++k) std::printf ("  warning %s\n", rep.warnings[k].c_str());
        if (! rep.ok()) exitCode = 1;

        if (validateOnly) continue;
        const auto res = writer::writeLibrary (lib, idx, outDir, bits, writeWavs);
        if (! res.ok) { std::cerr << "error: " << res.error << "\n"; return 2; }
        std::printf ("  -> %s  (%d WAV files, %d-bit)\n", res.sfzPath.string().c_str(), res.wavFiles, bits);
    }
    std::printf ("total render time %.1f ms\n", totalMs);
    return exitCode;
}
