#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace gexex
{
    // "Fruity aero": Windows-Aero glass chrome (light sky/glass palette,
    // set once via a custom ColourScheme so every stock JUCE widget --
    // ComboBox, ToggleButton, Label, TextButton -- inherits it for free)
    // combined with FL-Studio-style glossy candy-colored plastic knobs
    // (the one widget that needs a fully custom paint routine to get that
    // look, since no stock LookAndFeel scheme gets you there).
    class GexexLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        GexexLookAndFeel();

        void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional,
                               float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

        juce::Font getLabelFont(juce::Label&) override;
        juce::Font getComboBoxFont(juce::ComboBox&) override;

        // Assigns a knob's candy color by category index (0=osc, 1=filter,
        // ...) -- golden-angle hue rotation, so any number of categories
        // stays visually distinct without hand-picking each hex code.
        static juce::Colour categoryColour(int categoryIndex) noexcept;

        // Knob.h stashes the resolved category colour on the Slider's
        // Component::getProperties() under this key so drawRotarySlider
        // (a LookAndFeel method with no direct knowledge of "categories")
        // can read it back per-instance.
        static const juce::Identifier accentColourPropertyID;

        static juce::Colour panelFillColour() noexcept { return juce::Colour(0xE6FBFEFF); }
        static juce::Colour panelBorderColour() noexcept { return juce::Colour(0x40163049); }
        static juce::Colour inkColour() noexcept { return juce::Colour(0xFF163049); }
        static juce::Colour inkSoftColour() noexcept { return juce::Colour(0xFF4D6B82); }
    };
}
