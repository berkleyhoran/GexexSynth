#pragma once

#include <juce_core/juce_core.h>

namespace gexex
{
    enum class NoiseType
    {
        White,
        Pink,
        Brown
    };

    // A cheap per-voice noise source, mixed in alongside the 3 oscillators
    // before the filter -- same treatment a "standard synth" noise
    // generator gets (breath/pluck/hiss layers riding the same envelope
    // and filter as the tonal oscillators). Deliberately per-voice rather
    // than one shared global noise source: sharing one stream across 16
    // voices means every held note carries the exact same noise sample at
    // the same instant, which reads as one noise layer duplicated N times
    // rather than N independent ones -- audibly different (and worse) on
    // a stacked chord. Each instance seeds its own juce::Random off the
    // shared system RNG at construction, so voices don't correlate.
    class NoiseGenerator
    {
    public:
        NoiseGenerator() : random(juce::Random::getSystemRandom().nextInt64()) {}

        float renderSample(NoiseType type) noexcept
        {
            const float white = random.nextFloat() * 2.0f - 1.0f;

            switch (type)
            {
                case NoiseType::Pink:
                {
                    // Paul Kellet's "economy" pink noise filter -- three
                    // leaky integrators summed with the raw white sample,
                    // giving a -3dB/octave roll-off cheaply (no FFT/long
                    // FIR needed).
                    pinkB0 = 0.99886f * pinkB0 + white * 0.0555179f;
                    pinkB1 = 0.99332f * pinkB1 + white * 0.0750759f;
                    pinkB2 = 0.96900f * pinkB2 + white * 0.1538520f;
                    return (pinkB0 + pinkB1 + pinkB2 + white * 0.1848f) * 0.25f;
                }

                case NoiseType::Brown:
                {
                    // Leaky-integrated ("random walk") white noise -- the
                    // 0.996 leak keeps it from drifting off into DC over
                    // time instead of needing a hard clamp, and the 6x
                    // rescale brings its naturally-quiet output back up
                    // toward the same rough loudness as White/Pink at the
                    // same Level knob setting.
                    brownState = (brownState + white * 0.03f) * 0.996f;
                    return brownState * 6.0f;
                }

                case NoiseType::White:
                default:
                    return white;
            }
        }

    private:
        juce::Random random;
        float pinkB0 = 0.0f, pinkB1 = 0.0f, pinkB2 = 0.0f;
        float brownState = 0.0f;
    };
}
