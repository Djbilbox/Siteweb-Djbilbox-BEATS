cmake_minimum_required(VERSION 3.22)
project(VOODOO VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(VoodooAssets SOURCES Assets/camel.png Assets/logo.png)

juce_add_plugin(VOODOO
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Vdoo
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "VOODOO"
    VST3_CATEGORIES Instrument Synth)

target_sources(VOODOO PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(VOODOO PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VOODOO PRIVATE VoodooAssets
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Test console : rend les 70 presets et execute la suite de validation (desactiver : -DVOODOO_BUILD_TESTS=OFF)
option(VOODOO_BUILD_TESTS "Build VoodooTest" ON)
if(VOODOO_BUILD_TESTS)
    juce_add_console_app(VoodooTest PRODUCT_NAME "VoodooTest")
    target_sources(VoodooTest PRIVATE Tools/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(VoodooTest PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_include_directories(VoodooTest PRIVATE Source)
    target_link_libraries(VoodooTest PRIVATE VoodooAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
