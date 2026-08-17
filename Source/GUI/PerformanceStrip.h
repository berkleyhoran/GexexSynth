#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../ScopeDataSource.h"

namespace gexex
{
    // A fixed strip docked directly above the on-screen keyboard --
    // unlike BackgroundScene (which sits behind the scrollable
    // ModuleRack and scrolls out of view with it), this is always
    // visible, "sticky" the way the grass/daisy foreground reads in the
    // browser reference. Carries the gexex grass + mirrored-daisy
    // foreground art plus a wide, glowing live trace of the master
    // output -- the "what am I actually playing" animation, since it's
    // literally the audio reaching the DAW, brightening with it in real
    // time rather than a canned animation.
    class PerformanceStrip : public juce::Component, private juce::Timer
    {
    public:
        explicit PerformanceStrip(const ScopeDataSource<>& masterScopeSource);

        void paint(juce::Graphics& g) override;

    private:
        void timerCallback() override;
        void drawTiledLayer(juce::Graphics& g, const juce::Image& img, juce::Rectangle<int> area) const;

        const ScopeDataSource<>& scopeSource;
        juce::Image grassImage, daisyImage;
    };
}
