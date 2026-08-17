#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // 3-band (2-crossover) compressor/leveler, built from JUCE's own
    // tested juce::dsp::LinkwitzRileyFilter (a matched-phase crossover
    // filter specifically designed for splitting a signal into bands that
    // sum back flat -- see its own docs) and juce::dsp::Compressor,
    // rather than hand-derived crossover math. Two crossover points
    // cascade into three bands via the standard two-stage LR-crossover
    // topology: split once at crossoverLow into (low, rest), then split
    // `rest` again at crossoverHigh into (mid, high). A single Amount
    // knob drives threshold+ratio together for all three bands (kept
    // simple rather than exposing 6 separate compressor knobs); per-band
    // Gain trims are the main creative control after that -- boost/cut
    // low/mid/high independently, the most audibly useful multiband move.
    class MultibandCompressor
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            lowLowpass.prepare(spec);
            lowLowpass.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            lowHighpass.prepare(spec);
            lowHighpass.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
            midLowpass.prepare(spec);
            midLowpass.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            highHighpass.prepare(spec);
            highHighpass.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

            for (auto* comp : { &lowComp, &midComp, &highComp })
            {
                comp->prepare(spec);
                comp->setAttack(10.0f);
                comp->setRelease(80.0f);
            }

            // EffectsChain always hands this class a dual-mono/stereo
            // 2-channel work block regardless of the host's actual output
            // bus width (see EffectsChain::process()) -- sized here to
            // match that reality directly rather than trusting
            // spec.numChannels, which instead reflects the bus layout and
            // could be 1 on a mono-output host, undersizing these buffers
            // relative to the 2-channel block process() actually receives.
            const int numCh = 2;
            const int blockSize = (int) spec.maximumBlockSize;
            lowBuffer.setSize(numCh, blockSize);
            restBuffer.setSize(numCh, blockSize);
            midBuffer.setSize(numCh, blockSize);
            highBuffer.setSize(numCh, blockSize);
            dryBuffer.setSize(numCh, blockSize);
        }

        void reset() noexcept
        {
            lowLowpass.reset();
            lowHighpass.reset();
            midLowpass.reset();
            highHighpass.reset();
            lowComp.reset();
            midComp.reset();
            highComp.reset();
        }

        void setCrossoverLowHz(float hz) noexcept { crossoverLowHz = juce::jlimit(20.0f, 19000.0f, hz); }
        void setCrossoverHighHz(float hz) noexcept { crossoverHighHz = juce::jlimit(20.0f, 20000.0f, hz); }
        void setAmount(float amt) noexcept { amount = juce::jlimit(0.0f, 1.0f, amt); }
        void setLowGainDb(float db) noexcept { lowGain = juce::Decibels::decibelsToGain(db); }
        void setMidGainDb(float db) noexcept { midGain = juce::Decibels::decibelsToGain(db); }
        void setHighGainDb(float db) noexcept { highGain = juce::Decibels::decibelsToGain(db); }
        void setMix(float newMix) noexcept { mix = juce::jlimit(0.0f, 1.0f, newMix); }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            if (mix <= 0.0f)
                return;

            const int numCh = (int) block.getNumChannels();
            const int numSamples = (int) block.getNumSamples();
            if (numCh <= 0 || numSamples <= 0)
                return;

            // Crossover cutoffs must stay strictly ordered -- clamp apart
            // here rather than fighting it with interlocked parameter
            // ranges, so a user dragging one knob past the other doesn't
            // silently produce an inverted/empty band.
            const float lowHz = juce::jmin(crossoverLowHz, crossoverHighHz - 1.0f);
            const float highHz = juce::jmax(crossoverHighHz, lowHz + 1.0f);
            lowLowpass.setCutoffFrequency(lowHz);
            lowHighpass.setCutoffFrequency(lowHz);
            midLowpass.setCutoffFrequency(highHz);
            highHighpass.setCutoffFrequency(highHz);

            for (int ch = 0; ch < numCh; ++ch)
            {
                auto* src = block.getChannelPointer((size_t) ch);
                dryBuffer.copyFrom(ch, 0, src, numSamples);
                lowBuffer.copyFrom(ch, 0, src, numSamples);
                restBuffer.copyFrom(ch, 0, src, numSamples);
            }

            // Stage 1: split into (low, rest) at crossoverLow.
            juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
            auto lowTrimmed = lowBlock.getSubBlock(0, (size_t) numSamples);
            juce::dsp::ProcessContextReplacing<float> lowCtx(lowTrimmed);
            lowLowpass.process(lowCtx);

            juce::dsp::AudioBlock<float> restBlock(restBuffer);
            auto restTrimmed = restBlock.getSubBlock(0, (size_t) numSamples);
            juce::dsp::ProcessContextReplacing<float> restCtx(restTrimmed);
            lowHighpass.process(restCtx);

            // Stage 2: split `rest` into (mid, high) at crossoverHigh.
            for (int ch = 0; ch < numCh; ++ch)
            {
                midBuffer.copyFrom(ch, 0, restBuffer, ch, 0, numSamples);
                highBuffer.copyFrom(ch, 0, restBuffer, ch, 0, numSamples);
            }
            juce::dsp::AudioBlock<float> midBlock(midBuffer);
            auto midTrimmed = midBlock.getSubBlock(0, (size_t) numSamples);
            juce::dsp::ProcessContextReplacing<float> midCtx(midTrimmed);
            midLowpass.process(midCtx);

            juce::dsp::AudioBlock<float> highBlock(highBuffer);
            auto highTrimmed = highBlock.getSubBlock(0, (size_t) numSamples);
            juce::dsp::ProcessContextReplacing<float> highCtx(highTrimmed);
            highHighpass.process(highCtx);

            // amount=0 -> 0dB threshold/1:1 ratio, i.e. no gain reduction
            // for anything at or below digital full scale -- an honest
            // "off" default rather than a knob that needs a separate
            // bypass flag.
            const float threshold = juce::jmap(amount, 0.0f, 1.0f, 0.0f, -24.0f);
            const float ratio = juce::jmap(amount, 0.0f, 1.0f, 1.0f, 6.0f);
            for (auto* comp : { &lowComp, &midComp, &highComp })
            {
                comp->setThreshold(threshold);
                comp->setRatio(ratio);
            }
            lowComp.process(lowCtx);
            midComp.process(midCtx);
            highComp.process(highCtx);

            for (int ch = 0; ch < numCh; ++ch)
            {
                auto* out = block.getChannelPointer((size_t) ch);
                auto* low = lowBuffer.getReadPointer(ch);
                auto* mid = midBuffer.getReadPointer(ch);
                auto* high = highBuffer.getReadPointer(ch);
                auto* dry = dryBuffer.getReadPointer(ch);
                for (int i = 0; i < numSamples; ++i)
                {
                    const float wet = low[i] * lowGain + mid[i] * midGain + high[i] * highGain;
                    out[i] = dry[i] * (1.0f - mix) + wet * mix;
                }
            }
        }

    private:
        juce::dsp::LinkwitzRileyFilter<float> lowLowpass, lowHighpass, midLowpass, highHighpass;
        juce::dsp::Compressor<float> lowComp, midComp, highComp;

        juce::AudioBuffer<float> lowBuffer, restBuffer, midBuffer, highBuffer, dryBuffer;

        float crossoverLowHz = 200.0f;
        float crossoverHighHz = 2500.0f;
        float amount = 0.0f;
        float lowGain = 1.0f, midGain = 1.0f, highGain = 1.0f;
        float mix = 0.0f;
    };
}
