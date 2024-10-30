/*
  ==============================================================================

    ClickSettingsKnobRack.h
    Created: 29 Oct 2024 12:56:41am
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginEditor.h"

class ClickSettingsKnobRack : public juce::Component {

public:
    ClickSettingsKnobRack(BugsoundsAudioProcessor& processor)
        : audioProcessor(processor)
    {
        //set up the rack's title
        titleLabel.setFont(juce::Font(16.0f));
        titleLabel.setJustificationType(juce::Justification::left);
        titleLabel.setText("Click randomness settings", juce::dontSendNotification);
        addAndMakeVisible(titleLabel);

        //set up the knobs
        initializeKnob(clickTimingKnob, timingLabel, "Timing", "Click Timing Random", timingAttachment);
        initializeKnob(clickVolumeKnob, volumeLabel, "Volume", "Click Volume Random", volumeAttachment);
        initializeKnob(clickPitchKnob, pitchLabel, "Pitch", "Click Pitch Random", pitchAttachment);
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


    void resized() override
    {
        const int margin = 5;
        auto bounds = getLocalBounds().reduced(margin);

        // Position title
        auto titleHeight = 30;
        auto titleBounds = bounds.removeFromTop(titleHeight);
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
        const int upwardOffset = 10;  // Adjust this value to move knobs up/down

        // Position knobs and labels with upward offset
        auto timingBounds = knobSection.removeFromLeft(knobWidth);
        clickTimingKnob.setBounds(timingBounds.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
        timingLabel.setBounds(timingBounds.removeFromBottom(labelHeight).translated(0, -upwardOffset));

        auto volumeBounds = knobSection.removeFromLeft(knobWidth);
        clickVolumeKnob.setBounds(volumeBounds.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
        volumeLabel.setBounds(volumeBounds.removeFromBottom(labelHeight).translated(0, -upwardOffset));

        auto pitchBounds = knobSection;
        clickPitchKnob.setBounds(pitchBounds.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
        pitchLabel.setBounds(pitchBounds.removeFromBottom(labelHeight).translated(0, -upwardOffset));
    }
    

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    const int margin = 5;
    juce::Slider clickTimingKnob, clickVolumeKnob, clickPitchKnob;
    juce::Label titleLabel, timingLabel, volumeLabel, pitchLabel;
    std::unique_ptr<SliderAttachment> timingAttachment;
    std::unique_ptr<SliderAttachment> volumeAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment;

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