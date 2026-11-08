cmake_minimum_required(VERSION 3.22)
project(K2000 VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(K2kAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png Assets/fader.png Assets/faceplate.jpg)

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
target_include_directories(K2000 PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(K2000 PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET K2000_${fmt})
            target_link_options(K2000_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(K2000_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(K2000UiFrames PRODUCT_NAME "K2000UiFrames")
    target_sources(K2000UiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/DSP/Effects.cpp)
    target_compile_definitions(K2000UiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(K2000UiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(K2000UiFrames PRIVATE K2kAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(K2000UiFrames PRIVATE -static)
    endif()
endif()
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
