#include "ModuleRack.h"
#include "LookAndFeel.h"
#include "../Parameters.h"

namespace gexex
{
    ModuleRack::ModuleRack(juce::AudioProcessorValueTreeState& apvtsToUse, const ScopeDataSource<>& osc1Scope,
                           const ScopeDataSource<>& osc2Scope, const ScopeDataSource<>& osc3Scope,
                           const ScopeDataSource<>& masterScopeSource)
        : apvts(apvtsToUse)
    {
        const ScopeDataSource<>* oscScopeSources[3] = { &osc1Scope, &osc2Scope, &osc3Scope };
        viewport.setViewedComponent(&content, false);
        viewport.setScrollBarsShown(true, false);
        addAndMakeVisible(viewport);

        int category = 0;

        // --- Oscillators 1-3 (each its own hue), plus FM amount into osc1 ---
        for (int oscNumber = 1; oscNumber <= 3; ++oscNumber)
        {
            auto& card = addCard("Osc " + juce::String(oscNumber), category++);
            card.addCombo(apvts, oscParamID(oscNumber, ParamIDs::oscWaveSuffix), "Wave");
            card.addKnob(apvts, oscParamID(oscNumber, ParamIDs::oscPulseWidthSuffix), "PW");
            card.addKnob(apvts, oscParamID(oscNumber, ParamIDs::oscFoldSuffix), "Fold");
            card.addKnob(apvts, oscParamID(oscNumber, ParamIDs::oscOctaveSuffix), "Oct");
            card.addKnob(apvts, oscParamID(oscNumber, ParamIDs::oscSemitoneSuffix), "Semi");
            card.addKnob(apvts, oscParamID(oscNumber, ParamIDs::oscFineSuffix), "Fine");
            card.addKnob(apvts, oscParamID(oscNumber, ParamIDs::oscLevelSuffix), "Level");
            if (oscNumber != 1)
                card.addKnob(apvts, oscNumber == 2 ? ParamIDs::fmAmount2 : ParamIDs::fmAmount3, "FM->1");
            card.addToggle(apvts, oscParamID(oscNumber, ParamIDs::oscMuteSuffix), "Mute");

            auto* scope = scopes.add(new Scope(GexexLookAndFeel::categoryColour(oscNumber - 1)));
            scope->setSource(oscScopeSources[oscNumber - 1]);
            card.addCustomComponent(*scope, 44);
        }

        // --- Filter ---
        {
            auto& card = addCard("Filter", category++);
            card.addCombo(apvts, ParamIDs::filterType, "Type");
            card.addKnob(apvts, ParamIDs::filterCutoff, "Cutoff");
            card.addKnob(apvts, ParamIDs::filterResonance, "Res");
            card.addKnob(apvts, ParamIDs::filterVelSens, "Vel>Cut");
        }

        // --- LFO ---
        {
            auto& card = addCard("LFO", category);
            card.addCombo(apvts, ParamIDs::lfoWaveform, "Wave");
            card.addCombo(apvts, ParamIDs::lfoSyncDivision, "Sync");
            card.addCombo(apvts, ParamIDs::lfoTarget, "Target");
            lfoShapePreview = std::make_unique<LfoShapePreview>(apvts, ParamIDs::lfoWaveform,
                                                                  GexexLookAndFeel::categoryColour(category));
            card.addCustomComponent(*lfoShapePreview, 44);
            ++category;
            card.addKnob(apvts, ParamIDs::lfoRateHz, "Rate");
            card.addKnob(apvts, ParamIDs::lfoDepth, "Depth");
        }

        // --- Envelope: the custom draggable-node editor instead of 4 sliders ---
        {
            auto& card = addCard("Envelope", category);
            envelopeEditor = std::make_unique<EnvelopeEditor>(apvts, ParamIDs::envAttack, ParamIDs::envDecay,
                                                                ParamIDs::envSustain, ParamIDs::envRelease);
            envelopeEditor->setAccentColour(GexexLookAndFeel::categoryColour(category));
            ++category;
            card.addCustomComponent(*envelopeEditor, 130);
        }

        // --- Voice / Arp ---
        {
            auto& card = addCard("Voice / Arp", category++);
            card.addCombo(apvts, ParamIDs::voiceMode, "Voice Mode");
            card.addToggle(apvts, ParamIDs::arpEnabled, "Arp On");
            card.addCombo(apvts, ParamIDs::arpPattern, "Pattern");
            card.addCombo(apvts, ParamIDs::arpSyncDivision, "Sync");
            card.addKnob(apvts, ParamIDs::glideTime, "Glide");
            card.addKnob(apvts, ParamIDs::arpOctaveRange, "Oct Rng");
            card.addKnob(apvts, ParamIDs::arpRateHz, "Rate");
            card.addKnob(apvts, ParamIDs::arpGate, "Gate");
        }

        // --- Effects rack, in signal-flow order ---
        {
            auto& card = addCard("Bitcrush", category++);
            card.addKnob(apvts, ParamIDs::crushBits, "Depth");
            card.addKnob(apvts, ParamIDs::crushDownsample, "Downsmp");
        }
        {
            auto& card = addCard("Drive", category++);
            card.addKnob(apvts, ParamIDs::driveAmount, "Amount");
        }
        {
            auto& card = addCard("Delay", category);
            delayVisual = std::make_unique<DelayVisual>();
            delayVisual->setAccentColour(GexexLookAndFeel::categoryColour(category));
            ++category;
            card.addCustomComponent(*delayVisual, 54);
            card.addCombo(apvts, ParamIDs::delaySyncDivision, "Sync");
            card.addKnob(apvts, ParamIDs::delayTimeSeconds, "Time");
            card.addKnob(apvts, ParamIDs::delayFeedback, "Feedback");
            card.addKnob(apvts, ParamIDs::delayMix, "Mix");
        }
        {
            auto& card = addCard("Reverb", category);
            reverbVisual = std::make_unique<ReverbVisual>();
            reverbVisual->setAccentColour(GexexLookAndFeel::categoryColour(category));
            ++category;
            card.addCustomComponent(*reverbVisual, 70);
            card.addKnob(apvts, ParamIDs::reverbSize, "Size");
            card.addKnob(apvts, ParamIDs::reverbMix, "Mix");
        }
        {
            auto& card = addCard("Chorus", category++);
            card.addKnob(apvts, ParamIDs::chorusRateHz, "Rate");
            card.addKnob(apvts, ParamIDs::chorusDepthMs, "Depth");
            card.addKnob(apvts, ParamIDs::chorusMix, "Mix");
        }
        {
            auto& card = addCard("Phaser / Flanger", category++);
            card.addCombo(apvts, ParamIDs::pfMode, "Mode");
            card.addKnob(apvts, ParamIDs::pfRateHz, "Rate");
            card.addKnob(apvts, ParamIDs::pfDepth, "Depth");
            card.addKnob(apvts, ParamIDs::pfFeedback, "Feedback");
            card.addKnob(apvts, ParamIDs::pfMix, "Mix");
        }
        {
            auto& card = addCard("Saturator", category++);
            card.addCombo(apvts, ParamIDs::saturatorAlgorithm, "Algorithm");
            card.addKnob(apvts, ParamIDs::saturatorAmount, "Amount");
            card.addKnob(apvts, ParamIDs::saturatorCeiling, "Ceiling");
        }
        {
            auto& card = addCard("Master", category++);
            auto* scope = scopes.add(new Scope(GexexLookAndFeel::categoryColour(category)));
            scope->setSource(&masterScopeSource);
            card.addCustomComponent(*scope, 50);
            card.addToggle(apvts, ParamIDs::reducedMotion, "Reduced Motion");
            card.addKnob(apvts, ParamIDs::masterVolume, "Volume");
            card.addKnob(apvts, ParamIDs::masterPan, "Pan");
        }

        startTimerHz(30);
        layOutCards();
    }

