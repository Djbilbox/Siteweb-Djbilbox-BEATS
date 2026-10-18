cmake_minimum_required(VERSION 3.22)
project(HumpireChaabiOrchestra VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(HUMPIRE_COPY_PLUGIN "Copy the VST3 to the system plug-in folder after building" ON)
option(HUMPIRE_BUILD_TESTS "Build the headless smoke test" OFF)
option(HUMPIRE_BUILD_TOOLS "Build the console tools (ChaabiRenderLibrary)" ON)
set(HUMPIRE_JUCE_DIR "" CACHE PATH "Path to a local JUCE clone (otherwise FetchContent 8.0.4)")

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

# --- Project modules --------------------------------------------------------
add_subdirectory(Source/Plugin/Model)      # built-in modelled library (pure C++17, no JUCE)
add_subdirectory(Source/HumpireCore)
add_subdirectory(Source/Plugin)

# Console tool: renders the built-in library to WAV + SFZ for inspection (std only, no JUCE).
if(HUMPIRE_BUILD_TOOLS)
  add_executable(ChaabiRenderLibrary tools/render_library.cpp)
  target_link_libraries(ChaabiRenderLibrary PRIVATE HumpireModel)
  target_include_directories(ChaabiRenderLibrary PRIVATE tools)
endif()

if(HUMPIRE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(Tests)
  if(HUMPIRE_BUILD_TOOLS)
    # renders + validates the built-in library (key map, coverage, budgets) without JUCE
    add_test(NAME model_validate COMMAND ChaabiRenderLibrary --validate-only)
  endif()
endif()

# NOTE: "HUMPIRE Hub" (activation / updates) is the next deliverable;
#       it will plug in here via add_subdirectory(Source/Hub) + HumpireCore.
