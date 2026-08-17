#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // Thin wrapper around juce::dsp::Chorus -- its rate/depth/mix map
    // directly onto the reference's Chorus knobs, so there's no reason to
    // hand-roll a modulated-delay-line chorus from scratch (see the build
    // plan's §3).
    class Chorus
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec) { chorus.prepare(spec); }
        void reset() noexcept { chorus.reset(); }

        void setRateHz(float hz) noexcept { chorus.setRate(juce::jlimit(0.02f, 99.0f, hz)); }

        void setDepthMs(float ms) noexcept
        {
            // The reference's "depth ms" (0-12) drives both Chorus's 0..1
            // depth *and* widens the centre delay a touch with it, so a
            // deeper setting sweeps a visibly wider delay range instead of
            // just a louder LFO over a fixed-width sweep.
            chorus.setDepth(juce::jlimit(0.0f, 1.0f, ms / 12.0f));
            chorus.setCentreDelay(juce::jlimit(1.0f, 40.0f, 7.0f + ms));
        }

        void setMix(float mix) noexcept { chorus.setMix(juce::jlimit(0.0f, 1.0f, mix)); }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            juce::dsp::ProcessContextReplacing<float> ctx(block);
            chorus.process(ctx);
        }

    private:
        juce::dsp::Chorus<float> chorus;
    };
}
