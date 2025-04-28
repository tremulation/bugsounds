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
    initializeKnob(cooldownKnob, cooldownLabel, "Cooldown", "Chorus Cooldown Max", cooldownAttachment);
	initializeKnob(correlationKnob, correlationLabel, "Correlation", "Chorus Correlation", correlationAttachment);

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Chorus On", *powerButton);

    positionReadout = std::make_unique<ChorusPositionReadout>(processor);
    addAndMakeVisible(positionReadout.get());

    //fix spread knob value blocking the voice position readout
    spreadKnob.setPopupDisplayEnabled(false, false, nullptr);

    //randomize button
    randomizeButton = std::make_unique<juce::TextButton>();
    randomizeButton->setButtonText(""); // we'll draw text ourselves
    randomizeButton->setLookAndFeel(&randomizeButtonLAF);
    randomizeButton->onClick = [this](){
        audioProcessor.rerollChorusVoicePositions();
    };

    addAndMakeVisible(randomizeButton.get());

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("chorus"); });
    addAndMakeVisible(helpButton.get());
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


void ChorusKnobRack::paintOverChildren(juce::Graphics& g) {
    bool chorusOn = audioProcessor.apvts.getRawParameterValue("Chorus On")->load() > 0.5f;
    if (!chorusOn) {

        //mask the power button so it doesn't look like it's disabled too
        auto overlayBounds = getLocalBounds().reduced(5);
        auto powerButtonArea = powerButton->getBounds();
        auto originalClip = g.getClipBounds();
        g.excludeClipRegion(powerButtonArea);

        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRect(getLocalBounds().reduced(5));

        //draw the text
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("DISABLED", getLocalBounds(), juce::Justification::centred, true);
        repaint();
    }
}


void ChorusKnobRack::resized()
{
    const int margin = 5;
    auto bounds = getLocalBounds().reduced(margin);

    // title row
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight).reduced(5);
    powerButton->setBounds(powerButtonBounds);
    auto helpArea = titleBounds.removeFromRight(titleHeight).reduced(5);
    helpButton->setBounds(helpArea);
    titleLabel.setBounds(titleBounds.reduced(5, 0));
    

    // knob layout
    bounds.removeFromTop(5);
    const int knobSize = 60;
    const int labelHeight = 20;
    const int loweringOffset = -10;
    const int labelSpacing = 5;
    const int randomizeButtonSize = 15; //small square button

    // 2 rows: 2 knobs + readout on top, 3 knobs on bottom
    auto topRow = bounds.removeFromTop(bounds.getHeight() / 2);
    auto bottomRow = bounds;

    // Top row - split into 3 equal parts
    auto topRowWidth = topRow.getWidth();
    auto countArea = topRow.removeFromLeft(topRowWidth / 3);
    auto readoutArea = topRow.removeFromLeft(topRowWidth / 3);
    auto spreadArea = topRow; // whatever remains

    // Left - Count
    countKnob.setBounds(countArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    countLabel.setBounds(countArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    if (positionReadout){
        auto readoutBounds = readoutArea.reduced(10); // small margin inside
        positionReadout->setBounds(readoutBounds);

        if (randomizeButton){
            auto buttonX = readoutBounds.getRight();
            auto buttonY = readoutBounds.getY();
            randomizeButton->setBounds(buttonX, buttonY, randomizeButtonSize, randomizeButtonSize);
        }
    }

    // Right - Spread
    spreadKnob.setBounds(spreadArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    spreadLabel.setBounds(spreadArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    // Bottom row - split into 3
    auto distanceArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 3);
    auto excitationArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto correlationArea = bottomRow;

    distanceKnob.setBounds(distanceArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    distanceLabel.setBounds(distanceArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    cooldownKnob.setBounds(excitationArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    cooldownLabel.setBounds(excitationArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

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
