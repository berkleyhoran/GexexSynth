#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "PolyBlepOscillator.h"
#include "Pitch.h"
#include "NoiseGenerator.h"

namespace gexex
{
    struct OscillatorSettings
    {
        Waveform waveform = Waveform::Saw;
        float pulseWidth = 0.5f;
        float fold = 0.0f;
        int octave = 0;
        int semitone = 0;
        float fineCents = 0.0f;
        float level = 0.6f;
        bool muted = false;
    };

    struct NoiseSettings
    {
        NoiseType type = NoiseType::White;
        float level = 0.0f; // defaults silent, like osc2/3 -- raising it is all it takes to bring noise in
        bool muted = false;
    };

    // Sub-oscillator: a 5th sound source (after the noise generator),
    // pitched a selectable octave (or two) below the voice's root note --
    // reuses the main PolyBlepOscillator/Waveform machinery rather than a
    // bespoke sine-only generator, so it can be a square/triangle sub too.
    struct SubOscSettings
    {
        Waveform waveform = Waveform::Sine;
        int octaveDown = 1; // 1 or 2
        float level = 0.0f; // defaults silent, same convention as osc2/3/noise
        bool muted = false;
    };

    // Choice item order must match Parameters.cpp's filterRouting choices
    // exactly (Filter 1 Only, Series, Parallel).
    enum class FilterRouting
    {
        Filter1Only,
        Series,
        Parallel
    };

