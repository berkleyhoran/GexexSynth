#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

namespace gexex
{
    // A full-width divider between groups of module cards -- forces a row
    // break before and after itself in ModuleRack's flow layout, so
    // related modules (oscillators, modulation, effects, master) read as
    // clearly separated sections instead of one undifferentiated wrapping
    // grid.
    class SectionHeader : public juce::Component
    {
    public:
        SectionHeader(juce::String titleText, juce::Colour accent) : title(std::move(titleText)), colour(accent) {}

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(colour.withAlpha(0.6f));
            g.fillRoundedRectangle(bounds.removeFromLeft(4.0f), 2.0f);

            g.setColour(GexexLookAndFeel::inkColour());
            g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
            g.drawText(title, bounds.reduced(10.0f, 0.0f), juce::Justification::centredLeft);

            const float lineY = bounds.getCentreY();
            const float textWidth = juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), title) + 24.0f;
            g.setColour(colour.withAlpha(0.35f));
            g.drawLine(bounds.getX() + textWidth, lineY, bounds.getRight(), lineY, 1.5f);
        }

    private:
        juce::String title;
        juce::Colour colour;
    };
}
