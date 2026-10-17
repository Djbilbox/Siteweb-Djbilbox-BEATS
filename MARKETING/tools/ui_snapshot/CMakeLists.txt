# Generic headless UI-snapshot build for the camel synth template projects.
#
# Not meant to be configured by hand: render.sh auto-detects everything project specific
# (sources, include dirs, definitions, binary-data assets, class names ...) and writes it to a
# SNAP_CONFIG file that this CMakeLists includes. Variables the config file sets:
#
#   SNAP_PROJ_DIR                  absolute project dir (e.g. .../ARENA_BASS)
#   SNAP_SOURCES                   project .cpp files compiled into the snapshot tool
#   SNAP_INCLUDES                  include directories
#   SNAP_DEFS                      compile definitions (NAME or NAME=VALUE)
#   SNAP_MODULES                   JUCE modules to link (juce::juce_audio_utils ...)
#   SNAP_BD_COUNT                  number of juce_add_binary_data() targets of the project
#   SNAP_BD_<i>_SOURCES/_HEADER/_NAMESPACE   sources (absolute), HEADER_NAME, NAMESPACE of target i
#
# Required cache variables: SNAP_CONFIG, JUCE_DIR.

cmake_minimum_required(VERSION 3.22)
project(UiSnapshot VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# one frame is rendered, so compile time matters more than speed
set(CMAKE_CXX_FLAGS_RELEASE "-O1 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE   "-O1 -DNDEBUG")

if(NOT SNAP_CONFIG OR NOT EXISTS "${SNAP_CONFIG}")
    message(FATAL_ERROR "SNAP_CONFIG missing - run MARKETING/tools/ui_snapshot/render.sh instead of calling cmake directly")
endif()
if(NOT JUCE_DIR OR NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE_DIR must point at a JUCE checkout (got '${JUCE_DIR}')")
endif()

include("${SNAP_CONFIG}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SNAP_CONFIG}")

add_subdirectory("${JUCE_DIR}" juce_build EXCLUDE_FROM_ALL)

# --- binary data: exactly the juce_add_binary_data() calls the real project makes -------------
set(SNAP_BD_TARGETS "")
set(_i 0)
while(_i LESS "${SNAP_BD_COUNT}")
    set(_t SnapAssets${_i})
    set(_args "")
    if(SNAP_BD_${_i}_HEADER)
        list(APPEND _args HEADER_NAME "${SNAP_BD_${_i}_HEADER}")
    endif()
    if(SNAP_BD_${_i}_NAMESPACE)
        list(APPEND _args NAMESPACE "${SNAP_BD_${_i}_NAMESPACE}")
    endif()
    juce_add_binary_data(${_t} SOURCES ${SNAP_BD_${_i}_SOURCES} ${_args})
    list(APPEND SNAP_BD_TARGETS ${_t})
    math(EXPR _i "${_i} + 1")
endwhile()

# --- the snapshot tool -----------------------------------------------------------------------
juce_add_console_app(Snapshot PRODUCT_NAME "Snapshot")
target_sources(Snapshot PRIVATE snapshot_main.cpp ${SNAP_SOURCES})
target_include_directories(Snapshot PRIVATE ${SNAP_INCLUDES})
target_compile_definitions(Snapshot PRIVATE ${SNAP_DEFS})
target_link_libraries(Snapshot PRIVATE
    ${SNAP_BD_TARGETS}
    ${SNAP_MODULES}
    juce::juce_recommended_config_flags)
