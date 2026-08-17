#include "Parameters.h"
#include "DSP/TempoSync.h"
#include "DSP/Lfo.h"
#include <cmath>

namespace
{
    // Shared "simple number" display formatter for every continuous
    // (AudioParameterFloat) parameter -- JUCE's default text-from-value
    // formatting shows a fixed number of decimal places derived from the
    // param's range, which reads as noisy/hard-to-parse at a glance
    // ("0.283715 s", "12.4531 Hz"). This only changes what's *displayed*
    // in each knob's readout: the underlying automation value (and every
    // factory/user preset, which stores that same underlying value) keeps
    // its full float precision untouched -- purely a text-formatting
    // layer on top via AudioParameterFloatAttributes::withStringFromValueFunction.
    juce::String formatParamValue(float value, int)
    {
        if (std::abs(value) < 0.0005f)
            value = 0.0f;

        juce::String text = std::abs(value) >= 100.0f  ? juce::String(value, 0)
                             : std::abs(value) >= 10.0f ? juce::String(value, 1)
                                                         : juce::String(value, 2);

        // Trim trailing zeros/dot: "1.20" -> "1.2", "3.00" -> "3".
        if (text.containsChar('.'))
        {
            while (text.endsWithChar('0'))
                text = text.dropLastCharacters(1);
            if (text.endsWithChar('.'))
                text = text.dropLastCharacters(1);
        }
        return text;
    }

    juce::AudioParameterFloatAttributes floatAttrs()
    {
        return juce::AudioParameterFloatAttributes().withStringFromValueFunction(formatParamValue);
    }
} // namespace

juce::String gexex::oscParamID(int oscNumber, const juce::String& suffix)
{
    return "osc" + juce::String(oscNumber) + suffix;
}

