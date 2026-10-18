cmake_minimum_required(VERSION 3.22)
project(HumpireLoadoutSnapshot VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_DIR "/tmp/claude-0/juce" CACHE PATH "JUCE 8 checkout")
set(HL "${CMAKE_CURRENT_LIST_DIR}/../../../HUMPIRE_LOADOUT" CACHE PATH "HUMPIRE_LOADOUT root")
get_filename_component(HL "${HL}" ABSOLUTE)
add_subdirectory(${JUCE_DIR} juce EXCLUDE_FROM_ALL)

# ONE console app containing all 8 plugins (JUCE modules compiled once). Each plugin's
# createPluginFilter is renamed per source file; the plugin is picked at runtime by name.
set(PLUGINS Skilltree Clutch Overclock Hardcap Respawn Lag Glitch Combo)
set(SRCS ${CMAKE_CURRENT_LIST_DIR}/snapshot_main.cpp
    ${HL}/Source/HumpireCore/HumpireTheme.cpp ${HL}/Source/HumpireCore/HumpireLookAndFeel.cpp
    ${HL}/Source/HumpireCore/FilmstripKnob.cpp ${HL}/Source/HumpireCore/LedMeter.cpp
    ${HL}/Source/HumpireCore/LcdScreen.cpp ${HL}/Source/HumpireCore/XYPad.cpp
    ${HL}/Source/HumpireCore/ConsoleButton.cpp ${HL}/Source/HumpireCore/PresetManager.cpp
    ${HL}/Source/HumpireCore/TopBar.cpp ${HL}/Source/HumpireCore/HumpireProcessor.cpp
    ${HL}/Source/HumpireCore/GenericEditor.cpp)
foreach(p IN LISTS PLUGINS)
    if(p STREQUAL "Glitch")
        set(files ${HL}/plugins/Glitch/Source/PluginProcessor.cpp ${HL}/plugins/Glitch/Source/PluginEditor.cpp)
    else()
        set(files ${HL}/plugins/${p}/Source/Plugin.cpp)
    endif()
    foreach(f IN LISTS files)
        set_source_files_properties(${f} PROPERTIES COMPILE_DEFINITIONS "createPluginFilter=createFilter_${p}")
    endforeach()
    list(APPEND SRCS ${files})
endforeach()

juce_add_console_app(HumpireSnapshot PRODUCT_NAME "HumpireSnapshot")
target_sources(HumpireSnapshot PRIVATE ${SRCS})
target_include_directories(HumpireSnapshot PRIVATE ${HL}/Source ${HL}/plugins/Glitch/Source)
target_compile_definitions(HumpireSnapshot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_USE_XRANDR=0 JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_STANDALONE_APPLICATION=1)
if(EXISTS ${HL}/assets/fonts/PressStart2P-Regular.ttf)
    juce_add_binary_data(HumpireFonts HEADER_NAME HumpireFontsData.h NAMESPACE HumpireFontsData
        SOURCES ${HL}/assets/fonts/PressStart2P-Regular.ttf)
    set_target_properties(HumpireFonts PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(HumpireSnapshot PRIVATE HumpireFonts)
    target_compile_definitions(HumpireSnapshot PRIVATE HUMPIRE_HAS_FONTS=1)
endif()
set(GB ${HL}/plugins/Glitch/assets/glitch_body_2x.png)
if(EXISTS ${GB})
    juce_add_binary_data(GlitchAssets HEADER_NAME GlitchAssetsData.h NAMESPACE GlitchAssetsData SOURCES ${GB})
    set_target_properties(GlitchAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(HumpireSnapshot PRIVATE GlitchAssets)
    target_compile_definitions(HumpireSnapshot PRIVATE HUMPIRE_GLITCH_HAS_BODY=1)
endif()
target_link_libraries(HumpireSnapshot PRIVATE juce::juce_audio_utils juce::juce_audio_processors
    juce::juce_dsp juce::juce_gui_extra juce::juce_recommended_config_flags)
