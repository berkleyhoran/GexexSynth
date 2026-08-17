#include "Arpeggiator.h"

namespace gexex
{
    void Arpeggiator::setSampleRate(double newSampleRate) noexcept
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : sampleRate;
        recalculateStepLength();
    }

    void Arpeggiator::setEnabled(bool shouldBeEnabled) noexcept
    {
        if (enabled == shouldBeEnabled)
            return;
        enabled = shouldBeEnabled;
        if (! enabled)
            reset();
    }

    void Arpeggiator::setPattern(ArpPattern newPattern) noexcept
    {
        pattern = newPattern;
        rebuildSequence();
    }

    void Arpeggiator::setOctaveRange(int newRange) noexcept
    {
        octaveRange = juce::jlimit(1, 4, newRange);
        rebuildSequence();
    }

    void Arpeggiator::setRateHz(float hz) noexcept
    {
        rateHz = juce::jmax(0.05f, hz);
        recalculateStepLength();
    }

    void Arpeggiator::setGate(float newGate) noexcept
    {
        gate = juce::jlimit(0.05f, 1.0f, newGate);
        gateSamples = stepLengthSamples * (double) gate;
    }

    void Arpeggiator::noteHeld(int midiNote) noexcept
    {
        const bool wasEmpty = heldNotes.isEmpty();
        if (! heldNotes.contains(midiNote))
            heldNotes.add(midiNote);
        rebuildSequence();

        // Starting a chord from silence should sound the first arp step
        // immediately, not after waiting out a full step length.
        if (wasEmpty)
        {
            samplesIntoStep = stepLengthSamples;
            stepIndex = -1;
        }
    }

    void Arpeggiator::noteReleased(int midiNote) noexcept
    {
        heldNotes.removeAllInstancesOf(midiNote);
        rebuildSequence();
    }

    void Arpeggiator::reset() noexcept
    {
        heldNotes.clearQuick();
        upSequence.clearQuick();
        sequence.clearQuick();
        stepIndex = -1;
        samplesIntoStep = 0.0;
        noteIsSounding = false;
        currentPlayingNote = -1;
    }

    void Arpeggiator::rebuildSequence() noexcept
    {
        upSequence.clearQuick();

        if (heldNotes.isEmpty())
        {
            sequence.clearQuick();
            stepIndex = -1;
            return;
        }

        juce::Array<int> sorted = heldNotes;
        sorted.sort();

        for (int oct = 0; oct < octaveRange; ++oct)
            for (auto note : sorted)
                upSequence.add(note + 12 * oct);

        switch (pattern)
        {
            case ArpPattern::Down:
                sequence.clearQuick();
                for (int i = upSequence.size() - 1; i >= 0; --i)
                    sequence.add(upSequence.getUnchecked(i));
                break;

            case ArpPattern::UpDown:
                sequence = upSequence;
                // Skip the two endpoints on the way back down so they don't
                // double-trigger at the turnaround (1 2 3 4 3 2, not
                // 1 2 3 4 4 3 2 1).
                for (int i = upSequence.size() - 2; i >= 1; --i)
                    sequence.add(upSequence.getUnchecked(i));
                break;

            case ArpPattern::Up:
            case ArpPattern::Random:
            default:
                sequence = upSequence;
                break;
        }

        stepIndex = juce::jlimit(-1, sequence.size() - 1, stepIndex);
    }

    void Arpeggiator::recalculateStepLength() noexcept
    {
        stepLengthSamples = sampleRate / (double) rateHz;
        gateSamples = stepLengthSamples * (double) gate;
    }

    void Arpeggiator::advanceStepIndex() noexcept
    {
        if (sequence.isEmpty())
        {
            stepIndex = -1;
            return;
        }

        if (pattern == ArpPattern::Random)
            stepIndex = random.nextInt(sequence.size());
        else
            stepIndex = (stepIndex + 1) % sequence.size();
    }
}
