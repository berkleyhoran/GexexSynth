#pragma once

#include <cmath>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // Volume + constant-power pan, the final stage before the effects
    // chain's output. Excluded from "randomize" (see the build plan's §5),
    // matching the reference.
    class MasterBus
    {
    public:
        void setVolume(float newVolume) noexcept { volume = juce::jlimit(0.0f, 1.0f, newVolume); }
        void setPan(float newPan) noexcept { pan = juce::jlimit(-1.0f, 1.0f, newPan); }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            if (block.getNumChannels() < 2)
            {
                block.multiplyBy(volume);
                return;
            }

            // Constant-power (equal-power) pan law: -3dB centre, full
            // level at the extremes, so panning doesn't dip in perceived
            // loudness as it sweeps.
            const float panAngle = (pan * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;
            const float leftGain = volume * std::cos(panAngle);
            const float rightGain = volume * std::sin(panAngle);

            auto* left = block.getChannelPointer(0);
            auto* right = block.getChannelPointer(1);
            const auto numSamples = block.getNumSamples();
            for (size_t i = 0; i < numSamples; ++i)
            {
                left[i] *= leftGain;
                right[i] *= rightGain;
            }
        }

    private:
        float volume = 0.8f;
        float pan = 0.0f;
    };
}
