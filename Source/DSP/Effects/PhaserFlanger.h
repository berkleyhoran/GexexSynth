#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    enum class PhaserFlangerMode
    {
        Phaser,
        Flanger
    };

    // Shared rate/depth/feedback/mix knobs across both modes, matching the
    // reference's "shared engine, mode switch" design intent -- backed by
    // JUCE's own Phaser and Chorus DSP widgets rather than a hand-rolled
    // allpass cascade / modulated delay line, since JUCE's own docs note
    // Chorus doubles as a flanger "with a lower centre delay time and a
    // lot of feedback" (see the build plan's §3 for why this is a
    // reasonable substitution for a hand-written shared engine -- the
    // knobs genuinely drive both underlying processors in lockstep, so it
    // still reads as one shared engine with a mode switch from the
    // outside).
    class PhaserFlanger
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            phaser.prepare(spec);
            flanger.prepare(spec);
            flanger.setCentreDelay(2.5f); // short + high feedback = flanger, not chorus
        }

        void reset() noexcept
        {
            phaser.reset();
            flanger.reset();
        }

        void setMode(PhaserFlangerMode newMode) noexcept { mode = newMode; }

        void setRateHz(float hz) noexcept
        {
            phaser.setRate(juce::jlimit(0.02f, 99.0f, hz));
            flanger.setRate(juce::jlimit(0.02f, 99.0f, hz));
        }

        void setDepth(float depth01) noexcept
        {
            depth01 = juce::jlimit(0.0f, 1.0f, depth01);
            phaser.setDepth(depth01);
            flanger.setDepth(depth01);
        }

        void setFeedback(float feedback01) noexcept
        {
            feedback01 = juce::jlimit(0.0f, 0.95f, feedback01);
            phaser.setFeedback(feedback01);
            flanger.setFeedback(feedback01);
        }

        void setMix(float mix) noexcept
        {
            mix = juce::jlimit(0.0f, 1.0f, mix);
            phaser.setMix(mix);
            flanger.setMix(mix);
        }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            juce::dsp::ProcessContextReplacing<float> ctx(block);
            if (mode == PhaserFlangerMode::Phaser)
                phaser.process(ctx);
            else
                flanger.process(ctx);
        }

    private:
        PhaserFlangerMode mode = PhaserFlangerMode::Phaser;
        juce::dsp::Phaser<float> phaser;
        juce::dsp::Chorus<float> flanger;
    };
}
