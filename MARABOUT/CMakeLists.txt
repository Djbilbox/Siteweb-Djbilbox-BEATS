cmake_minimum_required(VERSION 3.22)
project(MARABOUT VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(MbAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(MARABOUT
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Mbo
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "MARABOUT"
    VST3_CATEGORIES Instrument Synth)

target_sources(MARABOUT PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(MARABOUT PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(MARABOUT PRIVATE MbAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DMARABOUT_BUILD_TESTS=OFF)
option(MARABOUT_BUILD_TESTS "Build MbTest" ON)
if(MARABOUT_BUILD_TESTS)
    juce_add_console_app(MbTest PRODUCT_NAME "MbTest")
    target_sources(MbTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(MbTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(MbTest PRIVATE Source)
    target_link_libraries(MbTest PRIVATE MbAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
