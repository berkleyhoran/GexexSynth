#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../ScopeDataSource.h"

namespace gexex
{
    // A reusable mini-oscilloscope: reads a ScopeDataSource on a ~30Hz
    // timer and draws it as a stroked polyline. Used once per oscillator
    // (fed from SynthEngine's "monitor voice", see SynthEngine.h) and once
    // for the master output.
    class Scope : public juce::Component, private juce::Timer
    {
    public:
        explicit Scope(juce::Colour lineColour) : colour(lineColour) { startTimerHz(30); }

        // Not owned -- ScopeDataSource instances live on PluginProcessor,
        // which outlives every editor that might point a Scope at them.
        void setSource(const ScopeDataSource<>* newSource) noexcept { source = newSource; }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(juce::Colours::white.withAlpha(0.55f));
            g.fillRoundedRectangle(bounds, 6.0f);

            if (source == nullptr)
                return;

            float samples[ScopeDataSource<>::size];
            source->copyOut(samples);

            const float midY = bounds.getCentreY();
            const float ampScale = bounds.getHeight() * 0.46f;
            juce::Path path;
            for (int i = 0; i < ScopeDataSource<>::size; ++i)
            {
                const float x =
                    bounds.getX() + bounds.getWidth() * (float) i / (float) (ScopeDataSource<>::size - 1);
                const float y = midY - juce::jlimit(-1.2f, 1.2f, samples[i]) * ampScale;
                if (i == 0)
                    path.startNewSubPath(x, y);
                else
                    path.lineTo(x, y);
            }
            g.setColour(colour);
            g.strokePath(path, juce::PathStrokeType(1.3f));
        }

    private:
        void timerCallback() override { repaint(); }

        const ScopeDataSource<>* source = nullptr;
        juce::Colour colour;
    };
}
