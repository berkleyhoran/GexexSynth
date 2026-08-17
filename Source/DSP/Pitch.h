#pragma once

#include <cmath>

namespace gexex
{
    // MIDI note number (can be fractional -- glide/portamento needs that) to
    // Hz, standard equal-temperament A440 formula. Used instead of
    // juce::MidiMessage::getMidiNoteInHertz (which only takes an int) since
    // each oscillator computes its own frequency from a continuously-glided
    // root note plus its own octave/semitone/fine offset.
    inline float noteNumberToHz(float noteNumber, float referenceHz = 440.0f) noexcept
    {
        return referenceHz * std::pow(2.0f, (noteNumber - 69.0f) / 12.0f);
    }
}
