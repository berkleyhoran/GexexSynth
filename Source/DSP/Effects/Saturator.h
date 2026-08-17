#pragma once

#include <cmath>
#include <memory>
#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    enum class SaturatorAlgorithm
    {
        Tanh,
        Cubic,
        Arctan,
        Hard
    };

    // Master-bus saturator: 4 selectable curves, amount (pre-gain into the
    // curve) and output ceiling (post-gain/limit), 2x-oversampled like
    // Drive (see the build plan's §1.5).
    class Saturator
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
                spec.numChannels, 1 /* 2x */, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
            oversampler->initProcessing((size_t) spec.maximumBlockSize);
        }

        void reset() noexcept
        {
            if (oversampler != nullptr)
                oversampler->reset();
        }

        void setAlgorithm(SaturatorAlgorithm newAlgorithm) noexcept { algorithm = newAlgorithm; }
        void setAmount(float newAmount) noexcept { amount = juce::jlimit(0.0f, 1.0f, newAmount); }
        void setCeiling(float newCeiling) noexcept { ceiling = juce::jlimit(0.05f, 1.0f, newCeiling); }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            if (oversampler == nullptr)
                return;

            if (amount > 0.0f)
            {
                // Not gain-compensated after the curve, same reasoning as
                // Drive.h -- the "ceiling" knob just below is the intended
                // way to tame the louder/denser result of pushing amount up.
                const float drive = 1.0f + amount * 9.0f;
                block.multiplyBy(drive);

                auto oversampledBlock = oversampler->processSamplesUp(block);
                const auto numCh = oversampledBlock.getNumChannels();
                const auto numSamples = oversampledBlock.getNumSamples();
                for (size_t ch = 0; ch < numCh; ++ch)
                {
                    auto* data = oversampledBlock.getChannelPointer(ch);
                    for (size_t i = 0; i < numSamples; ++i)
                        data[i] = applyCurve(data[i]);
                }
                oversampler->processSamplesDown(block);
            }

            // Output ceiling: a simple hard limit at +-ceiling, applied
            // whether or not saturation amount is active, matching the
            // reference's separate ceiling control.
            const auto numCh = block.getNumChannels();
            const auto numSamples = block.getNumSamples();
            for (size_t ch = 0; ch < numCh; ++ch)
            {
                auto* data = block.getChannelPointer(ch);
                for (size_t i = 0; i < numSamples; ++i)
                    data[i] = juce::jlimit(-ceiling, ceiling, data[i]);
            }
        }

    private:
        float applyCurve(float x) const noexcept
        {
            switch (algorithm)
            {
                case SaturatorAlgorithm::Tanh: return std::tanh(x);
                case SaturatorAlgorithm::Cubic: return juce::jlimit(-1.0f, 1.0f, x - (x * x * x) / 3.0f);
                case SaturatorAlgorithm::Arctan:
                    return (2.0f / juce::MathConstants<float>::pi) * std::atan(x);
                case SaturatorAlgorithm::Hard: return juce::jlimit(-1.0f, 1.0f, x);
                default: return x;
            }
        }

        std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
        SaturatorAlgorithm algorithm = SaturatorAlgorithm::Tanh;
        float amount = 0.0f;
        float ceiling = 1.0f;
    };
}
