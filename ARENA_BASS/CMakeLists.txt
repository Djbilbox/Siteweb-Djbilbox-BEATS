cmake_minimum_required(VERSION 3.22)
project(ARENA_BASS VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(AbAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png Assets/fader.png Assets/faceplate.jpg)

juce_add_plugin(ARENA_BASS
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Abl1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "ARENA BASS"
    VST3_CATEGORIES Instrument Synth)

target_sources(ARENA_BASS PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(ARENA_BASS PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(ARENA_BASS PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET ARENA_BASS_${fmt})
            target_link_options(ARENA_BASS_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(ARENA_BASS_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(ARENA_BASSUiFrames PRODUCT_NAME "ARENA_BASSUiFrames")
    target_sources(ARENA_BASSUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(ARENA_BASSUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(ARENA_BASSUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(ARENA_BASSUiFrames PRIVATE AbAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(ARENA_BASSUiFrames PRIVATE -static)
    endif()
endif()
target_compile_definitions(ARENA_BASS PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(ARENA_BASS PRIVATE AbAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DARENA_BASS_BUILD_TESTS=OFF)
option(ARENA_BASS_BUILD_TESTS "Build AbTest" ON)
if(ARENA_BASS_BUILD_TESTS)
    juce_add_console_app(AbTest PRODUCT_NAME "AbTest")
    target_sources(AbTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(AbTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(AbTest PRIVATE Source)
    target_link_libraries(AbTest PRIVATE AbAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
