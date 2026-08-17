#pragma once

#include <juce_core/juce_core.h>

namespace gexex
{
    // Shared by the LFO, delay time, and arp rate (see the build plan's
    // §4): each of those exposes a sync-mode choice alongside a free-Hz/
    // free-seconds float, resolved here against the host's tempo. Both
    // parameters stay independently automatable -- more host-capable than
    // one control trying to encode "free or synced" in a single range.
    enum class SyncDivision
    {
        Free,
        Whole,           // 1/1
        Half,            // 1/2
        Quarter,         // 1/4
        Eighth,          // 1/8
        Sixteenth,       // 1/16
        ThirtySecond,    // 1/32
        EighthTriplet,   // 1/8T
        SixteenthTriplet,// 1/16T
        QuarterDotted    // 1/4.
    };

    // Choice item order matches the enum exactly -- callers read a choice
    // parameter's index and static_cast it straight to SyncDivision.
    inline juce::StringArray syncDivisionChoices()
    {
        return { "Free", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/8T", "1/16T", "1/4." };
    }

    // How many quarter-note beats one cycle of this division spans.
    inline double beatsForDivision(SyncDivision division) noexcept
    {
        switch (division)
        {
            case SyncDivision::Whole:             return 4.0;
            case SyncDivision::Half:              return 2.0;
            case SyncDivision::Quarter:            return 1.0;
            case SyncDivision::Eighth:             return 0.5;
            case SyncDivision::Sixteenth:          return 0.25;
            case SyncDivision::ThirtySecond:       return 0.125;
            case SyncDivision::EighthTriplet:      return 1.0 / 3.0;
            case SyncDivision::SixteenthTriplet:   return 1.0 / 6.0;
            case SyncDivision::QuarterDotted:      return 1.5;
            case SyncDivision::Free:
            default:                               return 0.0;
        }
    }

    // Free (division == Free) just returns freeHz unchanged; otherwise
    // resolves the division's beat length against bpm. Falls back to
    // freeHz if bpm is somehow non-positive (shouldn't happen -- the
    // caller already falls back to 120 BPM when there's no host playhead).
    inline float resolveRateHz(SyncDivision division, float freeHz, double bpm) noexcept
    {
        if (division == SyncDivision::Free)
            return freeHz;
        const double beats = beatsForDivision(division);
        if (beats <= 0.0 || bpm <= 0.0)
            return freeHz;
        const double secondsPerCycle = beats * (60.0 / bpm);
        return secondsPerCycle > 0.0 ? (float) (1.0 / secondsPerCycle) : freeHz;
    }

    // Mirror of resolveRateHz for controls that want a time (delay) rather
    // than a rate (LFO/arp).
    inline float resolveTimeSeconds(SyncDivision division, float freeSeconds, double bpm) noexcept
    {
        if (division == SyncDivision::Free)
            return freeSeconds;
        const double beats = beatsForDivision(division);
        if (beats <= 0.0 || bpm <= 0.0)
            return freeSeconds;
        return (float) (beats * (60.0 / bpm));
    }
}
