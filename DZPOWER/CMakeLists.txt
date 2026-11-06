cmake_minimum_required(VERSION 3.22)
project(DZPOWER VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# faceplate.jpg: the whole front panel rendered photo-real with Google Flow (Nano Banana) from the old interface, same
# layout, without pointers, fader caps or texts in the screens (Tools/faceplate_layout.py); fader.png: the fader cap
# Assets/samples: the real instruments (notes from the FENNEC library, see Assets/samples/CREDITS.txt), made by
# ALL_PLUGINS/HumpireFX/tools/make_samples.py from Tools/samples.json
file(GLOB DZ_SAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/Assets/samples/*.flac)
juce_add_binary_data(DzAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png Assets/fader.png Assets/faceplate.jpg ${DZ_SAMPLES})

juce_add_plugin(DZPOWER
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Dzl1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "DZPOWER"
    VST3_CATEGORIES Instrument Synth)

target_sources(DZPOWER PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(DZPOWER PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h: neon and live animation

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET DZPOWER_${fmt})
            target_link_options(DZPOWER_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(DZPOWER_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()
target_compile_definitions(DZPOWER PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
target_link_libraries(DZPOWER PRIVATE DzAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DDZPOWER_BUILD_TESTS=OFF)
option(DZPOWER_BUILD_TESTS "Build DzTest" ON)
if(DZPOWER_BUILD_TESTS)
    juce_add_console_app(DzTest PRODUCT_NAME "DzTest")
    target_sources(DzTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(DzTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(DzTest PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(DzTest PRIVATE DzAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)

    # films the interface while it plays (one PNG per frame): checks the neon and live animation
    juce_add_console_app(DzUiFrames PRODUCT_NAME "DzUiFrames")
    target_sources(DzUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(DzUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(DzUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(DzUiFrames PRIVATE DzAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(DzUiFrames PRIVATE -static)   # runs without the compiler's DLLs on the PATH
    endif()
endif()
