#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel.h"
#include "GUI/ModuleRack.h"
#include "GUI/BackgroundScene.h"
#include "GUI/PerformanceStrip.h"
#include "GUI/PresetBar.h"
#include "GUI/GexexKeyboard.h"

// Phase 4: the real "fruity aero" GUI -- candy-knob ModuleRack (every
// module from the reference minus the dropped sequencer/drum-machine/
// chopper) plus an on-screen keyboard, replacing Phase 1-3's generic
// APVTS editor. The parallax cloud/bubble/grass/daisy background and
// mini-oscilloscopes land in Phase 5; this is a placeholder sky gradient
// in the meantime.
class GexexSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor&);
    ~GexexSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GexexSynthAudioProcessor& processorRef;
    gexex::GexexLookAndFeel lookAndFeel;
    gexex::BackgroundScene backgroundScene;
    gexex::PresetBar presetBar;
    gexex::ModuleRack moduleRack;
    gexex::GexexKeyboard keyboard;
    gexex::PerformanceStrip performanceStrip; // added/painted last -- grass sits in front of everything, incl. the keyboard

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GexexSynthAudioProcessorEditor)
};
