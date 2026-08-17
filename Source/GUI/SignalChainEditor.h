#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"
#include "../Parameters.h"

namespace gexex
{
    // Replaces what used to be 7 stacked ComboBox rows (functionally
    // fine, but reads as "a form to fill in" rather than a signal chain)
    // with an actual drag-to-reorder list: each row is one insert slot,
    // tinted by whichever effect currently occupies it (categoryColour
    // keyed to the effect's own choice index, so e.g. "Chorus" is always
    // the same hue no matter which slot it's in). Dragging a row past a
    // neighbour swaps them immediately (a "snap between slots" reorder,
    // not a smoothly-floating ghost -- simpler and just as clear).
    // Clicking a row without dragging opens a small popup to reassign
    // *which* effect that slot holds -- so the picker still exists, it's
    // just not permanently on-screen as a dropdown.
    //
    // Backed by the same 7 fxSlot0..6 AudioParameterChoice params
    // EffectsChain always used -- this is a different *editor* for that
    // data, not a different data model, so presets/automation are
    // unaffected.
    class SignalChainEditor : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
    {
    public:
        explicit SignalChainEditor(juce::AudioProcessorValueTreeState& apvtsToUse) : apvts(apvtsToUse)
        {
            for (int i = 0; i < numFxSlots; ++i)
                slotParams[(size_t) i] = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(fxSlotParamID(i)));
            if (slotParams[0] != nullptr)
                choices = slotParams[0]->choices;
            syncFromParams();
            setTooltip("Drag rows to reorder effects. Click a row to change what's in it.");
            startTimerHz(15);
        }

        void paint(juce::Graphics& g) override
        {
            const int rh = rowHeight();
            for (int i = 0; i < numFxSlots; ++i)
                drawRow(g, i, rh, draggingRow >= 0 && didDrag && i == draggedToRow);

            // Top-right "this is draggable" affordance -- a small grip
            // (2x3 dot grid, the same glyph a lot of apps use for
            // draggable list rows) rather than relying on the tooltip
            // alone to communicate it.
            auto bounds = getLocalBounds().toFloat();
            const float dotD = 3.0f;
            const float x0 = bounds.getRight() - 16.0f;
            const float y0 = bounds.getY() + 5.0f;
            g.setColour(GexexLookAndFeel::inkSoftColour().withAlpha(0.55f));
            for (int row = 0; row < 3; ++row)
                for (int col = 0; col < 2; ++col)
                    g.fillEllipse(x0 + (float) col * 7.0f, y0 + (float) row * 7.0f, dotD, dotD);
        }

        void resized() override { repaint(); }

        void mouseDown(const juce::MouseEvent& e) override
        {
            draggingRow = rowAt(e.position);
            draggedToRow = draggingRow;
            mouseDownPos = e.position;
            didDrag = false;
        }

        void mouseDrag(const juce::MouseEvent& e) override
        {
            if (draggingRow < 0)
                return;
            if (! didDrag && e.position.getDistanceFrom(mouseDownPos) > 4.0f)
                didDrag = true;
            if (! didDrag)
                return;

            const int targetRow = rowAt(e.position);
            if (targetRow != draggedToRow)
            {
                const int moved = rowValues[(size_t) draggedToRow];
                if (targetRow > draggedToRow)
                    for (int k = draggedToRow; k < targetRow; ++k)
                        rowValues[(size_t) k] = rowValues[(size_t) (k + 1)];
                else
                    for (int k = draggedToRow; k > targetRow; --k)
                        rowValues[(size_t) k] = rowValues[(size_t) (k - 1)];
                rowValues[(size_t) targetRow] = moved;
                draggedToRow = targetRow;
                commitToParams();
            }
            repaint();
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            if (draggingRow >= 0 && ! didDrag)
                showEffectPickerFor(draggingRow);

            draggingRow = -1;
            didDrag = false;
            repaint();
        }

    private:
        int rowHeight() const noexcept { return juce::jmax(18, getHeight() / numFxSlots); }

        int rowAt(juce::Point<float> pos) const noexcept
        {
            return juce::jlimit(0, numFxSlots - 1, (int) (pos.y / (float) rowHeight()));
        }

