#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace gexex
{
    // Every knob is declared here in one place -- grows into the full
    // ~100-parameter layout (effects rack lands in Phase 3) described in
    // the build plan's §4. Parameter *grouping* (AudioProcessorParameterGroup
    // per module, so hosts render a sane tree instead of a flat list) is
    // deferred to Phase 4, where the real GUI's ModuleRack introduces the
    // same module boundaries -- no point building the grouping twice.
    namespace ParamIDs
    {
        static constexpr auto filterCutoff = "filterCutoff";
        static constexpr auto filterResonance = "filterResonance";
        static constexpr auto envAttack = "envAttack";
        static constexpr auto envDecay = "envDecay";
        static constexpr auto envSustain = "envSustain";
        static constexpr auto envRelease = "envRelease";

        static constexpr auto voiceMode = "voiceMode";
        static constexpr auto glideTime = "glideTime";

        static constexpr auto fmAmount2 = "fmAmount2"; // osc2 -> osc1
        static constexpr auto fmAmount3 = "fmAmount3"; // osc3 -> osc1

        static constexpr auto arpEnabled = "arpEnabled";
        static constexpr auto arpPattern = "arpPattern";
        static constexpr auto arpOctaveRange = "arpOctaveRange";
        static constexpr auto arpSyncDivision = "arpSyncDivision";
        static constexpr auto arpRateHz = "arpRateHz";
        static constexpr auto arpGate = "arpGate";

        static constexpr auto lfoWaveform = "lfoWaveform";
        static constexpr auto lfoSyncDivision = "lfoSyncDivision";
        static constexpr auto lfoRateHz = "lfoRateHz";
        static constexpr auto lfoDepth = "lfoDepth";
        static constexpr auto lfoTarget = "lfoTarget";

        static constexpr auto crushBits = "crushBits";
        static constexpr auto crushDownsample = "crushDownsample";
        static constexpr auto driveAmount = "driveAmount";

        static constexpr auto delaySyncDivision = "delaySyncDivision";
        static constexpr auto delayTimeSeconds = "delayTimeSeconds";
        static constexpr auto delayFeedback = "delayFeedback";
        static constexpr auto delayMix = "delayMix";

        static constexpr auto reverbSize = "reverbSize";
        static constexpr auto reverbMix = "reverbMix";

        static constexpr auto chorusRateHz = "chorusRateHz";
        static constexpr auto chorusDepthMs = "chorusDepthMs";
        static constexpr auto chorusMix = "chorusMix";

        static constexpr auto pfMode = "pfMode";
        static constexpr auto pfRateHz = "pfRateHz";
        static constexpr auto pfDepth = "pfDepth";
        static constexpr auto pfFeedback = "pfFeedback";
        static constexpr auto pfMix = "pfMix";

        static constexpr auto saturatorAlgorithm = "saturatorAlgorithm";
        static constexpr auto saturatorAmount = "saturatorAmount";
        static constexpr auto saturatorCeiling = "saturatorCeiling";

        static constexpr auto masterVolume = "masterVolume";
        static constexpr auto masterPan = "masterPan";

        // Per-oscillator suffixes -- combine with oscParamID() below so the
        // same 3-oscillator layout isn't hand-typed three times.
        static constexpr auto oscWaveSuffix = "Wave";
        static constexpr auto oscPulseWidthSuffix = "PulseWidth";
        static constexpr auto oscFoldSuffix = "Fold";
        static constexpr auto oscOctaveSuffix = "Octave";
        static constexpr auto oscSemitoneSuffix = "Semitone";
        static constexpr auto oscFineSuffix = "Fine";
        static constexpr auto oscLevelSuffix = "Level";
        static constexpr auto oscMuteSuffix = "Mute";
    }

    // e.g. oscParamID(1, ParamIDs::oscWaveSuffix) -> "osc1Wave". oscNumber
    // is 1-based (1, 2, 3), matching the reference UI's "Osc 1/2/3" naming.
    juce::String oscParamID(int oscNumber, const juce::String& suffix);

    // Every choice parameter's item order matches its corresponding enum
    // exactly: Waveform (PolyBlepOscillator.h), VoiceMode (SynthEngine.h),
    // ArpPattern (Arpeggiator.h), SyncDivision (TempoSync.h), LfoWaveform /
    // ModTarget (Lfo.h / ModTarget.h), PhaserFlangerMode / SaturatorAlgorithm
    // (Effects/*.h) -- callers read a choice's getIndex() and static_cast
    // it straight to the enum.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
