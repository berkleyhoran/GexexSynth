#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Phase 0 placeholder editor: just proves the plugin loads and paints in a
// host. Phase 4 replaces this with the real "fruity aero" ModuleRack +
// custom LookAndFeel.
class GexexSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor&);
    ~GexexSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GexexSynthAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GexexSynthAudioProcessorEditor)
};
