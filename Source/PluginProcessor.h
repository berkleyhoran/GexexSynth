#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "DSP/SynthEngine.h"
#include "DSP/Lfo.h"
#include "DSP/ModMatrix.h"
#include "DSP/Effects/EffectsChain.h"
#include "ScopeDataSource.h"

// Phase 3: the full signal path -- SynthEngine (Phase 2's oscillators/FM/
// voices/arp) feeding EffectsChain (bitcrush/drive/delay/reverb/chorus/
// phaser-flanger/saturator/master), modulated by one routable LFO. Still
// JUCE's built-in generic editor (Phase 4 replaces it with the real
// "fruity aero" GUI).
//
// Architecture note: the build plan sketched SynthEngine as owning the LFO
// and effects chain too ("voice pool + arp + LFO + effects chain"). In
// practice, every LFO modulation target except Pitch is resolved into an
// already-computed *final* parameter value right here in
// updateEngineParameters() -- e.g. the LFO's contribution to filter cutoff
// is added to the cutoff value before it's ever passed to synthEngine --
// rather than threading a live (target, amount) pair down into SynthEngine/
// EffectsChain's internals. That keeps their APIs plain setters-for-
// resolved-values (same shape as Phase 1/2) instead of needing every
// consumer to understand ModTarget. Pitch is the one exception: it has to
// reach the audio-rate note calculation inside Voice directly, so it gets
// its own dedicated setPitchModSemitones() plumbed through SynthEngine.
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

    // Public so PluginEditor can hand it straight to a MidiKeyboardComponent
    // -- clicks on the on-screen keyboard merge into the real MIDI stream
    // in processBlock, so they go through exactly the same voice-allocation/
    // arp path a hardware controller's notes would.
    juce::MidiKeyboardState keyboardState;

    // Mini-oscilloscope feeds (Phase 5) -- public so GUI Scope components
    // can point at them directly; see ScopeDataSource.h for why plain
    // atomics are enough thread-safety for a visual-only consumer.
    const gexex::ScopeDataSource<>& getOscScope(int oscIndex) const noexcept { return oscScopes[(size_t) oscIndex]; }
    const gexex::ScopeDataSource<>& getMasterScope() const noexcept { return masterScope; }

    // Silences every voice immediately (a true hard-stop, not a fast
    // release -- see the build plan's §5) plus flushes the delay/reverb
    // tails. SynthEngine isn't safe to touch from the GUI thread directly
    // (unlike APVTS parameters, its internal voice/arp state has no
    // atomic protection), so the button handler just sets a flag here and
    // processBlock actions it on the audio thread.
    void requestPanic() noexcept { panicRequested.store(true, std::memory_order_relaxed); }

private:
    // Reads the current APVTS values (+ this block's LFO reading) and
    // pushes fully-resolved values into synthEngine/effectsChain -- called
    // once per block, not per-sample (see the class comment above).
    void updateEngineParameters(int blockNumSamples) noexcept;
    void renderVoiceBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;
    double getHostBpm() const noexcept;

    float getFloatParam(const char* paramID) const noexcept { return apvts.getRawParameterValue(paramID)->load(); }
    int getChoiceIndex(const char* paramID) const noexcept { return (int) getFloatParam(paramID); }

    gexex::SynthEngine synthEngine;
    gexex::Lfo lfo;
    gexex::EffectsChain effectsChain;

    juce::AudioBuffer<float> monoScratch;
    float ampModMultiplier = 1.0f; // resolved once per block in updateEngineParameters, applied per-sample below

    std::array<gexex::ScopeDataSource<>, 3> oscScopes;
    gexex::ScopeDataSource<> masterScope;
    std::atomic<bool> panicRequested { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GexexSynthAudioProcessor)
};
