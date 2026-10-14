cmake_minimum_required(VERSION 3.22)
project(HUMPIRE_LOADOUT VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")   # runtime statique (Windows)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

option(HUMPIRE_COPY_AFTER_BUILD "Copie les VST3 dans le dossier systeme apres build" OFF)
option(HUMPIRE_BUILD_STANDALONE "Construit aussi les versions Standalone (tests)" ON)

# ---------------------------------------------------------------- JUCE
# Priorite : external/JUCE (submodule) sinon FetchContent.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    ON)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------- Core partage
add_subdirectory(Source/HumpireCore)
include(cmake/HumpirePlugin.cmake)

# ---------------------------------------------------------------- Les 8 plugins
# Un plugin est construit des que plugins/<Nom>/CMakeLists.txt existe.
set(HUMPIRE_PLUGINS Skilltree Clutch Overclock Hardcap Respawn Lag Glitch Combo)
foreach(p IN LISTS HUMPIRE_PLUGINS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins/${p}/CMakeLists.txt")
        add_subdirectory(plugins/${p})
    else()
        message(STATUS "HUMPIRE: plugin '${p}' pas encore implemente (ignore)")
    endif()
endforeach()

# ---------------------------------------------------------------- Hub (app standalone)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Hub/CMakeLists.txt")
    add_subdirectory(Hub)
endif()
