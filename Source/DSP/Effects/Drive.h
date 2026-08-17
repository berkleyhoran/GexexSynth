#pragma once

#include <cmath>
#include <memory>
#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // A single tanh WaveShaper, 2x-oversampled (see the build plan's §1.5:
    // nonlinear waveshaping stages get scoped oversampling to control
    // aliasing cheaply -- linear stages like the filter/delay/reverb don't
    // need it).
    class Drive
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
                spec.numChannels, 1 /* 2x */, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
            oversampler->initProcessing((size_t) spec.maximumBlockSize);
            shaper.functionToUse = [](float x) { return std::tanh(x); };
        }

        void reset() noexcept
        {
            if (oversampler != nullptr)
                oversampler->reset();
        }

        void setAmount(float newAmount) noexcept { amount = juce::jlimit(0.0f, 1.0f, newAmount); }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            if (amount <= 0.0f || oversampler == nullptr)
                return;

            // Drive harder into the shaper as amount increases. Not gain-
            // compensated afterwards on purpose: tanh saturates toward
            // +-1 regardless of how hard it's driven, so fully dividing
            // back out by driveGain (an earlier version of this code did)
            // crushed the output far below the dry level instead of
            // making it sound driven -- real analog-style drive circuits
            // (and the browser reference's single "amount" knob) get
            // louder/denser as you push them harder, which is the whole
            // point; taming that back down is the ceiling/ master-volume
            // knobs' job, not this stage's.
            const float driveGain = 1.0f + amount * 9.0f;
            block.multiplyBy(driveGain);

            auto oversampledBlock = oversampler->processSamplesUp(block);
            juce::dsp::ProcessContextReplacing<float> ctx(oversampledBlock);
            shaper.process(ctx);
            oversampler->processSamplesDown(block);
        }

    private:
        std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
        juce::dsp::WaveShaper<float> shaper;
        float amount = 0.0f;
    };
}
