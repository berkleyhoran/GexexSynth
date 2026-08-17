#include "Parameters.h"

juce::AudioProcessorValueTreeState::ParameterLayout gexex::createParameterLayout()
{
    using namespace juce;

    // Cutoff/resonance/time knobs are skewed so the middle of the slider
    // lands somewhere musically useful (log-ish for cutoff and the longer
    // envelope stages) rather than a straight linear sweep -- matches the
    // ranges the browser reference (gexex/synth.html) uses for the same
    // controls.
    return AudioProcessorValueTreeState::ParameterLayout(
        std::make_unique<AudioParameterFloat>(
            ParamIDs::filterCutoff, "Filter Cutoff",
            NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f), 2000.0f,
            AudioParameterFloatAttributes().withLabel("Hz")),

        std::make_unique<AudioParameterFloat>(
            ParamIDs::filterResonance, "Filter Resonance",
            NormalisableRange<float>(0.1f, 20.0f, 0.0f, 0.35f), 0.707f),

        std::make_unique<AudioParameterFloat>(
            ParamIDs::envAttack, "Attack",
            NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.02f,
            AudioParameterFloatAttributes().withLabel("s")),

        std::make_unique<AudioParameterFloat>(
            ParamIDs::envDecay, "Decay",
            NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.15f,
            AudioParameterFloatAttributes().withLabel("s")),

        std::make_unique<AudioParameterFloat>(
            ParamIDs::envSustain, "Sustain",
            NormalisableRange<float>(0.0f, 1.0f), 0.8f),

        std::make_unique<AudioParameterFloat>(
            ParamIDs::envRelease, "Release",
            NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.3f), 0.3f,
            AudioParameterFloatAttributes().withLabel("s")));
}
