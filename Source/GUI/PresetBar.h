#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

namespace gexex
{
    // The toolbar sitting above the module rack: prev/next + a preset
    // combo (factory presets grouped by category, user presets in their
    // own section below), Save As..., Randomize, Init, and Panic --
    // ported conceptually from the reference's toolbar (see the build
    // plan's §5).
    class PresetBar : public juce::Component
    {
    public:
        explicit PresetBar(GexexSynthAudioProcessor& processorToUse);

        void resized() override;
        void paint(juce::Graphics& g) override;

    private:
        void rebuildComboItems();
        void loadSelectedItem();
        void stepPreset(int direction);
        void showSaveDialog();

        GexexSynthAudioProcessor& processor;

        juce::ComboBox presetBox;
        juce::TextButton prevButton { "<" }, nextButton { ">" };
        juce::TextButton saveButton { "Save As..." };
        juce::TextButton randomizeButton { "Randomize" };
        juce::TextButton initButton { "Init" };
        juce::TextButton panicButton { "Panic" };

        juce::Random random;
        juce::StringArray userPresetNames; // index i -> user preset name for combo id (userIdOffset + i)
        juce::Array<int> validIds;         // every real (non-heading) combo id, in display order -- for prev/next
        static constexpr int userIdOffset = 10000;
    };
}
