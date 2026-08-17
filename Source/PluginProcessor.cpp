#include "PluginProcessor.h"
#include "PluginEditor.h"

GexexSynthAudioProcessor::GexexSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

GexexSynthAudioProcessor::~GexexSynthAudioProcessor() = default;

bool GexexSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    // Instrument: no input bus to match.
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled();
}

void GexexSynthAudioProcessor::prepareToPlay(double, int)
{
    // Nothing to prepare yet -- Phase 1 adds the voice pool/filter/envelope
    // here.
}

void GexexSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    // Silent instrument for now: just make sure the output buffer is
    // definitely quiet (some hosts hand back non-zeroed scratch buffers).
    buffer.clear();
}

juce::AudioProcessorEditor* GexexSynthAudioProcessor::createEditor()
{
    return new GexexSynthAudioProcessorEditor(*this);
}

void GexexSynthAudioProcessor::getStateInformation(juce::MemoryBlock&)
{
    // No parameters yet -- Phase 1+ round-trips an APVTS state here.
}

void GexexSynthAudioProcessor::setStateInformation(const void*, int)
{
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GexexSynthAudioProcessor();
}
