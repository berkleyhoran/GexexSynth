#include "PluginProcessor.h"
#include "PluginEditor.h"

GexexSynthAudioProcessorEditor::GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), backgroundScene(p.apvts),
      moduleRack(p.apvts, p.getOscScope(0), p.getOscScope(1), p.getOscScope(2), p.getMasterScope()),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(backgroundScene); // added first -- paints behind everything else
    addAndMakeVisible(moduleRack);

    keyboard.setAvailableRange(24, 108); // C1 - C8, comfortably covers this synth's practical range
    keyboard.setKeyWidth(18.0f);
    addAndMakeVisible(keyboard);

    setResizable(true, true);
    setResizeLimits(760, 520, 2600, 1800);
    setSize(1100, 760);
}

GexexSynthAudioProcessorEditor::~GexexSynthAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GexexSynthAudioProcessorEditor::paint(juce::Graphics&)
{
    // BackgroundScene (a full-bounds child, painted first) handles all of
    // the actual drawing now.
}

void GexexSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    backgroundScene.setBounds(bounds);
    keyboard.setBounds(bounds.removeFromBottom(80));
    moduleRack.setBounds(bounds);
}
