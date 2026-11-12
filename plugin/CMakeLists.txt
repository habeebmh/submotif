cmake_minimum_required(VERSION 3.22)
project(Undertow VERSION 1.3.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
endif()

option(UNDERTOW_CORE_ONLY "Build the portable engine tests without JUCE" OFF)
enable_testing()
add_library(UndertowCore STATIC src/PatternEngine.cpp)
target_include_directories(UndertowCore PUBLIC src)
add_executable(UndertowEngineTests tests/EngineTests.cpp)
target_link_libraries(UndertowEngineTests PRIVATE UndertowCore)
add_test(NAME engine COMMAND UndertowEngineTests)
add_executable(UndertowGenerationProbe tests/GenerationProbe.cpp)
target_link_libraries(UndertowGenerationProbe PRIVATE UndertowCore)
if(UNDERTOW_CORE_ONLY)
    return()
endif()

set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.deps/JUCE" CACHE PATH "JUCE source location")
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG dabd5ad13cd869cba41046947fffc8e5ce5cd662 GIT_SHALLOW FALSE)
    FetchContent_MakeAvailable(JUCE)
else()
    add_subdirectory("${JUCE_DIR}" JUCE EXCLUDE_FROM_ALL)
endif()

juce_add_plugin(Undertow
    COMPANY_NAME "Habeeb"
    BUNDLE_ID "audio.undertow.bassline"
    PLUGIN_MANUFACTURER_CODE Udtw
    PLUGIN_CODE Utbw
    FORMATS VST3 Standalone
    PRODUCT_NAME "Submotif"
    PLUGIN_NAME "SUBMOTIF"
    DESCRIPTION "SUBMOTIF by Habeeb"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Tools
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)
target_sources(Undertow PRIVATE src/PluginProcessor.cpp src/PluginEditor.cpp)
target_include_directories(Undertow PUBLIC src)
target_compile_definitions(Undertow PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0)
target_link_libraries(Undertow PRIVATE UndertowCore juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags)

include(scripts/cmake/UndertowVST3InputGuard.cmake)
undertow_guard_vst3_input(Undertow)

add_executable(UndertowIntegrationTests tests/ProcessorTests.cpp)
target_link_libraries(UndertowIntegrationTests PRIVATE Undertow UndertowCore juce::juce_audio_utils)
add_test(NAME processor COMMAND UndertowIntegrationTests)

add_executable(UndertowEditorTests tests/EditorTests.cpp)
target_link_libraries(UndertowEditorTests PRIVATE Undertow UndertowCore juce::juce_audio_utils)
add_test(NAME editor COMMAND UndertowEditorTests)

# Exercise the real wrapper, including parameter-only calls that skip processBlock.
if(APPLE)
    get_target_property(juce_modules juce::juce_audio_plugin_client INTERFACE_JUCE_MODULE_PATH)
    set(vst3_sdk "${juce_modules}/juce_audio_processors_headless/format_types/VST3_SDK")
    add_executable(UndertowVST3Tests tests/VST3Tests.cpp
        "${vst3_sdk}/pluginterfaces/base/funknown.cpp"
        "${vst3_sdk}/pluginterfaces/base/coreiids.cpp"
        "${vst3_sdk}/public.sdk/source/vst/vstinitiids.cpp")
    target_include_directories(UndertowVST3Tests PRIVATE "${vst3_sdk}")
    target_link_libraries(UndertowVST3Tests PRIVATE "-framework CoreFoundation")
    add_dependencies(UndertowVST3Tests Undertow_VST3)
    add_test(NAME vst3-wrapper COMMAND UndertowVST3Tests "$<TARGET_BUNDLE_DIR:Undertow_VST3>")
endif()
