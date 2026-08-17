#include "PerformanceStrip.h"
#include <BinaryData.h>

namespace gexex
{
    PerformanceStrip::PerformanceStrip(const ScopeDataSource<>& masterScopeSource) : scopeSource(masterScopeSource)
    {
        grassImage = juce::ImageFileFormat::loadFrom(BinaryData::grass_png, (size_t) BinaryData::grass_pngSize);
        setInterceptsMouseClicks(false, false);
        startTimerHz(30);
    }

    void PerformanceStrip::drawTiledLayer(juce::Graphics& g, const juce::Image& img,
                                           juce::Rectangle<int> area) const
    {
        if (! img.isValid() || area.getHeight() <= 0)
            return;
        const float aspect = (float) img.getWidth() / (float) img.getHeight();
        const int drawH = area.getHeight();
        const int drawW = juce::jmax(1, (int) (drawH * aspect));
        for (int x = area.getX(); x < area.getRight(); x += drawW)
            g.drawImage(img, x, area.getY(), drawW, drawH, 0, 0, img.getWidth(), img.getHeight());
    }

    juce::Path PerformanceStrip::buildTracePath(const float* samples, juce::Rectangle<float> traceArea)
    {
        const float midY = traceArea.getCentreY();
        const float ampScale = traceArea.getHeight() * 0.5f;

        juce::Path path;
        for (int i = 0; i < ScopeDataSource<>::size; ++i)
        {
            const float x =
                traceArea.getX() + traceArea.getWidth() * (float) i / (float) (ScopeDataSource<>::size - 1);
            const float y = midY - juce::jlimit(-1.2f, 1.2f, samples[i]) * ampScale;
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        return path;
    }

    void PerformanceStrip::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        // No flat background fill here on purpose -- this component is
        // deliberately taller than its "ground line" and overlaps both
        // the rack above and the keyboard above it (see PluginEditor's
        // resized()). grass.png already has real alpha transparency at
        // its blade tips (confirmed directly against the asset), so
        // painting only the grass/trace and nothing else lets whatever's
        // underneath -- rack cards through the transparent tips, the
        // keyboard through the solid base's lower overlap -- show through
        // exactly where it should, instead of a flat color band fighting
        // the image's own transparency.
        drawTiledLayer(g, grassImage, bounds);

        float samples[ScopeDataSource<>::size];
        scopeSource.copyOut(samples);
        float peak = 0.0f;
        for (float s : samples)
            peak = juce::jmax(peak, std::abs(s));

        const auto traceArea = bounds.reduced(juce::jmin(140, bounds.getWidth() / 6), 0)
                                    .withY(bounds.getY() + (int) (bounds.getHeight() * 0.1f))
                                    .withHeight((int) (bounds.getHeight() * 0.42f))
                                    .toFloat();

        // A dark glass "screen" the trace lives on, like a little embedded
        // scope display rather than a bare line floating over the grass --
        // gives the whole thing a home instead of just hovering.
        const auto panelArea = traceArea.expanded(12.0f, 7.0f);
        g.setColour(juce::Colour(0xCC10202C));
        g.fillRoundedRectangle(panelArea, 12.0f);
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillRoundedRectangle(panelArea.withHeight(panelArea.getHeight() * 0.4f), 12.0f); // glassy top sheen
        g.setColour(juce::Colours::white.withAlpha(0.14f));
        g.drawRoundedRectangle(panelArea.reduced(0.5f), 12.0f, 1.1f);

        // Trail: recent frames drawn oldest-first beneath the live trace,
        // fading in alpha and drifting in hue -- a little comet tail of
        // where the waveform just was, instead of a single static stroke.
        // Skipped near-silence so the "empty screen" doesn't show a faint
        // frozen ghost of the last thing played.
        if (peak > 0.01f)
        {
            for (int j = 0; j < trailLength; ++j)
            {
                const int idx = (trailWriteIndex + j) % trailLength; // oldest (j=0) .. newest-of-trail
                const float age = (float) j / (float) (trailLength - 1);
                auto trailPath = buildTracePath(trailSamples[(size_t) idx].data(), traceArea);
                g.setColour(juce::Colour(0xffb84fd8)
                                .withRotatedHue((1.0f - age) * -0.16f)
                                .withAlpha(juce::jlimit(0.0f, 0.32f, age * 0.32f * peak * 2.0f)));
                g.strokePath(trailPath, juce::PathStrokeType(1.6f + age * 1.4f, juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::rounded));
            }
        }

        // Live trace on top: soft wide glow behind a bright core stroke --
        // both fade all the way to (near-)invisible at silence and bloom
        // in with the recent peak, so nothing reads as a static bar
        // floating over the grass when nothing's playing.
        if (peak > 0.003f)
        {
            auto path = buildTracePath(samples, traceArea);
            g.setColour(juce::Colours::white.withAlpha(juce::jlimit(0.0f, 0.5f, peak * 0.6f)));
            g.strokePath(path, juce::PathStrokeType(5.5f + peak * 5.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
            g.setColour(juce::Colour(0xff2aa9c9).withAlpha(juce::jlimit(0.0f, 1.0f, 0.15f + peak * 1.6f)));
            g.strokePath(path,
                         juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    void PerformanceStrip::timerCallback()
    {
        if (++tickCounter >= captureEveryNTicks)
        {
            tickCounter = 0;
            scopeSource.copyOut(trailSamples[(size_t) trailWriteIndex].data());
            trailWriteIndex = (trailWriteIndex + 1) % trailLength;
        }

        // The trace needs to keep updating even in reduced-motion mode
        // (it's reflecting live audio, not a decorative animation), so
        // unlike BackgroundScene this timer always keeps running --
        // reducedMotion only governs the ambient sky/cloud drift there.
        repaint();
    }
}