    // One polyphonic voice: 3 PolyBLEP oscillators (osc2/osc3 phase-
    // modulating osc1, i.e. FM) -> a mix -> one TPT state-variable filter ->
    // one ADSR envelope. Owned in a fixed pool by SynthEngine, which also
    // handles mono/poly voice allocation, the held-note stack, and the
    // arpeggiator sitting above all of it.
    //
    // Pitch glide (portamento) lives here rather than in SynthEngine: each
    // voice smooths its own *root* note number (glideSmoother), and each
    // oscillator derives its actual frequency from that smoothed root plus
    // its own octave/semitone/fine offset every sample -- so glide sweeps
    // all three oscillators (and therefore the FM ratio) together, matching
    // how portamento behaves on a real analog voice.
    class Voice
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            sampleRate = spec.sampleRate;
            for (auto& o : oscillators)
                o.setSampleRate(sampleRate);
            filter.prepare(spec);
            filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            filter2.prepare(spec);
            filter2.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            subOscillator.setSampleRate(sampleRate);
            envelope.setSampleRate(sampleRate);
            glideSmoother.reset(sampleRate, 0.0);
        }

        void setOscillatorSettings(int oscIndex, const OscillatorSettings& settings) noexcept
        {
            jassert(oscIndex >= 0 && oscIndex < 3);
            oscSettings[(size_t) oscIndex] = settings;
        }

        void setNoiseSettings(const NoiseSettings& settings) noexcept { noiseSettings = settings; }
        void setSubOscSettings(const SubOscSettings& settings) noexcept { subSettings = settings; }

        // amount: 0..1, osc2->osc1 (index 0) or osc3->osc1 (index 1).
        void setFmAmount(int modulatorIndex, float amount) noexcept
        {
            jassert(modulatorIndex == 0 || modulatorIndex == 1);
            fmAmount[(size_t) modulatorIndex] = amount;
        }

        // Base cutoff (the knob value) and velocity->cutoff sensitivity
        // combine into the filter's actual cutoff here, per-voice, since
        // each voice's velocity differs -- the block-level cutoff pushed
        // in from PluginProcessor is the same for every voice, but the
        // *effective* cutoff after velocity scaling isn't.
        void setFilterCutoffHz(float cutoffHz) noexcept
        {
            baseCutoffHz = cutoffHz;
            updateFilterCutoff();
        }
        void setFilterVelocitySensitivity(float sensitivity) noexcept
        {
            velocitySensitivity = sensitivity;
            updateFilterCutoff();
        }
        void setFilterResonance(float resonance) noexcept { filter.setResonance(resonance); }
        void setFilterType(juce::dsp::StateVariableTPTFilterType type) noexcept { filter.setType(type); }

        void setFilterRouting(FilterRouting routing) noexcept { filterRouting = routing; }
        void setFilter2CutoffHz(float cutoffHz) noexcept
        {
            filter2.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, cutoffHz));
        }
        void setFilter2Resonance(float resonance) noexcept { filter2.setResonance(resonance); }
        void setFilter2Type(juce::dsp::StateVariableTPTFilterType type) noexcept { filter2.setType(type); }

        void setEnvelopeParameters(const juce::ADSR::Parameters& params) noexcept { envelope.setParameters(params); }

        // The LFO's "Pitch (all osc)" target is the one modulation
        // destination that has to reach the audio-rate note calculation
        // directly (everything else is resolved once per block, before it
        // ever reaches Voice -- see PluginProcessor::updateEngineParameters'
        // comment on that design choice).
        void setPitchModSemitones(float semitones) noexcept { pitchModSemitones = semitones; }

        // SmoothedValue::reset() snaps currentValue to target as a side
        // effect of changing the ramp length (see JUCE's implementation) --
        // called unconditionally every block, that would finish any
        // in-progress glide instantly every 512 samples. Guarding on an
        // actual change keeps a slide smooth across block boundaries while
        // still picking up a genuine glide-time tweak from the user.
        void setGlideTimeSeconds(float seconds) noexcept
        {
            if (! juce::approximatelyEqual(seconds, lastGlideTimeSeconds))
            {
                lastGlideTimeSeconds = seconds;
                glideSmoother.reset(sampleRate, (double) seconds);
            }
        }

        // A brand-new note: retriggers the envelope's attack. Pitch glides
        // from whatever this voice was last playing if it has sounded
        // before and glide time > 0; otherwise it snaps straight there
        // (no glide-from-silence on a voice's very first note).
        void startNote(int midiNoteNumber, float velocity) noexcept
        {
            velocityLevel = velocity;
            updateFilterCutoff();
            const float target = (float) midiNoteNumber;
            if (hasSoundedBefore)
                glideSmoother.setTargetValue(target);
            else
                glideSmoother.setCurrentAndTargetValue(target);
            hasSoundedBefore = true;

            for (auto& o : oscillators)
                o.resetPhase();
            subOscillator.resetPhase();
            envelope.noteOn();
        }

        // Mono-mode legato: pitch glides to the new note, but the envelope
        // is left alone (keeps sustaining/releasing as it already was) --
        // this is what makes overlapping mono playing sound like one
        // continuous note sliding between pitches instead of a fresh pluck
        // each time.
        void glideToNote(int midiNoteNumber, float velocity) noexcept
        {
            velocityLevel = velocity;
            updateFilterCutoff();
            glideSmoother.setTargetValue((float) midiNoteNumber);
        }

        void stopNote() noexcept { envelope.noteOff(); }

        bool isActive() const noexcept { return envelope.isActive(); }
        float getVelocity() const noexcept { return velocityLevel; }

        float renderNextSample() noexcept
        {
            if (! envelope.isActive())
                return 0.0f;

            const float rootNote = glideSmoother.getNextValue() + pitchModSemitones;

            for (int i = 0; i < 3; ++i)
            {
                const auto& s = oscSettings[(size_t) i];
                const float note = rootNote + (float) (s.octave * 12 + s.semitone) + s.fineCents * 0.01f;
                oscillators[(size_t) i].setFrequency(noteNumberToHz(note));
            }

            // osc2/osc3 render first (their own frequency, unmodulated),
            // then phase-modulate osc1 with a scaled mix of both -- fixed
            // evaluation order, no feedback loop, matching the reference
            // (only osc2/osc3 -> osc1 exist; nothing modulates osc2/osc3).
            const float osc2Raw = oscillators[1].renderSample(oscSettings[1].waveform, oscSettings[1].pulseWidth);
            const float osc3Raw = oscillators[2].renderSample(oscSettings[2].waveform, oscSettings[2].pulseWidth);
            const float fmPhaseOffset = (osc2Raw * fmAmount[0] + osc3Raw * fmAmount[1]) * fmDepthScale;
            const float osc1Raw =
                oscillators[0].renderSample(oscSettings[0].waveform, oscSettings[0].pulseWidth, fmPhaseOffset);

            float mixed = 0.0f;
            mixed += oscSettings[0].muted ? 0.0f
                                           : PolyBlepOscillator::waveFold(osc1Raw, oscSettings[0].fold)
                                                 * oscSettings[0].level;
            mixed += oscSettings[1].muted ? 0.0f
                                           : PolyBlepOscillator::waveFold(osc2Raw, oscSettings[1].fold)
                                                 * oscSettings[1].level;
            mixed += oscSettings[2].muted ? 0.0f
                                           : PolyBlepOscillator::waveFold(osc3Raw, oscSettings[2].fold)
                                                 * oscSettings[2].level;

            // Noise: a 4th, unpitched sound source mixed in alongside the
            // oscillators -- rendered every sample the voice is active so
            // it rides the same filter/envelope, same as a real analog
            // noise generator would.
            mixed += noiseSettings.muted ? 0.0f : noiseGenerator.renderSample(noiseSettings.type) * noiseSettings.level;

            // Sub-oscillator: a 5th source, pitched an octave (or two)
            // below the root -- tracks glide/pitch-mod the same way the
            // main oscillators do since it's derived from the same
            // rootNote, just shifted down before the octave/semitone math
            // the main oscillators apply.
            if (! subSettings.muted && subSettings.level > 0.0f)
            {
                const float subNote = rootNote - 12.0f * (float) subSettings.octaveDown;
                subOscillator.setFrequency(noteNumberToHz(subNote));
                mixed += subOscillator.renderSample(subSettings.waveform, 0.5f) * subSettings.level;
            }

            // Filter routing: Filter1Only matches the original single-
            // filter behaviour exactly; Series feeds filter 1's output
            // into filter 2; Parallel runs both on the same dry mix and
            // averages them (halved rather than summed raw, so enabling
            // Parallel doesn't itself double the level versus Filter1Only).
            float filtered;
            switch (filterRouting)
            {
                case FilterRouting::Series:
                    filtered = filter2.processSample(0, filter.processSample(0, mixed));
                    break;
                case FilterRouting::Parallel:
                    filtered = 0.5f * (filter.processSample(0, mixed) + filter2.processSample(0, mixed));
                    break;
                case FilterRouting::Filter1Only:
                default:
                    filtered = filter.processSample(0, mixed);
                    break;
            }
            const float output = filtered * envelope.getNextSample();

            // Stashed for the mini-oscilloscopes (Phase 5) -- raw,
            // pre-fold/level oscillator output and the voice's final
            // output, read back by SynthEngine's "monitor voice" picker.
            lastOscRaw[0] = osc1Raw;
            lastOscRaw[1] = osc2Raw;
            lastOscRaw[2] = osc3Raw;
            lastOutputSample = output;

            return output;
        }

        float getLastOscSample(int oscIndex) const noexcept { return lastOscRaw[(size_t) oscIndex]; }
        float getLastOutputSample() const noexcept { return lastOutputSample; }

    private:
        // velocitySensitivity 0..1 scales up to +-4 octaves of cutoff shift
        // based on how far this voice's velocity sits below max (velocity
        // 1.0 = no shift at any sensitivity).
        void updateFilterCutoff() noexcept
        {
            const float mult = std::pow(2.0f, velocitySensitivity * 4.0f * (velocityLevel - 1.0f));
            filter.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, baseCutoffHz * mult));
        }

        // Tuned so the 0..1 FM-amount knob covers "no audible FM" up to
        // "clearly inharmonic/bell-like", matching the reference's
        // practical range -- see the build plan's §1.2.
        static constexpr float fmDepthScale = 3.0f;

        std::array<PolyBlepOscillator, 3> oscillators;
        std::array<OscillatorSettings, 3> oscSettings;
        std::array<float, 2> fmAmount { 0.0f, 0.0f };
        NoiseGenerator noiseGenerator;
        NoiseSettings noiseSettings;
        PolyBlepOscillator subOscillator;
        SubOscSettings subSettings;

        juce::dsp::StateVariableTPTFilter<float> filter;
        juce::dsp::StateVariableTPTFilter<float> filter2;
        FilterRouting filterRouting = FilterRouting::Filter1Only;
        juce::ADSR envelope;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> glideSmoother;
        bool hasSoundedBefore = false;
        float velocityLevel = 1.0f;
        float lastGlideTimeSeconds = -1.0f;
        float pitchModSemitones = 0.0f;
        float baseCutoffHz = 2000.0f;
        float velocitySensitivity = 0.0f;
        double sampleRate = 44100.0;
        std::array<float, 3> lastOscRaw { 0.0f, 0.0f, 0.0f };
        float lastOutputSample = 0.0f;
    };
}
