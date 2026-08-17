#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

namespace gexex
{
    struct PresetParam
    {
        juce::String id;
        float value; // real (denormalized) units -- Hz, seconds, 0..1, semitones, whatever that parameter's native range is
    };

    struct FactoryPreset
    {
        juce::String name;
        juce::String category;
        std::vector<PresetParam> overrides; // only params that differ from default -- initPatch() fills the rest
    };

    // Master volume/pan and the reduced-motion toggle are excluded from
    // randomize (matching the reference -- see the build plan's §5) --
    // co-located with the rest of PresetManager rather than buried in
    // PluginProcessor so the exclusion list can't silently drift out of
    // sync with new parameters added elsewhere.
    bool isRandomizable(const juce::String& paramID);

    class PresetManager
    {
    public:
        // Twenty hand-authored patches across five categories (Pads,
        // Leads, Keys, Bass, Ambient), all leaning into the same
        // melancholy/reverby/dreamy character -- see PresetManager.cpp
        // for the full list.
        static const std::vector<FactoryPreset>& getFactoryPresets();

        static void applyFactoryPreset(juce::AudioProcessorValueTreeState& apvts, int index);
        static void initPatch(juce::AudioProcessorValueTreeState& apvts);
        static void randomizePatch(juce::AudioProcessorValueTreeState& apvts, juce::Random& random);

        // User presets: full APVTS-state XML dumps (not partial overrides
        // like the factory list) in a per-OS user documents folder --
        // plain files a user could hand-copy/share, same idiom as most
        // JUCE plugins' preset folders.
        static juce::File getUserPresetDirectory();
        static bool saveUserPreset(juce::AudioProcessorValueTreeState& apvts, const juce::String& name);
        static juce::StringArray getUserPresetNames();
        static bool loadUserPreset(juce::AudioProcessorValueTreeState& apvts, const juce::String& name);
        static bool deleteUserPreset(const juce::String& name);

    private:
        static void setParamFromRealValue(juce::AudioProcessorValueTreeState& apvts, const juce::String& id,
                                           float realValue);
    };
}
