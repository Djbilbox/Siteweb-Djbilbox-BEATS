cmake_minimum_required(VERSION 3.22)
project(K2000 VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(K2kAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(K2000
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE K2kv
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "K2000"
    VST3_CATEGORIES Instrument Synth)

target_sources(K2000 PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/DSP/Effects.cpp)
target_compile_definitions(K2000 PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(K2000 PRIVATE K2kAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DK2000_BUILD_TESTS=OFF)
option(K2000_BUILD_TESTS "Build K2kTest" ON)
if(K2000_BUILD_TESTS)
    juce_add_console_app(K2kTest PRODUCT_NAME "K2kTest")
    target_sources(K2kTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/DSP/Effects.cpp)
    target_compile_definitions(K2kTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(K2kTest PRIVATE Source)
    target_link_libraries(K2kTest PRIVATE K2kAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
