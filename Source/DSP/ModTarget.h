#pragma once

namespace gexex
{
    // Direct 1:1 port of the browser reference's LFO-target dropdown, plus
    // AutoPan. Choice item order (see Parameters.cpp) matches this enum
    // exactly.
    enum class ModTarget
    {
        Off,
        Pitch,
        FilterCutoff,
        FilterResonance,
        Amp,
        Osc1Level,
        Osc2Level,
        Osc3Level,
        Osc2FmAmount,
        Osc3FmAmount,
        DelayTime,
        DelayFeedback,
        DelayMix,
        ReverbMix,
        BitcrushDepth,
        BitcrushDownsample,
        AutoPan
    };

    // How far a fully-deep (depth=1) LFO swings each target -- co-located
    // with the enum (see the build plan's §2) so adding a target and
    // forgetting its scale is a compile error waiting to happen, not a
    // silent no-op modulation. Units vary per target (semitones, octaves,
    // 0..1 fractions, ...) -- see each target's application site in
    // PluginProcessor::updateEngineParameters/renderVoiceBlock for how the
    // unit is actually used.
    inline float modDepthRange(ModTarget target) noexcept
    {
        switch (target)
        {
            case ModTarget::Pitch:              return 12.0f; // semitones
            case ModTarget::FilterCutoff:       return 4.0f;  // octaves, multiplicative
            case ModTarget::FilterResonance:    return 15.0f; // additive, Q units
            case ModTarget::Amp:                return 1.0f;  // 0..1 tremolo depth
            case ModTarget::Osc1Level:
            case ModTarget::Osc2Level:
            case ModTarget::Osc3Level:          return 0.5f;  // additive, 0..1 level units
            case ModTarget::Osc2FmAmount:
            case ModTarget::Osc3FmAmount:       return 0.5f;  // additive, 0..1 fm units
            case ModTarget::DelayTime:          return 0.3f;  // +-30%, multiplicative
            case ModTarget::DelayFeedback:      return 0.3f;  // additive
            case ModTarget::DelayMix:           return 0.5f;  // additive
            case ModTarget::ReverbMix:          return 0.5f;  // additive
            case ModTarget::BitcrushDepth:      return 8.0f;  // additive, bit units
            case ModTarget::BitcrushDownsample: return 20.0f; // additive, sample units
            case ModTarget::AutoPan:            return 1.0f;  // full pan range
            case ModTarget::Off:
            default:                            return 0.0f;
        }
    }
}
