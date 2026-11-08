cmake_minimum_required(VERSION 3.22)
project(HIGHROLLER_GOLD VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# Assets/samples: the real instruments (see Assets/samples/CREDITS.txt), made by
# ALL_PLUGINS/HumpireFX/tools/make_samples.py from Tools/samples.json
file(GLOB REAL_SAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/Assets/samples/*.flac)
juce_add_binary_data(HgAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png ${REAL_SAMPLES} Assets/fader.png Assets/faceplate.jpg)

juce_add_plugin(HIGHROLLER_GOLD
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Hrg1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "HIGHROLLER GOLD"
    VST3_CATEGORIES Instrument Synth)

target_sources(HIGHROLLER_GOLD PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(HIGHROLLER_GOLD PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(HIGHROLLER_GOLD PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET HIGHROLLER_GOLD_${fmt})
            target_link_options(HIGHROLLER_GOLD_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(HIGHROLLER_GOLD_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(HIGHROLLER_GOLDUiFrames PRODUCT_NAME "HIGHROLLER_GOLDUiFrames")
    target_sources(HIGHROLLER_GOLDUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(HIGHROLLER_GOLDUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(HIGHROLLER_GOLDUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(HIGHROLLER_GOLDUiFrames PRIVATE HgAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(HIGHROLLER_GOLDUiFrames PRIVATE -static)
    endif()
endif()
target_compile_definitions(HIGHROLLER_GOLD PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(HIGHROLLER_GOLD PRIVATE HgAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DHIGHROLLER_GOLD_BUILD_TESTS=OFF)
option(HIGHROLLER_GOLD_BUILD_TESTS "Build HgTest" ON)
if(HIGHROLLER_GOLD_BUILD_TESTS)
    juce_add_console_app(HgTest PRODUCT_NAME "HgTest")
    target_sources(HgTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(HgTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(HgTest PRIVATE Source)
    target_link_libraries(HgTest PRIVATE HgAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
