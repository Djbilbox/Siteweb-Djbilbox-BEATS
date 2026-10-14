cmake_minimum_required(VERSION 3.22)
project(HumpireChaabiOrchestra VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(HUMPIRE_COPY_PLUGIN "Copie le VST3 dans le dossier systeme apres build" ON)
option(HUMPIRE_BUILD_TESTS "Compile le test de fumee headless" OFF)
set(HUMPIRE_JUCE_DIR "" CACHE PATH "Chemin vers un clone local de JUCE (sinon FetchContent 8.0.4)")

# --- JUCE ------------------------------------------------------------------
if(HUMPIRE_JUCE_DIR)
  add_subdirectory(${HUMPIRE_JUCE_DIR} JUCE EXCLUDE_FROM_ALL)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    ON)
  FetchContent_MakeAvailable(JUCE)
endif()

# --- Modules du projet -----------------------------------------------------
add_subdirectory(Source/HumpireCore)
add_subdirectory(Source/Plugin)
if(HUMPIRE_BUILD_TESTS)
  add_subdirectory(Tests)
endif()

# NOTE : "HUMPIRE Hub" (activation / mises a jour) = livrable suivant,
#        il se branchera ici via add_subdirectory(Source/Hub) + HumpireCore.
