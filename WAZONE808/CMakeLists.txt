cmake_minimum_required(VERSION 3.22)
project(WARZONE808 VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(WarzoneAssets SOURCES
    Assets/camel.png Assets/db.png Assets/cassette.png Assets/EXPANSION_WZ_VOL1.wz808exp)

set(WZ_SOURCES
    Source/PluginProcessor.cpp Source/PluginEditor.cpp
    Source/DSP/SynthEngine.cpp Source/DSP/Oscillator.cpp Source/DSP/Filter.cpp Source/DSP/Envelope.cpp Source/DSP/LFO.cpp
    Source/Effects/DistortionEffect.cpp Source/Effects/ChorusEffect.cpp Source/Effects/DelayEffect.cpp Source/Effects/ReverbEffect.cpp
    Source/Params/ExpansionPack.cpp)

juce_add_plugin(WARZONE808
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Wz08
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "WARZONE808"
    VST3_CATEGORIES Instrument Synth)
juce_generate_juce_header(WARZONE808)
target_sources(WARZONE808 PRIVATE ${WZ_SOURCES})
target_include_directories(WARZONE808 PRIVATE Source Source/DSP Source/Effects Source/Params)
target_compile_definitions(WARZONE808 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(WARZONE808 PRIVATE WarzoneAssets juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

option(WZ_BUILD_TESTS "Build WzTest" ON)
if(WZ_BUILD_TESTS)
    juce_add_console_app(WzTest PRODUCT_NAME "WzTest")
    juce_generate_juce_header(WzTest)
    target_sources(WzTest PRIVATE Tools/RenderTest.cpp ${WZ_SOURCES})
    target_include_directories(WzTest PRIVATE Source Source/DSP Source/Effects Source/Params)
    target_compile_definitions(WzTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(WzTest PRIVATE WarzoneAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
