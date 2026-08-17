#pragma once

#include <juce_core/juce_core.h>

namespace gexex
{
    enum class ArpPattern
    {
        Up,
        Down,
        UpDown,
        Random
    };

    // Sits above voice allocation as a note generator (see the build plan's
    // §1.6): while enabled and one or more notes are held, it synthesizes
    // its own noteOn/noteOff stream at the arp rate/pattern/octave-range/
    // gate-length instead of passing held notes straight through to voice
    // allocation. SynthEngine feeds it held-note changes and polls it once
    // per sample via advanceSample().
    //
    // Free-running Hz rate for now -- Phase 3 adds the shared TempoSync
    // dual-mode helper (free Hz vs. host-BPM-synced division) used by the
    // LFO, delay, and this rate together, per the build plan.
    class Arpeggiator
    {
    public:
        void setSampleRate(double newSampleRate) noexcept;
        void setEnabled(bool shouldBeEnabled) noexcept;
        bool isEnabled() const noexcept { return enabled; }

        void setPattern(ArpPattern newPattern) noexcept;
        void setOctaveRange(int newRange) noexcept;
        void setRateHz(float hz) noexcept;
        void setGate(float newGate) noexcept;

        void noteHeld(int midiNote) noexcept;
        void noteReleased(int midiNote) noexcept;
        bool hasHeldNotes() const noexcept { return ! heldNotes.isEmpty(); }

        // All-notes-off: clears held notes without touching the enabled
        // flag (unlike setEnabled(false), which would also do this as a
        // side effect -- this is for MIDI panic messages, not the user
        // toggling the arp off).
        void clearHeldNotes() noexcept { reset(); }

        // Advances the clock by one sample; calls onNoteOn(note)/
        // onNoteOff(note) exactly on the sample a step boundary or gate-
        // close falls on. Templated on the callbacks so this stays
        // allocation-free on the audio thread (no std::function).
        template <typename NoteOnFn, typename NoteOffFn>
        void advanceSample(NoteOnFn&& onNoteOn, NoteOffFn&& onNoteOff) noexcept
        {
            if (! enabled || sequence.isEmpty())
                return;

            if (noteIsSounding && samplesIntoStep >= gateSamples)
            {
                onNoteOff(currentPlayingNote);
                noteIsSounding = false;
            }

            if (samplesIntoStep >= stepLengthSamples)
            {
                samplesIntoStep = 0.0;
                advanceStepIndex();
                if (juce::isPositiveAndBelow(stepIndex, sequence.size()))
                {
                    currentPlayingNote = sequence.getUnchecked(stepIndex);
                    onNoteOn(currentPlayingNote);
                    noteIsSounding = true;
                }
            }

            samplesIntoStep += 1.0;
        }

    private:
        void reset() noexcept;
        void rebuildSequence() noexcept;
        void recalculateStepLength() noexcept;
        void advanceStepIndex() noexcept;

        bool enabled = false;
        ArpPattern pattern = ArpPattern::Up;
        int octaveRange = 1;
        float rateHz = 8.0f;
        float gate = 0.7f;

        juce::Array<int> heldNotes;
        juce::Array<int> upSequence;
        juce::Array<int> sequence;
        int stepIndex = -1;

        double sampleRate = 44100.0;
        double stepLengthSamples = 44100.0 / 8.0;
        double gateSamples = 0.0;
        double samplesIntoStep = 0.0;
        bool noteIsSounding = false;
        int currentPlayingNote = -1;

        juce::Random random;
    };
}
