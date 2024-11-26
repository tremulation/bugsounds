/*
  ==============================================================================

    ResonatorKnobRack.h
    Created: 25 Nov 2024 2:53:02pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginEditor.h"

class PowerButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = button.getLocalBounds().toFloat();

        // Draw background based on state
        g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
        g.fillRect(bounds);

        // Draw border
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1.0f);

        // Draw power icon (simplified)
        g.setColour(juce::Colours::white);
        auto iconBounds = bounds.reduced(4);
        g.drawLine(iconBounds.getCentreX(), iconBounds.getY(),
            iconBounds.getCentreX(), iconBounds.getBottom(), 2.0f);
        g.drawEllipse(iconBounds.reduced(5), 2.0f);
    }
};


class ResonatorKnobRack : public juce::Component {

public:
    ResonatorKnobRack(BugsoundsAudioProcessor& processor)
        : audioProcessor(processor)
    {
        // Set up power toggle button
        powerButton = std::make_unique<juce::ToggleButton>("");
        powerButton->setClickingTogglesState(true);
        powerButton->setLookAndFeel(&powerButtonLAF);
        addAndMakeVisible(powerButton.get());

        // Set up the rack's title
        titleLabel.setFont(juce::Font(16.0f));
        titleLabel.setJustificationType(juce::Justification::left);
        titleLabel.setText("Resonator Settings", juce::dontSendNotification);
        addAndMakeVisible(titleLabel);

        // Set up the knobs
        initializeKnob(resonatorQKnob, qLabel, "Q", "Resonator Q", qAttachment);
        initializeKnob(resonatorGainKnob, gainLabel, "Gain", "Resonator Gain", gainAttachment);
        initializeKnob(resonatorHarmonicsKnob, harmonicsLabel, "Harmonics", "Resonator Harmonics", harmonicsAttachment);

        // Set up power button attachment
        powerButtonAttachment = std::make_unique<ButtonAttachment>(
            audioProcessor.apvts, "Resonator On", *powerButton);
    }


    void resized() override
    {
        const int margin = 5;
        auto bounds = getLocalBounds().reduced(margin);

        // Position title row with power button
        auto titleHeight = 30;
        auto titleBounds = bounds.removeFromTop(titleHeight);

        // Position power button on the far left
        auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight).reduced(5);
        powerButton->setBounds(powerButtonBounds);

        // Position title next to power button
        titleLabel.setBounds(titleBounds.reduced(5, 0));

        // Add spacing after title and separator line
        bounds.removeFromTop(5);

        // Calculate knob layout within the rack
        auto knobSection = bounds;
        auto numKnobs = 3;
        auto knobWidth = knobSection.getWidth() / numKnobs;
        auto knobSize = std::min(knobWidth, knobSection.getHeight()) - 20;
        auto labelHeight = 20;

        // Define upward offset for knobs
        const int upwardOffset = 10;

        // Position knobs and labels with upward offset
        auto qBounds = knobSection.removeFromLeft(knobWidth);
        resonatorQKnob.setBounds(qBounds.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
        qLabel.setBounds(qBounds.removeFromBottom(labelHeight).translated(0, -upwardOffset));

        auto gainBounds = knobSection.removeFromLeft(knobWidth);
        resonatorGainKnob.setBounds(gainBounds.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
        gainLabel.setBounds(gainBounds.removeFromBottom(labelHeight).translated(0, -upwardOffset));

        auto harmonicsBounds = knobSection;
        resonatorHarmonicsKnob.setBounds(harmonicsBounds.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
        harmonicsLabel.setBounds(harmonicsBounds.removeFromBottom(labelHeight).translated(0, -upwardOffset));
    }


    void paint(juce::Graphics& g) override
    {
        // Fill background
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        auto bounds = getLocalBounds();
        auto contentBounds = bounds.reduced(margin);

        // Draw outer white border
        g.setColour(juce::Colours::white);
        g.drawRect(contentBounds);

        // Draw border under title
        auto titleBounds = contentBounds.removeFromTop(30);
        g.drawLine(titleBounds.getX(),
            titleBounds.getBottom(),
            titleBounds.getRight(),
            titleBounds.getBottom(),
            1.0f);
    }


private:

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    const int margin = 5;
    juce::Slider resonatorQKnob, resonatorGainKnob, resonatorHarmonicsKnob;
    juce::Label titleLabel, qLabel, gainLabel, harmonicsLabel;
    PowerButtonLookAndFeel powerButtonLAF;
    std::unique_ptr<juce::ToggleButton> powerButton;
    

    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> harmonicsAttachment;
    std::unique_ptr<ButtonAttachment> powerButtonAttachment;

    BugsoundsAudioProcessor& audioProcessor;

    void initializeKnob(juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 70, 20);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        addAndMakeVisible(slider);

        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);

        attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramName, slider);
    }
};