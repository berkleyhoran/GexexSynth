#include "PluginProcessor.h"

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

void GexexSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec spec { sampleRate,
                                         (juce::uint32) samplesPerBlock,
                                         (juce::uint32) getTotalNumOutputChannels() };
    voice.prepare(spec);
}

void GexexSynthAudioProcessor::renderVoiceBlock(juce::AudioBuffer<float>& buffer, int startSample,
                                                 int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    const auto numChannels = buffer.getNumChannels();
    for (int i = 0; i < numSamples; ++i)
    {
        const float sample = voice.renderNextSample();
        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, startSample + i, sample);
    }
}

void GexexSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    voice.setFilterCutoffHz(apvts.getRawParameterValue(gexex::ParamIDs::filterCutoff)->load());
    voice.setFilterResonance(apvts.getRawParameterValue(gexex::ParamIDs::filterResonance)->load());
    voice.setEnvelopeParameters({ apvts.getRawParameterValue(gexex::ParamIDs::envAttack)->load(),
                                   apvts.getRawParameterValue(gexex::ParamIDs::envDecay)->load(),
                                   apvts.getRawParameterValue(gexex::ParamIDs::envSustain)->load(),
                                   apvts.getRawParameterValue(gexex::ParamIDs::envRelease)->load() });

    int samplePos = 0;

    for (const auto metadata : midi)
    {
        renderVoiceBlock(buffer, samplePos, metadata.samplePosition - samplePos);
        samplePos = metadata.samplePosition;

        const auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            currentNote = message.getNoteNumber();
            voice.setFrequencyFromMidiNote(currentNote);
            voice.noteOn();
        }
        else if (message.isNoteOff())
        {
            if (message.getNoteNumber() == currentNote)
            {
                voice.noteOff();
                currentNote = -1;
            }
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            voice.noteOff();
            currentNote = -1;
        }
    }

    renderVoiceBlock(buffer, samplePos, buffer.getNumSamples() - samplePos);
}

juce::AudioProcessorEditor* GexexSynthAudioProcessor::createEditor()
{
    // Placeholder editor: auto-builds a slider per APVTS parameter, so
    // every knob Phase 1 adds is immediately playable/automatable without
    // hand-building a GUI for it. Phase 4 replaces this with the real
    // "fruity aero" ModuleRack + custom LookAndFeel.
    return new juce::GenericAudioProcessorEditor(*this);
}

void GexexSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void GexexSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GexexSynthAudioProcessor();
}
