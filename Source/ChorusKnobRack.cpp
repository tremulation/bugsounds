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
    powerButton->setName("ChorusPower");
    addAndMakeVisible(powerButton.get());

    //set up the rack's title
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f).withExtraKerningFactor(.1f));
    titleLabel.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Chorus Settings", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    initializeKnob(countKnob, countLabel, "Count", "Chorus Count", countAttachment);
    initializeKnob(spreadKnob, spreadLabel, "Spread", "Chorus Stereo Spread", spreadAttachment);
    initializeKnob(distanceKnob, distanceLabel, "Distance", "Chorus Max Distance", distanceAttachment);
    initializeKnob(cooldownKnob, cooldownLabel, "Cooldown", "Chorus Cooldown Max", cooldownAttachment);
	initializeKnob(correlationKnob, correlationLabel, "Correlation", "Chorus Correlation", correlationAttachment);
    countKnob.setValueStyle(" bugs", 0, false);
    spreadKnob.setValueStyle(" %", 0, true);
    distanceKnob.setValueStyle(" m", 1, false);
    cooldownKnob.setValueStyle(" secs", 1, false);
    correlationKnob.setValueStyle("", 2, false);

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Chorus On", *powerButton);

    positionReadout = std::make_unique<ChorusPositionReadout>(processor);
    addAndMakeVisible(positionReadout.get());

    //randomize button
    randomizeButton = std::make_unique<juce::TextButton>();
    randomizeButton->setButtonText(""); // we'll draw text ourselves
    randomizeButton->setLookAndFeel(&randomizeButtonLAF);
    randomizeButton->onClick = [this](){
        audioProcessor.rerollChorusVoicePositions();
    };
    randomizeButton->setName("ChorusRandomize");

    addAndMakeVisible(randomizeButton.get());

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("chorus"); });
    helpButton->setName("ChorusHelp");
    addAndMakeVisible(helpButton.get());
}

void ChorusKnobRack::paint(juce::Graphics& g)
{
    auto  bounds = getLocalBounds();
    float scalar = getHeight() / 189.f;
    auto  headerBounds = bounds.removeFromTop(35.f * scalar);
    auto  bodyBounds = bounds;

    //header
    Colour c1 = Colour::fromString("#818BCA").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#818BCA").withAlpha(1.0f);
    drawUIBlock(g, headerBounds, c1, c2, false, true, scalar);

    //body
    c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bodyBounds, c1, c2, false, true, scalar);
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

void ChorusKnobRack::initializeKnob(AnimatedKnobSlider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment)
{
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(UIDrawer::getFontInterRegular());
    label.setColour(Label::textColourId, Colours::black);
    label.setJustificationType(juce::Justification::centred);
    slider.updateOriginalLabelText();
    addAndMakeVisible(label);

    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramName, slider);
}


void ChorusKnobRack::resized()
{
    //lay out header with buttons and title
    auto bounds = getLocalBounds();
    float scalar = getWidth() / 266.0f;
    auto titleHeight = 35.f * scalar;

    auto titleBounds = bounds.removeFromTop(titleHeight);
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight);
    powerButton->setBounds(powerButtonBounds);
    titleBounds.removeFromLeft(5.f * scalar);
    titleLabel.setBounds(titleBounds);
    float fontHeight = scalar * 21.f;
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight).withExtraKerningFactor(.05f));

    auto helpButtonBounds = titleBounds.removeFromRight(titleHeight);
    helpButton->setBounds(helpButtonBounds);
   
    //divide the remaining area into two rows.
    auto rowHeight = bounds.getHeight() / 2;
    auto topRow = bounds.removeFromTop(rowHeight).translated(0.f, 5.f * scalar);
    auto bottomRow = bounds; // what's left is the bottom row

    //combine knob/label height calculations
    const float knobSize = 60.f * scalar;
    const float labelHeight = 25.f * scalar;
    const float knobTotalHeight = knobSize + labelHeight;
    const float labelFontSize = 17.f * scalar;
    const int randomizeButtonSize = 15 * scalar;

    //loweringOffset shifts knobs downward, and labelSpacing adds extra space between knobs and labels.
    const int loweringOffset = -10 * scalar;
    const int labelSpacing = 10 * scalar;

    // ----- top row layout (countKnob, readout, and spread) -----
    float colWTop = topRow.getWidth() / 3.0f;

    auto colCount = topRow.removeFromLeft(colWTop);
    countKnob.setBounds(colCount.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    countLabel.setFont(countLabel.getFont().withHeight(labelFontSize));
    countLabel.setBounds(colCount.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));


    auto colRead = topRow.removeFromLeft(colWTop).reduced(10);
    if (positionReadout) positionReadout->setBounds(colRead);
    if (randomizeButton) {
        auto buttonX = colRead.getRight() + 5.f * scalar;
        auto buttonY = colRead.getY();
        randomizeButton->setBounds(buttonX, buttonY, randomizeButtonSize, randomizeButtonSize);
    }

    auto colSpread = topRow;
    spreadKnob.setBounds(colSpread.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    spreadLabel.setFont(spreadLabel.getFont().withHeight(labelFontSize));
    spreadLabel.setBounds(colSpread.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    // ----- top row layout (distance, cooldown, and correlation knobs) -----
    float colWBot = bottomRow.getWidth() / 3.0f;

    auto colDist = bottomRow.removeFromLeft(colWBot);
    distanceKnob.setBounds(colDist.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    distanceLabel.setFont(distanceLabel.getFont().withHeight(labelFontSize));
    distanceLabel.setBounds(colDist.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto colCool = bottomRow.removeFromLeft(colWBot);
    cooldownKnob.setBounds(colCool.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    cooldownLabel.setFont(cooldownLabel.getFont().withHeight(labelFontSize));
    cooldownLabel.setBounds(colCool.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto colCorr = bottomRow;
    correlationKnob.setBounds(colCorr.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    correlationLabel.setFont(correlationLabel.getFont().withHeight(labelFontSize));
    correlationLabel.setBounds(colCorr.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
}


ChorusKnobRack::~ChorusKnobRack()
{
    powerButton->setLookAndFeel(nullptr);
}
