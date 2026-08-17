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

        // ============================================================
        // Section: Sound Source -- the 3-osc/FM engine and its filter.
        // ============================================================
        addSection("Sound Source", GexexLookAndFeel::categoryColour(category));

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

        {
            auto& card = addCard("Noise", category++);
            card.addCombo(apvts, ParamIDs::noiseType, "Type");
            card.addKnob(apvts, ParamIDs::noiseLevel, "Level");
            card.addToggle(apvts, ParamIDs::noiseMute, "Mute");
        }

        {
            auto& card = addCard("Sub Osc", category++);
            card.addCombo(apvts, ParamIDs::subWaveform, "Wave");
            card.addKnob(apvts, ParamIDs::subOctaveDown, "Octave");
            card.addKnob(apvts, ParamIDs::subLevel, "Level");
            card.addToggle(apvts, ParamIDs::subMute, "Mute");
        }

        {
            auto& card = addCard("Filter 1", category++);
            card.addCombo(apvts, ParamIDs::filterType, "Type");
            card.addCombo(apvts, ParamIDs::filterRouting, "Routing");
            card.addKnob(apvts, ParamIDs::filterCutoff, "Cutoff");
            card.addKnob(apvts, ParamIDs::filterResonance, "Res");
            card.addKnob(apvts, ParamIDs::filterVelSens, "Vel>Cut");
        }

        {
            auto& card = addCard("Filter 2", category++);
            card.addCombo(apvts, ParamIDs::filter2Type, "Type");
            card.addKnob(apvts, ParamIDs::filter2Cutoff, "Cutoff");
            card.addKnob(apvts, ParamIDs::filter2Resonance, "Res");
        }

        // ============================================================
        // Section: Modulation -- LFO, envelope, voice/arp.
        // ============================================================
        addSection("Modulation", GexexLookAndFeel::categoryColour(category));

        {
            auto& card = addCard("LFO 1", category);
            card.addCombo(apvts, ParamIDs::lfoWaveform, "Wave");
            card.addCombo(apvts, ParamIDs::lfoSyncDivision, "Sync");
            card.addCombo(apvts, ParamIDs::lfoTarget, "Target");
            lfoShapePreview =
                std::make_unique<LfoShapePreview>(apvts, ParamIDs::lfoWaveform, ParamIDs::lfoDepth,
                                                    ParamIDs::lfoSyncDivision, ParamIDs::lfoRateHz,
                                                    GexexLookAndFeel::categoryColour(category));
            card.addCustomComponent(*lfoShapePreview, 44);
            ++category;
            card.addKnob(apvts, ParamIDs::lfoRateHz, "Rate");
            card.addKnob(apvts, ParamIDs::lfoDepth, "Depth");
        }

        {
            auto& card = addCard("LFO 2", category);
            card.addCombo(apvts, ParamIDs::lfo2Waveform, "Wave");
            card.addCombo(apvts, ParamIDs::lfo2SyncDivision, "Sync");
            card.addCombo(apvts, ParamIDs::lfo2Target, "Target");
            lfoShapePreview2 =
                std::make_unique<LfoShapePreview>(apvts, ParamIDs::lfo2Waveform, ParamIDs::lfo2Depth,
                                                    ParamIDs::lfo2SyncDivision, ParamIDs::lfo2RateHz,
                                                    GexexLookAndFeel::categoryColour(category));
            card.addCustomComponent(*lfoShapePreview2, 44);
            ++category;
            card.addKnob(apvts, ParamIDs::lfo2RateHz, "Rate");
            card.addKnob(apvts, ParamIDs::lfo2Depth, "Depth");
        }

        {
            auto& card = addCard("Envelope", category);
            envelopeEditor = std::make_unique<EnvelopeEditor>(apvts, ParamIDs::envAttack, ParamIDs::envDecay,
                                                                ParamIDs::envSustain, ParamIDs::envRelease);
            envelopeEditor->setAccentColour(GexexLookAndFeel::categoryColour(category));
            ++category;
            card.addCustomComponent(*envelopeEditor, 130);
        }

        {
            auto& card = addCard("Mod Envelope", category);
            modEnvelopeEditor = std::make_unique<EnvelopeEditor>(apvts, ParamIDs::modEnvAttack, ParamIDs::modEnvDecay,
                                                                    ParamIDs::modEnvSustain, ParamIDs::modEnvRelease);
            modEnvelopeEditor->setAccentColour(GexexLookAndFeel::categoryColour(category));
            ++category;
            card.addCustomComponent(*modEnvelopeEditor, 130);
            card.addCombo(apvts, ParamIDs::modEnvTarget, "Target");
            card.addKnob(apvts, ParamIDs::modEnvDepth, "Depth");
        }

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

        // ============================================================
        // Section: Effects -- signal-flow order (Drive -> Bitcrush ->
        // Delay/Reverb sends -> Chorus -> Phaser/Flanger -> Saturator).
        // ============================================================
        addSection("Effects", GexexLookAndFeel::categoryColour(category));

        {
            // The insert chain's execution order/inclusion -- 7 slots
            // (Empty or one of the serial effects below), backed by the
            // same fxSlot0..6 params as always, but edited as an actual
            // drag-to-reorder list instead of 7 stacked dropdowns (see
            // SignalChainEditor.h). Delay/Reverb aren't here -- they're
            // parallel sends, not inserts, and stay at their own fixed
            // point in the signal flow (see EffectsChain.h's class comment).
            auto& card = addCard("Signal Chain", category++);
            signalChainEditor = std::make_unique<SignalChainEditor>(apvts);
            card.addCustomComponent(*signalChainEditor, numFxSlots * 26);
        }

        {
            auto& card = addCard("Drive", category++);
            card.addKnob(apvts, ParamIDs::driveAmount, "Amount");
        }
        {
            auto& card = addCard("Bitcrush", category++);
            card.addKnob(apvts, ParamIDs::crushBits, "Depth");
            card.addKnob(apvts, ParamIDs::crushDownsample, "Downsmp");
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
            auto& card = addCard("Frequency Shifter", category++);
            card.addKnob(apvts, ParamIDs::freqShiftHz, "Shift");
            card.addKnob(apvts, ParamIDs::freqShiftMix, "Mix");
        }

        {
            auto& card = addCard("Multiband Comp", category++);
            card.addKnob(apvts, ParamIDs::mbCrossoverLow, "X-Low");
            card.addKnob(apvts, ParamIDs::mbCrossoverHigh, "X-High");
            card.addKnob(apvts, ParamIDs::mbAmount, "Amount");
            card.addKnob(apvts, ParamIDs::mbLowGain, "Low");
            card.addKnob(apvts, ParamIDs::mbMidGain, "Mid");
            card.addKnob(apvts, ParamIDs::mbHighGain, "High");
            card.addKnob(apvts, ParamIDs::mbMix, "Mix");
        }

        // ============================================================
        // Section: Master.
        // ============================================================
        addSection("Master", GexexLookAndFeel::categoryColour(category));
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
        flowItems.push_back({ card, false });
        return *card;
    }

    void ModuleRack::addSection(const juce::String& title, juce::Colour accent)
    {
        auto* header = sectionHeaders.add(new SectionHeader(title, accent));
        content.addAndMakeVisible(header);
        flowItems.push_back({ header, true });
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
        const int scrollbarAllowance = juce::jmax(24, viewport.getScrollBarThickness() + 8);
        const int rawWidth = getWidth() > 0 ? getWidth() : 900;
        const int viewportWidth = juce::jmax(cardWidth, rawWidth - scrollbarAllowance);
        const int perRow = juce::jmax(1, (viewportWidth - cardGap) / (cardWidth + cardGap));
        const int rowSpanWidth = perRow * (cardWidth + cardGap) + cardGap;

        // Pass 1: group flowItems into rows -- a header always starts a
        // fresh row of its own; cards accumulate up to perRow per row.
        // Every card in a row is later stretched to that row's tallest
        // card's height rather than keeping its own natural height, which
        // is what keeps the grid from reading as a ragged, distracting
        // mismatch when neighboring modules have very different control
        // counts (e.g. Envelope next to a 3-knob effect).
        struct Row
        {
            std::vector<ModuleCard*> rowCards;
            SectionHeader* header = nullptr;
            int height = 0;
        };
        std::vector<Row> rows;

        for (auto& item : flowItems)
        {
            if (item.isSectionHeader)
            {
                rows.push_back({ {}, static_cast<SectionHeader*>(item.component), sectionHeaderHeight });
                continue;
            }

            auto* card = static_cast<ModuleCard*>(item.component);
            const int h = card->getPreferredHeight(cardWidth);

            if (rows.empty() || rows.back().header != nullptr || (int) rows.back().rowCards.size() >= perRow)
                rows.push_back({});

            auto& row = rows.back();
            row.rowCards.push_back(card);
            row.height = juce::jmax(row.height, h);
        }

        // Pass 2: lay out each row at its equalized height.
        int y = cardGap;
        for (auto& row : rows)
        {
            if (row.header != nullptr)
            {
                row.header->setBounds(cardGap, y, rowSpanWidth - cardGap * 2, row.height);
            }
            else
            {
                int x = cardGap;
                for (auto* card : row.rowCards)
                {
                    card->setBounds(x, y, cardWidth, row.height);
                    x += cardWidth + cardGap;
                }
            }
            y += row.height + cardGap;
        }

        // Extra room below the last row -- PerformanceStrip is a fixed
        // overlay docked at the window's bottom (outside this viewport),
        // and its grass overlaps ~46px up into the rack area. Without this,
        // scrolling all the way down tucks the last row's cards behind the
        // grass instead of clearing it.
        content.setSize(rowSpanWidth, y + bottomScrollPadding);
    }

    void ModuleRack::resized()
    {
        viewport.setBounds(getLocalBounds());
        layOutCards();
    }
}
