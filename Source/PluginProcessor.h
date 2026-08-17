#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "DSP/Voice.h"

// Phase 1: a hard-coded mono voice (PolyBLEP saw -> TPT filter -> ADSR)
// driven straight from MIDI, with a minimal set of APVTS parameters
// (cutoff, resonance, ADSR) and JUCE's built-in generic editor -- proves
// the core DSP path sounds right before Phase 2 adds the full oscillator
// set, FM, voice pool, and arpeggiator, and Phase 4 replaces the editor
// with the real "fruity aero" GUI.
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
    void renderVoiceBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

    gexex::Voice voice;
    int currentNote = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GexexSynthAudioProcessor)
};
