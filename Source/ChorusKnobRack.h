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

class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;


//==============================================================================
class ChorusKnobRack : public juce::Component {
public:
    ChorusKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    ~ChorusKnobRack() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void initializeKnob(juce::Slider& slider, juce::Label& label,
        const juce::String& labelText,
        const juce::String& paramName,
        std::unique_ptr<SliderAttachment>& attachment);

    // Member variables
    const int margin = 5;
    juce::Slider countKnob, spreadKnob, distanceKnob, cooldownKnob, correlationKnob;
    juce::Label countLabel, spreadLabel, distanceLabel, cooldownLabel, correlationLabel;

    PowerButtonLookAndFeel powerButtonLAF;
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


    class RandomizeButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
            const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
            g.fillRect(bounds);

            g.setColour(juce::Colours::white);
            g.drawRect(bounds, 1.0f);
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
        {
            auto font = juce::Font(20.0f, juce::Font::bold);
            g.setFont(font);
            g.setColour(juce::Colours::white);
            g.drawText("R", button.getLocalBounds(), juce::Justification::centred);
        }
    };

    RandomizeButtonLookAndFeel randomizeButtonLAF;
    std::unique_ptr<juce::TextButton> randomizeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusKnobRack)
};