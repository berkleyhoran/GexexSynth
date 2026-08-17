#pragma once

#include <cmath>
#include <juce_core/juce_core.h>

namespace gexex
{
    enum class Waveform
    {
        Sine,
        Triangle,
        Saw,
        Square,
        Pulse
    };

    // Band-limited (PolyBLEP-corrected) oscillator core -- see the build
    // plan's rationale for choosing PolyBLEP over juce::dsp::Oscillator or a
    // literal port of the browser reference's native OscillatorNode: JUCE
    // has no built-in band-limiting, and this synth's range (octave +-2,
    // semitone +-12, FM pushing effective rates higher) will alias badly
    // without it. Only Saw/Square/Pulse route through the BLEP correction;
    // Sine has no discontinuity and Triangle's is in the derivative, not the
    // signal, so both are already low-alias as plain closed-form generators.
    class PolyBlepOscillator
    {
    public:
        void setSampleRate(double newSampleRate) noexcept
        {
            sampleRate = newSampleRate > 0.0 ? newSampleRate : sampleRate;
            updatePhaseIncrement();
        }

        void setFrequency(float newFrequencyHz) noexcept
        {
            frequencyHz = newFrequencyHz;
            updatePhaseIncrement();
        }

        void resetPhase(float newPhase = 0.0f) noexcept { phase = newPhase; }

        // phaseModulationCycles: an additional phase offset in cycles
        // (fractions of a full 0..1 cycle), applied only to *this* sample's
        // waveform lookup -- the internal phase accumulator keeps advancing
        // at its own frequency-derived rate regardless. This is how FM
        // (osc2/osc3 -> osc1) is implemented: the modulator's raw output
        // sample, scaled, is passed in here every sample (see Voice.h).
        float renderSample(Waveform waveform, float pulseWidth, float phaseModulationCycles = 0.0f) noexcept
        {
            const float t = wrap01(phase + phaseModulationCycles);
            const float value = renderAtPhase(waveform, pulseWidth, t);
            advancePhase();
            return value;
        }

        // Algorithmic wavefold: drives the signal harder as `amount`
        // increases, then reflects anything outside [-1, 1] back in like a
        // mirror (a cheap, self-similar recursive fold -- no lookup table
        // needed, unlike the browser reference's WaveShaper curve). Applied
        // by the caller *after* renderSample, since folding is a shared
        // post-process independent of which waveform generated the input.
        static float waveFold(float x, float amount) noexcept
        {
            if (amount <= 0.0f)
                return x;

            float y = x * (1.0f + amount * 4.0f);

            // amount is a 0..1 UI range, so this rarely needs more than a
            // couple of reflections; the iteration cap just guards against
            // runaway input.
            for (int i = 0; i < 8 && (y > 1.0f || y < -1.0f); ++i)
            {
                if (y > 1.0f)
                    y = 2.0f - y;
                else if (y < -1.0f)
                    y = -2.0f - y;
            }
            return y;
        }

    private:
        void updatePhaseIncrement() noexcept
        {
            phaseIncrement = sampleRate > 0.0 ? (float) (frequencyHz / sampleRate) : 0.0f;
        }

        void advancePhase() noexcept { phase = wrap01(phase + phaseIncrement); }

        static float wrap01(float x) noexcept
        {
            x = std::fmod(x, 1.0f);
            return x < 0.0f ? x + 1.0f : x;
        }

        static float polyBlep(float t, float dt) noexcept
        {
            if (dt <= 0.0f)
                return 0.0f;

            if (t < dt)
            {
                t /= dt;
                return t + t - t * t - 1.0f;
            }
            if (t > 1.0f - dt)
            {
                t = (t - 1.0f) / dt;
                return t * t + t + t + 1.0f;
            }
            return 0.0f;
        }

        // Shared by Square (fixed 50% duty) and Pulse (variable duty): the
        // build plan's PWM formulation -- two independently band-limited
        // edges (one at t=0, one at t=width) -- rather than shaping a fixed
        // square with a static WaveShaper curve, which smears when the
        // width knob is swept live.
        static float renderPulse(float t, float dt, float width) noexcept
        {
            float value = t < width ? 1.0f : -1.0f;
            value += polyBlep(t, dt);
            value -= polyBlep(wrap01(t - width), dt);
            return value;
        }

        float renderAtPhase(Waveform waveform, float pulseWidth, float t) const noexcept
        {
            switch (waveform)
            {
                case Waveform::Sine:
                    return std::sin(juce::MathConstants<float>::twoPi * t);

                case Waveform::Triangle:
                    // No BLEP correction needed -- the discontinuity is in
                    // the derivative, not the signal, so it's already low-
                    // alias as a plain closed-form generator.
                    return -(4.0f * std::abs(t - 0.5f) - 1.0f);

                case Waveform::Saw:
                {
                    float value = 2.0f * t - 1.0f;
                    value -= polyBlep(t, phaseIncrement);
                    return value;
                }

                case Waveform::Square:
                    return renderPulse(t, phaseIncrement, 0.5f);

                case Waveform::Pulse:
                default:
                    return renderPulse(t, phaseIncrement, pulseWidth);
            }
        }

        double sampleRate = 44100.0;
        float frequencyHz = 440.0f;
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
    };
}
