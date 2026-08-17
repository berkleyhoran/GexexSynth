#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace gexex
{
    // Draggable-node ADSR editor: draws the envelope as a shape on a grid
    // (attack ramp -> decay slope -> sustain plateau -> release tail) with
    // three draggable dots instead of four plain sliders. Directly
    // manipulates the envAttack/envDecay/envSustain/envRelease APVTS
    // parameters via setValueNotifyingHost -- there's no Slider underneath
    // (the shape itself *is* the control), so gestures are managed by hand
    // (beginChangeGesture/endChangeGesture) to stay host-automation-safe.
    //
    // The x-axis is *not* real seconds: attack/decay/release each get a
    // fixed visual-width budget and are plotted at their own normalized
    // (0..1) position within it, same convention most commercial synth
    // envelope displays use -- a literal seconds axis would make a 2ms
    // attack indistinguishable from an 800ms one right next to a 5s
    // release. A parameter's normalized value maps 1:1 onto its fraction
    // of that width, which conveniently means the pixel math and the
    // setValueNotifyingHost math are the same fraction -- no unit
    // conversion needed in either direction.
    class EnvelopeEditor : public juce::Component, private juce::Timer
    {
    public:
        EnvelopeEditor(juce::AudioProcessorValueTreeState& state, const juce::String& attackID,
                        const juce::String& decayID, const juce::String& sustainID, const juce::String& releaseID)
            : apvts(state),
              attackParam(apvts.getParameter(attackID)),
              decayParam(apvts.getParameter(decayID)),
              sustainParam(apvts.getParameter(sustainID)),
              releaseParam(apvts.getParameter(releaseID))
        {
            startTimerHz(30); // repaint if a host/automation move changes a param we didn't drag ourselves
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(10.0f, 6.0f);

            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

            drawGrid(g, bounds);

            const auto points = computePoints(bounds);

            // Filled shape under the envelope curve.
            juce::Path fillPath;
            fillPath.startNewSubPath(points.p0);
            fillPath.lineTo(points.p1);
            fillPath.lineTo(points.p2);
            fillPath.lineTo(points.p3);
            fillPath.lineTo(points.p4);
            fillPath.lineTo(points.p4.x, bounds.getBottom());
            fillPath.lineTo(points.p0.x, bounds.getBottom());
            fillPath.closeSubPath();
            g.setColour(accentColour.withAlpha(0.18f));
            g.fillPath(fillPath);

            // The envelope outline itself.
            juce::Path outline;
            outline.startNewSubPath(points.p0);
            outline.lineTo(points.p1);
            outline.lineTo(points.p2);
            outline.lineTo(points.p3);
            outline.lineTo(points.p4);
            g.setColour(accentColour);
            g.strokePath(outline,
                          juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Draggable dots.
            drawHandle(g, points.p1, draggingHandle == Handle::Attack || hoveredHandle == Handle::Attack);
            drawHandle(g, points.p2, draggingHandle == Handle::Decay || hoveredHandle == Handle::Decay);
            drawHandle(g, points.p4, draggingHandle == Handle::Release || hoveredHandle == Handle::Release);

            g.setColour(juce::Colour(0xFF4D6B82));
            g.setFont(juce::FontOptions(10.5f));
            g.drawText("A", juce::Rectangle<float>(points.p1.x - 10, bounds.getBottom() + 1, 20, 12),
                       juce::Justification::centred);
            g.drawText("D", juce::Rectangle<float>(points.p2.x - 10, bounds.getBottom() + 1, 20, 12),
                       juce::Justification::centred);
            g.drawText("R", juce::Rectangle<float>(points.p4.x - 10, bounds.getBottom() + 1, 20, 12),
                       juce::Justification::centred);
        }

        void resized() override { repaint(); }

        void mouseMove(const juce::MouseEvent& e) override
        {
            const auto points = computePoints(getLocalBounds().toFloat().reduced(10.0f, 6.0f));
            const auto newHover = hitTest(e.position, points);
            if (newHover != hoveredHandle)
            {
                hoveredHandle = newHover;
                setMouseCursor(hoveredHandle == Handle::None ? juce::MouseCursor::NormalCursor
                                                              : juce::MouseCursor::PointingHandCursor);
                repaint();
            }
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            const auto points = computePoints(getLocalBounds().toFloat().reduced(10.0f, 6.0f));
            draggingHandle = hitTest(e.position, points);
            if (draggingHandle == Handle::None)
                return;

            if (draggingHandle == Handle::Attack)
                beginGesture(attackParam);
            else if (draggingHandle == Handle::Decay)
            {
                beginGesture(decayParam);
                beginGesture(sustainParam);
            }
            else if (draggingHandle == Handle::Release)
                beginGesture(releaseParam);

            mouseDrag(e);
        }

        void mouseDrag(const juce::MouseEvent& e) override
        {
            if (draggingHandle == Handle::None)
                return;

            const auto bounds = getLocalBounds().toFloat().reduced(10.0f, 6.0f);
            const float attackW = bounds.getWidth() * attackFraction;
            const float decayW = bounds.getWidth() * decayFraction;
            const float releaseW = bounds.getWidth() * releaseFraction;
            const float plateauW = bounds.getWidth() * plateauFraction;

            if (draggingHandle == Handle::Attack)
            {
                const float x = juce::jlimit(0.0f, attackW, e.position.x - bounds.getX());
                setParamNormalized(attackParam, attackW > 0.0f ? x / attackW : 0.0f);
            }
            else if (draggingHandle == Handle::Decay)
            {
                const float decayOriginX = bounds.getX() + attackW;
                const float x = juce::jlimit(0.0f, decayW, e.position.x - decayOriginX);
                setParamNormalized(decayParam, decayW > 0.0f ? x / decayW : 0.0f);

                const float y = juce::jlimit(0.0f, 1.0f, (e.position.y - bounds.getY()) / bounds.getHeight());
                setParamNormalized(sustainParam, 1.0f - y); // y-down -> level-up
            }
            else if (draggingHandle == Handle::Release)
            {
                const float releaseOriginX = bounds.getX() + attackW + decayW + plateauW;
                const float x = juce::jlimit(0.0f, releaseW, e.position.x - releaseOriginX);
                setParamNormalized(releaseParam, releaseW > 0.0f ? x / releaseW : 0.0f);
            }

            repaint();
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            if (draggingHandle == Handle::Attack)
                endGesture(attackParam);
            else if (draggingHandle == Handle::Decay)
            {
                endGesture(decayParam);
                endGesture(sustainParam);
            }
            else if (draggingHandle == Handle::Release)
                endGesture(releaseParam);

            draggingHandle = Handle::None;
            repaint();
        }

    private:
        enum class Handle
        {
            None,
            Attack,
            Decay,
            Release
        };

        struct Points
        {
            juce::Point<float> p0, p1, p2, p3, p4;
        };

        static void beginGesture(juce::RangedAudioParameter* p)
        {
            if (p != nullptr)
                p->beginChangeGesture();
        }
        static void endGesture(juce::RangedAudioParameter* p)
        {
            if (p != nullptr)
                p->endChangeGesture();
        }
        static void setParamNormalized(juce::RangedAudioParameter* p, float normalized)
        {
            if (p != nullptr)
                p->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, normalized));
        }
        static float getParamNormalized(juce::RangedAudioParameter* p)
        {
            return p != nullptr ? p->getValue() : 0.0f;
        }

        Points computePoints(juce::Rectangle<float> bounds) const
        {
            const float attackW = bounds.getWidth() * attackFraction * getParamNormalized(attackParam);
            const float decayW = bounds.getWidth() * decayFraction * getParamNormalized(decayParam);
            const float plateauW = bounds.getWidth() * plateauFraction;
            const float releaseW = bounds.getWidth() * releaseFraction * getParamNormalized(releaseParam);
            const float sustainLevel = getParamNormalized(sustainParam);

            const float top = bounds.getY();
            const float bottom = bounds.getBottom();
            const float sustainY = juce::jmap(sustainLevel, 0.0f, 1.0f, bottom, top);

            Points pts;
            pts.p0 = { bounds.getX(), bottom };
            pts.p1 = { bounds.getX() + attackW, top };
            pts.p2 = { bounds.getX() + attackW + decayW, sustainY };
            pts.p3 = { bounds.getX() + attackW + decayW + plateauW, sustainY };
            pts.p4 = { pts.p3.x + releaseW, bottom };
            return pts;
        }

        static Handle hitTest(juce::Point<float> pos, const Points& pts)
        {
            constexpr float grabRadius = 11.0f;
            if (pos.getDistanceFrom(pts.p1) <= grabRadius)
                return Handle::Attack;
            if (pos.getDistanceFrom(pts.p2) <= grabRadius)
                return Handle::Decay;
            if (pos.getDistanceFrom(pts.p4) <= grabRadius)
                return Handle::Release;
            return Handle::None;
        }

        void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds) const
        {
            g.setColour(juce::Colour(0x18163049));
            for (int i = 1; i < 4; ++i)
            {
                const float y = bounds.getY() + bounds.getHeight() * (float) i / 4.0f;
                g.drawHorizontalLine((int) y, bounds.getX(), bounds.getRight());
            }
        }

        void drawHandle(juce::Graphics& g, juce::Point<float> p, bool active) const
        {
            const float r = active ? 6.5f : 5.0f;
            g.setColour(juce::Colours::black.withAlpha(0.15f));
            g.fillEllipse(p.x - r, p.y - r + 1.0f, r * 2.0f, r * 2.0f);
            g.setColour(active ? accentColour.brighter(0.3f) : accentColour);
            g.fillEllipse(p.x - r, p.y - r, r * 2.0f, r * 2.0f);
            g.setColour(juce::Colours::white);
            g.drawEllipse(p.x - r, p.y - r, r * 2.0f, r * 2.0f, 1.4f);
        }

        void timerCallback() override
        {
            const float a = getParamNormalized(attackParam);
            const float d = getParamNormalized(decayParam);
            const float s = getParamNormalized(sustainParam);
            const float r = getParamNormalized(releaseParam);
            if (a != lastAttack || d != lastDecay || s != lastSustain || r != lastRelease)
            {
                lastAttack = a;
                lastDecay = d;
                lastSustain = s;
                lastRelease = r;
                repaint();
            }
        }

        static constexpr float attackFraction = 0.24f;
        static constexpr float decayFraction = 0.28f;
        static constexpr float plateauFraction = 0.16f;
        static constexpr float releaseFraction = 0.28f;

        juce::AudioProcessorValueTreeState& apvts;
        juce::RangedAudioParameter* attackParam;
        juce::RangedAudioParameter* decayParam;
        juce::RangedAudioParameter* sustainParam;
        juce::RangedAudioParameter* releaseParam;

        juce::Colour accentColour = juce::Colour(0xFF3DBE6C);
        Handle draggingHandle = Handle::None;
        Handle hoveredHandle = Handle::None;
        float lastAttack = -1.0f, lastDecay = -1.0f, lastSustain = -1.0f, lastRelease = -1.0f;

    public:
        void setAccentColour(juce::Colour c) { accentColour = c; }
    };
}
