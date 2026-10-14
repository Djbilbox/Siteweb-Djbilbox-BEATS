# HUMPIRE GLITCH - lo-fi / bitcrusher
humpire_add_plugin(
    TARGET  HumpireGlitch
    NAME    "HUMPIRE GLITCH"
    CODE    Hgl1
    SOURCES Source/PluginProcessor.cpp
            Source/PluginEditor.cpp)

# Fond de la console (PNG 2x, 1520x1120) : optionnel, sinon rendu procedural en code.
set(GLITCH_BODY ${CMAKE_CURRENT_LIST_DIR}/assets/glitch_body_2x.png)
if(EXISTS ${GLITCH_BODY})
    juce_add_binary_data(GlitchAssets
        HEADER_NAME GlitchAssetsData.h
        NAMESPACE   GlitchAssetsData
        SOURCES     ${GLITCH_BODY})
    set_target_properties(GlitchAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(HumpireGlitch PRIVATE GlitchAssets)
    target_compile_definitions(HumpireGlitch PRIVATE HUMPIRE_GLITCH_HAS_BODY=1)
endif()
