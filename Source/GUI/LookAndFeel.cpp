#include "LookAndFeel.h"

namespace
{
    // Reimplements LookAndFeel_V2's private SliderLabelComp (the Label
    // JUCE's stock createSliderTextBox() returns) with one difference:
    // its mouseWheelMove is a no-op forwarding override instead of a
    // fully empty one, so a wheel event over a knob's text box reaches
    // the enclosing Viewport instead of vanishing. See LookAndFeel.h's
    // comment on createSliderTextBox for why this exists.
    class ScrollThroughSliderLabel : public juce::Label
    {
    public:
        ScrollThroughSliderLabel() : Label({}, {}) {}

        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
        {
            if (auto* parent = getParentComponent())
                parent->mouseWheelMove(e.getEventRelativeTo(parent), wheel);
        }
    };
} // namespace

namespace gexex
{
    const juce::Identifier GexexLookAndFeel::accentColourPropertyID { "accentColour" };

    GexexLookAndFeel::GexexLookAndFeel()
    {
        // A light glass scheme (LookAndFeel_V4's stock schemes are all
        // dark) -- built from the reference's own aero palette
        // (gexex/synth.html's --ink/--bg/--card custom properties) rather
        // than a generic light theme, so every stock widget (ComboBox,
        // ToggleButton, Label, scrollbars, ...) already matches without
        // needing per-widget overrides.
        setColourScheme({
            juce::Colour(0xFFEAF6FF), // windowBackground
            juce::Colour(0xE6FFFFFF), // widgetBackground
            juce::Colour(0xFFFFFFFF), // menuBackground
            juce::Colour(0x40163049), // outline
            juce::Colour(0xFF163049), // defaultText
            juce::Colour(0xFFDCEFFB), // defaultFill
            juce::Colour(0xFF163049), // highlightedText
            juce::Colour(0xFFB8E2FA), // highlightedFill
            juce::Colour(0xFF163049), // menuText
        });

        setColour(juce::Slider::textBoxTextColourId, inkColour());
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, inkColour());
        setColour(juce::ComboBox::outlineColourId, panelBorderColour());
        setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFFFF3B6E));

        // Every knob's numeric readout is a Slider text box, and
        // LookAndFeel_V4::createSliderTextBox() bakes its colours in as
        // *explicit* overrides on the Label/TextEditor it creates --
        // permanently, at the moment the Slider's setTextBoxStyle() is
        // called (see Knob.h), not re-resolved later. Since Knob objects
        // are built as part of ModuleRack's member-initializer (before
        // PluginEditor's constructor body ever reaches its own
        // setLookAndFeel(&lookAndFeel) call), every one of those text
        // boxes was baking in stock LookAndFeel_V4's default *dark-scheme*
        // text colour (near-white) instead of ours -- invisible on this
        // light aero background. Installing this instance as the JUCE-wide
        // default right here, in its own constructor, guarantees it's
        // already active before any child widget can be constructed,
        // however early that happens. (Clicking into one of those boxes
        // auto-selects its full text -- JUCE's Slider does that itself --
        // and *selected* text draws in highlightedTextColourId, a
        // different id we'd never set, which happens to default to black:
        // that's the "white normally, black when clicked" split.)
        juce::LookAndFeel::setDefaultLookAndFeel(this);

        setColour(juce::TextEditor::textColourId, inkColour());
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFFFFFFFF));
        setColour(juce::TextEditor::outlineColourId, panelBorderColour());
        setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFF2AA9C9));
        setColour(juce::TextEditor::highlightColourId, juce::Colour(0xFFB8E2FA));
        setColour(juce::TextEditor::highlightedTextColourId, inkColour());
        setColour(juce::CaretComponent::caretColourId, inkColour());
    }

    juce::Label* GexexLookAndFeel::createSliderTextBox(juce::Slider& slider)
    {
        // Mirrors LookAndFeel_V2::createSliderTextBox() exactly (colours,
        // justification, keyboard type) but for a ScrollThroughSliderLabel
        // instead of its private SliderLabelComp -- see this header's
        // comment and the anonymous namespace above for why.
        auto* l = new ScrollThroughSliderLabel();
        l->setJustificationType(juce::Justification::centred);
        l->setKeyboardType(juce::TextInputTarget::decimalKeyboard);
        l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
        l->setColour(juce::Label::backgroundColourId, slider.findColour(juce::Slider::textBoxBackgroundColourId));
        l->setColour(juce::Label::outlineColourId, slider.findColour(juce::Slider::textBoxOutlineColourId));
        l->setColour(juce::TextEditor::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
        l->setColour(juce::TextEditor::backgroundColourId, slider.findColour(juce::Slider::textBoxBackgroundColourId));
        l->setColour(juce::TextEditor::outlineColourId, slider.findColour(juce::Slider::textBoxOutlineColourId));
        l->setColour(juce::TextEditor::highlightColourId, slider.findColour(juce::Slider::textBoxHighlightColourId));
        return l;
    }

    juce::Colour GexexLookAndFeel::categoryColour(int categoryIndex) noexcept
    {
        constexpr float goldenAngle = 0.6180339887f;
        const float hue = std::fmod((float) categoryIndex * goldenAngle, 1.0f);
        return juce::Colour::fromHSV(hue, 0.58f, 0.92f, 1.0f);
    }

    juce::Font GexexLookAndFeel::getLabelFont(juce::Label&)
    {
        return juce::FontOptions(13.0f);
    }

    juce::Font GexexLookAndFeel::getComboBoxFont(juce::ComboBox&)
    {
        return juce::FontOptions(13.0f);
    }

    void GexexLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider& slider)
    {
        const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(3.0f);
        const auto centre = bounds.getCentre();
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto knobRadius = radius * 0.66f;
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        const auto accent = juce::Colour((juce::uint32) (int) slider.getProperties().getWithDefault(
            accentColourPropertyID, (int) juce::Colours::grey.getARGB()));

        // Drop shadow -- the "sitting on glass" depth cue.
        g.setColour(juce::Colours::black.withAlpha(0.16f));
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius + 2.5f, knobRadius * 2.0f, knobRadius * 2.0f);

        // Value arc: dim track for the full range, bright fill up to the
        // current value.
        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(GexexLookAndFeel::inkColour().withAlpha(0.16f));
        g.strokePath(track,
                      juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path fill;
        fill.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(accent);
        g.strokePath(fill, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Glossy plastic knob body.
        juce::ColourGradient body(accent.brighter(0.55f), centre.x - knobRadius * 0.35f,
                                    centre.y - knobRadius * 0.4f, accent.darker(0.25f), centre.x,
                                    centre.y + knobRadius, true);
        g.setGradientFill(body);
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        g.setColour(accent.darker(0.6f).withAlpha(0.7f));
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // Specular highlight.
        juce::Path highlight;
        highlight.addEllipse(centre.x - knobRadius * 0.55f, centre.y - knobRadius * 0.78f, knobRadius * 0.85f,
                              knobRadius * 0.5f);
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.fillPath(highlight);

        // Pointer.
        juce::Path pointer;
        const float pointerLength = knobRadius * 0.62f;
        const float pointerThickness = 2.4f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -knobRadius + 3.0f, pointerThickness, pointerLength,
                                     pointerThickness * 0.5f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre));
        g.setColour(juce::Colours::white);
        g.fillPath(pointer);
    }
}
