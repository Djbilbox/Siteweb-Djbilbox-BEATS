# humpire_add_plugin(
#     TARGET   HumpireGlitch
#     NAME     "HUMPIRE GLITCH"
#     CODE     Hgl1          # 4 caracteres, unique par plugin
#     SOURCES  Source/PluginProcessor.cpp ...
#     [NEEDS_MIDI])
function(humpire_add_plugin)
    cmake_parse_arguments(HP "NEEDS_MIDI" "TARGET;NAME;CODE" "SOURCES" ${ARGN})

    set(formats VST3)
    if(HUMPIRE_BUILD_STANDALONE)
        list(APPEND formats Standalone)
    endif()

    if(HP_NEEDS_MIDI)
        set(midi_in TRUE)
    else()
        set(midi_in FALSE)
    endif()

    juce_add_plugin(${HP_TARGET}
        COMPANY_NAME                "HUMPIRE by Djbilbox BEATS"
        PLUGIN_MANUFACTURER_CODE    Hmpr
        PLUGIN_CODE                 ${HP_CODE}
        PRODUCT_NAME                "${HP_NAME}"
        FORMATS                     ${formats}
        VST3_CATEGORIES             Fx
        IS_SYNTH                    FALSE
        NEEDS_MIDI_INPUT            ${midi_in}
        NEEDS_MIDI_OUTPUT           FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD     ${HUMPIRE_COPY_AFTER_BUILD})

    target_sources(${HP_TARGET} PRIVATE ${HP_SOURCES})

    target_compile_definitions(${HP_TARGET} PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0)

    target_link_libraries(${HP_TARGET}
        PRIVATE HumpireCore
        PUBLIC  juce::juce_recommended_config_flags
                juce::juce_recommended_lto_flags
                juce::juce_recommended_warning_flags)
endfunction()
