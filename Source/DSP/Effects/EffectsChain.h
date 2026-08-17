#pragma once

#include <juce_dsp/juce_dsp.h>
#include "Bitcrusher.h"
#include "Drive.h"
#include "StereoDelay.h"
#include "Reverb.h"
#include "Chorus.h"
#include "PhaserFlanger.h"
#include "Saturator.h"
#include "MasterBus.h"

namespace gexex
{
    // Signal flow (from the reference's own signal-flow comment, confirmed
    // against gexex/synth.html): voice sum -> Drive -> Bitcrusher ->
    // dry + [Delay send] + [Reverb send] -> Chorus -> Phaser/Flanger ->
    // Saturator -> Master. Delay and Reverb are parallel sends mixed back
    // into the bus at their own mix knob, not serial inserts, so this is a
    // hand-written chain rather than a single juce::dsp::ProcessorChain
    // (which only models purely-serial chains) -- see the build plan's §3.
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
        void setMasterVolume(float vol) noexcept { masterBus.setVolume(vol); }
        void setMasterPan(float pan) noexcept { masterBus.setPan(pan); }

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
        MasterBus masterBus;

        juce::AudioBuffer<float> workBuffer;
        juce::AudioBuffer<float> sendBuffer;
    };
}
