/* Loads the VST3 DLL from a host process WITHOUT a Common Controls v6 manifest, like Ableton Live's
   plug-in scanner. Before the manifest fix this failed with error 127 (TaskDialogIndirect not found).
   usage: LoadTest "<path>\Contents\x86_64-win\<name>.vst3" */
#include <windows.h>
#include <stdio.h>
int wmain (int argc, wchar_t** argv)
{
    if (argc < 2) { printf ("usage: LoadTest <vst3 dll>\n"); return 3; }
    HMODULE h = LoadLibraryW (argv[1]);
    if (! h) { printf ("LOAD FAILED, error %lu\n", GetLastError()); return 1; }
    if (! GetProcAddress (h, "GetPluginFactory")) { printf ("GetPluginFactory missing\n"); return 2; }
    printf ("LOAD OK (host without Common Controls v6)\n");
    return 0;
}
