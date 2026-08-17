#pragma once

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>

namespace gexex
{
    // Direct native port of the browser reference's AudioWorkletProcessor
    // logic -- bit-depth quantization plus sample-and-hold decimation.
    // Deliberately per-sample and *not* oversampled (see the build plan's
    // §1.5): the aliasing this produces is the intended lo-fi artifact,
    // not a mistake to correct.
    class Bitcrusher
    {
    public:
        void reset() noexcept
        {
            heldSample.fill(0.0f);
            samplesSinceHold.fill(0);
        }

        void setBitDepth(float bits) noexcept { bitDepth = juce::jlimit(1.0f, 16.0f, bits); }
        void setDownsample(int factor) noexcept { downsampleFactor = juce::jmax(1, factor); }

        void processBlock(juce::AudioBuffer<float>& buffer, int numSamples) noexcept
        {
            const float levels = std::pow(2.0f, bitDepth - 1.0f);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* data = buffer.getWritePointer(ch);
                for (int i = 0; i < numSamples; ++i)
                {
                    if (samplesSinceHold[(size_t) ch] <= 0)
                    {
                        const float quantized = std::round(data[i] * levels) / levels;
                        heldSample[(size_t) ch] = quantized;
                        samplesSinceHold[(size_t) ch] = downsampleFactor;
                    }
                    data[i] = heldSample[(size_t) ch];
                    --samplesSinceHold[(size_t) ch];
                }
            }
        }

    private:
        float bitDepth = 16.0f;
        int downsampleFactor = 1;
        std::array<float, 2> heldSample { 0.0f, 0.0f };
        std::array<int, 2> samplesSinceHold { 0, 0 };
    };
}
