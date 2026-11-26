# Common Controls v6 for every MSVC-built plug-in (VST3 DLL + standalone exe).
# JUCE imports TaskDialogIndirect, which only exists in Common Controls v6. Without a v6 dependency in the
# DLL's own manifest, hosts whose process has none (Ableton Live's plug-in scanner, FL Studio's bridge)
# get error 127 from LoadLibrary: the plug-in is scanned but never listed. MinGW builds use the
# resource-ID-2 manifest (HumpireFX/plugin_manifest.rc) instead.
# Used by ALL_PLUGINS/CMakeLists.txt and, in CI, by every configure through -DCMAKE_PROJECT_INCLUDE.
get_property(_djb_comctl6 GLOBAL PROPERTY DJB_COMCTL6_DONE)
if(MSVC AND NOT _djb_comctl6)
    set_property(GLOBAL PROPERTY DJB_COMCTL6_DONE ON)
    add_link_options("/MANIFESTDEPENDENCY:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'")
endif()
