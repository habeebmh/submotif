# Live requires instrument plug-ins to advertise an event input bus. Undertow
# intentionally ignores that bus. JUCE normally copies it to the output even
# when a zero-audio parameter flush or suspension skips the processor callback.
# Generate a narrowly guarded wrapper without modifying the pinned dependency.
function(undertow_guard_vst3_input target)
    get_target_property(modules juce::juce_audio_plugin_client INTERFACE_JUCE_MODULE_PATH)
    set(wrapper_dir "${modules}/juce_audio_plugin_client")
    set(original "${wrapper_dir}/juce_audio_plugin_client_VST3.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${original}")
    file(READ "${original}" source)
    set(anchor "\n        if (detail::PluginUtilities::getHostType().isWavelab())")
    string(FIND "${source}" "${anchor}" anchor_position)
    if(anchor_position EQUAL -1)
        message(FATAL_ERROR "JUCE VST3 input guard anchor changed; review the wrapper before building.")
    endif()
    string(REPLACE "${anchor}" "" remainder "${source}")
    string(LENGTH "${source}" original_length)
    string(LENGTH "${remainder}" remainder_length)
    string(LENGTH "${anchor}" anchor_length)
    math(EXPR removed_length "${original_length} - ${remainder_length}")
    if(NOT removed_length EQUAL anchor_length)
        message(FATAL_ERROR "JUCE VST3 input guard anchor is not unique.")
    endif()
    string(REPLACE "${anchor}"
        "\n        // Undertow accepts a host input bus for compatibility, never passthrough.\n        // Clear after event/parameter conversion, including callback-skipping paths.\n        midiBuffer.clear();\n${anchor}"
        guarded_source "${source}")
    set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/undertow-vst3-guard")
    file(MAKE_DIRECTORY "${generated_dir}")
    file(WRITE "${generated_dir}/juce_audio_plugin_client_VST3.cpp" "${guarded_source}")
    if(APPLE)
        set(extension mm)
        file(WRITE "${generated_dir}/juce_audio_plugin_client_VST3.mm"
            "#include \"juce_audio_plugin_client_VST3.cpp\"\n")
    else()
        set(extension cpp)
    endif()
    # The JUCE wrapper source is supplied via an interface target. Exclude that
    # original in this directory and compile our generated copy for VST3 only.
    set_source_files_properties("${wrapper_dir}/juce_audio_plugin_client_VST3.${extension}"
        PROPERTIES HEADER_FILE_ONLY TRUE)
    target_sources(${target}_VST3 PRIVATE "${generated_dir}/juce_audio_plugin_client_VST3.${extension}")
    target_include_directories(${target}_VST3 PRIVATE "${wrapper_dir}")
endfunction()
