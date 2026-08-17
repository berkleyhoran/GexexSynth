#include "PluginProcessor.h"
#include "PluginEditor.h"

GexexSynthAudioProcessorEditor::GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), moduleRack(p.apvts),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lookAndFeel);

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

void GexexSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Placeholder sky gradient -- Phase 5's BackgroundScene (parallax
    // clouds/bubbles/grass/daisies from gexex/assets) replaces this.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffbfe4fb), 0.0f, 0.0f, juce::Colour(0xffffffff), 0.0f,
                                            (float) getHeight(), false));
    g.fillAll();
}

void GexexSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    keyboard.setBounds(bounds.removeFromBottom(80));
    moduleRack.setBounds(bounds);
}
