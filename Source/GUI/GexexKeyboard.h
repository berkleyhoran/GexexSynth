#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

namespace gexex
{
    // A candy-glass reskin of MidiKeyboardComponent -- the stock look is
    // plain black-and-white and reads as a generic MIDI utility widget,
    // not part of the "fruity aero" instrument. Glossy gradient keys
    // (sky-blue press highlight on white keys, candy-pink press highlight
    // on black keys) and a soft glass background instead of the default
    // flat fills.
    class GexexKeyboard : public juce::MidiKeyboardComponent
    {
    public:
        GexexKeyboard(juce::MidiKeyboardState& state, Orientation orientation) : MidiKeyboardComponent(state, orientation)
        {
            // drawKeyboardBackground() is `final` in the base class and
            // just fillAll()s with this colour -- setting it is the
            // supported way to theme the background behind/around the
            // keys without overriding that method.
            setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xfff8fbfd));
        }

        void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown,
                            bool isOver, juce::Colour lineColour, juce::Colour textColour) override
        {
            juce::Colour top = juce::Colour(0xfffdfeff);
            juce::Colour bottom = juce::Colour(0xffe4eef5);
            if (isDown)
            {
                top = juce::Colour(0xffcdeafc);
                bottom = juce::Colour(0xff9fd6f2);
            }
            else if (isOver)
            {
                top = juce::Colour(0xfff0f8fc);
                bottom = juce::Colour(0xffdcedf5);
            }

            g.setGradientFill(juce::ColourGradient(top, area.getX(), area.getY(), bottom, area.getX(),
                                                     area.getBottom(), false));
            g.fillRect(area);

            if (isDown)
            {
                g.setColour(juce::Colour(0xff2aa9c9).withAlpha(0.55f));
                g.fillRect(area.withTop(area.getBottom() - area.getHeight() * 0.18f));
            }

            g.setColour(lineColour.withAlpha(0.6f));
            g.drawRect(area, 1.0f);

            juce::ignoreUnused(midiNoteNumber, textColour);
        }

        void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown,
                            bool isOver, juce::Colour noteFillColour) override
        {
            juce::ignoreUnused(midiNoteNumber, noteFillColour);

            juce::Colour top = juce::Colour(0xff3a4150);
            juce::Colour bottom = juce::Colour(0xff181c24);
            if (isDown)
            {
                top = juce::Colour(0xffff6f96);
                bottom = juce::Colour(0xffe0335f);
            }
            else if (isOver)
            {
                top = juce::Colour(0xff4d5666);
                bottom = juce::Colour(0xff262b35);
            }

            auto keyArea = area.reduced(1.0f, 0.0f);
            g.setGradientFill(
                juce::ColourGradient(top, keyArea.getX(), keyArea.getY(), bottom, keyArea.getX(), keyArea.getBottom(), false));
            g.fillRoundedRectangle(keyArea, 2.5f);

            // Specular highlight along the top edge -- the same glossy-
            // plastic cue the candy knobs use, for visual family
            // resemblance.
            g.setColour(juce::Colours::white.withAlpha(isDown ? 0.25f : 0.16f));
            g.fillRoundedRectangle(keyArea.withHeight(keyArea.getHeight() * 0.22f), 2.5f);
        }
    };
}
