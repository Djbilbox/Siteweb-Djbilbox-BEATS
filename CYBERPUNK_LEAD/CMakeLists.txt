cmake_minimum_required(VERSION 3.22)
project(CYBERPUNK_LEAD VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(CpAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png Assets/fader.png Assets/faceplate.jpg)

juce_add_plugin(CYBERPUNK_LEAD
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Cpl1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "CYBERPUNK LEAD"
    VST3_CATEGORIES Instrument Synth)

target_sources(CYBERPUNK_LEAD PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(CYBERPUNK_LEAD PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(CYBERPUNK_LEAD PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET CYBERPUNK_LEAD_${fmt})
            target_link_options(CYBERPUNK_LEAD_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(CYBERPUNK_LEAD_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(CYBERPUNK_LEADUiFrames PRODUCT_NAME "CYBERPUNK_LEADUiFrames")
    target_sources(CYBERPUNK_LEADUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(CYBERPUNK_LEADUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(CYBERPUNK_LEADUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(CYBERPUNK_LEADUiFrames PRIVATE CpAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(CYBERPUNK_LEADUiFrames PRIVATE -static)
    endif()
endif()
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
