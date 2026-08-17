#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

namespace gexex
{
    // A labeled rotary knob bound to one APVTS parameter, candy-tinted by
    // category via LookAndFeel's accentColour property. This is the
    // reusable building block ModuleCard uses for every plain-knob
    // parameter -- Envelope/Delay/Reverb get their own fully custom
    // visuals instead (EnvelopeEditor.h, DelayVisual.h, ReverbVisual.h).
    class Knob : public juce::Component
    {
    public:
        Knob(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText,
             juce::Colour accentColour)
        {
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 16);
            slider.getProperties().set(GexexLookAndFeel::accentColourPropertyID, (int) accentColour.getARGB());
            addAndMakeVisible(slider);

            label.setText(labelText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.setFont(juce::FontOptions(11.5f));
            addAndMakeVisible(label);

            attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID,
                                                                                                   slider);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            label.setBounds(bounds.removeFromTop(15));
            slider.setBounds(bounds);
        }

    private:
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
}
