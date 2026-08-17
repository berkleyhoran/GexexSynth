#include "PresetManager.h"
#include "../Parameters.h"

namespace gexex
{
    namespace
    {
        // Short local alias so the preset table below doesn't have to spell
        // out oscParamID(...) at every osc-numbered parameter.
        juce::String o(int n, const char* suffix) { return oscParamID(n, suffix); }
    } // namespace

    bool isRandomizable(const juce::String& paramID)
    {
        return paramID != ParamIDs::masterVolume && paramID != ParamIDs::masterPan
               && paramID != ParamIDs::reducedMotion;
    }

    // Forty-two hand-authored patches, six categories, all leaning into one
    // coherent character: melancholy, reverby, dreamy -- moody FM leads,
    // lush pads, wistful keys, deep-but-sad bass, slowly-unfolding ambient
    // washes, and a Textures category built around the noise generator
    // (wind, dust, tape hiss, rain, breath). Only the parameters that
    // differ from default are listed; PresetManager::applyFactoryPreset()
    // calls initPatch() first so every unlisted parameter lands on its
    // normal default.
    const std::vector<FactoryPreset>& PresetManager::getFactoryPresets()
    {
        static const std::vector<FactoryPreset> presets = {
            // ============================== PADS ==============================
            { "Velvet Fog", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 1 }, // Triangle
                { o(1, ParamIDs::oscLevelSuffix), 0.5f },
                { o(2, ParamIDs::oscWaveSuffix), 2 }, // Saw
                { o(2, ParamIDs::oscLevelSuffix), 0.35f },
                { o(2, ParamIDs::oscFineSuffix), 7.0f },
                { o(3, ParamIDs::oscWaveSuffix), 0 }, // Sine
                { o(3, ParamIDs::oscLevelSuffix), 0.3f },
                { o(3, ParamIDs::oscOctaveSuffix), -1 },
                { ParamIDs::filterCutoff, 1800.0f },
                { ParamIDs::filterResonance, 2.0f },
                { ParamIDs::envAttack, 1.2f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.75f },
                { ParamIDs::envRelease, 2.5f },
                { ParamIDs::voiceMode, 1 }, // Poly
                { ParamIDs::chorusMix, 0.5f },
                { ParamIDs::chorusDepthMs, 8.0f },
                { ParamIDs::chorusRateHz, 0.3f },
                { ParamIDs::reverbSize, 5.0f },
                { ParamIDs::reverbMix, 0.45f },
                { ParamIDs::delaySyncDivision, 3 }, // 1/4
                { ParamIDs::delayMix, 0.2f },
                { ParamIDs::delayFeedback, 0.3f },
                { ParamIDs::lfoTarget, 2 }, // FilterCutoff
                { ParamIDs::lfoRateHz, 0.15f },
                { ParamIDs::lfoDepth, 0.3f } } },

