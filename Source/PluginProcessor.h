#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "DSP/SynthEngine.h"

// Phase 2: the full 3-oscillator/FM engine with a real poly voice pool,
// mono/poly + legato/glide, and the arpeggiator, still driven straight from
// MIDI with JUCE's built-in generic editor (no effects chain yet -- that's
// Phase 3 -- and no custom GUI yet -- that's Phase 4).
class GexexSynthAudioProcessor : public juce::AudioProcessor
{
public:
    GexexSynthAudioProcessor();
    ~GexexSynthAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "PARAMETERS", gexex::createParameterLayout() };

private:
    // Reads the current APVTS values and pushes them into synthEngine --
    // called once per block rather than per-sample (control-rate is plenty
    // for these; the audio-rate stuff -- oscillator phase, filter
    // coefficients, envelope -- happens inside Voice::renderNextSample).
    void updateEngineParameters() noexcept;
    void renderVoiceBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

    float getFloatParam(const char* paramID) const noexcept { return apvts.getRawParameterValue(paramID)->load(); }
    int getChoiceIndex(const char* paramID) const noexcept { return (int) getFloatParam(paramID); }

    gexex::SynthEngine synthEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GexexSynthAudioProcessor)
};
