#include "Parameters.h"

juce::String gexex::oscParamID(int oscNumber, const juce::String& suffix)
{
    return "osc" + juce::String(oscNumber) + suffix;
}

juce::AudioProcessorValueTreeState::ParameterLayout gexex::createParameterLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // Choice item order must match gexex::Waveform exactly (Sine, Triangle,
    // Saw, Square, Pulse) -- see Parameters.h's comment.
    const StringArray waveformChoices { "Sine", "Triangle", "Saw", "Square", "Pulse" };

    // Osc1 defaults to a plain saw so the plugin makes sound the moment
    // it's loaded; osc2/osc3 default to silent (level 0) rather than
    // muted, so raising their level fader is all it takes to bring them in.
    struct OscDefault
    {
        int waveIndex;
        float level;
    };
    const OscDefault oscDefaults[3] = { { 2 /* Saw */, 0.7f }, { 3 /* Square */, 0.0f }, { 2 /* Saw */, 0.0f } };

    for (int oscNumber = 1; oscNumber <= 3; ++oscNumber)
    {
        const auto& d = oscDefaults[oscNumber - 1];

        params.push_back(std::make_unique<AudioParameterChoice>(
            oscParamID(oscNumber, ParamIDs::oscWaveSuffix), "Osc " + String(oscNumber) + " Wave", waveformChoices,
            d.waveIndex));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscPulseWidthSuffix), "Osc " + String(oscNumber) + " Pulse Width",
            NormalisableRange<float>(0.05f, 0.95f), 0.5f));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscFoldSuffix), "Osc " + String(oscNumber) + " Fold",
            NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        params.push_back(std::make_unique<AudioParameterInt>(
            oscParamID(oscNumber, ParamIDs::oscOctaveSuffix), "Osc " + String(oscNumber) + " Octave", -2, 2, 0));

        params.push_back(std::make_unique<AudioParameterInt>(
            oscParamID(oscNumber, ParamIDs::oscSemitoneSuffix), "Osc " + String(oscNumber) + " Semitone", -12, 12,
            0));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscFineSuffix), "Osc " + String(oscNumber) + " Fine",
            NormalisableRange<float>(-100.0f, 100.0f), 0.0f, AudioParameterFloatAttributes().withLabel("cents")));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscLevelSuffix), "Osc " + String(oscNumber) + " Level",
            NormalisableRange<float>(0.0f, 1.0f), d.level));

        params.push_back(std::make_unique<AudioParameterBool>(
            oscParamID(oscNumber, ParamIDs::oscMuteSuffix), "Osc " + String(oscNumber) + " Mute", false));
    }

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::fmAmount2, "FM Osc2->Osc1", NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::fmAmount3, "FM Osc3->Osc1", NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterCutoff, "Filter Cutoff", NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f), 2000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterResonance, "Filter Resonance", NormalisableRange<float>(0.1f, 20.0f, 0.0f, 0.35f), 0.707f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envAttack, "Attack", NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.02f,
        AudioParameterFloatAttributes().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envDecay, "Decay", NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.15f,
        AudioParameterFloatAttributes().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envSustain, "Sustain", NormalisableRange<float>(0.0f, 1.0f), 0.8f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envRelease, "Release", NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.3f), 0.3f,
        AudioParameterFloatAttributes().withLabel("s")));

    // Choice item order must match gexex::VoiceMode (Mono, Poly). Defaults
    // to Mono, matching the browser reference's default voice-mode button
    // state.
    params.push_back(
        std::make_unique<AudioParameterChoice>(ParamIDs::voiceMode, "Voice Mode", StringArray { "Mono", "Poly" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::glideTime, "Glide", NormalisableRange<float>(0.0f, 2.0f, 0.0f, 0.3f), 0.0f,
        AudioParameterFloatAttributes().withLabel("s")));

    params.push_back(std::make_unique<AudioParameterBool>(ParamIDs::arpEnabled, "Arp On", false));
    // Choice item order must match gexex::ArpPattern (Up, Down, UpDown, Random).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::arpPattern, "Arp Pattern", StringArray { "Up", "Down", "Up-Down", "Random" }, 0));
    params.push_back(
        std::make_unique<AudioParameterInt>(ParamIDs::arpOctaveRange, "Arp Octave Range", 1, 4, 1));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::arpRateHz, "Arp Rate", NormalisableRange<float>(1.0f, 20.0f, 0.0f, 0.5f), 8.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::arpGate, "Arp Gate", NormalisableRange<float>(0.1f, 1.0f), 0.7f));

    return AudioProcessorValueTreeState::ParameterLayout(params.begin(), params.end());
}
