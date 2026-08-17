#pragma once

#include <cmath>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

namespace gexex
{
    // A stylized hallway receding to a vanishing point, standing in for
    // the Reverb module's size/mix knobs: size widens/deepens the
    // hallway, mix brightens the glow and how visible the rings are.
    // Idly animated (concentric rings pulse outward) so it reads as
    // "space," not a static diagram, even before Phase 5 wires it to real
    // audio metering.
    class ReverbVisual : public juce::Component, private juce::Timer
    {
    public:
        ReverbVisual() { startTimerHz(30); }

        void setSize01(float s) noexcept { size01 = juce::jlimit(0.0f, 1.0f, s); }
        void setMix01(float m) noexcept { mix01 = juce::jlimit(0.0f, 1.0f, m); }
        void setAccentColour(juce::Colour c) noexcept { accent = c; }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.fillRoundedRectangle(bounds, 8.0f);

            const float vpX = bounds.getCentreX();
            const float vpY = bounds.getY() + bounds.getHeight() * 0.2f;
            constexpr int numRings = 6;

            for (int i = numRings; i >= 1; --i)
            {
                const float depth = (float) i / (float) numRings; // 1 = far/small, ~0 = close/large
                const float w = juce::jmap(depth, 1.0f, 0.12f, bounds.getWidth() * 0.12f,
                                            bounds.getWidth() * (0.35f + size01 * 0.6f));
                const float h = juce::jmap(depth, 1.0f, 0.12f, bounds.getHeight() * 0.1f, bounds.getHeight() * 0.92f);
                juce::Rectangle<float> r(w, h);
                r.setCentre(vpX, vpY + h * 0.42f);

                const float pulse = 0.5f + 0.5f * std::sin(phase - depth * 3.1f);
                const float alpha = juce::jlimit(0.04f, 0.55f, mix01 * pulse * (1.0f - depth * 0.6f) + 0.04f);
                g.setColour(accent.withAlpha(alpha));
                g.drawRoundedRectangle(r, 5.0f, 1.4f);
            }

            juce::ColourGradient glow(juce::Colours::white.withAlpha(0.3f * mix01 + 0.05f), vpX, vpY,
                                       juce::Colours::transparentWhite, vpX, vpY + bounds.getHeight() * 0.35f, true);
            g.setGradientFill(glow);
            g.fillEllipse(vpX - 30, vpY - 16, 60, 46);
        }

    private:
        void timerCallback() override
        {
            phase += 0.045f;
            repaint();
        }

        float size01 = 0.3f;
        float mix01 = 0.0f;
        float phase = 0.0f;
        juce::Colour accent = GexexLookAndFeel::categoryColour(9);
    };
}
