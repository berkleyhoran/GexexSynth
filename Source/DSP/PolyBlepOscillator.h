#pragma once

namespace gexex
{
    // Band-limited (PolyBLEP-corrected) oscillator core -- see the build
    // plan's rationale for choosing PolyBLEP over juce::dsp::Oscillator or a
    // literal port of the browser reference's native OscillatorNode: JUCE
    // has no built-in band-limiting, and this synth's range (octave +-2,
    // semitone +-12, FM pushing effective rates higher) will alias badly
    // without it.
    //
    // Phase 1 only wires up renderSaw() (the mono voice's single
    // oscillator); Phase 2 adds square/pulse (two BLEP-corrected edges
    // instead of one) and sine/triangle (no correction needed) alongside
    // FM phase modulation and wavefolding.
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

        // Naive rising sawtooth (2*phase - 1) with a PolyBLEP correction
        // subtracted at the phase-wrap discontinuity -- the classic
        // Valimaki/Huovilainen two-piece polynomial correction. Returns a
        // sample in roughly [-1, 1] and advances the phase.
        float renderSaw() noexcept
        {
            float value = 2.0f * phase - 1.0f;
            value -= polyBlep(phase, phaseIncrement);
            advancePhase();
            return value;
        }

    private:
        void updatePhaseIncrement() noexcept
        {
            phaseIncrement = sampleRate > 0.0 ? (float) (frequencyHz / sampleRate) : 0.0f;
        }

        void advancePhase() noexcept
        {
            phase += phaseIncrement;
            if (phase >= 1.0f)
                phase -= 1.0f;
            else if (phase < 0.0f)
                phase += 1.0f;
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

        double sampleRate = 44100.0;
        float frequencyHz = 440.0f;
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
    };
}
