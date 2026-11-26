cmake_minimum_required(VERSION 3.22)
project(NEON_SYNTH_80S VERSION 2.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(NeonAssets SOURCES Assets/knob.png Assets/hero.jpg)

# Code plugin distinct de la v1 (Ns80) : la 2.0 s installe a cote de la 1.0, les deux restent dans les DAW.
juce_add_plugin(NeonSynth80s
    COMPANY_NAME "Djbilbox BEATS"
    PLUGIN_MANUFACTURER_CODE DjBx
    PLUGIN_CODE Ns82
    FORMATS VST3 AU Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "NEON SYNTH 80s 2.0"
    BUNDLE_ID com.djbilboxbeats.neonsynth80s2
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    VST3_CATEGORIES Instrument Synth)

target_sources(NeonSynth80s PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/DSP/Effects.cpp)
target_compile_definitions(NeonSynth80s PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
target_link_libraries(NeonSynth80s PRIVATE NeonAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Common Controls v6 manifest inside the VST3 DLL (MinGW; MSVC gets it from the #pragma in PluginProcessor.cpp):
# without it Ableton Live scans the plug-in but cannot load it (TaskDialogIndirect, error 127) and hides it.
if(MINGW)
    target_sources(NeonSynth80s_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source/win/plugin_manifest.rc)
endif()
if(WIN32)
    add_executable(NeonLoadTest Tools/LoadTest.c)
    if(MINGW)
        target_link_options(NeonLoadTest PRIVATE -municode)
    endif()
endif()

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DNEON_SYNTH_80S_BUILD_TESTS=OFF)
option(NEON_SYNTH_80S_BUILD_TESTS "Build NeonTest" ON)
if(NEON_SYNTH_80S_BUILD_TESTS)
    juce_add_console_app(NeonTest PRODUCT_NAME "NeonTest")
    target_sources(NeonTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/DSP/Effects.cpp)
    target_compile_definitions(NeonTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0 JUCE_MODAL_LOOPS_PERMITTED=1)
    target_include_directories(NeonTest PRIVATE Source)
    target_link_libraries(NeonTest PRIVATE NeonAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
