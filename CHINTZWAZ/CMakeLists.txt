cmake_minimum_required(VERSION 3.22)
project(CHINTZWAZ VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

# Assets/flute: the real flute of the Hippie Flute engine (VSCO 2 Community Edition, CC0), see Tools/make_flute.py
file(GLOB CZ_FLUTE ${CMAKE_CURRENT_SOURCE_DIR}/Assets/flute/*.flac)
juce_add_binary_data(CzAssets SOURCES Assets/camel.png Assets/logo.png Assets/cassette.png Assets/fader.png Assets/faceplate.jpg ${CZ_FLUTE})

juce_add_plugin(CHINTZWAZ
    COMPANY_NAME "DJBILBOX BEATS"
    PLUGIN_MANUFACTURER_CODE Djbb
    PLUGIN_CODE Czl1
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    PRODUCT_NAME "CHINTAWAZ"   # the product is CHINTAWAZ (targets keep the folder's name)
    VST3_CATEGORIES Instrument Synth)

target_sources(CHINTZWAZ PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(CHINTZWAZ PRIVATE ../ALL_PLUGINS/HumpireFX)   # NeonFX.h / PremiumSkin.h: premium skin and live light
target_compile_definitions(CHINTZWAZ PRIVATE JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

# MinGW (llvm-mingw): C++ runtime linked statically (no libc++.dll on the customer's machine) and a Common Controls v6
# manifest inside the VST3 DLL (FL Studio's bridge has none: without it the plug-in would not load)
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET CHINTZWAZ_${fmt})
            target_link_options(CHINTZWAZ_${fmt} PRIVATE -static)
        endif()
    endforeach()
    target_sources(CHINTZWAZ_VST3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ALL_PLUGINS/HumpireFX/plugin_manifest.rc)
endif()

# films / captures the interface (Tools/UiFrames.cpp): checks the premium skin; not part of the plug-in
option(HUMPIRE_UI_TOOLS "Build the interface capture tool" OFF)
if(HUMPIRE_UI_TOOLS)
    juce_add_console_app(CHINTZWAZUiFrames PRODUCT_NAME "CHINTZWAZUiFrames")
    target_sources(CHINTZWAZUiFrames PRIVATE Tools/UiFrames.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(CHINTZWAZUiFrames PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
    target_include_directories(CHINTZWAZUiFrames PRIVATE Source ../ALL_PLUGINS/HumpireFX)
    target_link_libraries(CHINTZWAZUiFrames PRIVATE CzAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
    if(MINGW)
        target_link_options(CHINTZWAZUiFrames PRIVATE -static)
    endif()
endif()
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
