#pragma once

#include <JuceHeader.h>

// Phase 0 scaffolding: a silent, GUI-less instrument that just needs to
// build and load in a host/Standalone as proof the CMake/JUCE plumbing is
// wired up correctly. No DSP yet -- see the plan doc for what Phase 1+
// adds (PolyBLEP oscillator, TPT filter, ADSR, voice pool, effects chain,
// APVTS parameters, custom GUI, presets, installer).
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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GexexSynthAudioProcessor)
};
