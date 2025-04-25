/*
  ==============================================================================

    ChorusKnobRack.cpp
    Created: 21 Apr 2025 11:46:36pm
    Author:  Taro

  ==============================================================================
*/

#include "ChorusKnobRack.h"
#include "PluginEditor.h"


//==============================================================================
ChorusKnobRack::ChorusKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor)
    : audioProcessor(processor), audioEditor(editor)
{
    powerButton = std::make_unique<juce::ToggleButton>("");
    powerButton->setClickingTogglesState(true);
    powerButton->setLookAndFeel(&powerButtonLAF);
    addAndMakeVisible(powerButton.get());

    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Chorus Mode", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    initializeKnob(countKnob, countLabel, "Count", "Chorus Count", countAttachment);
    initializeKnob(spreadKnob, spreadLabel, "Spread", "Chorus Stereo Spread", spreadAttachment);
    initializeKnob(distanceKnob, distanceLabel, "Distance", "Chorus Max Distance", distanceAttachment);
    initializeKnob(excitationKnob, excitationLabel, "Excitation", "Chorus Excitation", excitationAttachment);
	initializeKnob(correlationKnob, correlationLabel, "Correlation", "Chorus Correlation", correlationAttachment);

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Chorus On", *powerButton);
}

void ChorusKnobRack::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(margin);

    g.setColour(juce::Colours::white);
    g.drawRect(contentBounds);

    auto titleBounds = contentBounds.removeFromTop(30);
    g.drawLine(titleBounds.getX(),
        titleBounds.getBottom(),
        titleBounds.getRight(),
        titleBounds.getBottom(),
        1.0f);
}

void ChorusKnobRack::resized()
{
    const int margin = 5;
    auto bounds = getLocalBounds().reduced(margin);

    //title row
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight).reduced(5);
    powerButton->setBounds(powerButtonBounds);
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    //knob layout
    bounds.removeFromTop(5);
    const int knobSize = 60;
    const int labelHeight = 20;
    const int loweringOffset = -10;
    const int labelSpacing = 5;

    //2 rows: 2 knobs top, 3 knobs bottom
    auto topRow = bounds.removeFromTop(bounds.getHeight() / 2);
    auto bottomRow = bounds;

    // Top row
    auto countArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    countKnob.setBounds(countArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    countLabel.setBounds(countArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto spreadArea = topRow;
    spreadKnob.setBounds(spreadArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    spreadLabel.setBounds(spreadArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    // Bottom row
    auto distanceArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 3);
    distanceKnob.setBounds(distanceArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    distanceLabel.setBounds(distanceArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto excitationArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    excitationKnob.setBounds(excitationArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    excitationLabel.setBounds(excitationArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto correlationArea = bottomRow;
    correlationKnob.setBounds(correlationArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    correlationLabel.setBounds(correlationArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
}

void ChorusKnobRack::initializeKnob(juce::Slider& slider, juce::Label& label,
    const juce::String& labelText, const juce::String& paramName,
    std::unique_ptr<SliderAttachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 70, 20);
    slider.setPopupDisplayEnabled(true, true, nullptr);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramName, slider);
}

ChorusKnobRack::~ChorusKnobRack()
{
    powerButton->setLookAndFeel(nullptr);
}
