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

void GexexSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec spec { sampleRate,
                                         (juce::uint32) samplesPerBlock,
                                         (juce::uint32) getTotalNumOutputChannels() };
    synthEngine.prepare(spec);
    lfo.setSampleRate(sampleRate);
    effectsChain.prepare(spec);
    monoScratch.setSize(1, samplesPerBlock);
}

double GexexSynthAudioProcessor::getHostBpm() const noexcept
{
    if (auto* currentPlayHead = getPlayHead())
        if (const auto position = currentPlayHead->getPosition())
            if (const auto bpm = position->getBpm())
                return *bpm;
    return 120.0; // Standalone / no host clock: a sane default so sync-mode controls still do something musical.
}

void GexexSynthAudioProcessor::updateEngineParameters(int blockNumSamples) noexcept
{
    using namespace gexex;

    const double hostBpm = getHostBpm();

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

    {
        NoiseSettings noiseSettings;
        noiseSettings.type = (NoiseType) getChoiceIndex(ParamIDs::noiseType);
        noiseSettings.level = getFloatParam(ParamIDs::noiseLevel);
        noiseSettings.muted = getFloatParam(ParamIDs::noiseMute) >= 0.5f;
        synthEngine.setNoiseSettings(noiseSettings);
    }

    // --- LFO: advance once per block (control-rate), then resolve this
    // block's modulation amount for whichever single target is selected.
    lfo.setWaveform((LfoWaveform) getChoiceIndex(ParamIDs::lfoWaveform));
    lfo.setSyncDivision((SyncDivision) getChoiceIndex(ParamIDs::lfoSyncDivision));
    lfo.setFreeRateHz(getFloatParam(ParamIDs::lfoRateHz));
    lfo.setHostBpm(hostBpm);
    const float lfoValue = lfo.advanceBlock(blockNumSamples);

    const ModRoute modRoute { (ModTarget) getChoiceIndex(ParamIDs::lfoTarget), getFloatParam(ParamIDs::lfoDepth) };
    const float mod = modRoute.resolve(lfoValue);
    const auto isTarget = [&](ModTarget t) { return modRoute.target == t; };

    // FM: additive, matching Osc2/3Level's units.
    float fmAmount2 = getFloatParam(ParamIDs::fmAmount2) + (isTarget(ModTarget::Osc2FmAmount) ? mod : 0.0f);
    float fmAmount3 = getFloatParam(ParamIDs::fmAmount3) + (isTarget(ModTarget::Osc3FmAmount) ? mod : 0.0f);
    synthEngine.setFmAmount(0, juce::jlimit(0.0f, 1.0f, fmAmount2));
    synthEngine.setFmAmount(1, juce::jlimit(0.0f, 1.0f, fmAmount3));

    // Osc level modulation is applied on top of the base level already
    // pushed via setOscillatorSettings above -- re-push just the level for
    // whichever oscillator (if any) is the current target, since
    // OscillatorSettings has already been sent for this block.
    if (isTarget(ModTarget::Osc1Level) || isTarget(ModTarget::Osc2Level) || isTarget(ModTarget::Osc3Level))
    {
        const int targetOsc = isTarget(ModTarget::Osc1Level) ? 1 : (isTarget(ModTarget::Osc2Level) ? 2 : 3);
        OscillatorSettings settings;
        settings.waveform = (Waveform) getChoiceIndex(oscParamID(targetOsc, ParamIDs::oscWaveSuffix).toRawUTF8());
        settings.pulseWidth = getFloatParam(oscParamID(targetOsc, ParamIDs::oscPulseWidthSuffix).toRawUTF8());
        settings.fold = getFloatParam(oscParamID(targetOsc, ParamIDs::oscFoldSuffix).toRawUTF8());
        settings.octave = (int) getFloatParam(oscParamID(targetOsc, ParamIDs::oscOctaveSuffix).toRawUTF8());
        settings.semitone = (int) getFloatParam(oscParamID(targetOsc, ParamIDs::oscSemitoneSuffix).toRawUTF8());
        settings.fineCents = getFloatParam(oscParamID(targetOsc, ParamIDs::oscFineSuffix).toRawUTF8());
        settings.level = juce::jlimit(
            0.0f, 1.0f, getFloatParam(oscParamID(targetOsc, ParamIDs::oscLevelSuffix).toRawUTF8()) + mod);
        settings.muted = getFloatParam(oscParamID(targetOsc, ParamIDs::oscMuteSuffix).toRawUTF8()) >= 0.5f;
        synthEngine.setOscillatorSettings(targetOsc - 1, settings);
    }

    // Filter cutoff: multiplicative (octaves), matching how ears perceive
    // pitch/frequency -- an additive Hz offset would sound wildly uneven
    // across the cutoff's range.
    float cutoffHz = getFloatParam(ParamIDs::filterCutoff);
    if (isTarget(ModTarget::FilterCutoff))
        cutoffHz *= std::pow(2.0f, mod);
    synthEngine.setFilterCutoffHz(juce::jlimit(20.0f, 20000.0f, cutoffHz));

    float resonance = getFloatParam(ParamIDs::filterResonance) + (isTarget(ModTarget::FilterResonance) ? mod : 0.0f);
    synthEngine.setFilterResonance(juce::jlimit(0.1f, 20.0f, resonance));
    synthEngine.setFilterType((juce::dsp::StateVariableTPTFilterType) getChoiceIndex(ParamIDs::filterType));
    synthEngine.setFilterVelocitySensitivity(getFloatParam(ParamIDs::filterVelSens));

    synthEngine.setEnvelopeParameters({ getFloatParam(ParamIDs::envAttack), getFloatParam(ParamIDs::envDecay),
                                         getFloatParam(ParamIDs::envSustain), getFloatParam(ParamIDs::envRelease) });

    synthEngine.setVoiceMode((VoiceMode) getChoiceIndex(ParamIDs::voiceMode));
    synthEngine.setGlideTimeSeconds(getFloatParam(ParamIDs::glideTime));
    synthEngine.setPitchModSemitones(isTarget(ModTarget::Pitch) ? mod : 0.0f);

    synthEngine.setArpEnabled(getFloatParam(ParamIDs::arpEnabled) >= 0.5f);
    synthEngine.setArpPattern((ArpPattern) getChoiceIndex(ParamIDs::arpPattern));
    synthEngine.setArpOctaveRange((int) getFloatParam(ParamIDs::arpOctaveRange));
    const auto arpSync = (SyncDivision) getChoiceIndex(ParamIDs::arpSyncDivision);
    synthEngine.setArpRateHz(resolveRateHz(arpSync, getFloatParam(ParamIDs::arpRateHz), hostBpm));
    synthEngine.setArpGate(getFloatParam(ParamIDs::arpGate));

    // --- Effects chain ---
    effectsChain.setDriveAmount(getFloatParam(ParamIDs::driveAmount));

    float crushBits = getFloatParam(ParamIDs::crushBits) + (isTarget(ModTarget::BitcrushDepth) ? mod : 0.0f);
    effectsChain.setBitcrushBits(juce::jlimit(1.0f, 16.0f, crushBits));
    float crushDownsample =
        getFloatParam(ParamIDs::crushDownsample) + (isTarget(ModTarget::BitcrushDownsample) ? mod : 0.0f);
    effectsChain.setBitcrushDownsample(juce::jlimit(1, 40, (int) crushDownsample));

    const auto delaySync = (SyncDivision) getChoiceIndex(ParamIDs::delaySyncDivision);
    float delayTime = resolveTimeSeconds(delaySync, getFloatParam(ParamIDs::delayTimeSeconds), hostBpm);
    if (isTarget(ModTarget::DelayTime))
        delayTime *= (1.0f + mod);
    effectsChain.setDelayTimeSeconds(juce::jlimit(0.001f, 2.0f, delayTime));

    float delayFeedback = getFloatParam(ParamIDs::delayFeedback) + (isTarget(ModTarget::DelayFeedback) ? mod : 0.0f);
    effectsChain.setDelayFeedback(juce::jlimit(0.0f, 0.95f, delayFeedback));
    float delayMix = getFloatParam(ParamIDs::delayMix) + (isTarget(ModTarget::DelayMix) ? mod : 0.0f);
    effectsChain.setDelayMix(juce::jlimit(0.0f, 1.0f, delayMix));

    effectsChain.setReverbSize(getFloatParam(ParamIDs::reverbSize));
    float reverbMix = getFloatParam(ParamIDs::reverbMix) + (isTarget(ModTarget::ReverbMix) ? mod : 0.0f);
    effectsChain.setReverbMix(juce::jlimit(0.0f, 1.0f, reverbMix));

    effectsChain.setChorusRateHz(getFloatParam(ParamIDs::chorusRateHz));
    effectsChain.setChorusDepthMs(getFloatParam(ParamIDs::chorusDepthMs));
    effectsChain.setChorusMix(getFloatParam(ParamIDs::chorusMix));

    effectsChain.setPhaserFlangerMode((PhaserFlangerMode) getChoiceIndex(ParamIDs::pfMode));
    effectsChain.setPhaserFlangerRateHz(getFloatParam(ParamIDs::pfRateHz));
    effectsChain.setPhaserFlangerDepth(getFloatParam(ParamIDs::pfDepth));
    effectsChain.setPhaserFlangerFeedback(getFloatParam(ParamIDs::pfFeedback));
    effectsChain.setPhaserFlangerMix(getFloatParam(ParamIDs::pfMix));

    effectsChain.setSaturatorAlgorithm((SaturatorAlgorithm) getChoiceIndex(ParamIDs::saturatorAlgorithm));
    effectsChain.setSaturatorAmount(getFloatParam(ParamIDs::saturatorAmount));
    effectsChain.setSaturatorCeiling(getFloatParam(ParamIDs::saturatorCeiling));

    // Amp (tremolo): resolved here, applied as a post-synthesis gain in
    // renderVoiceBlock (it multiplies the raw voice sum, before the
    // effects chain, rather than being a synthEngine/effectsChain setter).
    // Clamped to [0,1] so full-depth-down troughs can reach silence while
    // the "up" half of the swing doesn't boost past unity.
    ampModMultiplier = isTarget(ModTarget::Amp) ? juce::jlimit(0.0f, 1.0f, 1.0f + mod) : 1.0f;

    // Master volume/pan stay untouched by modulation and randomize alike
    // (matches the reference -- see the build plan's §5).
    effectsChain.setMasterVolume(getFloatParam(ParamIDs::masterVolume));
    float pan = getFloatParam(ParamIDs::masterPan) + (isTarget(ModTarget::AutoPan) ? mod : 0.0f);
    effectsChain.setMasterPan(juce::jlimit(-1.0f, 1.0f, pan));
}

void GexexSynthAudioProcessor::renderVoiceBlock(juce::AudioBuffer<float>& buffer, int startSample,
                                                 int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        const float raw = synthEngine.renderNextSample();
        monoScratch.setSample(0, i, raw * ampModMultiplier);

        const auto monitor = synthEngine.getMonitorSamples();
        oscScopes[0].push(monitor.osc[0]);
        oscScopes[1].push(monitor.osc[1]);
        oscScopes[2].push(monitor.osc[2]);
    }

    effectsChain.process(monoScratch.getReadPointer(0), buffer, startSample, numSamples);

    // Master scope reads the *actual* final output (post effects chain),
    // not the pre-effects voice sum -- pushed after effectsChain::process
    // has written into `buffer` for exactly that reason.
    for (int i = 0; i < numSamples; ++i)
        masterScope.push(buffer.getSample(0, startSample + i));
}

void GexexSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (panicRequested.exchange(false, std::memory_order_relaxed))
    {
        synthEngine.handleMidiEvent(juce::MidiMessage::allNotesOff(1));
        effectsChain.reset(); // also flushes delay/reverb tails for true instant silence
    }

    updateEngineParameters(buffer.getNumSamples());
    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);

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
    return new GexexSynthAudioProcessorEditor(*this);
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
