cmake_minimum_required(VERSION 3.22)
project(CHINTZWAZ VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(CzAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(CHINTZWAZ
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Czl1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "CHINTZWAZ"
    VST3_CATEGORIES Instrument Synth)

target_sources(CHINTZWAZ PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(CHINTZWAZ PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(CHINTZWAZ PRIVATE CzAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DCHINTZWAZ_BUILD_TESTS=OFF)
option(CHINTZWAZ_BUILD_TESTS "Build CzTest" ON)
if(CHINTZWAZ_BUILD_TESTS)
    juce_add_console_app(CzTest PRODUCT_NAME "CzTest")
    target_sources(CzTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(CzTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(CzTest PRIVATE Source)
    target_link_libraries(CzTest PRIVATE CzAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
