#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ModuleCard.h"
#include "SectionHeader.h"
#include "EnvelopeEditor.h"
#include "ReverbVisual.h"
#include "DelayVisual.h"
#include "Scope.h"
#include "LfoShapePreview.h"
#include "../ScopeDataSource.h"

namespace gexex
{
    // Builds every module card (minus the dropped sequencer/drum-machine/
    // chopper) grouped into labeled sections (Sound Source, Modulation,
    // Effects, Master) and flows them into a wrapping grid -- a
    // SectionHeader forces a row break before and after itself so each
    // group reads as clearly separated, not one undifferentiated wall of
    // cards -- inside a Viewport so the whole rack scrolls.
    class ModuleRack : public juce::Component, private juce::Timer
    {
    public:
        // Scope sources are references, not owned -- they live on
        // PluginProcessor, which outlives every editor.
        ModuleRack(juce::AudioProcessorValueTreeState& apvtsToUse, const ScopeDataSource<>& osc1Scope,
                   const ScopeDataSource<>& osc2Scope, const ScopeDataSource<>& osc3Scope,
                   const ScopeDataSource<>& masterScopeSource);

        void resized() override;

    private:
        void timerCallback() override;
        ModuleCard& addCard(const juce::String& title, int categoryIndex);
        void addSection(const juce::String& title, juce::Colour accent);
        void layOutCards();

        juce::AudioProcessorValueTreeState& apvts;
        juce::Viewport viewport;
        juce::Component content; // scrolled child; cards/headers are parented here

        struct FlowItem
        {
            juce::Component* component;
            bool isSectionHeader;
        };
        std::vector<FlowItem> flowItems;

        juce::OwnedArray<ModuleCard> cards;
        juce::OwnedArray<SectionHeader> sectionHeaders;
        std::unique_ptr<EnvelopeEditor> envelopeEditor;
        std::unique_ptr<ReverbVisual> reverbVisual;
        std::unique_ptr<DelayVisual> delayVisual;
        std::unique_ptr<LfoShapePreview> lfoShapePreview;
        juce::OwnedArray<Scope> scopes;

        static constexpr int cardWidth = 220;
        static constexpr int cardGap = 10;
        static constexpr int sectionHeaderHeight = 30;
    };
}
