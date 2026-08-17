#include "PluginProcessor.h"
#include "PluginEditor.h"

GexexSynthAudioProcessorEditor::GexexSynthAudioProcessorEditor(GexexSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), backgroundScene(p.apvts), presetBar(p),
      moduleRack(p.apvts, p.getOscScope(0), p.getOscScope(1), p.getOscScope(2), p.getMasterScope()),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      performanceStrip(p.getMasterScope())
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(backgroundScene); // added first -- paints behind everything else
    addAndMakeVisible(presetBar);
    addAndMakeVisible(moduleRack);

    // The full 88-key range, "long" enough that JUCE's own built-in
    // horizontal scroll-into-view handles panning as you play near either
    // end -- rather than a cramped range trimmed to only what fits, the
    // point being to actually *see* a real keyboard, not just a playable
    // strip.
    keyboard.setAvailableRange(21, 108); // A0 - C8
    keyboard.setKeyWidth(22.0f);
    keyboard.setLowestVisibleKey(36);
    addAndMakeVisible(keyboard);

    // Added last: PerformanceStrip paints on top of both the rack and the
    // keyboard, so the grass's naturally-transparent blade tips can
    // overlap the bottom of the module cards above it and its solid base
    // can overlap the top edge of the keys below it -- "grass growing in
    // front of everything," per this session's request.
    addAndMakeVisible(performanceStrip);

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

    constexpr int keyboardHeight = 130;
    constexpr int stripOverlapIntoRack = 110;   // grass's transparent blade tips reach up over the cards
    constexpr int stripOverlapIntoKeyboard = 28; // grass's solid base dips slightly over the keys' top edge
    constexpr int stripHeight = stripOverlapIntoRack + stripOverlapIntoKeyboard;

    keyboard.setBounds(bounds.removeFromBottom(keyboardHeight));
    presetBar.setBounds(bounds.removeFromTop(44).reduced(8, 6));

    backgroundScene.setBounds(bounds); // sky/clouds behind the scrollable rack area
    moduleRack.setBounds(bounds);      // the rack keeps its full normal space; the strip below just paints over
                                        // its bottom edge (z-order), it isn't actually squeezed smaller for this

    // Straddles the rack/keyboard boundary rather than sitting flush
    // above the keyboard -- see the constants above.
    const int stripBottom = getHeight() - keyboardHeight + stripOverlapIntoKeyboard;
    performanceStrip.setBounds(0, stripBottom - stripHeight, getWidth(), stripHeight);
}
