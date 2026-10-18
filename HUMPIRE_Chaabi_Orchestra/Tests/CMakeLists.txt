# Headless smoke test: exercises the built-in modelled library (no sample folder needed),
# the processor, the editor and, optionally, a real SFZ library.
juce_add_console_app(HumpireSmokeTest PRODUCT_NAME "HumpireSmokeTest")
target_sources(HumpireSmokeTest PRIVATE
  SmokeTest.cpp
  ../Source/Plugin/PluginProcessor.cpp
  ../Source/Plugin/PluginEditor.cpp
  ../Source/Plugin/ChaabiSynth.cpp
  ../Source/Plugin/SampleBank.cpp
  ../Source/Plugin/SfzParser.cpp
  ../Source/Plugin/PhraseBank.cpp
  ../Source/Plugin/PhrasePlayer.cpp)
target_include_directories(HumpireSmokeTest PRIVATE ../Source/Plugin ../tools)
target_link_libraries(HumpireSmokeTest PRIVATE HumpireCore HumpireModel)
target_compile_definitions(HumpireSmokeTest PRIVATE JUCE_STANDALONE_APPLICATION=1 JUCE_PLUGINHOST_VST3=0)

add_test(NAME smoke COMMAND HumpireSmokeTest)
