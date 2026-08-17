#include "PresetBar.h"
#include "LookAndFeel.h"
#include "../Presets/PresetManager.h"

namespace gexex
{
    PresetBar::PresetBar(GexexSynthAudioProcessor& processorToUse) : processor(processorToUse)
    {
        addAndMakeVisible(presetBox);
        presetBox.onChange = [this] { loadSelectedItem(); };

        addAndMakeVisible(prevButton);
        addAndMakeVisible(nextButton);
        prevButton.onClick = [this] { stepPreset(-1); };
        nextButton.onClick = [this] { stepPreset(1); };

        addAndMakeVisible(saveButton);
        saveButton.onClick = [this] { showSaveDialog(); };

        addAndMakeVisible(randomizeButton);
        randomizeButton.onClick = [this] {
            PresetManager::randomizePatch(processor.apvts, random);
            presetBox.setSelectedId(0, juce::dontSendNotification); // no longer matches any named preset
        };

        addAndMakeVisible(initButton);
        initButton.onClick = [this] {
            PresetManager::initPatch(processor.apvts);
            presetBox.setSelectedId(0, juce::dontSendNotification);
        };

        addAndMakeVisible(panicButton);
        panicButton.onClick = [this] { processor.requestPanic(); };

        rebuildComboItems();
    }

    void PresetBar::rebuildComboItems()
    {
        presetBox.clear(juce::dontSendNotification);
        validIds.clear();

        const auto& factory = PresetManager::getFactoryPresets();
        juce::String lastCategory;
        for (int i = 0; i < (int) factory.size(); ++i)
        {
            const auto& preset = factory[(size_t) i];
            if (preset.category != lastCategory)
            {
                lastCategory = preset.category;
                presetBox.addSectionHeading(lastCategory);
            }
            const int id = i + 1;
            presetBox.addItem(preset.name, id);
            validIds.add(id);
        }

        userPresetNames = PresetManager::getUserPresetNames();
        if (userPresetNames.size() > 0)
        {
            presetBox.addSectionHeading("User Presets");
            for (int i = 0; i < userPresetNames.size(); ++i)
            {
                const int id = userIdOffset + i;
                presetBox.addItem(userPresetNames[i], id);
                validIds.add(id);
            }
        }
    }

    void PresetBar::loadSelectedItem()
    {
        const int id = presetBox.getSelectedId();
        if (id <= 0)
            return;

        if (id < userIdOffset)
        {
            PresetManager::applyFactoryPreset(processor.apvts, id - 1);
        }
        else
        {
            const int idx = id - userIdOffset;
            if (idx >= 0 && idx < userPresetNames.size())
                PresetManager::loadUserPreset(processor.apvts, userPresetNames[idx]);
        }
    }

    void PresetBar::stepPreset(int direction)
    {
        if (validIds.isEmpty())
            return;

        const int currentId = presetBox.getSelectedId();
        int index = validIds.indexOf(currentId);
        if (index < 0)
            index = direction > 0 ? -1 : 0; // nothing selected yet: next starts at the first item

        index = juce::jlimit(0, validIds.size() - 1, index + direction);
        presetBox.setSelectedId(validIds.getUnchecked(index)); // triggers onChange -> loadSelectedItem
    }

    void PresetBar::showSaveDialog()
    {
        auto* aw = new juce::AlertWindow("Save Preset", "Name this patch:", juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor("name", "My Patch");
        aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        aw->enterModalState(
            true,
            juce::ModalCallbackFunction::create([this, aw](int result) {
                if (result == 1)
                {
                    const auto name = aw->getTextEditorContents("name").trim();
                    if (name.isNotEmpty())
                    {
                        PresetManager::saveUserPreset(processor.apvts, name);
                        rebuildComboItems();
                        const int idx = userPresetNames.indexOf(name);
                        if (idx >= 0)
                            presetBox.setSelectedId(userIdOffset + idx, juce::dontSendNotification);
                    }
                }
            }),
            true); // deleteWhenDismissed
    }

    void PresetBar::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(GexexLookAndFeel::panelFillColour());
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(GexexLookAndFeel::panelBorderColour());
        g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
    }

    void PresetBar::resized()
    {
        auto bounds = getLocalBounds().reduced(8, 6);
        prevButton.setBounds(bounds.removeFromLeft(28));
        bounds.removeFromLeft(4);
        nextButton.setBounds(bounds.removeFromLeft(28));
        bounds.removeFromLeft(8);

        panicButton.setBounds(bounds.removeFromRight(70));
        bounds.removeFromRight(6);
        initButton.setBounds(bounds.removeFromRight(60));
        bounds.removeFromRight(6);
        randomizeButton.setBounds(bounds.removeFromRight(90));
        bounds.removeFromRight(6);
        saveButton.setBounds(bounds.removeFromRight(90));
        bounds.removeFromRight(8);

        presetBox.setBounds(bounds);
    }
}
