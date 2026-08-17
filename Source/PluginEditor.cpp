#include "PluginProcessor.h"
#include "PluginEditor.h"

GexexSynthAudioProcessorEditor::GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(900, 600);
}

GexexSynthAudioProcessorEditor::~GexexSynthAudioProcessorEditor() = default;

void GexexSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Placeholder sky-gradient nod to the "fruity aero" look coming in
    // Phase 4/5 -- real BackgroundScene (clouds/bubbles/grass/daisies) and
    // ModuleRack land later.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffbfe4fb), 0.0f, 0.0f,
                                            juce::Colour(0xffffffff), 0.0f, (float) getHeight(), false));
    g.fillAll();

    g.setColour(juce::Colour(0xff163049));
    g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    g.drawText("gexex synth", getLocalBounds(), juce::Justification::centred);
}

void GexexSynthAudioProcessorEditor::resized()
{
}
