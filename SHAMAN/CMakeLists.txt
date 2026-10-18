cmake_minimum_required(VERSION 3.22)
project(SHAMAN VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(ShAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png)

juce_add_plugin(SHAMAN
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Shl1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "SHAMAN"
    VST3_CATEGORIES Instrument Synth)

target_sources(SHAMAN PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(SHAMAN PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SHAMAN PRIVATE ShAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DSHAMAN_BUILD_TESTS=OFF)
option(SHAMAN_BUILD_TESTS "Build ShTest" ON)
if(SHAMAN_BUILD_TESTS)
    juce_add_console_app(ShTest PRODUCT_NAME "ShTest")
    target_sources(ShTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(ShTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(ShTest PRIVATE Source)
    target_link_libraries(ShTest PRIVATE ShAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
