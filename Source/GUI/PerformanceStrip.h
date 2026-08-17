#pragma once

#include <array>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../ScopeDataSource.h"

namespace gexex
{
    // A fixed strip docked directly above the on-screen keyboard --
    // unlike BackgroundScene (which sits behind the scrollable
    // ModuleRack and scrolls out of view with it), this is always
    // visible, "sticky" the way the grass foreground reads in the
    // browser reference. Carries the gexex grass foreground art plus a
    // wide, glowing live trace of the master output -- the "what am I
    // actually playing" animation, since it's literally the audio
    // reaching the DAW, brightening with it in real time rather than a
    // canned animation. (The mirrored-daisy sprite that used to sit here
    // was dropped -- at this strip's now-skinnier size it read as
    // clutter rather than detail.)
    class PerformanceStrip : public juce::Component, private juce::Timer
    {
    public:
        explicit PerformanceStrip(const ScopeDataSource<>& masterScopeSource);

        void paint(juce::Graphics& g) override;

    private:
        void timerCallback() override;
        void drawTiledLayer(juce::Graphics& g, const juce::Image& img, juce::Rectangle<int> area) const;
        static juce::Path buildTracePath(const float* samples, juce::Rectangle<float> traceArea);

        const ScopeDataSource<>& scopeSource;
        juce::Image grassImage;

        // A short trailing history of past frames, captured at a slower
        // cadence than the 30Hz repaint -- drawn beneath the live trace at
        // decreasing alpha/hue for a "phosphor comet" effect (ghost copies
        // of recent motion fading out behind the current line), rather
        // than a single static stroke.
        static constexpr int trailLength = 4;
        static constexpr int captureEveryNTicks = 3; // ~10Hz trail capture off the 30Hz timer
        std::array<std::array<float, ScopeDataSource<>::size>, trailLength> trailSamples {};
        int trailWriteIndex = 0;
        int tickCounter = 0;
    };
}
