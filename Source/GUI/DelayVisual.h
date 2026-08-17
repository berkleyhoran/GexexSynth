#pragma once

#include <cmath>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

namespace gexex
{
    // A left-to-right timeline of cascading echo dots standing in for the
    // Delay module's time/feedback/mix knobs: dot spacing/travel speed
    // reflects the actual delay time, dot count and fade reflect feedback,
    // and overall brightness reflects mix -- so sweeping any of those
    // three knobs visibly changes the animation, not just a label.
    class DelayVisual : public juce::Component, private juce::Timer
    {
    public:
        DelayVisual() { startTimerHz(30); }

        void setTimeSeconds(float t) noexcept { timeSeconds = juce::jmax(0.02f, t); }
        void setFeedback01(float fb) noexcept { feedback01 = juce::jlimit(0.0f, 0.97f, fb); }
        void setMix01(float m) noexcept { mix01 = juce::jlimit(0.0f, 1.0f, m); }
        void setAccentColour(juce::Colour c) noexcept { accent = c; }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.fillRoundedRectangle(bounds, 8.0f);

            const float midY = bounds.getCentreY();
            g.setColour(juce::Colour(0x18163049));
            g.drawHorizontalLine((int) midY, bounds.getX() + 6.0f, bounds.getRight() - 6.0f);

            constexpr int maxTaps = 8;
            const float cycleFraction = std::fmod(elapsedSeconds / timeSeconds, 1.0f);
            const float usableWidth = bounds.getWidth() - 20.0f;

            for (int i = 0; i < maxTaps; ++i)
            {
                const float tapGain = std::pow(feedback01, (float) i);
                if (tapGain < 0.03f)
                    break;

                const float travel = ((float) i + cycleFraction) / (float) maxTaps;
                if (travel > 1.0f)
                    continue;

                const float x = bounds.getX() + 10.0f + usableWidth * travel;
                const float alpha = juce::jlimit(0.0f, 1.0f, tapGain * (0.25f + mix01 * 0.9f));
                const float radius = juce::jmap(tapGain, 0.0f, 1.0f, 3.5f, 12.0f);

                g.setColour(accent.withAlpha(alpha));
                g.fillEllipse(x - radius * 0.5f, midY - radius * 0.5f, radius, radius);
            }

            // Source pulse at the left edge, flashing once per cycle.
            const float sourcePulse = 1.0f - cycleFraction;
            g.setColour(accent.withAlpha(juce::jlimit(0.0f, 0.9f, sourcePulse * 0.9f)));
            g.fillEllipse(bounds.getX() + 4.0f, midY - 5.0f, 10.0f, 10.0f);
        }

    private:
        void timerCallback() override
        {
            elapsedSeconds += 1.0f / 30.0f;
            repaint();
        }

        float timeSeconds = 0.28f;
        float feedback01 = 0.3f;
        float mix01 = 0.0f;
        float elapsedSeconds = 0.0f;
        juce::Colour accent = GexexLookAndFeel::categoryColour(8);
    };
}
