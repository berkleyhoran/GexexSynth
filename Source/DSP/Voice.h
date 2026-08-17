#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "PolyBlepOscillator.h"

namespace gexex
{
    // Phase 1's entire signal path: one PolyBLEP saw oscillator into a TPT
    // state-variable filter, amplitude-shaped by an ADSR envelope. Hard-coded
    // mono (a single Voice instance, owned directly by the processor) -- the
    // voice pool, FM, and the extra two oscillators land in Phase 2. See the
    // build plan for why StateVariableTPT specifically (stays stable under
    // fast cutoff/resonance modulation, which the LFO and velocity-sensitivity
    // both need later).
    class Voice
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            oscillator.setSampleRate(spec.sampleRate);
            filter.prepare(spec);
            filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            envelope.setSampleRate(spec.sampleRate);
        }

        void setFrequencyFromMidiNote(int midiNoteNumber) noexcept
        {
            oscillator.setFrequency((float) juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
        }

        void setFilterCutoffHz(float cutoffHz) noexcept { filter.setCutoffFrequency(cutoffHz); }
        void setFilterResonance(float resonance) noexcept { filter.setResonance(resonance); }
        void setEnvelopeParameters(const juce::ADSR::Parameters& params) noexcept { envelope.setParameters(params); }

        void noteOn() noexcept
        {
            oscillator.resetPhase();
            envelope.noteOn();
        }

        void noteOff() noexcept { envelope.noteOff(); }

        bool isActive() const noexcept { return envelope.isActive(); }

        float renderNextSample() noexcept
        {
            if (! envelope.isActive())
                return 0.0f;

            const float osc = oscillator.renderSaw();
            const float filtered = filter.processSample(0, osc);
            return filtered * envelope.getNextSample();
        }

    private:
        PolyBlepOscillator oscillator;
        juce::dsp::StateVariableTPTFilter<float> filter;
        juce::ADSR envelope;
    };
}
