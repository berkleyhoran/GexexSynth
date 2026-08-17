#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // Tempo-syncable stereo delay (see TempoSync.h for the sync-mode
    // resolution, done by the caller before setTimeSeconds). Outputs the
    // WET delayed signal only, in place -- EffectsChain mixes it back with
    // the dry bus at the configured mix amount (delay is a parallel send,
    // not a serial insert, per the reference's signal-flow comment), so
    // this class doesn't know about "mix" at all.
    class StereoDelay
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            delayLine.prepare(spec);
            delayLine.setMaximumDelayInSamples((int) (spec.sampleRate * 2.0)); // 2s ceiling, plenty for this synth
        }

        void reset() noexcept
        {
            delayLine.reset();
            feedbackState.fill(0.0f);
        }

        void setTimeSeconds(float seconds) noexcept { timeSeconds = juce::jmax(0.001f, seconds); }
        void setFeedback(float newFeedback) noexcept { feedback = juce::jlimit(0.0f, 0.95f, newFeedback); }

        // Processes `block` in place, replacing dry input with 100% wet
        // delayed signal. One DelayLine instance, addressed per channel
        // (0=L, 1=R) -- not one instance per channel.
        void process(juce::dsp::AudioBlock<float>& block, double sampleRate) noexcept
        {
            const float delaySamples = juce::jlimit(1.0f, (float) delayLine.getMaximumDelayInSamples(),
                                                      timeSeconds * (float) sampleRate);
            delayLine.setDelay(delaySamples);

            const auto numCh = juce::jmin((size_t) 2, block.getNumChannels());
            const auto numSamples = block.getNumSamples();

            for (size_t ch = 0; ch < numCh; ++ch)
            {
                auto* data = block.getChannelPointer(ch);
                for (size_t i = 0; i < numSamples; ++i)
                {
                    const float input = data[i] + feedbackState[ch] * feedback;
                    delayLine.pushSample((int) ch, input);
                    const float wet = delayLine.popSample((int) ch);
                    feedbackState[ch] = wet;
                    data[i] = wet;
                }
            }
        }

    private:
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine;
        std::array<float, 2> feedbackState { 0.0f, 0.0f };
        float timeSeconds = 0.28f;
        float feedback = 0.3f;
    };
}
