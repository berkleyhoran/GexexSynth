cmake_minimum_required(VERSION 3.22)

# Overridable from the command line (`-DGEXEXSYNTH_VERSION=0.2.0`) so CI can
# drive the plugin/installer version straight from a git tag without editing
# this file -- see .github/workflows/release.yml. Local/manual builds that
# don't pass it just get this default. (Same pattern as Kaleidosonic's
# KALEIDOSONIC_VERSION.)
set(GEXEXSYNTH_VERSION "0.1.0" CACHE STRING "Gexex Synth version (plugin + installers)")

project(GexexSynth VERSION ${GEXEXSYNTH_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# JUCE
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.15
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# Background art -> BinaryData
# ---------------------------------------------------------------------------
# Downscaled/re-exported from gexex/assets (the source PNGs are 1.7-4.5MB at
# full res -- not appropriate to embed directly; see Assets/background/'s
# generation notes in the Phase 5 commit).
file(GLOB GEXEXSYNTH_BACKGROUND_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/Assets/background/*.png)
juce_add_binary_data(GexexSynthAssets SOURCES ${GEXEXSYNTH_BACKGROUND_ASSETS})
set_target_properties(GexexSynthAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# Plugin target
# ---------------------------------------------------------------------------
# App/installer icon: drop a square-ish PNG at Assets/logo.png and it's
# picked up automatically on the next configure (JUCE generates the Windows
# .ico for the Standalone exe from it). Absent file = no icon, no error.
set(GEXEXSYNTH_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/Assets/logo.png")
set(GEXEXSYNTH_ICON_ARGS "")
if(EXISTS ${GEXEXSYNTH_LOGO})
    set(GEXEXSYNTH_ICON_ARGS ICON_BIG "${GEXEXSYNTH_LOGO}" ICON_SMALL "${GEXEXSYNTH_LOGO}")
endif()

# AU only makes sense (and only builds) on Apple -- most Mac DAWs (Logic Pro
# in particular) expect it over VST3, so it's worth the extra format on that
# platform specifically rather than everywhere.
set(GEXEXSYNTH_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND GEXEXSYNTH_FORMATS AU)
endif()

juce_add_plugin(GexexSynth
    ${GEXEXSYNTH_ICON_ARGS}
    COMPANY_NAME "gexex"
    PLUGIN_MANUFACTURER_CODE Gxex
    PLUGIN_CODE Gsy1
    FORMATS ${GEXEXSYNTH_FORMATS}
    PRODUCT_NAME "Gexex Synth"
    VST3_CATEGORIES "Instrument" "Synth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(GexexSynth)

target_sources(GexexSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Parameters.cpp
    Source/DSP/Arpeggiator.cpp
    Source/DSP/SynthEngine.cpp
    Source/DSP/Effects/EffectsChain.cpp
    Source/GUI/LookAndFeel.cpp
    Source/GUI/ModuleRack.cpp
    Source/GUI/BackgroundScene.cpp
    Source/GUI/PerformanceStrip.cpp
    Source/Presets/PresetManager.cpp
    Source/GUI/PresetBar.cpp)

target_compile_definitions(GexexSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(GexexSynth PRIVATE
    GexexSynthAssets
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
