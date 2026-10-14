# HumpireCore = bibliotheque INTERFACE : les sources sont compilees dans chaque
# plugin (evite les doublons de modules JUCE entre libs statiques).
add_library(HumpireCore INTERFACE)

set(HC_DIR ${CMAKE_CURRENT_LIST_DIR})
target_sources(HumpireCore INTERFACE
    ${HC_DIR}/HumpireTheme.cpp
    ${HC_DIR}/HumpireLookAndFeel.cpp
    ${HC_DIR}/FilmstripKnob.cpp
    ${HC_DIR}/LedMeter.cpp
    ${HC_DIR}/LcdScreen.cpp
    ${HC_DIR}/XYPad.cpp
    ${HC_DIR}/ConsoleButton.cpp
    ${HC_DIR}/PresetManager.cpp
    ${HC_DIR}/TopBar.cpp
    ${HC_DIR}/HumpireProcessor.cpp
    ${HC_DIR}/GenericEditor.cpp)

target_include_directories(HumpireCore INTERFACE ${HC_DIR}/..)

target_link_libraries(HumpireCore INTERFACE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra)

# Police LCD (Press Start 2P, OFL) : optionnelle. Lancer scripts/fetch_fonts.sh
set(HC_FONT ${CMAKE_SOURCE_DIR}/assets/fonts/PressStart2P-Regular.ttf)
if(EXISTS ${HC_FONT})
    juce_add_binary_data(HumpireFonts
        HEADER_NAME HumpireFontsData.h
        NAMESPACE   HumpireFontsData
        SOURCES     ${HC_FONT})
    set_target_properties(HumpireFonts PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(HumpireCore INTERFACE HumpireFonts)
    target_compile_definitions(HumpireCore INTERFACE HUMPIRE_HAS_FONTS=1)
else()
    message(STATUS "HUMPIRE: police Press Start 2P absente -> fallback monospace (scripts/fetch_fonts.sh)")
endif()
