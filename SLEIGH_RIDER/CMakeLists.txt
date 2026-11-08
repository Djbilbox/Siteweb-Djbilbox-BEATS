cmake_minimum_required(VERSION 3.22)
project(SLEIGH_RIDER VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# Assets/samples: the real instruments (see Assets/samples/CREDITS.txt), made by
# ALL_PLUGINS/HumpireFX/tools/make_samples.py from Tools/samples.json
file(GLOB REAL_SAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/Assets/samples/*.flac)
juce_add_binary_data(SrAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png ${REAL_SAMPLES} Assets/fader.png Assets/faceplate.jpg)

juce_add_plugin(SLEIGH_RIDER
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Slr1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "SLEIGH RIDER"
    VST3_CATEGORIES Instrument Synth)

target_sources(SLEIGH_RIDER PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(SLEIGH_RIDER PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(SLEIGH_RIDER PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET SLEIGH_RIDER_${fmt})
            target_link_options(SLEIGH_RIDER_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(SLEIGH_RIDER_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(SLEIGH_RIDERUiFrames PRODUCT_NAME "SLEIGH_RIDERUiFrames")
    target_sources(SLEIGH_RIDERUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(SLEIGH_RIDERUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(SLEIGH_RIDERUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(SLEIGH_RIDERUiFrames PRIVATE SrAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(SLEIGH_RIDERUiFrames PRIVATE -static)
    endif()
endif()
target_compile_definitions(SLEIGH_RIDER PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SLEIGH_RIDER PRIVATE SrAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DSLEIGH_RIDER_BUILD_TESTS=OFF)
option(SLEIGH_RIDER_BUILD_TESTS "Build SrTest" ON)
if(SLEIGH_RIDER_BUILD_TESTS)
    juce_add_console_app(SrTest PRODUCT_NAME "SrTest")
    target_sources(SrTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(SrTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(SrTest PRIVATE Source)
    target_link_libraries(SrTest PRIVATE SrAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
