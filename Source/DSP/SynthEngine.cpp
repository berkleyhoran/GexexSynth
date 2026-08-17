#include "SynthEngine.h"

namespace gexex
{
    void SynthEngine::prepare(const juce::dsp::ProcessSpec& spec)
    {
        for (auto& v : voices)
            v.prepare(spec);
        voiceNote.fill(-1);
        arp.setSampleRate(spec.sampleRate);
    }

    void SynthEngine::setOscillatorSettings(int oscIndex, const OscillatorSettings& settings) noexcept
    {
        for (auto& v : voices)
            v.setOscillatorSettings(oscIndex, settings);
    }

    void SynthEngine::setNoiseSettings(const NoiseSettings& settings) noexcept
    {
        for (auto& v : voices)
            v.setNoiseSettings(settings);
    }

    void SynthEngine::setFmAmount(int modulatorIndex, float amount) noexcept
    {
        for (auto& v : voices)
            v.setFmAmount(modulatorIndex, amount);
    }

    void SynthEngine::setFilterCutoffHz(float hz) noexcept
    {
        for (auto& v : voices)
            v.setFilterCutoffHz(hz);
    }

    void SynthEngine::setFilterResonance(float resonance) noexcept
    {
        for (auto& v : voices)
            v.setFilterResonance(resonance);
    }

    void SynthEngine::setFilterType(juce::dsp::StateVariableTPTFilterType type) noexcept
    {
        for (auto& v : voices)
            v.setFilterType(type);
    }

    void SynthEngine::setFilterVelocitySensitivity(float sensitivity) noexcept
    {
        for (auto& v : voices)
            v.setFilterVelocitySensitivity(sensitivity);
    }

    void SynthEngine::setEnvelopeParameters(const juce::ADSR::Parameters& params) noexcept
    {
        for (auto& v : voices)
            v.setEnvelopeParameters(params);
    }

    void SynthEngine::setGlideTimeSeconds(float seconds) noexcept
    {
        for (auto& v : voices)
            v.setGlideTimeSeconds(seconds);
    }

    void SynthEngine::setPitchModSemitones(float semitones) noexcept
    {
        for (auto& v : voices)
            v.setPitchModSemitones(semitones);
    }

    float SynthEngine::getStoredVelocity(int midiNote) const noexcept
    {
        return noteVelocity[(size_t) juce::jlimit(0, 127, midiNote)];
    }

    void SynthEngine::handleMidiEvent(const juce::MidiMessage& message) noexcept
    {
        if (message.isNoteOn())
        {
            const int note = message.getNoteNumber();
            noteVelocity[(size_t) juce::jlimit(0, 127, note)] = message.getFloatVelocity();

            if (arp.isEnabled())
                arp.noteHeld(note);
            else
                internalNoteOn(note, message.getFloatVelocity());
        }
        else if (message.isNoteOff())
        {
            const int note = message.getNoteNumber();
            if (arp.isEnabled())
                arp.noteReleased(note);
            else
                internalNoteOff(note);
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            allNotesOff();
        }
    }

    void SynthEngine::internalNoteOn(int midiNote, float velocity) noexcept
    {
        if (voiceMode == VoiceMode::Mono)
        {
            const bool wasEmpty = monoNoteStack.isEmpty();
            monoNoteStack.removeAllInstancesOf(midiNote); // re-pressing a held note moves it to the top
            monoNoteStack.add(midiNote);

            if (wasEmpty)
                voices[0].startNote(midiNote, velocity);
            else
                voices[0].glideToNote(midiNote, velocity); // legato: pitch glides, envelope keeps running
            return;
        }

        // Poly: prefer a free voice; steal the oldest-triggered one otherwise.
        int targetIndex = -1;
        for (int i = 0; i < maxVoices; ++i)
        {
            if (! voices[(size_t) i].isActive())
            {
                targetIndex = i;
                break;
            }
        }
        if (targetIndex < 0)
        {
            targetIndex = 0;
            for (int i = 1; i < maxVoices; ++i)
                if (voiceTriggerOrder[(size_t) i] < voiceTriggerOrder[(size_t) targetIndex])
                    targetIndex = i;
        }

        voices[(size_t) targetIndex].startNote(midiNote, velocity);
        voiceNote[(size_t) targetIndex] = midiNote;
        voiceTriggerOrder[(size_t) targetIndex] = ++triggerCounter;
    }

    void SynthEngine::internalNoteOff(int midiNote) noexcept
    {
        if (voiceMode == VoiceMode::Mono)
        {
            monoNoteStack.removeAllInstancesOf(midiNote);
            if (monoNoteStack.isEmpty())
                voices[0].stopNote();
            else
                voices[0].glideToNote(monoNoteStack.getLast(), voices[0].getVelocity());
            return;
        }

        for (int i = 0; i < maxVoices; ++i)
        {
            if (voiceNote[(size_t) i] == midiNote && voices[(size_t) i].isActive())
            {
                voices[(size_t) i].stopNote();
                voiceNote[(size_t) i] = -1;
                break; // exactly one voice per held note -- chords are one key, one voice
            }
        }
    }

    void SynthEngine::allNotesOff() noexcept
    {
        for (auto& v : voices)
            v.stopNote();
        voiceNote.fill(-1);
        monoNoteStack.clearQuick();
        arp.clearHeldNotes();
    }

    int SynthEngine::getMonitorVoiceIndex() const noexcept
    {
        if (voiceMode == VoiceMode::Mono)
            return voices[0].isActive() ? 0 : -1;

        int best = -1;
        for (int i = 0; i < maxVoices; ++i)
        {
            if (voices[(size_t) i].isActive()
                && (best < 0 || voiceTriggerOrder[(size_t) i] > voiceTriggerOrder[(size_t) best]))
                best = i;
        }
        return best;
    }

    SynthEngine::MonitorSamples SynthEngine::getMonitorSamples() const noexcept
    {
        MonitorSamples result;
        const int idx = getMonitorVoiceIndex();
        if (idx < 0)
            return result;

        const auto& v = voices[(size_t) idx];
        result.osc[0] = v.getLastOscSample(0);
        result.osc[1] = v.getLastOscSample(1);
        result.osc[2] = v.getLastOscSample(2);
        result.output = v.getLastOutputSample();
        return result;
    }

    float SynthEngine::renderNextSample() noexcept
    {
        arp.advanceSample([this](int note) { internalNoteOn(note, getStoredVelocity(note)); },
                           [this](int note) { internalNoteOff(note); });

        float sum = 0.0f;
        for (auto& v : voices)
            sum += v.renderNextSample();
        return sum;
    }
}
