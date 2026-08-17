#pragma once

#include <cmath>
#include <juce_core/juce_core.h>
#include "TempoSync.h"
#include "ModulationSource.h"

namespace gexex
{
    enum class LfoWaveform
    {
        Sine,
        Square,
        Saw,
        Triangle
    };

    // One routable LFO (see the build plan's §2). LFO rates are always
    // sub-audio, so unlike PolyBlepOscillator this is evaluated once per
    // *block* (control-rate) rather than per-sample -- no band-limiting
    // needed, plain closed-form waveforms are fine and cheaper.
    class Lfo : public ModulationSource
    {
    public:
        void setSampleRate(double newSampleRate) noexcept { sampleRate = newSampleRate; }
        void setWaveform(LfoWaveform newWaveform) noexcept { waveform = newWaveform; }
        void setSyncDivision(SyncDivision newDivision) noexcept { division = newDivision; }
        void setFreeRateHz(float hz) noexcept { freeHz = hz; }
        void setHostBpm(double bpm) noexcept { hostBpm = bpm; }

        // Advances the LFO's phase by however many samples this block
        // covers and returns the new current value (also retrievable via
        // getCurrentValue()).
        float advanceBlock(int numSamples) noexcept
        {
            const float rateHz = resolveRateHz(division, freeHz, hostBpm);
            if (sampleRate > 0.0)
                phase += (float) ((double) rateHz * (double) numSamples / sampleRate);
            phase -= std::floor(phase);
            currentValue = renderWaveform(waveform, phase);
            return currentValue;
        }

        float getCurrentValue() const noexcept override { return currentValue; }

        static juce::StringArray waveformChoices() { return { "Sine", "Square", "Saw", "Triangle" }; }

    private:
        static float renderWaveform(LfoWaveform w, float t) noexcept
        {
            switch (w)
            {
                case LfoWaveform::Sine: return std::sin(juce::MathConstants<float>::twoPi * t);
                case LfoWaveform::Square: return t < 0.5f ? 1.0f : -1.0f;
                case LfoWaveform::Saw: return 2.0f * t - 1.0f;
                case LfoWaveform::Triangle: return -(4.0f * std::abs(t - 0.5f) - 1.0f);
                default: return 0.0f;
            }
        }

        LfoWaveform waveform = LfoWaveform::Sine;
        SyncDivision division = SyncDivision::Free;
        float freeHz = 4.5f;
        double hostBpm = 120.0;
        double sampleRate = 44100.0;
        float phase = 0.0f;
        float currentValue = 0.0f;
    };
}
