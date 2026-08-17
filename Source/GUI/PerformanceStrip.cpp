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

    void PerformanceStrip::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        // No flat background fill here on purpose -- this component is
        // deliberately taller than its "ground line" and overlaps both
        // the rack above and the keyboard above it (see PluginEditor's
        // resized()). grass.png already has real alpha transparency at
        // its blade tips (confirmed directly against the asset), so
        // painting only the grass/daisies/trace and nothing else lets
        // whatever's underneath -- rack cards through the transparent
        // tips, the keyboard through the solid base's lower overlap --
        // show through exactly where it should, instead of a flat color
        // band fighting the image's own transparency.
        drawTiledLayer(g, grassImage, bounds);

        // The glowing master trace, floating in the upper (transparent-
        // blade-tip) zone so it reads as hovering just above the grass
        // rather than buried in the dense lower half.
        float samples[ScopeDataSource<>::size];
        scopeSource.copyOut(samples);
        float peak = 0.0f;
        for (float s : samples)
            peak = juce::jmax(peak, std::abs(s));

        const auto traceArea = bounds.reduced(juce::jmin(140, bounds.getWidth() / 6), 0)
                                    .withY(bounds.getY() + (int) (bounds.getHeight() * 0.06f))
                                    .withHeight((int) (bounds.getHeight() * 0.34f));
        const float midY = (float) traceArea.getCentreY();
        const float ampScale = (float) traceArea.getHeight() * 0.5f;

        juce::Path path;
        for (int i = 0; i < ScopeDataSource<>::size; ++i)
        {
            const float x =
                (float) traceArea.getX() + (float) traceArea.getWidth() * (float) i / (float) (ScopeDataSource<>::size - 1);
            const float y = midY - juce::jlimit(-1.2f, 1.2f, samples[i]) * ampScale;
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }

        // Soft wide glow behind a bright core stroke -- both fade all the
        // way to (near-)invisible at silence and bloom in with the recent
        // peak, so nothing reads as a static bar floating over the grass
        // when nothing's playing.
        if (peak > 0.003f)
        {
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
        // The trace needs to keep updating even in reduced-motion mode
        // (it's reflecting live audio, not a decorative animation), so
        // unlike BackgroundScene this timer always keeps running --
        // reducedMotion only governs the ambient sky/cloud drift there.
        repaint();
    }
}
