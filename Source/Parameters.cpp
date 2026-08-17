#include "Parameters.h"
#include "DSP/TempoSync.h"
#include "DSP/Lfo.h"

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

    // Choice item order matches juce::dsp::StateVariableTPTFilterType exactly
    // (Lowpass, Bandpass, Highpass) so the index casts straight across.
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::filterType, "Filter Type", StringArray { "Lowpass", "Bandpass", "Highpass" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterCutoff, "Filter Cutoff", NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f), 2000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterResonance, "Filter Resonance", NormalisableRange<float>(0.1f, 20.0f, 0.0f, 0.35f), 0.707f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterVelSens, "Filter Vel->Cutoff", NormalisableRange<float>(0.0f, 1.0f), 0.3f));

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
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::arpSyncDivision, "Arp Sync", syncDivisionChoices(), 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::arpRateHz, "Arp Rate", NormalisableRange<float>(1.0f, 20.0f, 0.0f, 0.5f), 8.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::arpGate, "Arp Gate", NormalisableRange<float>(0.1f, 1.0f), 0.7f));

    // --- LFO (§2 -- one routable LFO, tempo-syncable) ---
    params.push_back(
        std::make_unique<AudioParameterChoice>(ParamIDs::lfoWaveform, "LFO Wave", Lfo::waveformChoices(), 0));
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::lfoSyncDivision, "LFO Sync", syncDivisionChoices(), 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::lfoRateHz, "LFO Rate", NormalisableRange<float>(0.05f, 20.0f, 0.0f, 0.4f), 4.5f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::lfoDepth, "LFO Depth", NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    // Choice item order must match gexex::ModTarget exactly (ModTarget.h).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::lfoTarget, "LFO Target",
        StringArray { "Off", "Pitch (all osc)", "Filter Cutoff", "Filter Resonance", "Amp", "Osc1 Level",
                       "Osc2 Level", "Osc3 Level", "Osc2 FM Amount", "Osc3 FM Amount", "Delay Time",
                       "Delay Feedback", "Delay Mix", "Reverb Mix", "Bitcrush Depth", "Bitcrush Downsample",
                       "Auto-Pan" },
        0));

    // --- Effects rack (§3), in signal-flow order ---
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::crushBits, "Bitcrush Depth", NormalisableRange<float>(1.0f, 16.0f), 16.0f,
        AudioParameterFloatAttributes().withLabel("bits")));
    params.push_back(
        std::make_unique<AudioParameterInt>(ParamIDs::crushDownsample, "Bitcrush Downsample", 1, 40, 1));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::driveAmount, "Drive", NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::delaySyncDivision, "Delay Sync", syncDivisionChoices(), 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::delayTimeSeconds, "Delay Time", NormalisableRange<float>(0.02f, 1.0f, 0.0f, 0.4f), 0.28f,
        AudioParameterFloatAttributes().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::delayFeedback, "Delay Feedback", NormalisableRange<float>(0.0f, 0.85f), 0.3f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::delayMix, "Delay Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::reverbSize, "Reverb Size", NormalisableRange<float>(0.1f, 8.0f, 0.0f, 0.4f), 1.8f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::reverbMix, "Reverb Mix", NormalisableRange<float>(0.0f, 1.0f), 0.12f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::chorusRateHz, "Chorus Rate", NormalisableRange<float>(0.05f, 5.0f, 0.0f, 0.5f), 0.4f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::chorusDepthMs, "Chorus Depth", NormalisableRange<float>(0.0f, 12.0f), 4.0f,
        AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::chorusMix, "Chorus Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // Choice item order must match gexex::PhaserFlangerMode (Phaser, Flanger).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::pfMode, "Phaser/Flanger Mode", StringArray { "Phaser", "Flanger" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfRateHz, "Phaser/Flanger Rate", NormalisableRange<float>(0.02f, 6.0f, 0.0f, 0.4f), 0.3f,
        AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfDepth, "Phaser/Flanger Depth", NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfFeedback, "Phaser/Flanger Feedback", NormalisableRange<float>(0.0f, 0.85f), 0.3f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfMix, "Phaser/Flanger Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // Choice item order must match gexex::SaturatorAlgorithm (Tanh, Cubic, Arctan, Hard).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::saturatorAlgorithm, "Saturator Algorithm",
        StringArray { "Tanh (warm)", "Cubic (gentle)", "Arctan (smooth)", "Hard (edgy)" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::saturatorAmount, "Saturator Amount", NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::saturatorCeiling, "Saturator Ceiling", NormalisableRange<float>(0.3f, 1.0f), 1.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::masterVolume, "Master Volume", NormalisableRange<float>(0.0f, 1.0f), 0.8f));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::masterPan, "Master Pan", NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterBool>(ParamIDs::reducedMotion, "Reduced Motion", false));

    return AudioProcessorValueTreeState::ParameterLayout(params.begin(), params.end());
}
