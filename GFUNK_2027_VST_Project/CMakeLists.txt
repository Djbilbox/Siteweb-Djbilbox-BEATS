cmake_minimum_required(VERSION 3.22)
project(GFUNK2027 VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

# Use a local JUCE checkout with -DJUCE_DIR=/path/to/JUCE, otherwise fetch JUCE 8.0.4.
set(JUCE_DIR "" CACHE PATH "Optional path to a local JUCE checkout")
option(GFUNK_FREE "Build GFUNK 2027 Free (100 presets, separate plug-in ID)" OFF)
option(GFUNK_BUILD_TESTS "Build the offline render test" ON)
if(JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory(${JUCE_DIR} juce EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
endif()

if(APPLE)
    set(GFUNK_FORMATS VST3 AU Standalone)
else()
    set(GFUNK_FORMATS VST3 Standalone)
endif()

file(GLOB GFUNK_ASSETS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.png" "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.jpg")
list(FILTER GFUNK_ASSETS EXCLUDE REGEX "hero_user")
if(GFUNK_ASSETS)
    juce_add_binary_data(GFunkAssets SOURCES ${GFUNK_ASSETS})
    set_target_properties(GFunkAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()

set(GFUNK_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Generated/GFunkSpec.cpp)

if(GFUNK_FREE)
    set(GFUNK_PRODUCT "GFUNK 2027 Free")
    set(GFUNK_CODE Gfk8)
else()
    set(GFUNK_PRODUCT "GFUNK 2027")
    set(GFUNK_CODE Gfk7)
endif()

juce_add_plugin(GFUNK2027
    COMPANY_NAME "HUMPIRE by Djbilbox BEATS"
    PRODUCT_NAME "${GFUNK_PRODUCT}"
    PLUGIN_MANUFACTURER_CODE Hmpr
    PLUGIN_CODE ${GFUNK_CODE}
    FORMATS ${GFUNK_FORMATS}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice)

target_sources(GFUNK2027 PRIVATE ${GFUNK_SOURCES})

target_compile_definitions(GFUNK2027 PUBLIC
    $<$<BOOL:${GFUNK_FREE}>:GFUNK_FREE=1>
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0)

target_link_libraries(GFUNK2027
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags)
if(GFUNK_ASSETS)
    target_link_libraries(GFUNK2027 PRIVATE GFunkAssets)
endif()

if(GFUNK_BUILD_TESTS)
    juce_add_console_app(GFunkRenderTest PRODUCT_NAME "GFunkRenderTest")
    target_sources(GFunkRenderTest PRIVATE Tests/RenderTest.cpp ${GFUNK_SOURCES})
    target_compile_definitions(GFunkRenderTest PRIVATE $<$<BOOL:${GFUNK_FREE}>:GFUNK_FREE=1> JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_WINDOWS_MEDIA_FORMAT=0 JUCE_MODAL_LOOPS_PERMITTED=1)
    target_link_libraries(GFunkRenderTest
        PRIVATE juce::juce_audio_utils juce::juce_dsp
        PUBLIC juce::juce_recommended_config_flags)
    if(GFUNK_ASSETS)
        target_link_libraries(GFunkRenderTest PRIVATE GFunkAssets)
    endif()
endif()

# MinGW (clang/gcc on Windows): link the C++ runtime statically so the plug-in has no extra DLL dependencies.
if(MINGW)
    foreach(fmt IN ITEMS VST3 Standalone)
        if(TARGET GFUNK2027_${fmt})
            target_link_options(GFUNK2027_${fmt} PRIVATE -static)
        endif()
    endforeach()
endif()
