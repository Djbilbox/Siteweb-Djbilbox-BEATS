# Built-in modelled library: pure C++17, standard library only (no JUCE).
# Linked by the plugin, the smoke test and the ChaabiRenderLibrary console tool.
add_library(HumpireModel STATIC
  ModelLibrary.cpp
  PluckedModels.cpp
  BowedWindModels.cpp
  PercussionModels.cpp)

target_compile_features(HumpireModel PUBLIC cxx_std_17)
set_target_properties(HumpireModel PROPERTIES POSITION_INDEPENDENT_CODE ON)

# "Model/ModelLibrary.h" resolves from Source/Plugin.
target_include_directories(HumpireModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(MSVC)
  target_compile_options(HumpireModel PRIVATE /W4 /utf-8)
else()
  # -ffp-contract=off: no fused multiply-add differences between compilers / CPUs (reproducible renders).
  # -O2 even in Debug builds so that the background render stays fast during development.
  target_compile_options(HumpireModel PRIVATE -Wall -Wextra -ffp-contract=off
    $<$<CONFIG:Debug>:-O2>)
endif()