    ModuleCard& ModuleRack::addCard(const juce::String& title, int categoryIndex)
    {
        auto* card = cards.add(new ModuleCard(title, GexexLookAndFeel::categoryColour(categoryIndex)));
        content.addAndMakeVisible(card);
        return *card;
    }

    void ModuleRack::timerCallback()
    {
        // Push current parameter values into the reverb/delay visuals --
        // they're plain Components, not Slider-backed, so nothing else
        // keeps them in sync (mirrors EnvelopeEditor's self-polling, just
        // driven from here since these two read multiple parameters each).
        if (reverbVisual != nullptr)
        {
            const float size = apvts.getRawParameterValue(ParamIDs::reverbSize)->load();
            reverbVisual->setSize01(juce::jmap(size, 0.1f, 8.0f, 0.0f, 1.0f));
            reverbVisual->setMix01(apvts.getRawParameterValue(ParamIDs::reverbMix)->load());
        }
        if (delayVisual != nullptr)
        {
            delayVisual->setTimeSeconds(apvts.getRawParameterValue(ParamIDs::delayTimeSeconds)->load());
            delayVisual->setFeedback01(apvts.getRawParameterValue(ParamIDs::delayFeedback)->load());
            delayVisual->setMix01(apvts.getRawParameterValue(ParamIDs::delayMix)->load());
        }
    }

    void ModuleRack::layOutCards()
    {
        // Reserve room for the vertical scrollbar this content will almost
        // always need -- otherwise the last column's cards get laid out
        // assuming the full window width is available, then the scrollbar
        // appears (because the content is taller than the viewport) and
        // clips straight through them. Generous on purpose: a slightly
        // under-filled last column reads fine; an overflowing one doesn't.
        const int scrollbarAllowance = juce::jmax(24, viewport.getScrollBarThickness() + 8);
        const int rawWidth = getWidth() > 0 ? getWidth() : 900;
        const int viewportWidth = juce::jmax(cardWidth, rawWidth - scrollbarAllowance);
        const int perRow = juce::jmax(1, (viewportWidth - cardGap) / (cardWidth + cardGap));

        int x = cardGap;
        int y = cardGap;
        int col = 0;
        int rowMaxHeight = 0;

        for (auto* card : cards)
        {
            const int h = card->getPreferredHeight(cardWidth);
            card->setBounds(x, y, cardWidth, h);
            rowMaxHeight = juce::jmax(rowMaxHeight, h);

            ++col;
            if (col >= perRow)
            {
                col = 0;
                x = cardGap;
                y += rowMaxHeight + cardGap;
                rowMaxHeight = 0;
            }
            else
            {
                x += cardWidth + cardGap;
            }
        }

        const int contentHeight = y + rowMaxHeight + cardGap;
        content.setSize(perRow * (cardWidth + cardGap) + cardGap, contentHeight);
    }

    void ModuleRack::resized()
    {
        viewport.setBounds(getLocalBounds());
        layOutCards();
    }
}
