/*
  ==============================================================================

    ChorusKnobRack.h
    Created: 21 Apr 2025 11:46:36pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ChorusPositionReadout.h"
#include "ButtonsAndStuff.h"
#include "AnimatedKnob.h"
#include "UIDrawer.h"

class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;


//==============================================================================
class ChorusKnobRack : public juce::Component, public UIDrawer {
public:
    ChorusKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    ~ChorusKnobRack() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Member variables
    const int margin = 5;
    
    juce::Label countLabel, spreadLabel, distanceLabel, cooldownLabel, correlationLabel;

    AnimatedKnobSlider countKnob{ countLabel };
    AnimatedKnobSlider spreadKnob{ spreadLabel };
    AnimatedKnobSlider distanceKnob{ distanceLabel };
    AnimatedKnobSlider cooldownKnob{ cooldownLabel };
    AnimatedKnobSlider correlationKnob{ correlationLabel };

    std::unique_ptr<juce::ToggleButton> powerButton;
    std::unique_ptr<HelpButton>         helpButton;

    std::unique_ptr<SliderAttachment> countAttachment;
    std::unique_ptr<SliderAttachment> spreadAttachment;
    std::unique_ptr<SliderAttachment> distanceAttachment;
    std::unique_ptr<SliderAttachment> cooldownAttachment;
    std::unique_ptr<SliderAttachment> correlationAttachment;
    std::unique_ptr<ButtonAttachment> powerButtonAttachment;

    juce::Label titleLabel;

    std::unique_ptr<ChorusPositionReadout> positionReadout;

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;

    void initializeKnob(AnimatedKnobSlider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment);

    class RandomizeButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
            const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        { 
            float scale = button.getWidth() / 15.f;
            float padding = 1 * scale;
            g.setColour(Colour::fromString("#1B241B").withAlpha(1.0f));
            Rectangle<float> topRect = button.getLocalBounds().toFloat().reduced(padding).translated(-1.f * scale, -1.f * scale);
            Rectangle<float> bottomRect = button.getLocalBounds().toFloat().reduced(padding);
            if (!shouldDrawButtonAsDown) {
                g.fillRect(topRect);
                g.fillRect(bottomRect);
            } else {
                g.fillRect(bottomRect);
            }
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown) override
        {
            float scale = button.getWidth() / 15.f;
            float padding = 1 * scale;
            g.setColour(juce::Colours::white);
            if (!shouldDrawButtonAsDown) {
                g.drawText("R", button.getLocalBounds().reduced(padding).translated(-1.f * scale, -1.f * scale), juce::Justification::centred);
            } else {
                g.drawText("R", button.getLocalBounds().reduced(padding), juce::Justification::centred);
            }
        }
    };

    RandomizeButtonLookAndFeel randomizeButtonLAF;
    std::unique_ptr<juce::TextButton> randomizeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusKnobRack)
};