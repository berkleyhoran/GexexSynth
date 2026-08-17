#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace gexex
{
    // The gexex "aero" scene (sky gradient, two parallax cloud layers,
    // rising bubbles, a grass foreground strip, mirrored daisies) ported
    // from synth.html's CSS background into a JUCE Component. Plain
    // Timer-driven `Graphics::drawImage` compositing rather than OpenGL --
    // see the build plan's §6.3: this is a handful of static PNG layers
    // needing simple 2D compositing/scroll/opacity, not the procedural
    // GPU-shader workload Kaleidosonic's OpenGL usage exists for, so
    // standing up a context here would be disproportionate machinery for
    // no real benefit.
    //
    // Reads the reducedMotion APVTS bool itself (rather than needing
    // PluginEditor to push it in) and, when set, drops from a 30Hz
    // animation timer to a 2Hz idle poll that only checks whether the
    // param changed back -- mirrors the browser's prefers-reduced-motion
    // handling, but as an explicit in-plugin toggle since a VST3 has no
    // OS-level signal to key off automatically.
    class BackgroundScene : public juce::Component, private juce::Timer
    {
    public:
        explicit BackgroundScene(juce::AudioProcessorValueTreeState& apvtsToUse);

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        void timerCallback() override;
        void drawTiledLayer(juce::Graphics& g, const juce::Image& img, juce::Rectangle<int> area, float xOffset,
                             float opacity) const;

        struct Bubble
        {
            float xFraction;
            float sizePx;
            float cycleSeconds;
            float phaseSeconds;
        };

        juce::AudioProcessorValueTreeState& apvts;

        juce::Image cloudsA, cloudsB, bubbleImage, daisyImage, grassImage;
        std::vector<Bubble> bubbles;
        juce::Random random;

        double elapsedSeconds = 0.0;
        float cloudOffsetA = 0.0f;
        float cloudOffsetB = 0.0f;
        bool reducedMotion = false;
    };
}
