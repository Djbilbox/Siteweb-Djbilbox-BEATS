juce_add_plugin(HumpireChaabiOrchestra
  COMPANY_NAME "HUMPIRE by Djbilbox BEATS"
  COMPANY_WEBSITE "https://humpire.example"
  BUNDLE_ID com.humpire.chaabiorchestra
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD ${HUMPIRE_COPY_PLUGIN}
  PLUGIN_MANUFACTURER_CODE Hmpr
  PLUGIN_CODE Hcho
  FORMATS VST3 Standalone
  PRODUCT_NAME "HUMPIRE CHAABI ORCHESTRA"
  VST3_CATEGORIES Instrument Sampler
  VST3_AUTO_MANIFEST FALSE)

target_sources(HumpireChaabiOrchestra PRIVATE
  PluginProcessor.cpp
  PluginEditor.cpp
  ChaabiSynth.cpp
  SampleBank.cpp
  SfzParser.cpp
  PhraseBank.cpp
  PhrasePlayer.cpp)

target_link_libraries(HumpireChaabiOrchestra PRIVATE HumpireCore HumpireModel)

target_compile_definitions(HumpireChaabiOrchestra PUBLIC
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_STRICT_REFCOUNTEDPOINTER=1)
