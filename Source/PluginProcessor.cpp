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
    lfo2.setSampleRate(sampleRate);
    modEnvelope.setSampleRate(sampleRate);
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

    {
        SubOscSettings subSettings;
        subSettings.waveform = (Waveform) getChoiceIndex(ParamIDs::subWaveform);
        subSettings.octaveDown = (int) getFloatParam(ParamIDs::subOctaveDown);
        subSettings.level = getFloatParam(ParamIDs::subLevel);
        subSettings.muted = getFloatParam(ParamIDs::subMute) >= 0.5f;
        synthEngine.setSubOscSettings(subSettings);
    }

    synthEngine.setFilterRouting((FilterRouting) getChoiceIndex(ParamIDs::filterRouting));
    synthEngine.setFilter2CutoffHz(getFloatParam(ParamIDs::filter2Cutoff));
    synthEngine.setFilter2Resonance(juce::jlimit(0.1f, 20.0f, getFloatParam(ParamIDs::filter2Resonance)));
    synthEngine.setFilter2Type((juce::dsp::StateVariableTPTFilterType) getChoiceIndex(ParamIDs::filter2Type));

    // --- Modulation sources: 2 LFOs + 1 mod envelope, each independently
    // routable to the same ModTarget list, all resolved once per block
    // (control-rate -- see the class comment on why) and summed per
    // target via modFor()/isAnyTarget() below, rather than the earlier
    // single-source design's plain "mod"/"isTarget()" -- every call site
    // downstream just changed its `mod` reads to modFor(thatTarget) and
    // its isTarget() reads to isAnyTarget(), the actual modulation *math*
    // at each site is unchanged.
    lfo.setWaveform((LfoWaveform) getChoiceIndex(ParamIDs::lfoWaveform));
    lfo.setSyncDivision((SyncDivision) getChoiceIndex(ParamIDs::lfoSyncDivision));
    lfo.setFreeRateHz(getFloatParam(ParamIDs::lfoRateHz));
    lfo.setHostBpm(hostBpm);
    const float lfoValue = lfo.advanceBlock(blockNumSamples);
    const ModRoute route1 { (ModTarget) getChoiceIndex(ParamIDs::lfoTarget), getFloatParam(ParamIDs::lfoDepth) };
    const float mod1 = route1.resolve(lfoValue);

    lfo2.setWaveform((LfoWaveform) getChoiceIndex(ParamIDs::lfo2Waveform));
    lfo2.setSyncDivision((SyncDivision) getChoiceIndex(ParamIDs::lfo2SyncDivision));
    lfo2.setFreeRateHz(getFloatParam(ParamIDs::lfo2RateHz));
    lfo2.setHostBpm(hostBpm);
    const float lfo2Value = lfo2.advanceBlock(blockNumSamples);
    const ModRoute route2 { (ModTarget) getChoiceIndex(ParamIDs::lfo2Target), getFloatParam(ParamIDs::lfo2Depth) };
    const float mod2 = route2.resolve(lfo2Value);

    // Mod envelope: block-rate like the LFOs (not per-voice -- see
    // Parameters.h's comment), so it's advanced blockNumSamples times in
    // one shot rather than per-sample inside the render loop; juce::ADSR
    // has no direct "jump forward N samples" API, but N tight calls here
    // cost the same as N calls anywhere else and this only runs once per
    // block, not once per voice.
    modEnvelope.setParameters({ getFloatParam(ParamIDs::modEnvAttack), getFloatParam(ParamIDs::modEnvDecay),
                                 getFloatParam(ParamIDs::modEnvSustain), getFloatParam(ParamIDs::modEnvRelease) });
    float modEnvValue = 0.0f;
    for (int i = 0; i < blockNumSamples; ++i)
        modEnvValue = modEnvelope.getNextSample();
    const ModRoute route3 { (ModTarget) getChoiceIndex(ParamIDs::modEnvTarget), getFloatParam(ParamIDs::modEnvDepth) };
    const float mod3 = route3.resolve(modEnvValue); // envelope is 0..1 (unipolar), so this rises from 0 rather than swinging +-

    const auto modFor = [&](ModTarget t) {
        float sum = 0.0f;
        if (route1.target == t)
            sum += mod1;
        if (route2.target == t)
            sum += mod2;
        if (route3.target == t)
            sum += mod3;
        return sum;
    };
    const auto isAnyTarget = [&](ModTarget t) {
        return route1.target == t || route2.target == t || route3.target == t;
    };

    // FM: additive, matching Osc2/3Level's units.
    float fmAmount2 = getFloatParam(ParamIDs::fmAmount2) + modFor(ModTarget::Osc2FmAmount);
    float fmAmount3 = getFloatParam(ParamIDs::fmAmount3) + modFor(ModTarget::Osc3FmAmount);
    synthEngine.setFmAmount(0, juce::jlimit(0.0f, 1.0f, fmAmount2));
    synthEngine.setFmAmount(1, juce::jlimit(0.0f, 1.0f, fmAmount3));

    // Osc level modulation is applied on top of the base level already
    // pushed via setOscillatorSettings above -- re-push just the level for
    // whichever oscillator(s) are a current target, since OscillatorSettings
    // has already been sent for this block. (With 2 LFOs + a mod envelope,
    // more than one osc's level can be targeted at once now, unlike the
    // single-source design -- so this re-pushes each targeted osc in turn
    // rather than assuming at most one.)
    for (int oscNumber = 1; oscNumber <= 3; ++oscNumber)
    {
        const auto levelTarget = oscNumber == 1 ? ModTarget::Osc1Level
                                                 : (oscNumber == 2 ? ModTarget::Osc2Level : ModTarget::Osc3Level);
        if (! isAnyTarget(levelTarget))
            continue;

        OscillatorSettings settings;
        settings.waveform = (Waveform) getChoiceIndex(oscParamID(oscNumber, ParamIDs::oscWaveSuffix).toRawUTF8());
        settings.pulseWidth = getFloatParam(oscParamID(oscNumber, ParamIDs::oscPulseWidthSuffix).toRawUTF8());
        settings.fold = getFloatParam(oscParamID(oscNumber, ParamIDs::oscFoldSuffix).toRawUTF8());
        settings.octave = (int) getFloatParam(oscParamID(oscNumber, ParamIDs::oscOctaveSuffix).toRawUTF8());
        settings.semitone = (int) getFloatParam(oscParamID(oscNumber, ParamIDs::oscSemitoneSuffix).toRawUTF8());
        settings.fineCents = getFloatParam(oscParamID(oscNumber, ParamIDs::oscFineSuffix).toRawUTF8());
        settings.level = juce::jlimit(0.0f, 1.0f,
            getFloatParam(oscParamID(oscNumber, ParamIDs::oscLevelSuffix).toRawUTF8()) + modFor(levelTarget));
        settings.muted = getFloatParam(oscParamID(oscNumber, ParamIDs::oscMuteSuffix).toRawUTF8()) >= 0.5f;
        synthEngine.setOscillatorSettings(oscNumber - 1, settings);
    }

    // Filter cutoff: multiplicative (octaves), matching how ears perceive
    // pitch/frequency -- an additive Hz offset would sound wildly uneven
    // across the cutoff's range.
    float cutoffHz = getFloatParam(ParamIDs::filterCutoff) * std::pow(2.0f, modFor(ModTarget::FilterCutoff));
    synthEngine.setFilterCutoffHz(juce::jlimit(20.0f, 20000.0f, cutoffHz));

    float resonance = getFloatParam(ParamIDs::filterResonance) + modFor(ModTarget::FilterResonance);
    synthEngine.setFilterResonance(juce::jlimit(0.1f, 20.0f, resonance));
    synthEngine.setFilterType((juce::dsp::StateVariableTPTFilterType) getChoiceIndex(ParamIDs::filterType));
    synthEngine.setFilterVelocitySensitivity(getFloatParam(ParamIDs::filterVelSens));

    synthEngine.setEnvelopeParameters({ getFloatParam(ParamIDs::envAttack), getFloatParam(ParamIDs::envDecay),
                                         getFloatParam(ParamIDs::envSustain), getFloatParam(ParamIDs::envRelease) });

    synthEngine.setVoiceMode((VoiceMode) getChoiceIndex(ParamIDs::voiceMode));
    synthEngine.setGlideTimeSeconds(getFloatParam(ParamIDs::glideTime));
    synthEngine.setPitchModSemitones(modFor(ModTarget::Pitch));

    synthEngine.setArpEnabled(getFloatParam(ParamIDs::arpEnabled) >= 0.5f);
    synthEngine.setArpPattern((ArpPattern) getChoiceIndex(ParamIDs::arpPattern));
    synthEngine.setArpOctaveRange((int) getFloatParam(ParamIDs::arpOctaveRange));
    const auto arpSync = (SyncDivision) getChoiceIndex(ParamIDs::arpSyncDivision);
    synthEngine.setArpRateHz(resolveRateHz(arpSync, getFloatParam(ParamIDs::arpRateHz), hostBpm));
    synthEngine.setArpGate(getFloatParam(ParamIDs::arpGate));

    // --- Effects chain ---
    effectsChain.setDriveAmount(getFloatParam(ParamIDs::driveAmount));

    float crushBits = getFloatParam(ParamIDs::crushBits) + modFor(ModTarget::BitcrushDepth);
    effectsChain.setBitcrushBits(juce::jlimit(1.0f, 16.0f, crushBits));
    float crushDownsample = getFloatParam(ParamIDs::crushDownsample) + modFor(ModTarget::BitcrushDownsample);
    effectsChain.setBitcrushDownsample(juce::jlimit(1, 40, (int) crushDownsample));

    const auto delaySync = (SyncDivision) getChoiceIndex(ParamIDs::delaySyncDivision);
    float delayTime = resolveTimeSeconds(delaySync, getFloatParam(ParamIDs::delayTimeSeconds), hostBpm);
    delayTime *= (1.0f + modFor(ModTarget::DelayTime));
    effectsChain.setDelayTimeSeconds(juce::jlimit(0.001f, 2.0f, delayTime));

    float delayFeedback = getFloatParam(ParamIDs::delayFeedback) + modFor(ModTarget::DelayFeedback);
    effectsChain.setDelayFeedback(juce::jlimit(0.0f, 0.95f, delayFeedback));
    float delayMix = getFloatParam(ParamIDs::delayMix) + modFor(ModTarget::DelayMix);
    effectsChain.setDelayMix(juce::jlimit(0.0f, 1.0f, delayMix));

    effectsChain.setReverbSize(getFloatParam(ParamIDs::reverbSize));
    float reverbMix = getFloatParam(ParamIDs::reverbMix) + modFor(ModTarget::ReverbMix);
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

    effectsChain.setFreqShiftHz(getFloatParam(ParamIDs::freqShiftHz));
    effectsChain.setFreqShiftMix(getFloatParam(ParamIDs::freqShiftMix));

    effectsChain.setMbCrossoverLowHz(getFloatParam(ParamIDs::mbCrossoverLow));
    effectsChain.setMbCrossoverHighHz(getFloatParam(ParamIDs::mbCrossoverHigh));
    effectsChain.setMbAmount(getFloatParam(ParamIDs::mbAmount));
    effectsChain.setMbLowGainDb(getFloatParam(ParamIDs::mbLowGain));
    effectsChain.setMbMidGainDb(getFloatParam(ParamIDs::mbMidGain));
    effectsChain.setMbHighGainDb(getFloatParam(ParamIDs::mbHighGain));
    effectsChain.setMbMix(getFloatParam(ParamIDs::mbMix));

    for (int slot = 0; slot < numFxSlots; ++slot)
        effectsChain.setSlotEffect(slot, (FxSlotEffect) getChoiceIndex(fxSlotParamID(slot)));

    // Amp (tremolo): resolved here, applied as a post-synthesis gain in
    // renderVoiceBlock (it multiplies the raw voice sum, before the
    // effects chain, rather than being a synthEngine/effectsChain setter).
    // Clamped to [0,1] so full-depth-down troughs can reach silence while
    // the "up" half of the swing doesn't boost past unity.
    ampModMultiplier = juce::jlimit(0.0f, 1.0f, 1.0f + modFor(ModTarget::Amp));

    // Master volume/pan stay untouched by modulation and randomize alike
    // (matches the reference -- see the build plan's §5).
    effectsChain.setMasterVolume(getFloatParam(ParamIDs::masterVolume));
    float pan = getFloatParam(ParamIDs::masterPan) + modFor(ModTarget::AutoPan);
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
        heldNoteCount = 0;
        modEnvelope.noteOff();
    }

    updateEngineParameters(buffer.getNumSamples());
    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);

    int samplePos = 0;
    for (const auto metadata : midi)
    {
        renderVoiceBlock(buffer, samplePos, metadata.samplePosition - samplePos);
        samplePos = metadata.samplePosition;

        const auto message = metadata.getMessage();
        synthEngine.handleMidiEvent(message);

        // Mod Envelope trigger: edge-detected on the *first* note held
        // and the *last* note released (a single shared envelope, not
        // per-voice -- see Parameters.h's comment), the same "mono
        // envelope" convention plenty of hardware/software synths use for
        // a shared filter/pitch envelope alongside per-voice amp envelopes.
        if (message.isNoteOn())
        {
            if (heldNoteCount++ == 0)
                modEnvelope.noteOn();
        }
        else if (message.isNoteOff())
        {
            if (heldNoteCount > 0 && --heldNoteCount == 0)
                modEnvelope.noteOff();
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            heldNoteCount = 0;
            modEnvelope.noteOff();
        }
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
