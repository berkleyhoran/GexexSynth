#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"
#include "Knob.h"

namespace gexex
{
    // A glass-panel module card: title, then any number of full-width
    // "top rows" (dropdowns/toggles/a custom visual like EnvelopeEditor or
    // ReverbVisual, in whatever order the caller adds them), then any
    // number of knobs packed into a wrapping grid underneath. This one
    // data-driven class stands in for what the build plan sketched as ~13
    // hand-written Module classes (OscillatorModule, FilterModule, ...) --
    // every module in ModuleRack.cpp is just a sequence of addKnob/
    // addCombo/addToggle/addCustomComponent calls against one of these,
    // which keeps ~70 parameters' worth of UI to one reusable component
    // instead of thirteen near-duplicate ones.
    class ModuleCard : public juce::Component
    {
    public:
        ModuleCard(juce::String cardTitle, juce::Colour accent) : title(std::move(cardTitle)), accentColour(accent)
        {
            titleLabel.setText(title, juce::dontSendNotification);
            titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
            titleLabel.setColour(juce::Label::textColourId, accentColour.darker(0.35f));
            addAndMakeVisible(titleLabel);
        }

        Knob& addKnob(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
                       const juce::String& label)
        {
            auto* k = knobs.add(new Knob(apvts, paramID, label, accentColour));
            addAndMakeVisible(k);
            return *k;
        }

        juce::ComboBox& addCombo(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
                                  const juce::String& label)
        {
            auto* row = comboRows.add(new ComboRow());
            row->label.setText(label, juce::dontSendNotification);
            row->label.setFont(juce::FontOptions(12.0f));
            addAndMakeVisible(row->label);
            addAndMakeVisible(row->combo);
            if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID)))
                row->combo.addItemList(choiceParam->choices, 1);
            row->attachment =
                std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, row->combo);
            return row->combo;
        }

        juce::ToggleButton& addToggle(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
                                       const juce::String& label)
        {
            auto* row = toggleRows.add(new ToggleRow());
            row->toggle.setButtonText(label);
            addAndMakeVisible(row->toggle);
            row->attachment =
                std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramID, row->toggle);
            return row->toggle;
        }

        // `component` is owned by the caller (ModuleRack keeps the real
        // custom-visual instances alive) -- this just parents it and
        // reserves `height` px of vertical space for it, in call order
        // relative to addCombo/addToggle.
        void addCustomComponent(juce::Component& component, int height)
        {
            addAndMakeVisible(component);
            customComponents.push_back({ &component, height });
        }

        int getPreferredHeight(int width) const
        {
            int h = titleHeight;
            h += comboRows.size() * (rowHeight + rowGap);
            h += toggleRows.size() * (rowHeight + rowGap);
            for (auto& c : customComponents)
                h += c.height + rowGap;
            if (knobs.size() > 0)
            {
                const int perRow = juce::jmax(1, (width - margin * 2) / (knobSize + knobGap));
                const int rows = (knobs.size() + perRow - 1) / perRow;
                h += rows * (knobSize + knobGap) + margin;
            }
            return h + margin;
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(GexexLookAndFeel::panelFillColour());
            g.fillRoundedRectangle(bounds, 10.0f);
            g.setColour(GexexLookAndFeel::panelBorderColour());
            g.drawRoundedRectangle(bounds.reduced(0.5f), 10.0f, 1.2f);

            // A thin accent-colored strip along the top, tying the card to
            // its category color even where it has no knobs (e.g. Envelope).
            auto stripArea = bounds.removeFromTop(4.0f).reduced(10.0f, 0.0f);
            g.setColour(accentColour);
            g.fillRoundedRectangle(stripArea, 2.0f);
        }

        void resized() override
        {
            auto bounds = getLocalBounds().reduced(margin, 0);
            bounds.removeFromTop(6);
            titleLabel.setBounds(bounds.removeFromTop(titleHeight));

            for (auto* row : comboRows)
            {
                auto rowBounds = bounds.removeFromTop(rowHeight);
                row->label.setBounds(rowBounds.removeFromLeft(rowBounds.getWidth() / 2));
                row->combo.setBounds(rowBounds);
                bounds.removeFromTop(rowGap);
            }

            for (auto* row : toggleRows)
            {
                row->toggle.setBounds(bounds.removeFromTop(rowHeight));
                bounds.removeFromTop(rowGap);
            }

            for (auto& c : customComponents)
            {
                c.component->setBounds(bounds.removeFromTop(c.height));
                bounds.removeFromTop(rowGap);
            }

            if (knobs.size() > 0)
            {
                const int perRow = juce::jmax(1, bounds.getWidth() / (knobSize + knobGap));
                int x = bounds.getX();
                int y = bounds.getY();
                int col = 0;
                for (auto* k : knobs)
                {
                    k->setBounds(x, y, knobSize, knobSize);
                    ++col;
                    if (col >= perRow)
                    {
                        col = 0;
                        x = bounds.getX();
                        y += knobSize + knobGap;
                    }
                    else
                    {
                        x += knobSize + knobGap;
                    }
                }
            }
        }

        static constexpr int knobSize = 62;

    private:
        struct ComboRow
        {
            juce::Label label;
            juce::ComboBox combo;
            std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
        };

        struct ToggleRow
        {
            juce::ToggleButton toggle;
            std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
        };

        struct CustomEntry
        {
            juce::Component* component;
            int height;
        };

        static constexpr int titleHeight = 20;
        static constexpr int rowHeight = 24;
        static constexpr int rowGap = 4;
        static constexpr int knobGap = 6;
        static constexpr int margin = 8;

        juce::String title;
        juce::Colour accentColour;
        juce::Label titleLabel;

        juce::OwnedArray<ComboRow> comboRows;
        juce::OwnedArray<ToggleRow> toggleRows;
        juce::OwnedArray<Knob> knobs;
        std::vector<CustomEntry> customComponents;
    };
}
