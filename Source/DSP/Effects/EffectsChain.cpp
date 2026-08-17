#include "EffectsChain.h"

namespace gexex
{
    void EffectsChain::prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        drive.prepare(spec);
        bitcrusher.reset();
        delay.prepare(spec);
        reverb.prepare(spec);
        chorus.prepare(spec);
        phaserFlanger.prepare(spec);
        saturator.prepare(spec);

        workBuffer.setSize(2, (int) spec.maximumBlockSize);
        sendBuffer.setSize(2, (int) spec.maximumBlockSize);
    }

    void EffectsChain::reset() noexcept
    {
        drive.reset();
        bitcrusher.reset();
        delay.reset();
        reverb.reset();
        chorus.reset();
        phaserFlanger.reset();
        saturator.reset();
    }

    void EffectsChain::process(const float* monoIn, juce::AudioBuffer<float>& stereoOut, int destStartSample,
                                int numSamples) noexcept
    {
        if (numSamples <= 0)
            return;
        jassert(numSamples <= workBuffer.getNumSamples());

        // Dual-mono the synth's mono voice sum into the working stereo
        // buffer -- everything downstream operates in stereo even though
        // nothing's panned yet at this point in the chain.
        workBuffer.copyFrom(0, 0, monoIn, numSamples);
        workBuffer.copyFrom(1, 0, monoIn, numSamples);

        juce::dsp::AudioBlock<float> workBlock(workBuffer);
        auto trimmedBlock = workBlock.getSubBlock(0, (size_t) numSamples);

        drive.process(trimmedBlock);
        bitcrusher.processBlock(workBuffer, numSamples);

        // Delay and reverb are parallel sends (see the class comment), not
        // serial inserts: process a *copy* of the current bus through
        // each, then mix the wet result back in at its own mix knob,
        // rather than replacing the bus with the wet signal. Skipped
        // entirely at mix=0 -- free CPU when a send isn't in use.
        if (delayMix > 0.0f)
        {
            sendBuffer.copyFrom(0, 0, workBuffer, 0, 0, numSamples);
            sendBuffer.copyFrom(1, 0, workBuffer, 1, 0, numSamples);
            juce::dsp::AudioBlock<float> sendBlock(sendBuffer);
            auto trimmedSend = sendBlock.getSubBlock(0, (size_t) numSamples);
            delay.setTimeSeconds(delayTimeSeconds);
            delay.process(trimmedSend, sampleRate);
            workBuffer.addFrom(0, 0, sendBuffer, 0, 0, numSamples, delayMix);
            workBuffer.addFrom(1, 0, sendBuffer, 1, 0, numSamples, delayMix);
        }

        if (reverbMix > 0.0f)
        {
            sendBuffer.copyFrom(0, 0, workBuffer, 0, 0, numSamples);
            sendBuffer.copyFrom(1, 0, workBuffer, 1, 0, numSamples);
            juce::dsp::AudioBlock<float> sendBlock(sendBuffer);
            auto trimmedSend = sendBlock.getSubBlock(0, (size_t) numSamples);
            reverb.process(trimmedSend);
            workBuffer.addFrom(0, 0, sendBuffer, 0, 0, numSamples, reverbMix);
            workBuffer.addFrom(1, 0, sendBuffer, 1, 0, numSamples, reverbMix);
        }

        chorus.process(trimmedBlock);
        phaserFlanger.process(trimmedBlock);
        saturator.process(trimmedBlock);
        masterBus.process(trimmedBlock);

        for (int ch = 0; ch < juce::jmin(2, stereoOut.getNumChannels()); ++ch)
            stereoOut.copyFrom(ch, destStartSample, workBuffer, ch, 0, numSamples);
    }
}
