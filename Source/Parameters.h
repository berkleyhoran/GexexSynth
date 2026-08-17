#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace gexex
{
    // Phase 1: just enough parameters to hear the core voice (oscillator ->
    // filter -> envelope). This file grows into the full ~100-parameter
    // layout (oscillators, FM, LFO, effects rack -- see the build plan's
    // §4) as later phases add them; every parameter is declared here in one
    // place so a future randomize-exclusion set / parameter groups have one
    // spot to stay in sync with.
    namespace ParamIDs
    {
        static constexpr auto filterCutoff = "filterCutoff";
        static constexpr auto filterResonance = "filterResonance";
        static constexpr auto envAttack = "envAttack";
        static constexpr auto envDecay = "envDecay";
        static constexpr auto envSustain = "envSustain";
        static constexpr auto envRelease = "envRelease";
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
