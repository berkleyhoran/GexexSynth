#pragma once

#include <array>
#include <cmath>
#include <vector>
#include <juce_dsp/juce_dsp.h>

namespace gexex
{
    // Single-sideband frequency shifter -- distinct from a pitch shifter:
    // it moves every partial by a fixed Hz *offset* rather than a fixed
    // *ratio*, so harmonic content turns inharmonic/bell-like/metallic as
    // the shift grows. The classic "frequency shifter" character, and a
    // genuinely different effect from anything else in this rack.
    //
    // Built from the discrete Hilbert transform's closed form rather than
    // a borrowed allpass coefficient table (which would be hard to verify
    // by eye): h[k] = 0 for even k, 2/(pi*k) for odd k, where k is the tap
    // offset from the kernel's centre -- windowed here with a Blackman
    // window to control ringing from the truncated (finite-length) kernel.
    // The "real" branch is delayed by the FIR's exact integer group delay
    // ((numTaps-1)/2 samples) through a small integer sample buffer -- no
    // interpolation needed, since that delay is an exact integer count by
    // construction -- so the two branches stay time-aligned.
    //
    // Output = real*cos(theta) - imag*sin(theta): one formula shifts up
    // for positive Hz and down for negative Hz for free, since cos is an
    // even function of the shift sign and sin is odd.
    class FrequencyShifter
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;
            buildKernel();

            const juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };
            for (auto& ch : channels)
            {
                ch.fir.coefficients = kernelCoefficients;
                ch.fir.prepare(monoSpec);
                ch.delayBuffer.assign((size_t) delaySamples + 1, 0.0f);
                ch.delayWritePos = 0;
            }
            phase = 0.0f;
        }

        void reset() noexcept
        {
            for (auto& ch : channels)
            {
                ch.fir.reset();
                std::fill(ch.delayBuffer.begin(), ch.delayBuffer.end(), 0.0f);
                ch.delayWritePos = 0;
            }
            phase = 0.0f;
        }

        void setShiftHz(float hz) noexcept { shiftHz = hz; }
        void setMix(float newMix) noexcept { mix = juce::jlimit(0.0f, 1.0f, newMix); }

        void process(juce::dsp::AudioBlock<float>& block) noexcept
        {
            if (mix <= 0.0f)
                return;

            const auto numCh = juce::jmin((size_t) 2, block.getNumChannels());
            const auto numSamples = block.getNumSamples();
            const double phaseIncrement = juce::MathConstants<double>::twoPi * (double) shiftHz / sampleRate;

            double localPhase = phase;
            for (size_t i = 0; i < numSamples; ++i)
            {
                const float cosTheta = (float) std::cos(localPhase);
                const float sinTheta = (float) std::sin(localPhase);

                for (size_t ch = 0; ch < numCh; ++ch)
                {
                    auto& c = channels[ch];
                    auto* data = block.getChannelPointer(ch);
                    const float dry = data[i];

                    // Delay-compensated real branch: write this sample,
                    // read back the one written delaySamples ago.
                    const int writePos = c.delayWritePos;
                    c.delayBuffer[(size_t) writePos] = dry;
                    const int bufferSize = (int) c.delayBuffer.size();
                    const int readPos = (writePos + 1) % bufferSize;
                    const float delayedReal = c.delayBuffer[(size_t) readPos];
                    c.delayWritePos = readPos;

                    const float imag = c.fir.processSample(dry);

                    const float shifted = delayedReal * cosTheta - imag * sinTheta;
                    data[i] = dry * (1.0f - mix) + shifted * mix;
                }

                localPhase += phaseIncrement;
                if (localPhase > juce::MathConstants<double>::twoPi)
                    localPhase -= juce::MathConstants<double>::twoPi;
                else if (localPhase < -juce::MathConstants<double>::twoPi)
                    localPhase += juce::MathConstants<double>::twoPi;
            }
            phase = localPhase;
        }

    private:
        static constexpr int numTaps = 65; // odd; ~32-sample (<1ms) group delay -- inaudible as latency
        static constexpr int delaySamples = (numTaps - 1) / 2;

        void buildKernel()
        {
            std::vector<float> taps((size_t) numTaps, 0.0f);
            const int half = (numTaps - 1) / 2;
            for (int n = 0; n < numTaps; ++n)
            {
                const int k = n - half; // recentre so k=0 is the kernel's middle tap
                float h = 0.0f;
                if (k != 0 && (k % 2) != 0)
                    h = 2.0f / (juce::MathConstants<float>::pi * (float) k);

                const float w =
                    0.42f
                    - 0.5f * std::cos(juce::MathConstants<float>::twoPi * (float) n / (float) (numTaps - 1))
                    + 0.08f * std::cos(2.0f * juce::MathConstants<float>::twoPi * (float) n / (float) (numTaps - 1));
                taps[(size_t) n] = h * w;
            }
            kernelCoefficients = new juce::dsp::FIR::Coefficients<float>(taps.data(), (size_t) numTaps);
        }

        struct Channel
        {
            juce::dsp::FIR::Filter<float> fir;
            std::vector<float> delayBuffer;
            int delayWritePos = 0;
        };

        double sampleRate = 44100.0;
        float shiftHz = 0.0f;
        float mix = 0.0f;
        double phase = 0.0;
        juce::dsp::FIR::Coefficients<float>::Ptr kernelCoefficients;
        std::array<Channel, 2> channels;
    };
}
