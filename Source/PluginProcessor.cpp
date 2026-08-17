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
    synthEngine.prepare(spec);
}

void GexexSynthAudioProcessor::updateEngineParameters() noexcept
{
    using namespace gexex;

    for (int oscNumber = 1; oscNumber <= 3; ++oscNumber)
    {
        OscillatorSettings settings;
        settings.waveform = (Waveform) getChoiceIndex(oscParamID(oscNumber, ParamIDs::oscWaveSuffix).toRawUTF8());
        settings.pulseWidth = getFloatParam(oscParamID(oscNumber, ParamIDs::oscPulseWidthSuffix).toRawUTF8());
        settings.fold = getFloatParam(oscParamID(oscNumber, ParamIDs::oscFoldSuffix).toRawUTF8());
        settings.octave = (int) getFloatParam(oscParamID(oscNumber, ParamIDs::oscOctaveSuffix).toRawUTF8());
        settings.semitone = (int) getFloatParam(oscParamID(oscNumber, ParamIDs::oscSemitoneSuffix).toRawUTF8());
        settings.fineCents = getFloatParam(oscParamID(oscNumber, ParamIDs::oscFineSuffix).toRawUTF8());
        settings.level = getFloatParam(oscParamID(oscNumber, ParamIDs::oscLevelSuffix).toRawUTF8());
        settings.muted = getFloatParam(oscParamID(oscNumber, ParamIDs::oscMuteSuffix).toRawUTF8()) >= 0.5f;
        synthEngine.setOscillatorSettings(oscNumber - 1, settings);
    }

    synthEngine.setFmAmount(0, getFloatParam(ParamIDs::fmAmount2));
    synthEngine.setFmAmount(1, getFloatParam(ParamIDs::fmAmount3));

    synthEngine.setFilterCutoffHz(getFloatParam(ParamIDs::filterCutoff));
    synthEngine.setFilterResonance(getFloatParam(ParamIDs::filterResonance));

    synthEngine.setEnvelopeParameters({ getFloatParam(ParamIDs::envAttack), getFloatParam(ParamIDs::envDecay),
                                         getFloatParam(ParamIDs::envSustain), getFloatParam(ParamIDs::envRelease) });

    synthEngine.setVoiceMode((VoiceMode) getChoiceIndex(ParamIDs::voiceMode));
    synthEngine.setGlideTimeSeconds(getFloatParam(ParamIDs::glideTime));

    synthEngine.setArpEnabled(getFloatParam(ParamIDs::arpEnabled) >= 0.5f);
    synthEngine.setArpPattern((ArpPattern) getChoiceIndex(ParamIDs::arpPattern));
    synthEngine.setArpOctaveRange((int) getFloatParam(ParamIDs::arpOctaveRange));
    synthEngine.setArpRateHz(getFloatParam(ParamIDs::arpRateHz));
    synthEngine.setArpGate(getFloatParam(ParamIDs::arpGate));
}

void GexexSynthAudioProcessor::renderVoiceBlock(juce::AudioBuffer<float>& buffer, int startSample,
                                                 int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    const auto numChannels = buffer.getNumChannels();
    for (int i = 0; i < numSamples; ++i)
    {
        const float sample = synthEngine.renderNextSample();
        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, startSample + i, sample);
    }
}

void GexexSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    updateEngineParameters();

    int samplePos = 0;
    for (const auto metadata : midi)
    {
        renderVoiceBlock(buffer, samplePos, metadata.samplePosition - samplePos);
        samplePos = metadata.samplePosition;
        synthEngine.handleMidiEvent(metadata.getMessage());
    }
    renderVoiceBlock(buffer, samplePos, buffer.getNumSamples() - samplePos);
}

juce::AudioProcessorEditor* GexexSynthAudioProcessor::createEditor()
{
    // Placeholder editor: auto-builds a slider/combo/toggle per APVTS
    // parameter, so every knob Phase 2 adds is immediately playable/
    // automatable without hand-building a GUI for it. Phase 4 replaces
    // this with the real "fruity aero" ModuleRack + custom LookAndFeel.
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
