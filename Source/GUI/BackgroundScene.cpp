#include "BackgroundScene.h"
#include "../Parameters.h"
#include <BinaryData.h>

namespace gexex
{
    BackgroundScene::BackgroundScene(juce::AudioProcessorValueTreeState& apvtsToUse) : apvts(apvtsToUse)
    {
        cloudsA = juce::ImageFileFormat::loadFrom(BinaryData::clouds_a_png, (size_t) BinaryData::clouds_a_pngSize);
        cloudsB = juce::ImageFileFormat::loadFrom(BinaryData::clouds_b_png, (size_t) BinaryData::clouds_b_pngSize);
        bubbleImage = juce::ImageFileFormat::loadFrom(BinaryData::bubble_png, (size_t) BinaryData::bubble_pngSize);

        // A fixed pool of bubbles, each with its own randomized column/
        // size/cycle-length/phase (mirrors synth.html's per-bubble
        // randomized left%/size/duration/delay), continuously recycled by
        // computing position from elapsed-time-modulo-cycle rather than
        // integrating per-frame state -- deterministic, no drift.
        for (int i = 0; i < 9; ++i)
        {
            bubbles.push_back({ random.nextFloat(), 18.0f + random.nextFloat() * 34.0f,
                                 11.0f + random.nextFloat() * 10.0f, random.nextFloat() * 20.0f });
        }

        setInterceptsMouseClicks(false, false); // purely decorative, never steals clicks from the rack above it
        startTimerHz(30);
    }

    void BackgroundScene::resized() {}

    void BackgroundScene::drawTiledLayer(juce::Graphics& g, const juce::Image& img, juce::Rectangle<int> area,
                                          float xOffset, float opacity) const
    {
        if (! img.isValid() || area.getHeight() <= 0)
            return;

        const float aspect = (float) img.getWidth() / (float) img.getHeight();
        const int drawH = area.getHeight();
        const int drawW = juce::jmax(1, (int) (drawH * aspect));

        g.setOpacity(opacity);
        int x = area.getX() - (((int) xOffset % drawW) + drawW) % drawW - drawW;
        for (; x < area.getRight(); x += drawW)
            g.drawImage(img, x, area.getY(), drawW, drawH, 0, 0, img.getWidth(), img.getHeight());
        g.setOpacity(1.0f);
    }

    void BackgroundScene::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffbfe4fb), 0.0f, 0.0f, juce::Colour(0xffffffff), 0.0f,
                                                (float) bounds.getHeight(), false));
        g.fillAll();

        // Two parallax cloud layers, drifting at different speeds in
        // opposite directions -- mirrors the CSS's cloudDrift keyframes
        // (one layer `reverse`d) via two independently-advancing offsets.
        drawTiledLayer(g, cloudsA, bounds.withHeight((int) (bounds.getHeight() * 0.55f)), cloudOffsetA, 0.85f);
        drawTiledLayer(g, cloudsB,
                       bounds.withY((int) (bounds.getHeight() * 0.42f)).withHeight((int) (bounds.getHeight() * 0.3f)),
                       cloudOffsetB, 0.45f);

        // Rising bubbles: position and opacity are pure functions of
        // (elapsedSeconds, per-bubble phase/cycle) -- see the constructor.
        for (const auto& b : bubbles)
        {
            const float t = std::fmod((float) elapsedSeconds + b.phaseSeconds, b.cycleSeconds) / b.cycleSeconds;
            const float x = b.xFraction * (float) bounds.getWidth();
            const float y = (float) bounds.getHeight() * (1.12f - t * 1.3f);
            float alpha = 0.0f;
            if (t < 0.1f)
                alpha = t / 0.1f * 0.8f;
            else if (t < 0.7f)
                alpha = 0.8f;
            else
                alpha = juce::jmax(0.0f, 0.8f * (1.0f - (t - 0.7f) / 0.3f));

            if (alpha > 0.01f && bubbleImage.isValid())
            {
                g.setOpacity(alpha);
                g.drawImage(bubbleImage, (int) x, (int) y, (int) b.sizePx, (int) b.sizePx, 0, 0,
                            bubbleImage.getWidth(), bubbleImage.getHeight());
            }
        }
        g.setOpacity(1.0f);
    }

    void BackgroundScene::timerCallback()
    {
        const bool nowReduced = apvts.getRawParameterValue(ParamIDs::reducedMotion)->load() >= 0.5f;
        if (nowReduced != reducedMotion)
        {
            reducedMotion = nowReduced;
            startTimerHz(reducedMotion ? 2 : 30);
            repaint();
        }

        if (reducedMotion)
            return; // idle poll only -- no animation advance, no repaint spam

        elapsedSeconds += 1.0 / 30.0;
        cloudOffsetA += 0.25f;
        cloudOffsetB -= 0.12f;
        repaint();
    }
}
