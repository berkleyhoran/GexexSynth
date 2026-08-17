#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ModuleCard.h"
#include "EnvelopeEditor.h"
#include "ReverbVisual.h"
#include "DelayVisual.h"

namespace gexex
{
    // Builds every module card (minus the dropped sequencer/drum-machine/
    // chopper) in signal-flow order and flows them into a wrapping grid,
    // inside a Viewport so the whole rack scrolls rather than needing a
    // single window tall/wide enough for ~70 parameters at once.
    class ModuleRack : public juce::Component, private juce::Timer
    {
    public:
        explicit ModuleRack(juce::AudioProcessorValueTreeState& apvtsToUse);

        void resized() override;

    private:
        void timerCallback() override;
        ModuleCard& addCard(const juce::String& title, int categoryIndex);
        void layOutCards();

        juce::AudioProcessorValueTreeState& apvts;
        juce::Viewport viewport;
        juce::Component content; // scrolled child; cards are parented here

        juce::OwnedArray<ModuleCard> cards;
        std::unique_ptr<EnvelopeEditor> envelopeEditor;
        std::unique_ptr<ReverbVisual> reverbVisual;
        std::unique_ptr<DelayVisual> delayVisual;

        static constexpr int cardWidth = 220;
        static constexpr int cardGap = 10;
    };
}
