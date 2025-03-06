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
        initializeKnob(clickTimingKnob, timingLabel, "Time Rand", "Click Timing Random", timingAttachment);
        initializeKnob(clickPitchKnob, pitchLabel, "Pitch Rand", "Click Pitch Random", pitchAttachment);
        initializeKnob(adRatioKnob, adRatioLabel, "A/D Ratio", "Click Atack Decay Ratio", adRatioAttachment);
		initializeKnob(lowFreqAttenuationKnob, lowFreqAttenuationLabel, "Freq Fadeout", "Click Low Frequency Attenuation", lowFreqAttenuationAttachment);
		initializeKnob(minVolFreqKnob, minVolFreqLabel, "Min Vol Freq", "Click Min Volume Frequency", minVolFreqAttachment);
		initializeKnob(maxVolFreqKnob, maxVolFreqLabel, "Max Vol Freq", "Click Max Volume Frequency", maxVolFreqAttachment);
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

        // Position the title at the top.
        const int titleHeight = 30;
        auto titleBounds = bounds.removeFromTop(titleHeight);
        titleLabel.setBounds(titleBounds.reduced(5, 0));

        // Add some spacing after the title.
        bounds.removeFromTop(5);

        // Divide the remaining area into two rows.
        auto rowHeight = bounds.getHeight() / 2;
        auto topRow = bounds.removeFromTop(rowHeight);
        auto bottomRow = bounds; // what's left is the bottom row

        // Hardcoded knob size and label height.
        const int knobSize = 60;
        const int labelHeight = 20;

        // New variables: loweringOffset shifts knobs downward, and labelSpacing adds extra space between knobs and labels.
        const int loweringOffset = -10;
        const int labelSpacing = 5;

        // ----- Top row layout (Timing, Pitch, and A/D Ratio) -----
        int topColumnWidth = topRow.getWidth() / 3;

        auto topCol0 = topRow.removeFromLeft(topColumnWidth);
        clickTimingKnob.setBounds(topCol0.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
        timingLabel.setBounds(topCol0.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

        auto topCol1 = topRow.removeFromLeft(topColumnWidth);
        clickPitchKnob.setBounds(topCol1.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
        pitchLabel.setBounds(topCol1.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

        auto topCol2 = topRow; // remaining area
        adRatioKnob.setBounds(topCol2.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
        adRatioLabel.setBounds(topCol2.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

        // ----- Bottom row layout (Min Vol Freq, Max Vol Freq, and Low Freq Attenuation) -----
        int bottomColumnWidth = bottomRow.getWidth() / 3;

        auto bottomCol0 = bottomRow.removeFromLeft(bottomColumnWidth);
        minVolFreqKnob.setBounds(bottomCol0.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
        minVolFreqLabel.setBounds(bottomCol0.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

        auto bottomCol1 = bottomRow.removeFromLeft(bottomColumnWidth);
        maxVolFreqKnob.setBounds(bottomCol1.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
        maxVolFreqLabel.setBounds(bottomCol1.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

        auto bottomCol2 = bottomRow; // remaining area
        lowFreqAttenuationKnob.setBounds(bottomCol2.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
        lowFreqAttenuationLabel.setBounds(bottomCol2.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    }

    //void paintOverChildren(juce::Graphics& g) override
    //{
    //    g.setColour(juce::Colours::red);
    //    // Draw a 2-pixel wide rectangle around each knob.
    //    g.drawRect(clickTimingKnob.getBounds().toFloat(), 2);
    //    g.drawRect(clickPitchKnob.getBounds().toFloat(), 2);
    //    g.drawRect(adRatioKnob.getBounds().toFloat(), 2);
    //    g.drawRect(minVolFreqKnob.getBounds().toFloat(), 2);
    //    g.drawRect(maxVolFreqKnob.getBounds().toFloat(), 2);
    //    g.drawRect(lowFreqAttenuationKnob.getBounds().toFloat(), 2);
    //}


private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    const int margin = 5;
    juce::Slider clickTimingKnob, clickPitchKnob, adRatioKnob, lowFreqAttenuationKnob, minVolFreqKnob, maxVolFreqKnob;
    juce::Label titleLabel, timingLabel, pitchLabel, adRatioLabel, lowFreqAttenuationLabel, minVolFreqLabel, maxVolFreqLabel;
    std::unique_ptr<SliderAttachment> timingAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<SliderAttachment> adRatioAttachment;
    std::unique_ptr<SliderAttachment> lowFreqAttenuationAttachment;
    std::unique_ptr<SliderAttachment> minVolFreqAttachment;
    std::unique_ptr<SliderAttachment> maxVolFreqAttachment;


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