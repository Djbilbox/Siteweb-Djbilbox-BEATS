# HumpireCore : module partage (LookAndFeel, Knob, Meter, PresetManager, theme).
# Cible INTERFACE (pattern "SharedCode" de JUCE) : les sources sont compilees
# dans chaque cible qui la lie (plugin, Hub), ce qui evite tout doublon de modules JUCE.
add_library(HumpireCore INTERFACE)

target_sources(HumpireCore INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/HumpireLookAndFeel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/HumpirePresetManager.cpp)

target_include_directories(HumpireCore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(HumpireCore INTERFACE
  juce::juce_audio_utils
  juce::juce_audio_processors
  juce::juce_audio_formats
  juce::juce_dsp
  juce::juce_gui_basics
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags)

target_compile_definitions(HumpireCore INTERFACE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_MODAL_LOOPS_PERMITTED=1)

target_compile_options(HumpireCore INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
