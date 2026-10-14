cmake_minimum_required(VERSION 3.22)
project(CYBERPUNK_LEAD VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(CpAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(CYBERPUNK_LEAD
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Cpl
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "CYBERPUNK_LEAD"
    VST3_CATEGORIES Instrument Synth)

target_sources(CYBERPUNK_LEAD PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(CYBERPUNK_LEAD PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(CYBERPUNK_LEAD PRIVATE CpAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DCYBERPUNK_LEAD_BUILD_TESTS=OFF)
option(CYBERPUNK_LEAD_BUILD_TESTS "Build CpTest" ON)
if(CYBERPUNK_LEAD_BUILD_TESTS)
    juce_add_console_app(CpTest PRODUCT_NAME "CpTest")
    target_sources(CpTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(CpTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(CpTest PRIVATE Source)
    target_link_libraries(CpTest PRIVATE CpAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
