#pragma once

#include <array>
#include <juce_dsp/juce_dsp.h>
#include "Bitcrusher.h"
#include "Drive.h"
#include "StereoDelay.h"
#include "Reverb.h"
#include "Chorus.h"
#include "PhaserFlanger.h"
#include "Saturator.h"
#include "FrequencyShifter.h"
#include "MultibandCompressor.h"
#include "MasterBus.h"
#include "../../Parameters.h"

namespace gexex
{
    // Choice item order must match Parameters.cpp's fxSlot* choices exactly.
    enum class FxSlotEffect
    {
        Empty,
        Drive,
        Bitcrush,
        Chorus,
        PhaserFlanger,
        Saturator,
        FrequencyShifter,
        MultibandCompressor
    };

    // Signal flow: voice sum -> [7 reorderable insert slots, each an
    // independent Empty/Drive/Bitcrush/Chorus/Phaser-Flanger/Saturator/
    // Frequency-Shifter/Multiband-Compressor choice, executed in slot
    // order] -> dry + [Delay send] + [Reverb send] -> Master.
    //
    // Delay and Reverb are parallel sends mixed back into the bus at their
    // own mix knob, not serial inserts (unchanged from the original
    // design -- see the build plan's §3), so they aren't slot effects and
    // can't be reordered relative to the others. They used to sit at a
    // fixed point *inside* the old fixed order (between Bitcrush and
    // Chorus); now that the insert chain itself is reorderable, they're
    // applied once at a fixed point *after* the whole slot chain instead
    // -- simpler than trying to interleave a fixed send between arbitrary
    // reordered slots, and still reads as "spatial effects near the end,"
    // which is where sends conventionally sit anyway.
    class EffectsChain
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec);
        void reset() noexcept;

        void setDriveAmount(float amt) noexcept { drive.setAmount(amt); }
        void setBitcrushBits(float bits) noexcept { bitcrusher.setBitDepth(bits); }
        void setBitcrushDownsample(int factor) noexcept { bitcrusher.setDownsample(factor); }
        void setDelayTimeSeconds(float seconds) noexcept { delayTimeSeconds = seconds; }
        void setDelayFeedback(float fb) noexcept { delay.setFeedback(fb); }
        void setDelayMix(float mix) noexcept { delayMix = juce::jlimit(0.0f, 1.0f, mix); }
        void setReverbSize(float size) noexcept { reverb.setSize(size); }
        void setReverbMix(float mix) noexcept { reverbMix = juce::jlimit(0.0f, 1.0f, mix); }
        void setChorusRateHz(float hz) noexcept { chorus.setRateHz(hz); }
        void setChorusDepthMs(float ms) noexcept { chorus.setDepthMs(ms); }
        void setChorusMix(float mix) noexcept { chorus.setMix(mix); }
        void setPhaserFlangerMode(PhaserFlangerMode mode) noexcept { phaserFlanger.setMode(mode); }
        void setPhaserFlangerRateHz(float hz) noexcept { phaserFlanger.setRateHz(hz); }
        void setPhaserFlangerDepth(float depth) noexcept { phaserFlanger.setDepth(depth); }
        void setPhaserFlangerFeedback(float fb) noexcept { phaserFlanger.setFeedback(fb); }
        void setPhaserFlangerMix(float mix) noexcept { phaserFlanger.setMix(mix); }
        void setSaturatorAlgorithm(SaturatorAlgorithm algo) noexcept { saturator.setAlgorithm(algo); }
        void setSaturatorAmount(float amt) noexcept { saturator.setAmount(amt); }
        void setSaturatorCeiling(float ceiling) noexcept { saturator.setCeiling(ceiling); }
        void setFreqShiftHz(float hz) noexcept { freqShifter.setShiftHz(hz); }
        void setFreqShiftMix(float mix) noexcept { freqShifter.setMix(mix); }
        void setMbCrossoverLowHz(float hz) noexcept { multibandComp.setCrossoverLowHz(hz); }
        void setMbCrossoverHighHz(float hz) noexcept { multibandComp.setCrossoverHighHz(hz); }
        void setMbAmount(float amt) noexcept { multibandComp.setAmount(amt); }
        void setMbLowGainDb(float db) noexcept { multibandComp.setLowGainDb(db); }
        void setMbMidGainDb(float db) noexcept { multibandComp.setMidGainDb(db); }
        void setMbHighGainDb(float db) noexcept { multibandComp.setHighGainDb(db); }
        void setMbMix(float mix) noexcept { multibandComp.setMix(mix); }
        void setMasterVolume(float vol) noexcept { masterBus.setVolume(vol); }
        void setMasterPan(float pan) noexcept { masterBus.setPan(pan); }

        void setSlotEffect(int slotIndex, FxSlotEffect effect) noexcept
        {
            jassert(slotIndex >= 0 && slotIndex < numFxSlots);
            slotEffects[(size_t) slotIndex] = effect;
        }

        // monoIn: numSamples of mono synth-voice output starting at index
        // 0 (a small scratch chunk, not offset into the caller's buffer).
        // stereoOut: the actual output buffer; the fully-processed stereo
        // result is written at [destStartSample, destStartSample+numSamples).
        void process(const float* monoIn, juce::AudioBuffer<float>& stereoOut, int destStartSample,
                     int numSamples) noexcept;

    private:
        double sampleRate = 44100.0;
        float delayTimeSeconds = 0.28f;
        float delayMix = 0.0f;
        float reverbMix = 0.12f;

        Drive drive;
        Bitcrusher bitcrusher;
        StereoDelay delay;
        Reverb reverb;
        Chorus chorus;
        PhaserFlanger phaserFlanger;
        Saturator saturator;
        FrequencyShifter freqShifter;
        MultibandCompressor multibandComp;
        MasterBus masterBus;

        std::array<FxSlotEffect, (size_t) numFxSlots> slotEffects {};

        juce::AudioBuffer<float> workBuffer;
        juce::AudioBuffer<float> sendBuffer;
    };
}