        void syncFromParams()
        {
            for (int i = 0; i < numFxSlots; ++i)
                rowValues[(size_t) i] = slotParams[(size_t) i] != nullptr ? slotParams[(size_t) i]->getIndex() : 0;
        }

        void commitToParams()
        {
            for (int i = 0; i < numFxSlots; ++i)
            {
                auto* p = slotParams[(size_t) i];
                if (p == nullptr)
                    continue;
                if (p->getIndex() != rowValues[(size_t) i])
                {
                    p->beginChangeGesture();
                    p->setValueNotifyingHost(p->convertTo0to1((float) rowValues[(size_t) i]));
                    p->endChangeGesture();
                }
            }
        }

        void showEffectPickerFor(int rowIndex)
        {
            juce::PopupMenu menu;
            for (int i = 0; i < choices.size(); ++i)
                menu.addItem(i + 1, choices[i], true, rowValues[(size_t) rowIndex] == i);

            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                                [this, rowIndex](int result) {
                                    if (result <= 0)
                                        return;
                                    rowValues[(size_t) rowIndex] = result - 1;
                                    commitToParams();
                                    repaint();
                                });
        }

        void drawRow(juce::Graphics& g, int rowIndex, int rh, bool isBeingDragged) const
        {
            auto rowBounds =
                juce::Rectangle<float>(0.0f, (float) (rowIndex * rh), (float) getWidth(), (float) rh).reduced(1.0f, 1.5f);

            const int effectIndex = rowValues[(size_t) rowIndex];
            const bool isEmpty = effectIndex == 0; // choice index 0 == FxSlotEffect::Empty

            auto tint = isEmpty ? juce::Colours::white.withAlpha(0.4f)
                                 : GexexLookAndFeel::categoryColour(effectIndex).withAlpha(isBeingDragged ? 0.95f : 0.75f);
            g.setColour(tint);
            g.fillRoundedRectangle(rowBounds, 5.0f);

            if (isBeingDragged)
            {
                g.setColour(juce::Colours::white.withAlpha(0.85f));
                g.drawRoundedRectangle(rowBounds, 5.0f, 1.8f);
            }

            // Grip dots, same glyph as the card-level affordance -- marks
            // every row as individually draggable, not just the card as
            // a whole.
            const float gx = rowBounds.getX() + 7.0f;
            const float gy = rowBounds.getCentreY();
            g.setColour((isEmpty ? GexexLookAndFeel::inkSoftColour() : juce::Colours::white).withAlpha(0.5f));
            for (int r = -1; r <= 1; ++r)
                g.fillEllipse(gx, gy + (float) r * 5.0f - 1.3f, 2.6f, 2.6f);

            g.setColour(isEmpty ? GexexLookAndFeel::inkSoftColour() : juce::Colours::white);
            g.setFont(juce::FontOptions(12.5f, isEmpty ? juce::Font::plain : juce::Font::bold));
            g.drawText(effectIndex < choices.size() ? choices[effectIndex] : juce::String(),
                        rowBounds.withTrimmedLeft(20.0f).withTrimmedRight(4.0f), juce::Justification::centredLeft);
        }

        void timerCallback() override
        {
            if (draggingRow >= 0)
                return; // don't fight an in-progress drag with an external-change resync

            for (int i = 0; i < numFxSlots; ++i)
            {
                if (slotParams[(size_t) i] != nullptr && rowValues[(size_t) i] != slotParams[(size_t) i]->getIndex())
                {
                    syncFromParams(); // preset load / undo / host automation changed a slot underneath us
                    repaint();
                    return;
                }
            }
        }

        juce::AudioProcessorValueTreeState& apvts;
        std::array<juce::AudioParameterChoice*, (size_t) numFxSlots> slotParams {};
        juce::StringArray choices;
        std::array<int, (size_t) numFxSlots> rowValues {};

        int draggingRow = -1; // row index pressed at mouseDown, -1 when not dragging
        int draggedToRow = -1; // that item's current (possibly moved) row index
        juce::Point<float> mouseDownPos;
        bool didDrag = false;
    };
}
