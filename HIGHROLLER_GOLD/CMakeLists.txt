cmake_minimum_required(VERSION 3.22)
project(HIGHROLLER_GOLD VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(HgAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(HIGHROLLER_GOLD
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Hgl
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "HIGHROLLER_GOLD"
    VST3_CATEGORIES Instrument Synth)

target_sources(HIGHROLLER_GOLD PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
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
