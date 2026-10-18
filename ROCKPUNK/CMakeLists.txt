cmake_minimum_required(VERSION 3.22)

project(ROCKPUNK VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# JUCE
# Provide the path to a JUCE checkout via -DJUCE_DIR=...
# (Clone: git clone --branch 8.0.4 --depth 1 https://github.com/juce-framework/JUCE.git)
# ---------------------------------------------------------------------------
if (NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to a JUCE source checkout")
endif()

if (NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE_DIR must point to a JUCE checkout containing CMakeLists.txt. Current: ${JUCE_DIR}")
endif()

# Direct2D needs a full D2D1 SDK that MinGW winlibs headers don't provide.
# JUCE falls back to software rendering, which is fine (and more portable).
add_compile_definitions(JUCE_USE_DIRECT2D=0)

# Allow JUCE's modal FileChooser dialogs (used by the expansion cassette)
# inside the VST3 editor. JUCE disables these by default for plugins.
add_compile_definitions(JUCE_MODAL_LOOPS_PERMITTED=1)

add_subdirectory(${JUCE_DIR} juce)

# ---------------------------------------------------------------------------
# Artwork and layout are compiled into the binary, so that the editor shows on
# any machine. They are produced from the master mockup by Tools/make_ui_assets.py
# (faceplate with the moving parts removed, the sprites drawn back at run time,
# and every measured position).
# ---------------------------------------------------------------------------
juce_add_binary_data(ROCKPUNK_Data
    SOURCES
        Assets/faceplate.jpg
        Assets/sprites.png
        Assets/layout.json)

# ---------------------------------------------------------------------------
# Plugin
# ---------------------------------------------------------------------------
if (APPLE)
    set(ROCKPUNK_FORMATS VST3 AU Standalone)
else()
    set(ROCKPUNK_FORMATS VST3 Standalone)
endif()

juce_add_plugin(ROCKPUNK
    VERSION ${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE HUMP
    PLUGIN_CODE RPK1
    PRODUCT_NAME "ROCKPUNK"
    COMPANY_NAME "DJBILBOX BEATS"
    COMPANY_WEBSITE "https://djbilboxbeats.com"
    COMPANY_EMAIL "support@djbilboxbeats.com"
    BUNDLE_ID "com.djbilboxbeats.rockpunk"
    DESCRIPTION "ROCKPUNK GUITAR - hybrid rock'n'roll guitar engine"
    FORMATS ${ROCKPUNK_FORMATS}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
    # Without these the standalone carries no icon resource, so Windows falls
    # back to the blank-document glyph in the taskbar and on the shortcut.
    ICON_BIG   "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon_source.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon_source.png")

target_sources(ROCKPUNK
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_include_directories(ROCKPUNK PRIVATE Source)

target_compile_definitions(ROCKPUNK
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        ROCKPUNK_VERSION_STRING="${PROJECT_VERSION}")

target_link_libraries(ROCKPUNK
    PRIVATE
        ROCKPUNK_Data
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Contenu embarque dans le bundle
#
# Les presets d'usine voyagent dans Resources : l'installeur Windows tourne en
# mode eleve et ne peut donc pas ecrire dans les Documents de l'utilisateur -
# c'est le plugin qui les y recopie au premier lancement.
#
# La banque de sons est facultative. ROCKPUNK n'en livre aucune : les deux
# cordes sont calculees et la couche echantillonnee joue le dossier de
# l'utilisateur. Le chemin reste cable pour pouvoir embarquer un jour une banque
# dont on a la licence.
# ---------------------------------------------------------------------------
set(ROCKPUNK_BANK "${CMAKE_CURRENT_SOURCE_DIR}/SampleBank/ROCKPUNK.rockpunkbank")

# Repertoire "Resources" de chaque cible : sur macOS tout est dans le bundle,
# sur Windows le VST3 en a un et l'application autonome pose ses fichiers a cote
# de l'executable.
function(rockpunk_stage_content target resources)
    # Le dossier est efface avant d'etre recopie : copy_directory ajoute mais ne
    # retire rien, et la banque renumerotee laissait les presets de l'ancienne
    # serie a cote des neufs.
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${resources}/Presets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Presets" "${resources}/Presets"
        VERBATIM)

    if (EXISTS "${ROCKPUNK_BANK}")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${resources}/SampleBank"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ROCKPUNK_BANK}" "${resources}/SampleBank/ROCKPUNK.rockpunkbank"
            VERBATIM)
    endif()
endfunction()

rockpunk_stage_content(ROCKPUNK_VST3 "$<TARGET_FILE_DIR:ROCKPUNK_VST3>/../Resources")

if (TARGET ROCKPUNK_Standalone)
    if (APPLE)
        rockpunk_stage_content(ROCKPUNK_Standalone
            "$<TARGET_FILE_DIR:ROCKPUNK_Standalone>/../Resources")
    else()
        rockpunk_stage_content(ROCKPUNK_Standalone
            "$<TARGET_FILE_DIR:ROCKPUNK_Standalone>")
    endif()
endif()

if (NOT EXISTS "${ROCKPUNK_BANK}")
    message(STATUS
        "No bank in SampleBank/: the product is built without a sound library. "
        "To bundle one (content you hold the licence for), build it with the "
        "ROCKPUNK_ConvertSamples target, then reconfigure.")
endif()

# ---------------------------------------------------------------------------
# Sound bank builder: trims, normalises, measures the root note and FLAC-encodes
# the source recordings into ROCKPUNK's own bank.
# ---------------------------------------------------------------------------
juce_add_console_app(ROCKPUNK_ConvertSamples PRODUCT_NAME "ROCKPUNK_ConvertSamples")
set_property(TARGET ROCKPUNK_ConvertSamples PROPERTY EXCLUDE_FROM_ALL TRUE)
juce_generate_juce_header(ROCKPUNK_ConvertSamples)
target_sources(ROCKPUNK_ConvertSamples PRIVATE Tools/convert_samples.cpp)
target_compile_features(ROCKPUNK_ConvertSamples PRIVATE cxx_std_17)
target_compile_definitions(ROCKPUNK_ConvertSamples PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
target_link_libraries(ROCKPUNK_ConvertSamples PRIVATE
    juce::juce_audio_formats juce::juce_audio_basics juce::juce_core
    juce::juce_recommended_config_flags)

# ---------------------------------------------------------------------------
# Headless check: opens the bank and renders a note through every sound path.
# A plug-in that installs cleanly and then plays nothing is the failure this
# catches, and no GUI or host is needed to see it.
# ---------------------------------------------------------------------------
juce_add_console_app(ROCKPUNK_RenderTest PRODUCT_NAME "ROCKPUNK_RenderTest")
set_property(TARGET ROCKPUNK_RenderTest PROPERTY EXCLUDE_FROM_ALL TRUE)
juce_generate_juce_header(ROCKPUNK_RenderTest)
target_sources(ROCKPUNK_RenderTest PRIVATE
    Tests/render_test.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)
target_include_directories(ROCKPUNK_RenderTest PRIVATE Source)
target_compile_features(ROCKPUNK_RenderTest PRIVATE cxx_std_17)
target_compile_definitions(ROCKPUNK_RenderTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1
    ROCKPUNK_VERSION_STRING="${PROJECT_VERSION}")
target_link_libraries(ROCKPUNK_RenderTest PRIVATE
    ROCKPUNK_Data
    juce::juce_audio_processors juce::juce_audio_formats juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_recommended_config_flags)

rockpunk_stage_content(ROCKPUNK_RenderTest "$<TARGET_FILE_DIR:ROCKPUNK_RenderTest>")
