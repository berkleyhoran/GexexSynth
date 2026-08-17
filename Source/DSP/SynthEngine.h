#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "Voice.h"
#include "Arpeggiator.h"

namespace gexex
{
    enum class VoiceMode
    {
        Mono,
        Poly
    };

    // Owns the voice pool, mono/poly allocation, and the arpeggiator --
    // everything upstream of the effects chain (which lands in Phase 3).
    // PluginProcessor pushes current parameter values in once per block and
    // routes each incoming MidiMessage through handleMidiEvent(); the audio
    // callback then just sums renderNextSample() across all voices.
    class SynthEngine
    {
    public:
        static constexpr int maxVoices = 16; // see the build plan's Open Decision #3 -- revisit after profiling

        void prepare(const juce::dsp::ProcessSpec& spec);

        void setVoiceMode(VoiceMode mode) noexcept { voiceMode = mode; }
        void setOscillatorSettings(int oscIndex, const OscillatorSettings& settings) noexcept;
        void setFmAmount(int modulatorIndex, float amount) noexcept;
        void setFilterCutoffHz(float hz) noexcept;
        void setFilterResonance(float resonance) noexcept;
        void setEnvelopeParameters(const juce::ADSR::Parameters& params) noexcept;
        void setGlideTimeSeconds(float seconds) noexcept;
        void setPitchModSemitones(float semitones) noexcept;

        void setArpEnabled(bool enabled) noexcept { arp.setEnabled(enabled); }
        void setArpPattern(ArpPattern pattern) noexcept { arp.setPattern(pattern); }
        void setArpOctaveRange(int range) noexcept { arp.setOctaveRange(range); }
        void setArpRateHz(float hz) noexcept { arp.setRateHz(hz); }
        void setArpGate(float gate) noexcept { arp.setGate(gate); }

        void handleMidiEvent(const juce::MidiMessage& message) noexcept;

        float renderNextSample() noexcept;

    private:
        void internalNoteOn(int midiNote, float velocity) noexcept;
        void internalNoteOff(int midiNote) noexcept;
        void allNotesOff() noexcept;
        float getStoredVelocity(int midiNote) const noexcept;

        std::array<Voice, (size_t) maxVoices> voices;
        std::array<int, (size_t) maxVoices> voiceNote; // MIDI note each voice is currently holding, -1 if free
        std::array<int, (size_t) maxVoices> voiceTriggerOrder {}; // higher = triggered more recently
        int triggerCounter = 0;

        std::array<float, 128> noteVelocity {}; // last physical velocity per MIDI note, for arp-generated events

        Arpeggiator arp;
        VoiceMode voiceMode = VoiceMode::Poly;

        // Mono mode: last-note-priority held-note stack (see Voice.h's
        // startNote/glideToNote for the retrigger-vs-legato split this
        // drives). Only voices[0] is used while in Mono mode.
        juce::Array<int> monoNoteStack;
    };
}