const char* gexex::fxSlotParamID(int slotIndex) noexcept
{
    static const char* ids[gexex::numFxSlots] = { gexex::ParamIDs::fxSlot0, gexex::ParamIDs::fxSlot1,
                                                    gexex::ParamIDs::fxSlot2, gexex::ParamIDs::fxSlot3,
                                                    gexex::ParamIDs::fxSlot4, gexex::ParamIDs::fxSlot5,
                                                    gexex::ParamIDs::fxSlot6 };
    jassert(slotIndex >= 0 && slotIndex < gexex::numFxSlots);
    return ids[slotIndex];
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
            NormalisableRange<float>(0.05f, 0.95f), 0.5f, floatAttrs()));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscFoldSuffix), "Osc " + String(oscNumber) + " Fold",
            NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

        params.push_back(std::make_unique<AudioParameterInt>(
            oscParamID(oscNumber, ParamIDs::oscOctaveSuffix), "Osc " + String(oscNumber) + " Octave", -2, 2, 0));

        params.push_back(std::make_unique<AudioParameterInt>(
            oscParamID(oscNumber, ParamIDs::oscSemitoneSuffix), "Osc " + String(oscNumber) + " Semitone", -12, 12,
            0));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscFineSuffix), "Osc " + String(oscNumber) + " Fine",
            NormalisableRange<float>(-100.0f, 100.0f), 0.0f, floatAttrs().withLabel("cents")));

        params.push_back(std::make_unique<AudioParameterFloat>(
            oscParamID(oscNumber, ParamIDs::oscLevelSuffix), "Osc " + String(oscNumber) + " Level",
            NormalisableRange<float>(0.0f, 1.0f), d.level, floatAttrs()));

        params.push_back(std::make_unique<AudioParameterBool>(
            oscParamID(oscNumber, ParamIDs::oscMuteSuffix), "Osc " + String(oscNumber) + " Mute", false));
    }

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::fmAmount2, "FM Osc2->Osc1", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::fmAmount3, "FM Osc3->Osc1", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    // Choice item order must match gexex::NoiseType exactly (White, Pink, Brown).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::noiseType, "Noise Type", StringArray { "White", "Pink", "Brown" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::noiseLevel, "Noise Level", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterBool>(ParamIDs::noiseMute, "Noise Mute", false));

    // Sub-oscillator: same waveformChoices/index convention as osc1-3 (see
    // ParamIDs.h's comment), so this casts straight to gexex::Waveform too.
    params.push_back(
        std::make_unique<AudioParameterChoice>(ParamIDs::subWaveform, "Sub Wave", waveformChoices, 0 /* Sine */));
    params.push_back(std::make_unique<AudioParameterInt>(ParamIDs::subOctaveDown, "Sub Octave", 1, 2, 1));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::subLevel, "Sub Level", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterBool>(ParamIDs::subMute, "Sub Mute", false));

    // Choice item order matches juce::dsp::StateVariableTPTFilterType exactly
    // (Lowpass, Bandpass, Highpass) so the index casts straight across.
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::filterType, "Filter Type", StringArray { "Lowpass", "Bandpass", "Highpass" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterCutoff, "Filter Cutoff", NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f), 2000.0f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterResonance, "Filter Resonance", NormalisableRange<float>(0.1f, 20.0f, 0.0f, 0.35f), 0.707f,
        floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filterVelSens, "Filter Vel->Cutoff", NormalisableRange<float>(0.0f, 1.0f), 0.3f, floatAttrs()));

    // Choice item order must match gexex::FilterRouting exactly (Filter1Only, Series, Parallel).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::filterRouting, "Filter Routing", StringArray { "Filter 1 Only", "Series", "Parallel" }, 0));
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::filter2Type, "Filter 2 Type", StringArray { "Lowpass", "Bandpass", "Highpass" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filter2Cutoff, "Filter 2 Cutoff", NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.25f), 8000.0f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::filter2Resonance, "Filter 2 Resonance", NormalisableRange<float>(0.1f, 20.0f, 0.0f, 0.35f), 0.707f,
        floatAttrs()));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envAttack, "Attack", NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.02f,
        floatAttrs().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envDecay, "Decay", NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.15f,
        floatAttrs().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envSustain, "Sustain", NormalisableRange<float>(0.0f, 1.0f), 0.8f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::envRelease, "Release", NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.3f), 0.3f,
        floatAttrs().withLabel("s")));

    // --- Mod Envelope (a 2nd, independently-routable ADSR -- see the
    // ParamIDs.h comment on why it isn't per-voice like the amp envelope) ---
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::modEnvAttack, "Mod Env Attack", NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.02f,
        floatAttrs().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::modEnvDecay, "Mod Env Decay", NormalisableRange<float>(0.001f, 3.0f, 0.0f, 0.3f), 0.4f,
        floatAttrs().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::modEnvSustain, "Mod Env Sustain", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::modEnvRelease, "Mod Env Release", NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.3f), 0.3f,
        floatAttrs().withLabel("s")));
    // Choice item order must match gexex::ModTarget exactly (ModTarget.h) -- same list LFO 1/2 use.
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::modEnvTarget, "Mod Env Target",
        StringArray { "Off", "Pitch (all osc)", "Filter Cutoff", "Filter Resonance", "Amp", "Osc1 Level",
                       "Osc2 Level", "Osc3 Level", "Osc2 FM Amount", "Osc3 FM Amount", "Delay Time",
                       "Delay Feedback", "Delay Mix", "Reverb Mix", "Bitcrush Depth", "Bitcrush Downsample",
                       "Auto-Pan" },
        0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::modEnvDepth, "Mod Env Depth", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    // Choice item order must match gexex::VoiceMode (Mono, Poly). Defaults
    // to Mono, matching the browser reference's default voice-mode button
    // state.
    params.push_back(
        std::make_unique<AudioParameterChoice>(ParamIDs::voiceMode, "Voice Mode", StringArray { "Mono", "Poly" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::glideTime, "Glide", NormalisableRange<float>(0.0f, 2.0f, 0.0f, 0.3f), 0.0f,
        floatAttrs().withLabel("s")));

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
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::arpGate, "Arp Gate", NormalisableRange<float>(0.1f, 1.0f), 0.7f, floatAttrs()));

    // --- LFO (§2 -- one routable LFO, tempo-syncable) ---
    params.push_back(
        std::make_unique<AudioParameterChoice>(ParamIDs::lfoWaveform, "LFO Wave", Lfo::waveformChoices(), 0));
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::lfoSyncDivision, "LFO Sync", syncDivisionChoices(), 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::lfoRateHz, "LFO Rate", NormalisableRange<float>(0.05f, 20.0f, 0.0f, 0.4f), 4.5f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::lfoDepth, "LFO Depth", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    // Choice item order must match gexex::ModTarget exactly (ModTarget.h).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::lfoTarget, "LFO Target",
        StringArray { "Off", "Pitch (all osc)", "Filter Cutoff", "Filter Resonance", "Amp", "Osc1 Level",
                       "Osc2 Level", "Osc3 Level", "Osc2 FM Amount", "Osc3 FM Amount", "Delay Time",
                       "Delay Feedback", "Delay Mix", "Reverb Mix", "Bitcrush Depth", "Bitcrush Downsample",
                       "Auto-Pan" },
        0));

    // --- LFO 2 (independent of LFO 1, same shape/knob set) ---
    params.push_back(
        std::make_unique<AudioParameterChoice>(ParamIDs::lfo2Waveform, "LFO 2 Wave", Lfo::waveformChoices(), 0));
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::lfo2SyncDivision, "LFO 2 Sync", syncDivisionChoices(), 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::lfo2RateHz, "LFO 2 Rate", NormalisableRange<float>(0.05f, 20.0f, 0.0f, 0.4f), 2.0f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::lfo2Depth, "LFO 2 Depth", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::lfo2Target, "LFO 2 Target",
        StringArray { "Off", "Pitch (all osc)", "Filter Cutoff", "Filter Resonance", "Amp", "Osc1 Level",
                       "Osc2 Level", "Osc3 Level", "Osc2 FM Amount", "Osc3 FM Amount", "Delay Time",
                       "Delay Feedback", "Delay Mix", "Reverb Mix", "Bitcrush Depth", "Bitcrush Downsample",
                       "Auto-Pan" },
        0));

    // --- Effects rack (§3), in signal-flow order ---
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::crushBits, "Bitcrush Depth", NormalisableRange<float>(1.0f, 16.0f), 16.0f,
        floatAttrs().withLabel("bits")));
    params.push_back(
        std::make_unique<AudioParameterInt>(ParamIDs::crushDownsample, "Bitcrush Downsample", 1, 40, 1));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::driveAmount, "Drive", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::delaySyncDivision, "Delay Sync", syncDivisionChoices(), 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::delayTimeSeconds, "Delay Time", NormalisableRange<float>(0.02f, 1.0f, 0.0f, 0.4f), 0.28f,
        floatAttrs().withLabel("s")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::delayFeedback, "Delay Feedback", NormalisableRange<float>(0.0f, 0.85f), 0.3f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::delayMix, "Delay Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::reverbSize, "Reverb Size", NormalisableRange<float>(0.1f, 8.0f, 0.0f, 0.4f), 1.8f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::reverbMix, "Reverb Mix", NormalisableRange<float>(0.0f, 1.0f), 0.12f, floatAttrs()));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::chorusRateHz, "Chorus Rate", NormalisableRange<float>(0.05f, 5.0f, 0.0f, 0.5f), 0.4f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::chorusDepthMs, "Chorus Depth", NormalisableRange<float>(0.0f, 12.0f), 4.0f,
        floatAttrs().withLabel("ms")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::chorusMix, "Chorus Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    // Choice item order must match gexex::PhaserFlangerMode (Phaser, Flanger).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::pfMode, "Phaser/Flanger Mode", StringArray { "Phaser", "Flanger" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfRateHz, "Phaser/Flanger Rate", NormalisableRange<float>(0.02f, 6.0f, 0.0f, 0.4f), 0.3f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfDepth, "Phaser/Flanger Depth", NormalisableRange<float>(0.0f, 1.0f), 0.5f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfFeedback, "Phaser/Flanger Feedback", NormalisableRange<float>(0.0f, 0.85f), 0.3f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::pfMix, "Phaser/Flanger Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    // Choice item order must match gexex::SaturatorAlgorithm (Tanh, Cubic, Arctan, Hard).
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParamIDs::saturatorAlgorithm, "Saturator Algorithm",
        StringArray { "Tanh (warm)", "Cubic (gentle)", "Arctan (smooth)", "Hard (edgy)" }, 0));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::saturatorAmount, "Saturator Amount", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::saturatorCeiling, "Saturator Ceiling", NormalisableRange<float>(0.3f, 1.0f), 1.0f, floatAttrs()));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::freqShiftHz, "Freq Shift", NormalisableRange<float>(-1000.0f, 1000.0f), 0.0f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::freqShiftMix, "Freq Shift Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbCrossoverLow, "MB Crossover Low", NormalisableRange<float>(40.0f, 2000.0f, 0.0f, 0.35f), 200.0f,
        floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbCrossoverHigh, "MB Crossover High", NormalisableRange<float>(500.0f, 12000.0f, 0.0f, 0.3f),
        2500.0f, floatAttrs().withLabel("Hz")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbAmount, "MB Amount", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbLowGain, "MB Low Gain", NormalisableRange<float>(-12.0f, 12.0f), 0.0f,
        floatAttrs().withLabel("dB")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbMidGain, "MB Mid Gain", NormalisableRange<float>(-12.0f, 12.0f), 0.0f,
        floatAttrs().withLabel("dB")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbHighGain, "MB High Gain", NormalisableRange<float>(-12.0f, 12.0f), 0.0f,
        floatAttrs().withLabel("dB")));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::mbMix, "MB Mix", NormalisableRange<float>(0.0f, 1.0f), 0.0f, floatAttrs()));

    // Insert-chain slot assignment -- choice item order must match
    // gexex::FxSlotEffect exactly (Empty, Drive, Bitcrush, Chorus,
    // PhaserFlanger, Saturator, FrequencyShifter, MultibandCompressor).
    // Defaults reproduce the original fixed order across slots 0-4;
    // slots 5/6 default Empty (room to drop in Freq Shift / Multiband
    // Comp without removing anything).
    const StringArray fxSlotChoices { "Empty",         "Drive",     "Bitcrush",          "Chorus",
                                       "Phaser/Flanger","Saturator", "Frequency Shifter", "Multiband Compressor" };
    const int fxSlotDefaults[numFxSlots] = { 1, 2, 3, 4, 5, 0, 0 };
    for (int slot = 0; slot < numFxSlots; ++slot)
        params.push_back(std::make_unique<AudioParameterChoice>(
            fxSlotParamID(slot), "FX Slot " + String(slot + 1), fxSlotChoices, fxSlotDefaults[slot]));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::masterVolume, "Master Volume", NormalisableRange<float>(0.0f, 1.0f), 0.8f, floatAttrs()));
    params.push_back(std::make_unique<AudioParameterFloat>(
        ParamIDs::masterPan, "Master Pan", NormalisableRange<float>(-1.0f, 1.0f), 0.0f, floatAttrs()));

    params.push_back(std::make_unique<AudioParameterBool>(ParamIDs::reducedMotion, "Reduced Motion", false));

    return AudioProcessorValueTreeState::ParameterLayout(params.begin(), params.end());
}
