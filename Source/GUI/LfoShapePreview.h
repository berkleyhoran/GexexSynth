#pragma once

#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace gexex
{
    // A small shape preview for the LFO's selected waveform. Unlike Scope
    // (which captures an audio-thread signal via a ring buffer), there's
    // nothing to capture here -- the waveform is fully determined by
    // already-known parameter state, so this just draws one cycle of it
    // directly (see the build plan's §6.4).
    class LfoShapePreview : public juce::Component, private juce::Timer
    {
    public:
        LfoShapePreview(juce::AudioProcessorValueTreeState& apvtsToUse, juce::String waveformParamID,
                         juce::Colour lineColour)
            : apvts(apvtsToUse), paramID(std::move(waveformParamID)), colour(lineColour)
        {
            startTimerHz(24);
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(juce::Colours::white.withAlpha(0.55f));
            g.fillRoundedRectangle(bounds, 6.0f);

            const int waveIndex = (int) apvts.getRawParameterValue(paramID)->load();
            const float midY = bounds.getCentreY();
            const float amp = bounds.getHeight() * 0.4f;

            juce::Path path;
            constexpr int steps = 64;
            for (int i = 0; i <= steps; ++i)
            {
                const float t = (float) i / (float) steps;
                const float v = renderShape(waveIndex, t);
                const float x = bounds.getX() + bounds.getWidth() * t;
                const float y = midY - v * amp;
                if (i == 0)
                    path.startNewSubPath(x, y);
                else
                    path.lineTo(x, y);
            }
            g.setColour(colour);
            g.strokePath(path, juce::PathStrokeType(1.4f));

            // Phase-indicator dot -- decorative only, not synced to the
            // real LFO's actual phase (that state lives on the audio
            // thread's Lfo instance, not exposed here).
            const float dotT = std::fmod(phase, 1.0f);
            const float dotV = renderShape(waveIndex, dotT);
            g.setColour(juce::Colours::white);
            g.fillEllipse(bounds.getX() + bounds.getWidth() * dotT - 3.0f, midY - dotV * amp - 3.0f, 6.0f, 6.0f);
        }

    private:
        // Item order matches gexex::LfoWaveform exactly (Sine, Square, Saw, Triangle).
        static float renderShape(int waveIndex, float t) noexcept
        {
            switch (waveIndex)
            {
                case 0: return std::sin(juce::MathConstants<float>::twoPi * t);
                case 1: return t < 0.5f ? 1.0f : -1.0f;
                case 2: return 2.0f * t - 1.0f;
                case 3: return -(4.0f * std::abs(t - 0.5f) - 1.0f);
                default: return 0.0f;
            }
        }

        void timerCallback() override
        {
            phase += 1.0f / 24.0f / 2.0f; // ~2s per cycle -- purely decorative pacing
            repaint();
        }

        juce::AudioProcessorValueTreeState& apvts;
        juce::String paramID;
        juce::Colour colour;
        float phase = 0.0f;
    };
}
