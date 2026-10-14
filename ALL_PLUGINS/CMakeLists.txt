# =============================================================================
#  DJBILBOX BEATS - SUPER-BUILD : tous les synthes + HUMPIRE LOADOUT en un seul build
#  (HUMPIRE ORBIT et HUMPIRE CHAABI ORCHESTRA se construisent a part : leurs cibles
#   "HumpireCore" entreraient en collision -> voir build_all.ps1 / workflow CI)
#
#  cmake -S ALL_PLUGINS -B build/all -G Ninja -DCMAKE_BUILD_TYPE=Release
#  ninja -C build/all -k 0 <Cible>_VST3 ...
# =============================================================================
cmake_minimum_required(VERSION 3.22)
project(DJBILBOX_ALL_PLUGINS VERSION 1.0.0 LANGUAGES C CXX)

# JUCE une seule fois : les FetchContent_Declare(JUCE) des sous-projets deviennent des no-op.
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

set(DJB_SYNTHS
    ARENA_BASS CHINTZWAZ CYBERPUNK_LEAD DZPOWER FUNKYKOPS GOLDHAWK HIGHROLLER_GOLD K2000
    MARABOUT MIDNIGHT_REELS OCCITANIE OCTOPUS PHANTOM_RIDER SAN_ANDREAS_LEAD SHAMAN
    STREETLIGHT_TRAP VOODOO WAZONE808)

# Pas de tests console dans le super-build
foreach(s IN LISTS DJB_SYNTHS)
    set(${s}_BUILD_TESTS OFF CACHE BOOL "" FORCE)
endforeach()
set(WZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(HUMPIRE_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)

foreach(s IN LISTS DJB_SYNTHS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${s}/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../${s}" "${s}")
    else()
        message(WARNING "DJB: ${s} absent")
    endif()
endforeach()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../HUMPIRE_LOADOUT" "HUMPIRE_LOADOUT")
