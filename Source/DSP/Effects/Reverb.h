#pragma once

#include <array>
#include <vector>
#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // Algorithmic 4-line FDN (feedback delay network) reverb -- a
    // deliberate deviation from the browser reference's technique
    // (convolution against a runtime-synthesized noise impulse), for
    // real-time-CPU reasons: see the build plan's §3. Continuous, cheap,
    // size-independent control over the same two knobs (size, mix) the
    // reference exposes, with no IR-buffer reallocation on parameter
    // change.
    //
    // Householder feedback mix (a reflection: each line's output combines
    // with -2/N * the sum of all lines -- cheap, N additions, no explicit
    // matrix multiply) plus a one-pole damping lowpass per line in the
    // feedback path, so the tail loses high frequencies as it decays like
    // a real space. 100% wet, in place -- EffectsChain mixes it back with
    // the dry bus at the configured mix amount (reverb is a parallel send,
    // not a serial insert).
    class Reverb
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            sampleRate = spec.sampleRate;
            for (int l = 0; l < numLines; ++l)
            {
                const int maxLen = (int) (baseDelayMs[(size_t) l] * 0.001 * sampleRate * maxSizeScale) + 16;
                buffers[(size_t) l].assign((size_t) maxLen, 0.0f);
                writePos[(size_t) l] = 0;
                dampState[(size_t) l] = 0.0f;
            }
            updateDerivedParameters();
        }

        void reset() noexcept
        {
            for (auto& b : buffers)
                std::fill(b.begin(), b.end(), 0.0f);
            dampState.fill(0.0f);
        }

        void setSize(float newSize) noexcept
        {
            size = juce::jlimit(0.1f, 8.0f, newSize);
            updateDerivedParameters();
        }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            const auto numCh = juce::jmin((size_t) 2, block.getNumChannels());
            const auto numSamples = block.getNumSamples();
            auto* left = block.getChannelPointer(0);
            auto* right = numCh > 1 ? block.getChannelPointer(1) : nullptr;

            for (size_t i = 0; i < numSamples; ++i)
            {
                float lineOut[numLines];
                for (int l = 0; l < numLines; ++l)
                    lineOut[l] = buffers[(size_t) l][(size_t) writePos[(size_t) l]];

                float sum = 0.0f;
                for (int l = 0; l < numLines; ++l)
                    sum += lineOut[l];
                const float sumScaled = sum * (2.0f / (float) numLines);

                const float inputSample = 0.5f * (left[i] + (right != nullptr ? right[i] : left[i]));

                for (int l = 0; l < numLines; ++l)
                {
                    const float fedBack = (lineOut[l] - sumScaled) * feedbackGain;
                    auto& damp = dampState[(size_t) l];
                    damp = damp * damping + fedBack * (1.0f - damping);
                    buffers[(size_t) l][(size_t) writePos[(size_t) l]] = inputSample + damp;
                    writePos[(size_t) l] = (writePos[(size_t) l] + 1) % lineLengths[(size_t) l];
                }

                // Pair lines into L/R for a bit of stereo spread rather
                // than summing all 4 identically into both channels.
                left[i] = (lineOut[0] + lineOut[2]) * 0.5f;
                if (right != nullptr)
                    right[i] = (lineOut[1] + lineOut[3]) * 0.5f;
            }
        }

    private:
        void updateDerivedParameters() noexcept
        {
            const float sizeScale = juce::jmap(size, 0.1f, 8.0f, 0.5f, 2.0f);
            feedbackGain = juce::jmap(size, 0.1f, 8.0f, 0.6f, 0.97f); // stays < 1: stable by construction
            for (int l = 0; l < numLines; ++l)
            {
                int len = (int) ((double) baseDelayMs[(size_t) l] * 0.001 * sampleRate * (double) sizeScale);
                len = juce::jlimit(8, (int) buffers[(size_t) l].size() - 1, len);
                lineLengths[(size_t) l] = len;
                writePos[(size_t) l] = juce::jmin(writePos[(size_t) l], len - 1);
            }
        }

        static constexpr int numLines = 4;
        // Mutually-incommensurate delay lengths (ms) -- avoids the metallic
        // ringing that rationally-related delay times produce.
        static constexpr std::array<float, 4> baseDelayMs { 29.7f, 37.1f, 41.3f, 43.7f };
        static constexpr float maxSizeScale = 2.0f;
        static constexpr float damping = 0.25f;

        double sampleRate = 44100.0;
        float size = 1.8f;
        float feedbackGain = 0.8f;

        std::array<std::vector<float>, (size_t) numLines> buffers;
        std::array<int, (size_t) numLines> lineLengths {};
        std::array<int, (size_t) numLines> writePos {};
        std::array<float, (size_t) numLines> dampState {};
    };
}
