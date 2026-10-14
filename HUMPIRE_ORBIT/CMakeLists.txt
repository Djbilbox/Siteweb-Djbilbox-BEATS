# =============================================================================
#  HUMPIRE ORBIT - bundle de 10 plugins d'effets vocaux (VST3 + Standalone)
#  by DJBILBOX BEATS - JUCE 8 / C++17 / CMake
#
#  Build :  cmake -B build -G "Visual Studio 17 2022" -A x64
#           cmake --build build --config Release --target HumpireSolar_VST3
#  Un plugin dont le dossier Plugins/<Nom>/Source ne contient pas encore de .cpp
#  est simplement ignore (le bundle se construit au fur et a mesure).
# =============================================================================
cmake_minimum_required(VERSION 3.22)
project(HUMPIRE_ORBIT VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(HUMPIRE_BUILD_STANDALONE "Construire aussi les versions Standalone (tests)" ON)
option(HUMPIRE_BUILD_TESTS      "Construire les tests DSP console"                 ON)
option(HUMPIRE_COPY_AFTER_BUILD "Copier les VST3 dans le dossier systeme"          OFF)

# --- JUCE (FetchContent, version figee) --------------------------------------
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

# --- HumpireCore : LookAndFeel, Knob, Meter, Waveform, Orbit, Presets, TopBar --
add_library(HumpireCore STATIC
    Core/HumpireLookAndFeel.cpp
    Core/PresetManager.cpp
    Core/TopBar.cpp)
set_target_properties(HumpireCore PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(HumpireCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(HumpireCore PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0)
target_link_libraries(HumpireCore PUBLIC
    juce::juce_audio_utils juce::juce_dsp
    PRIVATE juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# --- Fonction d'ajout d'un plugin du bundle -----------------------------------
# humpire_add_plugin(<NomCible> <Dossier> <CodePlugin4> "<Nom produit>" "<Description>")
function(humpire_add_plugin TARGET FOLDER CODE PRODUCT DESCRIPTION)
    file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/${FOLDER}/Source/*.cpp")
    if(NOT SRCS)
        message(STATUS "HUMPIRE: ${PRODUCT} pas encore implemente -> ignore")
        return()
    endif()

    set(FORMATS VST3)
    if(HUMPIRE_BUILD_STANDALONE)
        list(APPEND FORMATS Standalone)
    endif()

    juce_add_plugin(${TARGET}
        COMPANY_NAME                "DJBILBOX BEATS"
        BUNDLE_ID                   "com.djbilbox.humpire.${FOLDER}"
        PLUGIN_MANUFACTURER_CODE    Hmpr
        PLUGIN_CODE                 ${CODE}
        FORMATS                     ${FORMATS}
        PRODUCT_NAME                "${PRODUCT}"
        DESCRIPTION                 "${DESCRIPTION}"
        IS_SYNTH                    FALSE
        NEEDS_MIDI_INPUT            FALSE
        NEEDS_MIDI_OUTPUT           FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD     ${HUMPIRE_COPY_AFTER_BUILD}
        VST3_CATEGORIES             Fx Pitch\ Shift)

    target_sources(${TARGET} PRIVATE ${SRCS})
    target_include_directories(${TARGET} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/${FOLDER}/Source")
    target_link_libraries(${TARGET}
        PRIVATE HumpireCore
        PUBLIC  juce::juce_recommended_config_flags
                juce::juce_recommended_warning_flags)
    message(STATUS "HUMPIRE: ${PRODUCT} -> cible ${TARGET}")

    # Test DSP console generique : Plugins/<Dossier>/Tools/Test.cpp -> cible <Cible>Test
    set(TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/${FOLDER}/Tools/Test.cpp")
    if(HUMPIRE_BUILD_TESTS AND EXISTS "${TEST_SRC}")
        juce_add_console_app(${TARGET}Test PRODUCT_NAME "${TARGET}Test")
        target_sources(${TARGET}Test PRIVATE "${TEST_SRC}" ${SRCS})
        target_include_directories(${TARGET}Test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/${FOLDER}/Source")
        target_compile_definitions(${TARGET}Test PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
        target_link_libraries(${TARGET}Test PRIVATE HumpireCore juce::juce_recommended_config_flags)
    endif()
endfunction()

# --- Les 10 plugins -----------------------------------------------------------
humpire_add_plugin(HumpireSolar      Solar      HmSo "HUMPIRE SOLAR"      "Autotune temps reel : Auto + courbe graphique")
humpire_add_plugin(HumpireSaturn     Saturn     HmSa "HUMPIRE SATURN"     "Generateur d'harmonies 4 voix")
humpire_add_plugin(HumpireTwin       Twin       HmTw "HUMPIRE TWIN"       "Auto-doubler vocal")
humpire_add_plugin(HumpireNebula     Nebula     HmNe "HUMPIRE NEBULA"     "Multiplicateur de voix 1 a 32")
humpire_add_plugin(HumpireSolarFlare SolarFlare HmSf "HUMPIRE SOLAR FLARE" "Saturation a tube")
humpire_add_plugin(HumpireVacuum     Vacuum     HmVa "HUMPIRE VACUUM"     "Processeur de souffle")
humpire_add_plugin(HumpireGravity    Gravity    HmGr "HUMPIRE GRAVITY"    "Modelage de timbre / formants")
humpire_add_plugin(HumpireSignal     Signal     HmSi "HUMPIRE SIGNAL"     "Talkbox numerique / vocoder")
humpire_add_plugin(HumpireSupernova  Supernova  HmSn "HUMPIRE SUPERNOVA"  "Punch et presence vocale")
humpire_add_plugin(HumpireVoid       Void       HmVo "HUMPIRE VOID"       "De-esser a frequence variable")

# --- Tests DSP console (Solar) -----------------------------------------------
if(HUMPIRE_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Solar/Tools/SolarTest.cpp")
    juce_add_console_app(SolarTest PRODUCT_NAME "SolarTest")
    target_sources(SolarTest PRIVATE
        Plugins/Solar/Tools/SolarTest.cpp
        Plugins/Solar/Source/PluginProcessor.cpp
        Plugins/Solar/Source/PluginEditor.cpp
        Plugins/Solar/Source/SolarPresets.cpp)
    target_include_directories(SolarTest PRIVATE Plugins/Solar/Source)
    target_link_libraries(SolarTest PRIVATE HumpireCore juce::juce_recommended_config_flags)
endif()

# --- HUMPIRE Hub (app standalone : activation / mises a jour) ------------------
file(GLOB_RECURSE HUB_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Hub/Source/*.cpp")
if(HUB_SRCS)
    juce_add_gui_app(HumpireHub PRODUCT_NAME "HUMPIRE Hub" COMPANY_NAME "DJBILBOX BEATS")
    target_sources(HumpireHub PRIVATE ${HUB_SRCS})
    target_link_libraries(HumpireHub PRIVATE HumpireCore juce::juce_recommended_config_flags)
else()
    message(STATUS "HUMPIRE: Hub pas encore implemente -> ignore")
endif()
