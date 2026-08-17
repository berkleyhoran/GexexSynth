#include "PluginProcessor.h"
#include "PluginEditor.h"

GexexSynthAudioProcessorEditor::GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), backgroundScene(p.apvts), presetBar(p),
      moduleRack(p.apvts, p.getOscScope(0), p.getOscScope(1), p.getOscScope(2), p.getMasterScope()),
      performanceStrip(p.getMasterScope()),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(backgroundScene); // added first -- paints behind everything else
    addAndMakeVisible(presetBar);
    addAndMakeVisible(moduleRack);
    addAndMakeVisible(performanceStrip);

    // The full 88-key range, "long" enough that JUCE's own built-in
    // horizontal scroll-into-view handles panning as you play near either
    // end -- rather than a cramped range trimmed to only what fits, the
    // point being to actually *see* a real keyboard, not just a playable
    // strip.
    keyboard.setAvailableRange(21, 108); // A0 - C8
    keyboard.setKeyWidth(20.0f);
    keyboard.setLowestVisibleKey(36);
    addAndMakeVisible(keyboard);

    setResizable(true, true);
    setResizeLimits(760, 560, 2600, 1900);
    setSize(1100, 820);
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
    keyboard.setBounds(bounds.removeFromBottom(120));
    performanceStrip.setBounds(bounds.removeFromBottom(96));
    presetBar.setBounds(bounds.removeFromTop(44).reduced(8, 6));
    backgroundScene.setBounds(bounds); // sky/clouds behind the scrollable rack area only
    moduleRack.setBounds(bounds);
}