            { "Halflight", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.55f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.3f },
                { o(2, ParamIDs::oscOctaveSuffix), 1 },
                { ParamIDs::filterCutoff, 900.0f },
                { ParamIDs::filterResonance, 1.5f },
                { ParamIDs::envAttack, 1.5f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 2.2f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::glideTime, 0.1f },
                { ParamIDs::saturatorAmount, 0.1f },
                { ParamIDs::saturatorAlgorithm, 0 }, // Tanh
                { ParamIDs::chorusMix, 0.35f },
                { ParamIDs::reverbSize, 4.5f },
                { ParamIDs::reverbMix, 0.4f } } },

            { "Ashen Bloom", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 2 }, // Saw
                { o(1, ParamIDs::oscLevelSuffix), 0.5f },
                { o(1, ParamIDs::oscFoldSuffix), 0.15f },
                { o(2, ParamIDs::oscWaveSuffix), 3 }, // Square
                { o(2, ParamIDs::oscLevelSuffix), 0.3f },
                { o(2, ParamIDs::oscFineSuffix), -9.0f },
                { ParamIDs::filterCutoff, 1500.0f },
                { ParamIDs::filterResonance, 2.5f },
                { ParamIDs::envAttack, 1.8f },
                { ParamIDs::envDecay, 0.6f },
                { ParamIDs::envSustain, 0.8f },
                { ParamIDs::envRelease, 2.8f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.4f },
                { ParamIDs::reverbSize, 6.0f },
                { ParamIDs::reverbMix, 0.55f },
                { ParamIDs::pfMode, 0 }, // Phaser
                { ParamIDs::pfMix, 0.15f },
                { ParamIDs::pfRateHz, 0.12f } } },

            { "Snowbound", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.5f },
                { o(2, ParamIDs::oscWaveSuffix), 1 },
                { o(2, ParamIDs::oscLevelSuffix), 0.4f },
                { o(2, ParamIDs::oscOctaveSuffix), 1 },
                { ParamIDs::filterCutoff, 2200.0f },
                { ParamIDs::envAttack, 2.5f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.8f },
                { ParamIDs::envRelease, 3.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 16 }, // AutoPan
                { ParamIDs::lfoRateHz, 0.1f },
                { ParamIDs::lfoDepth, 0.4f },
                { ParamIDs::delaySyncDivision, 4 }, // 1/8
                { ParamIDs::delayMix, 0.25f },
                { ParamIDs::delayFeedback, 0.35f },
                { ParamIDs::reverbSize, 4.0f },
                { ParamIDs::reverbMix, 0.35f } } },

            // ============================== LEADS ==============================
            { "Amber Ache", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.7f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { ParamIDs::fmAmount2, 0.35f },
                { ParamIDs::filterCutoff, 3500.0f },
                { ParamIDs::filterResonance, 3.0f },
                { ParamIDs::filterVelSens, 0.4f },
                { ParamIDs::envAttack, 0.02f },
                { ParamIDs::envDecay, 0.4f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 0.8f },
                { ParamIDs::voiceMode, 0 }, // Mono
                { ParamIDs::glideTime, 0.08f },
                { ParamIDs::driveAmount, 0.2f },
                { ParamIDs::delaySyncDivision, 7 }, // 1/8T
                { ParamIDs::delayMix, 0.3f },
                { ParamIDs::delayFeedback, 0.35f },
                { ParamIDs::reverbMix, 0.25f } } },

            { "Wilted Bell", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.65f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 7 },
                { ParamIDs::fmAmount2, 0.5f },
                { ParamIDs::filterCutoff, 5000.0f },
                { ParamIDs::envAttack, 0.005f },
                { ParamIDs::envDecay, 1.2f },
                { ParamIDs::envSustain, 0.2f },
                { ParamIDs::envRelease, 1.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.2f },
                { ParamIDs::reverbSize, 3.5f },
                { ParamIDs::reverbMix, 0.35f } } },

            { "Rustlight", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 3 }, // Square
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(3, ParamIDs::oscWaveSuffix), 0 },
                { o(3, ParamIDs::oscLevelSuffix), 0.0f },
                { ParamIDs::fmAmount3, 0.3f },
                { ParamIDs::filterType, 1 }, // Bandpass
                { ParamIDs::filterCutoff, 2800.0f },
                { ParamIDs::filterResonance, 6.0f },
                { ParamIDs::envAttack, 0.01f },
                { ParamIDs::envDecay, 0.5f },
                { ParamIDs::envSustain, 0.5f },
                { ParamIDs::envRelease, 1.0f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.03f },
                { ParamIDs::pfMode, 0 },
                { ParamIDs::pfMix, 0.3f },
                { ParamIDs::pfRateHz, 0.4f },
                { ParamIDs::reverbMix, 0.2f } } },

            { "Faded Signal", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.65f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { ParamIDs::fmAmount2, 0.25f },
                { ParamIDs::filterCutoff, 3000.0f },
                { ParamIDs::crushBits, 12.0f },
                { ParamIDs::crushDownsample, 2.0f },
                { ParamIDs::envAttack, 0.02f },
                { ParamIDs::envDecay, 0.5f },
                { ParamIDs::envSustain, 0.5f },
                { ParamIDs::envRelease, 1.0f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::delaySyncDivision, 3 },
                { ParamIDs::delayMix, 0.4f },
                { ParamIDs::delayFeedback, 0.45f },
                { ParamIDs::reverbSize, 4.0f },
                { ParamIDs::reverbMix, 0.3f } } },

            // ============================== KEYS ==============================
            { "Rainlit Keys", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 12 },
                { ParamIDs::fmAmount2, 0.25f },
                { ParamIDs::filterCutoff, 4500.0f },
                { ParamIDs::envAttack, 0.005f },
                { ParamIDs::envDecay, 0.6f },
                { ParamIDs::envSustain, 0.3f },
                { ParamIDs::envRelease, 1.2f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.3f },
                { ParamIDs::chorusRateHz, 0.3f },
                { ParamIDs::reverbSize, 2.5f },
                { ParamIDs::reverbMix, 0.3f } } },

            { "Grey Piano", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 12 },
                { ParamIDs::fmAmount2, 0.18f },
                { ParamIDs::filterCutoff, 4000.0f },
                { ParamIDs::envAttack, 0.004f },
                { ParamIDs::envDecay, 0.7f },
                { ParamIDs::envSustain, 0.25f },
                { ParamIDs::envRelease, 1.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::driveAmount, 0.05f },
                { ParamIDs::saturatorAmount, 0.1f },
                { ParamIDs::saturatorAlgorithm, 2 }, // Arctan
                { ParamIDs::reverbSize, 2.0f },
                { ParamIDs::reverbMix, 0.25f } } },

            { "Nocturne Bell", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.55f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 12 },
                { ParamIDs::fmAmount2, 0.5f },
                { ParamIDs::filterCutoff, 6000.0f },
                { ParamIDs::envAttack, 0.005f },
                { ParamIDs::envDecay, 1.5f },
                { ParamIDs::envSustain, 0.15f },
                { ParamIDs::envRelease, 1.8f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 4 }, // Amp
                { ParamIDs::lfoRateHz, 3.5f },
                { ParamIDs::lfoDepth, 0.15f },
                { ParamIDs::reverbSize, 3.0f },
                { ParamIDs::reverbMix, 0.35f } } },

            { "Hollow Reed", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 3 },
                { o(1, ParamIDs::oscLevelSuffix), 0.55f },
                { o(1, ParamIDs::oscFoldSuffix), 0.1f },
                { ParamIDs::filterCutoff, 2500.0f },
                { ParamIDs::filterResonance, 1.8f },
                { ParamIDs::envAttack, 0.01f },
                { ParamIDs::envDecay, 0.5f },
                { ParamIDs::envSustain, 0.4f },
                { ParamIDs::envRelease, 1.3f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::delaySyncDivision, 4 },
                { ParamIDs::delayMix, 0.2f },
                { ParamIDs::reverbSize, 2.8f },
                { ParamIDs::reverbMix, 0.3f } } },

            // ============================== BASS ==============================
            { "Undertow", "Bass",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.8f },
                { o(1, ParamIDs::oscOctaveSuffix), -1 },
                { o(2, ParamIDs::oscWaveSuffix), 2 },
                { o(2, ParamIDs::oscLevelSuffix), 0.3f },
                { o(2, ParamIDs::oscOctaveSuffix), -1 },
                { ParamIDs::filterCutoff, 600.0f },
                { ParamIDs::filterResonance, 4.0f },
                { ParamIDs::filterVelSens, 0.3f },
                { ParamIDs::envAttack, 0.005f },
                { ParamIDs::envDecay, 0.3f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 0.4f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.05f },
                { ParamIDs::driveAmount, 0.3f },
                { ParamIDs::reverbMix, 0.12f } } },

            { "Bruised Low", "Bass",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.75f },
                { o(1, ParamIDs::oscOctaveSuffix), -1 },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscOctaveSuffix), -1 },
                { ParamIDs::fmAmount2, 0.2f },
                { ParamIDs::filterCutoff, 750.0f },
                { ParamIDs::filterResonance, 3.0f },
                { ParamIDs::envAttack, 0.004f },
                { ParamIDs::envDecay, 0.4f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 0.35f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::saturatorAmount, 0.2f },
                { ParamIDs::saturatorAlgorithm, 3 }, // Hard
                { ParamIDs::saturatorCeiling, 0.8f },
                { ParamIDs::reverbMix, 0.1f } } },

            { "Slow Fathom", "Bass",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.75f },
                { o(1, ParamIDs::oscOctaveSuffix), -2 },
                { o(2, ParamIDs::oscWaveSuffix), 1 },
                { o(2, ParamIDs::oscLevelSuffix), 0.25f },
                { o(2, ParamIDs::oscOctaveSuffix), -1 },
                { ParamIDs::filterCutoff, 500.0f },
                { ParamIDs::envAttack, 0.3f },
                { ParamIDs::envDecay, 0.5f },
                { ParamIDs::envSustain, 0.8f },
                { ParamIDs::envRelease, 1.2f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.15f },
                { ParamIDs::reverbSize, 3.0f },
                { ParamIDs::reverbMix, 0.2f } } },

            // ============================== AMBIENT ==============================
            { "Drift Chapel", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.45f },
                { o(2, ParamIDs::oscWaveSuffix), 1 },
                { o(2, ParamIDs::oscLevelSuffix), 0.35f },
                { o(2, ParamIDs::oscFineSuffix), 5.0f },
                { ParamIDs::filterCutoff, 2200.0f },
                { ParamIDs::envAttack, 3.0f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 4.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::delaySyncDivision, 1 }, // 1/1
                { ParamIDs::delayMix, 0.35f },
                { ParamIDs::delayFeedback, 0.5f },
                { ParamIDs::chorusMix, 0.4f },
                { ParamIDs::reverbSize, 8.0f },
                { ParamIDs::reverbMix, 0.7f },
                { ParamIDs::lfoTarget, 13 }, // ReverbMix
                { ParamIDs::lfoRateHz, 0.08f },
                { ParamIDs::lfoDepth, 0.2f } } },

            { "Static Hymn", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.4f },
                { o(3, ParamIDs::oscWaveSuffix), 1 },
                { o(3, ParamIDs::oscLevelSuffix), 0.3f },
                { ParamIDs::filterType, 2 }, // Highpass
                { ParamIDs::filterCutoff, 400.0f },
                { ParamIDs::crushBits, 8.0f },
                { ParamIDs::crushDownsample, 6.0f },
                { ParamIDs::envAttack, 2.0f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 3.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 2 }, // FilterCutoff
                { ParamIDs::lfoRateHz, 0.06f },
                { ParamIDs::lfoDepth, 0.5f },
                { ParamIDs::reverbSize, 7.0f },
                { ParamIDs::reverbMix, 0.6f } } },

            { "Glass Orbit", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.55f },
                { o(2, ParamIDs::oscWaveSuffix), 1 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 12 },
                { ParamIDs::fmAmount2, 0.15f },
                { ParamIDs::filterCutoff, 5000.0f },
                { ParamIDs::envAttack, 0.01f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.3f },
                { ParamIDs::envRelease, 1.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::arpEnabled, 1.0f },
                { ParamIDs::arpPattern, 2 }, // UpDown
                { ParamIDs::arpOctaveRange, 2 },
                { ParamIDs::arpSyncDivision, 5 }, // 1/16
                { ParamIDs::arpGate, 0.6f },
                { ParamIDs::delaySyncDivision, 7 },
                { ParamIDs::delayMix, 0.3f },
                { ParamIDs::reverbSize, 4.5f },
                { ParamIDs::reverbMix, 0.5f } } },

            { "Vapor Trail", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.5f },
                { o(3, ParamIDs::oscWaveSuffix), 0 },
                { o(3, ParamIDs::oscLevelSuffix), 0.3f },
                { o(3, ParamIDs::oscOctaveSuffix), 1 },
                { ParamIDs::filterCutoff, 2600.0f },
                { ParamIDs::envAttack, 1.5f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 2.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 16 }, // AutoPan
                { ParamIDs::lfoRateHz, 0.12f },
                { ParamIDs::lfoDepth, 0.6f },
                { ParamIDs::pfMode, 1 }, // Flanger
                { ParamIDs::pfMix, 0.3f },
                { ParamIDs::pfRateHz, 0.2f },
                { ParamIDs::reverbSize, 5.5f },
                { ParamIDs::reverbMix, 0.45f } } },

            { "Last Ember", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 7 },
                { ParamIDs::fmAmount2, 0.2f },
                { ParamIDs::filterCutoff, 2000.0f },
                { ParamIDs::filterResonance, 2.0f },
                { ParamIDs::envAttack, 0.05f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.5f },
                { ParamIDs::envRelease, 3.0f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.06f },
                { ParamIDs::saturatorAmount, 0.15f },
                { ParamIDs::saturatorAlgorithm, 0 },
                { ParamIDs::delaySyncDivision, 3 },
                { ParamIDs::delayMix, 0.3f },
                { ParamIDs::delayFeedback, 0.4f },
                { ParamIDs::reverbSize, 6.5f },
                { ParamIDs::reverbMix, 0.55f } } },

            // ============================== PADS (cont.) =======================
            { "Cathedral Hush", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.45f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.3f },
                { o(2, ParamIDs::oscOctaveSuffix), 1 },
                { ParamIDs::noiseType, 1 }, // Pink
                { ParamIDs::noiseLevel, 0.06f },
                { ParamIDs::filterCutoff, 1600.0f },
                { ParamIDs::envAttack, 2.2f },
                { ParamIDs::envDecay, 1.2f },
                { ParamIDs::envSustain, 0.75f },
                { ParamIDs::envRelease, 3.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.3f },
                { ParamIDs::reverbSize, 7.5f },
                { ParamIDs::reverbMix, 0.65f } } },

            { "Faded Photograph", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.45f },
                { o(2, ParamIDs::oscWaveSuffix), 1 },
                { o(2, ParamIDs::oscLevelSuffix), 0.35f },
                { o(2, ParamIDs::oscFineSuffix), 6.0f },
                { ParamIDs::filterCutoff, 2000.0f },
                { ParamIDs::crushBits, 11.0f },
                { ParamIDs::envAttack, 1.0f },
                { ParamIDs::envDecay, 0.9f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 2.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.45f },
                { ParamIDs::chorusDepthMs, 6.0f },
                { ParamIDs::reverbSize, 4.0f },
                { ParamIDs::reverbMix, 0.4f } } },

            { "Winter Glass", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.5f },
                { o(2, ParamIDs::oscWaveSuffix), 3 },
                { o(2, ParamIDs::oscLevelSuffix), 0.2f },
                { o(2, ParamIDs::oscOctaveSuffix), 1 },
                { ParamIDs::filterType, 2 }, // Highpass
                { ParamIDs::filterCutoff, 700.0f },
                { ParamIDs::envAttack, 1.8f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.65f },
                { ParamIDs::envRelease, 2.8f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 16 }, // AutoPan
                { ParamIDs::lfoRateHz, 0.09f },
                { ParamIDs::lfoDepth, 0.3f },
                { ParamIDs::reverbSize, 5.5f },
                { ParamIDs::reverbMix, 0.5f } } },

            { "Quiet Harbor", "Pads",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.55f },
                { o(3, ParamIDs::oscWaveSuffix), 2 },
                { o(3, ParamIDs::oscLevelSuffix), 0.25f },
                { o(3, ParamIDs::oscOctaveSuffix), -1 },
                { ParamIDs::filterCutoff, 1200.0f },
                { ParamIDs::envAttack, 1.6f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 2.4f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 2 }, // FilterCutoff
                { ParamIDs::lfoRateHz, 0.05f },
                { ParamIDs::lfoDepth, 0.4f },
                { ParamIDs::delaySyncDivision, 1 }, // 1/1
                { ParamIDs::delayMix, 0.3f },
                { ParamIDs::delayFeedback, 0.4f },
                { ParamIDs::reverbSize, 6.0f },
                { ParamIDs::reverbMix, 0.5f } } },

            // ============================== LEADS (cont.) =======================
            { "Broken Radio", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.65f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { ParamIDs::fmAmount2, 0.4f },
                { ParamIDs::filterType, 1 }, // Bandpass
                { ParamIDs::filterCutoff, 2200.0f },
                { ParamIDs::crushBits, 7.0f },
                { ParamIDs::crushDownsample, 4.0f },
                { ParamIDs::envAttack, 0.015f },
                { ParamIDs::envDecay, 0.4f },
                { ParamIDs::envSustain, 0.5f },
                { ParamIDs::envRelease, 0.7f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.04f },
                { ParamIDs::reverbMix, 0.2f } } },

            { "Paper Moon", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 12 },
                { ParamIDs::fmAmount2, 0.15f },
                { ParamIDs::filterCutoff, 4200.0f },
                { ParamIDs::envAttack, 0.05f },
                { ParamIDs::envDecay, 0.6f },
                { ParamIDs::envSustain, 0.55f },
                { ParamIDs::envRelease, 1.4f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.25f },
                { ParamIDs::reverbSize, 3.5f },
                { ParamIDs::reverbMix, 0.35f } } },

            { "Static Bloom", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 3 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { ParamIDs::fmAmount2, 0.3f },
                { ParamIDs::filterCutoff, 3200.0f },
                { ParamIDs::filterResonance, 4.0f },
                { ParamIDs::envAttack, 0.01f },
                { ParamIDs::envDecay, 0.45f },
                { ParamIDs::envSustain, 0.55f },
                { ParamIDs::envRelease, 0.9f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.02f },
                { ParamIDs::pfMode, 0 },
                { ParamIDs::pfMix, 0.35f },
                { ParamIDs::pfRateHz, 0.5f },
                { ParamIDs::reverbMix, 0.25f } } },

            { "Distant Call", "Leads",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { ParamIDs::filterCutoff, 2600.0f },
                { ParamIDs::envAttack, 0.03f },
                { ParamIDs::envDecay, 0.5f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 1.2f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.1f },
                { ParamIDs::lfoTarget, 1 }, // Pitch (all osc) -- gentle vibrato
                { ParamIDs::lfoRateHz, 5.0f },
                { ParamIDs::lfoDepth, 0.08f },
                { ParamIDs::delaySyncDivision, 1 }, // 1/1
                { ParamIDs::delayMix, 0.4f },
                { ParamIDs::delayFeedback, 0.5f },
                { ParamIDs::reverbSize, 5.0f },
                { ParamIDs::reverbMix, 0.4f } } },

            // ============================== KEYS (cont.) ========================
            { "Dust Chimes", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.55f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 19 },
                { ParamIDs::fmAmount2, 0.6f },
                { ParamIDs::filterCutoff, 6500.0f },
                { ParamIDs::envAttack, 0.004f },
                { ParamIDs::envDecay, 1.6f },
                { ParamIDs::envSustain, 0.1f },
                { ParamIDs::envRelease, 2.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::reverbSize, 4.0f },
                { ParamIDs::reverbMix, 0.4f } } },

            { "Faded Rhodes", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.6f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 12 },
                { ParamIDs::fmAmount2, 0.3f },
                { ParamIDs::filterCutoff, 3800.0f },
                { ParamIDs::driveAmount, 0.15f },
                { ParamIDs::envAttack, 0.005f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.3f },
                { ParamIDs::envRelease, 1.1f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.35f },
                { ParamIDs::chorusRateHz, 0.4f },
                { ParamIDs::reverbSize, 2.5f },
                { ParamIDs::reverbMix, 0.28f } } },

            { "Attic Bells", "Keys",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.5f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.0f },
                { o(2, ParamIDs::oscSemitoneSuffix), 24 },
                { ParamIDs::fmAmount2, 0.45f },
                { ParamIDs::filterCutoff, 7500.0f },
                { ParamIDs::envAttack, 0.003f },
                { ParamIDs::envDecay, 2.0f },
                { ParamIDs::envSustain, 0.08f },
                { ParamIDs::envRelease, 2.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::reverbSize, 5.5f },
                { ParamIDs::reverbMix, 0.45f } } },

            // ============================== BASS (cont.) ========================
            { "Concrete Low", "Bass",
              { { o(1, ParamIDs::oscWaveSuffix), 3 },
                { o(1, ParamIDs::oscLevelSuffix), 0.8f },
                { o(1, ParamIDs::oscOctaveSuffix), -1 },
                { ParamIDs::filterCutoff, 650.0f },
                { ParamIDs::filterResonance, 3.5f },
                { ParamIDs::envAttack, 0.003f },
                { ParamIDs::envDecay, 0.25f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 0.3f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::driveAmount, 0.4f },
                { ParamIDs::saturatorAmount, 0.3f },
                { ParamIDs::saturatorAlgorithm, 3 }, // Hard
                { ParamIDs::saturatorCeiling, 0.75f },
                { ParamIDs::reverbMix, 0.08f } } },

            { "Tidepool", "Bass",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.8f },
                { o(1, ParamIDs::oscOctaveSuffix), -2 },
                { ParamIDs::filterCutoff, 450.0f },
                { ParamIDs::envAttack, 0.25f },
                { ParamIDs::envDecay, 0.6f },
                { ParamIDs::envSustain, 0.75f },
                { ParamIDs::envRelease, 1.0f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.2f },
                { ParamIDs::reverbSize, 2.5f },
                { ParamIDs::reverbMix, 0.18f } } },

            { "Rust Belt", "Bass",
              { { o(1, ParamIDs::oscWaveSuffix), 3 },
                { o(1, ParamIDs::oscLevelSuffix), 0.75f },
                { o(1, ParamIDs::oscOctaveSuffix), -1 },
                { o(2, ParamIDs::oscWaveSuffix), 2 },
                { o(2, ParamIDs::oscLevelSuffix), 0.25f },
                { o(2, ParamIDs::oscOctaveSuffix), -1 },
                { o(2, ParamIDs::oscFineSuffix), -8.0f },
                { ParamIDs::filterCutoff, 900.0f },
                { ParamIDs::filterResonance, 5.0f },
                { ParamIDs::envAttack, 0.004f },
                { ParamIDs::envDecay, 0.35f },
                { ParamIDs::envSustain, 0.55f },
                { ParamIDs::envRelease, 0.3f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.02f },
                { ParamIDs::driveAmount, 0.25f },
                { ParamIDs::crushBits, 10.0f },
                { ParamIDs::reverbMix, 0.1f } } },

            // ============================== AMBIENT (cont.) =====================
            { "Hollow Tide", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.3f },
                { ParamIDs::noiseType, 2 }, // Brown
                { ParamIDs::noiseLevel, 0.35f },
                { ParamIDs::filterCutoff, 900.0f },
                { ParamIDs::envAttack, 2.5f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 3.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 2 }, // FilterCutoff
                { ParamIDs::lfoRateHz, 0.05f },
                { ParamIDs::lfoDepth, 0.4f },
                { ParamIDs::reverbSize, 7.0f },
                { ParamIDs::reverbMix, 0.6f } } },

            { "Ash Fall", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.3f },
                { ParamIDs::noiseType, 1 }, // Pink
                { ParamIDs::noiseLevel, 0.25f },
                { ParamIDs::filterCutoff, 1400.0f },
                { ParamIDs::envAttack, 3.5f },
                { ParamIDs::envDecay, 1.5f },
                { ParamIDs::envSustain, 0.65f },
                { ParamIDs::envRelease, 4.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.3f },
                { ParamIDs::reverbSize, 8.0f },
                { ParamIDs::reverbMix, 0.75f } } },

            { "Radio Silence", "Ambient",
              { { o(1, ParamIDs::oscWaveSuffix), 2 },
                { o(1, ParamIDs::oscLevelSuffix), 0.25f },
                { ParamIDs::noiseType, 0 }, // White
                { ParamIDs::noiseLevel, 0.12f },
                { ParamIDs::filterType, 2 }, // Highpass
                { ParamIDs::filterCutoff, 350.0f },
                { ParamIDs::crushBits, 6.0f },
                { ParamIDs::crushDownsample, 8.0f },
                { ParamIDs::envAttack, 1.8f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 3.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::reverbSize, 6.5f },
                { ParamIDs::reverbMix, 0.55f } } },

            // ============================== TEXTURES ============================
            // A new category built specifically to show off the noise
            // generator -- wind, dust, tape hiss, rain, breath: the sounds
            // that need an unpitched source at their core, not just an
            // oscillator with a filter thrown at it.
            { "Vinyl Dust", "Textures",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.3f },
                { ParamIDs::noiseType, 2 }, // Brown
                { ParamIDs::noiseLevel, 0.2f },
                { ParamIDs::filterCutoff, 3500.0f },
                { ParamIDs::crushBits, 9.0f },
                { ParamIDs::crushDownsample, 3.0f },
                { ParamIDs::envAttack, 1.0f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 2.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::reverbSize, 3.5f },
                { ParamIDs::reverbMix, 0.35f } } },

            { "Wind Through Reeds", "Textures",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.15f },
                { ParamIDs::noiseType, 1 }, // Pink
                { ParamIDs::noiseLevel, 0.5f },
                { ParamIDs::filterType, 1 }, // Bandpass
                { ParamIDs::filterCutoff, 1100.0f },
                { ParamIDs::filterResonance, 3.5f },
                { ParamIDs::envAttack, 2.0f },
                { ParamIDs::envDecay, 1.0f },
                { ParamIDs::envSustain, 0.8f },
                { ParamIDs::envRelease, 3.0f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 2 }, // FilterCutoff
                { ParamIDs::lfoRateHz, 0.12f },
                { ParamIDs::lfoDepth, 0.6f },
                { ParamIDs::reverbSize, 6.0f },
                { ParamIDs::reverbMix, 0.5f } } },

            { "Tape Hiss Choir", "Textures",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.4f },
                { o(2, ParamIDs::oscWaveSuffix), 0 },
                { o(2, ParamIDs::oscLevelSuffix), 0.3f },
                { o(2, ParamIDs::oscFineSuffix), 6.0f },
                { ParamIDs::noiseType, 0 }, // White
                { ParamIDs::noiseLevel, 0.06f },
                { ParamIDs::filterCutoff, 2600.0f },
                { ParamIDs::envAttack, 1.5f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.7f },
                { ParamIDs::envRelease, 2.5f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::chorusMix, 0.4f },
                { ParamIDs::delaySyncDivision, 4 }, // 1/8
                { ParamIDs::delayMix, 0.25f },
                { ParamIDs::reverbSize, 5.0f },
                { ParamIDs::reverbMix, 0.5f } } },

            { "Rainfall Veil", "Textures",
              { { o(1, ParamIDs::oscWaveSuffix), 0 },
                { o(1, ParamIDs::oscLevelSuffix), 0.15f },
                { ParamIDs::noiseType, 1 }, // Pink
                { ParamIDs::noiseLevel, 0.6f },
                { ParamIDs::filterType, 1 }, // Bandpass
                { ParamIDs::filterCutoff, 3800.0f },
                { ParamIDs::filterResonance, 2.5f },
                { ParamIDs::envAttack, 0.8f },
                { ParamIDs::envDecay, 0.8f },
                { ParamIDs::envSustain, 0.75f },
                { ParamIDs::envRelease, 2.2f },
                { ParamIDs::voiceMode, 1 },
                { ParamIDs::lfoTarget, 2 },
                { ParamIDs::lfoRateHz, 0.2f },
                { ParamIDs::lfoDepth, 0.5f },
                { ParamIDs::reverbSize, 7.0f },
                { ParamIDs::reverbMix, 0.6f } } },

            { "Breath of Ash", "Textures",
              { { o(1, ParamIDs::oscWaveSuffix), 1 },
                { o(1, ParamIDs::oscLevelSuffix), 0.35f },
                { ParamIDs::noiseType, 2 }, // Brown
                { ParamIDs::noiseLevel, 0.4f },
                { ParamIDs::filterType, 0 }, // Lowpass
                { ParamIDs::filterCutoff, 1000.0f },
                { ParamIDs::filterResonance, 1.5f },
                { ParamIDs::driveAmount, 0.15f },
                { ParamIDs::envAttack, 0.6f },
                { ParamIDs::envDecay, 0.7f },
                { ParamIDs::envSustain, 0.6f },
                { ParamIDs::envRelease, 1.6f },
                { ParamIDs::voiceMode, 0 },
                { ParamIDs::glideTime, 0.12f },
                { ParamIDs::reverbSize, 4.5f },
                { ParamIDs::reverbMix, 0.4f } } },
        };
        return presets;
    }

    void PresetManager::setParamFromRealValue(juce::AudioProcessorValueTreeState& apvts, const juce::String& id,
                                               float realValue)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(realValue));
    }

    void PresetManager::initPatch(juce::AudioProcessorValueTreeState& apvts)
    {
        for (auto* param : apvts.processor.getParameters())
            param->setValueNotifyingHost(param->getDefaultValue());
    }

    void PresetManager::applyFactoryPreset(juce::AudioProcessorValueTreeState& apvts, int index)
    {
        const auto& presets = getFactoryPresets();
        if (index < 0 || index >= (int) presets.size())
            return;

        initPatch(apvts); // every unlisted parameter falls back to its default
        for (const auto& p : presets[(size_t) index].overrides)
            setParamFromRealValue(apvts, p.id, p.value);
    }

    void PresetManager::randomizePatch(juce::AudioProcessorValueTreeState& apvts, juce::Random& random)
    {
        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
                if (isRandomizable(ranged->paramID))
                    ranged->setValueNotifyingHost(random.nextFloat());
        }
    }

    juce::File PresetManager::getUserPresetDirectory()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                       .getChildFile("gexex")
                       .getChildFile("Gexex Synth")
                       .getChildFile("Presets");
        dir.createDirectory();
        return dir;
    }

    bool PresetManager::saveUserPreset(juce::AudioProcessorValueTreeState& apvts, const juce::String& name)
    {
        auto file = getUserPresetDirectory().getChildFile(name + ".xml");
        if (auto state = apvts.copyState(); auto xml = state.createXml())
            return xml->writeTo(file);
        return false;
    }

    juce::StringArray PresetManager::getUserPresetNames()
    {
        juce::StringArray names;
        for (const auto& file :
             getUserPresetDirectory().findChildFiles(juce::File::findFiles, false, "*.xml"))
            names.add(file.getFileNameWithoutExtension());
        names.sort(true);
        return names;
    }

    bool PresetManager::loadUserPreset(juce::AudioProcessorValueTreeState& apvts, const juce::String& name)
    {
        auto file = getUserPresetDirectory().getChildFile(name + ".xml");
        if (! file.existsAsFile())
            return false;
        if (auto xml = juce::XmlDocument::parse(file))
        {
            if (xml->hasTagName(apvts.state.getType()))
            {
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
                return true;
            }
        }
        return false;
    }

    bool PresetManager::deleteUserPreset(const juce::String& name)
    {
        return getUserPresetDirectory().getChildFile(name + ".xml").deleteFile();
    }
}
