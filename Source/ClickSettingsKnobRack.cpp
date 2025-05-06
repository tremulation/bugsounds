/*
  ==============================================================================

    ClickSettingsKnobRack.cpp
    Created: 4 May 2025 2:02:09am
    Author:  Taro

  ==============================================================================
*/

#include "ClickSettingsKnobRack.h"
#include "PluginEditor.h"

ClickSettingsKnobRack::ClickSettingsKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor)
    : audioProcessor(processor), audioEditor(editor)
{
    //set up the rack's title
    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Click settings", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    //set up the knobs
    initializeKnob(clickTimingKnob, timingLabel, "Time Rand", "Click Timing Random", timingAttachment);
    initializeKnob(clickPitchKnob, pitchLabel, "Pitch Rand", "Click Pitch Random", pitchAttachment);
    initializeKnob(adRatioKnob, adRatioLabel, "A/D Ratio", "Click Atack Decay Ratio", adRatioAttachment);
    initializeKnob(startFadeoutknob, startFadeoutLabel, "Start Fade", "Click Start Fadeout", lowFreqAttenuationAttachment);
    initializeKnob(floorFreqKnob, floorFreqLabel, "Floor Freq", "Click Floor Frequency", maxVolFreqAttachment);
    initializeKnob(startJitterKnob, startJitterLabel, "Start Jitter", "Click Start Jitter", minVolFreqAttachment);

    //setup click vol slider
    clickVolumeLabel.setText("vol", juce::dontSendNotification);
    clickVolumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(clickVolumeLabel);
    clickVolumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    clickVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(clickVolumeSlider);
    clickVolumeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Click Volume", clickVolumeSlider);

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("clickSettings"); });
    addAndMakeVisible(helpButton.get());
}


void ClickSettingsKnobRack::paint(juce::Graphics& g) {
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



void ClickSettingsKnobRack::resized() {
    const int margin = 5;
    auto bounds = getLocalBounds().reduced(margin);

    //position the title at the top.
    const int titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    //click volume slider
    const int volLabelWidth = 30;
    const int volSliderWidth = 100;
    auto sliderContainer = titleBounds.reduced(5, 5).removeFromRight(volLabelWidth + volSliderWidth).withTrimmedLeft(10);

    auto helpArea = sliderContainer.removeFromRight(titleHeight - 10);
    helpButton->setBounds(helpArea);

    clickVolumeLabel.setBounds(sliderContainer.removeFromLeft(volLabelWidth));
    clickVolumeSlider.setBounds(sliderContainer.removeFromLeft(volSliderWidth));

    //add some spacing after the title.
    bounds.removeFromTop(5);

    //divide the remaining area into two rows.
    auto rowHeight = bounds.getHeight() / 2;
    auto topRow = bounds.removeFromTop(rowHeight);
    auto bottomRow = bounds; // what's left is the bottom row

    //hardcoded knob size and label height.
    const int knobSize = 60;
    const int labelHeight = 20;

    //loweringOffset shifts knobs downward, and labelSpacing adds extra space between knobs and labels.
    const int loweringOffset = -10;
    const int labelSpacing = 5;

    // ----- top row layout (timing, pitch, and A/D Ratio) -----
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

    // ----- bottom row layout (min Vol Freq, max Vol Freq, and low freq attenuation) -----
    int bottomColumnWidth = bottomRow.getWidth() / 3;

    auto bottomCol0 = bottomRow.removeFromLeft(bottomColumnWidth);
    floorFreqKnob.setBounds(bottomCol0.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    floorFreqLabel.setBounds(bottomCol0.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto bottomCol1 = bottomRow.removeFromLeft(bottomColumnWidth);
    startJitterKnob.setBounds(bottomCol1.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    startJitterLabel.setBounds(bottomCol1.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto bottomCol2 = bottomRow; //remaining area
    startFadeoutknob.setBounds(bottomCol2.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    startFadeoutLabel.setBounds(bottomCol2.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
}


void ClickSettingsKnobRack::initializeKnob(juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 70, 20);
    slider.setPopupDisplayEnabled(true, true, nullptr);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramName, slider);
}





