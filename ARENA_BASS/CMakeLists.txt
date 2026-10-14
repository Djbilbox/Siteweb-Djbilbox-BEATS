cmake_minimum_required(VERSION 3.22)
project(ARENA_BASS VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(AbAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(ARENA_BASS
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Abl
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "ARENA_BASS"
    VST3_CATEGORIES Instrument Synth)

target_sources(ARENA_BASS PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
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
