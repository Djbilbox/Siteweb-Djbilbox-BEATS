# Test de fumee headless : charge les SFZ, joue notes / phrases / sequenceur, verifie l'audio.
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
target_include_directories(HumpireSmokeTest PRIVATE ../Source/Plugin)
target_link_libraries(HumpireSmokeTest PRIVATE HumpireCore)
target_compile_definitions(HumpireSmokeTest PRIVATE JUCE_STANDALONE_APPLICATION=1 JUCE_PLUGINHOST_VST3=0)
