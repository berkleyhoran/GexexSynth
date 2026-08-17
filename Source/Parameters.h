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
        static constexpr auto filterType = "filterType";
        static constexpr auto filterCutoff = "filterCutoff";
        static constexpr auto filterResonance = "filterResonance";
        static constexpr auto filterVelSens = "filterVelSens";

        // Filter 2 + routing: a second filter, off by default (routing ==
        // Filter1Only), that can be chained in series after filter 1 or
        // run in parallel with it.
        static constexpr auto filterRouting = "filterRouting";
        static constexpr auto filter2Type = "filter2Type";
        static constexpr auto filter2Cutoff = "filter2Cutoff";
        static constexpr auto filter2Resonance = "filter2Resonance";

        static constexpr auto envAttack = "envAttack";
        static constexpr auto envDecay = "envDecay";
        static constexpr auto envSustain = "envSustain";
        static constexpr auto envRelease = "envRelease";

        // A second, independently-routable envelope (same ModTarget list
        // the LFOs use) for filter/pitch/etc. sweeps that need an ADSR
        // shape rather than a cycling LFO. Unlike the amp envelope, it's
        // not per-voice -- see SynthEngine.h's ModEnvelope note.
        static constexpr auto modEnvAttack = "modEnvAttack";
        static constexpr auto modEnvDecay = "modEnvDecay";
        static constexpr auto modEnvSustain = "modEnvSustain";
        static constexpr auto modEnvRelease = "modEnvRelease";
        static constexpr auto modEnvTarget = "modEnvTarget";
        static constexpr auto modEnvDepth = "modEnvDepth";

        static constexpr auto voiceMode = "voiceMode";
        static constexpr auto glideTime = "glideTime";

        static constexpr auto fmAmount2 = "fmAmount2"; // osc2 -> osc1
        static constexpr auto fmAmount3 = "fmAmount3"; // osc3 -> osc1

        static constexpr auto noiseType = "noiseType";
        static constexpr auto noiseLevel = "noiseLevel";
        static constexpr auto noiseMute = "noiseMute";

        // Sub-oscillator: a 4th sound source pitched a selectable octave
        // (or two) below the voice's root note -- reuses the same
        // Waveform enum/choice list as osc1-3 (see createParameterLayout)
        // rather than a smaller bespoke set, so its index still casts
        // straight to gexex::Waveform like everywhere else.
        static constexpr auto subWaveform = "subWaveform";
        static constexpr auto subOctaveDown = "subOctaveDown";
        static constexpr auto subLevel = "subLevel";
        static constexpr auto subMute = "subMute";

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

        // A second, independent LFO -- same shape/rate/depth/target
        // controls as LFO 1, resolved and summed alongside it and the mod
        // envelope (see PluginProcessor::updateEngineParameters' modFor()).
        static constexpr auto lfo2Waveform = "lfo2Waveform";
        static constexpr auto lfo2SyncDivision = "lfo2SyncDivision";
        static constexpr auto lfo2RateHz = "lfo2RateHz";
        static constexpr auto lfo2Depth = "lfo2Depth";
        static constexpr auto lfo2Target = "lfo2Target";

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

        static constexpr auto freqShiftHz = "freqShiftHz";
        static constexpr auto freqShiftMix = "freqShiftMix";

        static constexpr auto mbCrossoverLow = "mbCrossoverLow";
        static constexpr auto mbCrossoverHigh = "mbCrossoverHigh";
        static constexpr auto mbAmount = "mbAmount";
        static constexpr auto mbLowGain = "mbLowGain";
        static constexpr auto mbMidGain = "mbMidGain";
        static constexpr auto mbHighGain = "mbHighGain";
        static constexpr auto mbMix = "mbMix";

        // The insert chain's execution order/inclusion -- 7 slots, each an
        // independent choice of which serial effect (if any) runs there,
        // read in slot order by EffectsChain::process(). Defaults replicate
        // the original fixed Drive->Bitcrush->Chorus->Phaser/Flanger->
        // Saturator order exactly (slots 5/6 default Empty), so existing
        // presets -- which don't reference these params -- still land on
        // the same effect order via initPatch(). Delay/Reverb aren't slot
        // effects: they stay fixed parallel sends, now applied once right
        // after the whole slot chain (see EffectsChain.cpp's comment).
        static constexpr auto fxSlot0 = "fxSlot0";
        static constexpr auto fxSlot1 = "fxSlot1";
        static constexpr auto fxSlot2 = "fxSlot2";
        static constexpr auto fxSlot3 = "fxSlot3";
        static constexpr auto fxSlot4 = "fxSlot4";
        static constexpr auto fxSlot5 = "fxSlot5";
        static constexpr auto fxSlot6 = "fxSlot6";

        static constexpr auto masterVolume = "masterVolume";
        static constexpr auto masterPan = "masterPan";

        // Not a DSP parameter -- controls BackgroundScene's animation. Made
        // a real APVTS bool rather than a plain property (unlike, say, a
        // "current preset name" string) since it's cheap, host-savable,
        // and genuinely worth automating-away-from-default just once per
        // session even if automating it *live* would be silly.
        static constexpr auto reducedMotion = "reducedMotion";

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

    // fxSlotParamID(0..6) -> ParamIDs::fxSlot0..fxSlot6, so EffectsChain and
    // ModuleRack can loop over all 7 slots instead of hand-typing each one.
    static constexpr int numFxSlots = 7;
    const char* fxSlotParamID(int slotIndex) noexcept;

    // Every choice parameter's item order matches its corresponding enum
    // exactly: Waveform (PolyBlepOscillator.h), NoiseType (NoiseGenerator.h),
    // FilterRouting (Voice.h), VoiceMode (SynthEngine.h), ArpPattern
    // (Arpeggiator.h), SyncDivision (TempoSync.h), LfoWaveform / ModTarget
    // (Lfo.h / ModTarget.h), PhaserFlangerMode / SaturatorAlgorithm /
    // FxSlotEffect (Effects/*.h) -- callers read a choice's getIndex() and
    // static_cast it straight to the enum.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
