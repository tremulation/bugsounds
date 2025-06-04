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
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f).withExtraKerningFactor(.1f));
    titleLabel.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Click settings", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    //set up the knobs
    initializeKnob(clickTimingKnob, timingLabel, "Time Rand", "Click Timing Random", timingAttachment);
    initializeKnob(clickPitchKnob, pitchLabel, "Pitch Rand", "Click Pitch Random", pitchAttachment);
    initializeKnob(adRatioKnob, adRatioLabel, "A/D Ratio", "Click Atack Decay Ratio", adRatioAttachment);
    initializeKnob(startFadeoutKnob, startFadeoutLabel, "Start Fade", "Click Start Fadeout", lowFreqAttenuationAttachment);
    initializeKnob(floorFreqKnob, floorFreqLabel, "Floor Freq", "Click Floor Frequency", maxVolFreqAttachment);
    initializeKnob(startJitterKnob, startJitterLabel, "Start Jitter", "Click Start Jitter", minVolFreqAttachment);
    clickTimingKnob.setValueStyle(" %", 1, true);
    clickPitchKnob.setValueStyle(" %", 1, true);
    adRatioKnob.setValueStyle(" %", 0, true);
    startFadeoutKnob.setValueStyle(" %", 0, true);
    startJitterKnob.setValueStyle(" %", 0, true);
    floorFreqKnob.setValueStyle("Hz", 0);
    

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("clickSettings"); });
    helpButton->setName("ClickHelp");
    addAndMakeVisible(helpButton.get());
}


void ClickSettingsKnobRack::paint(juce::Graphics& g) {
    auto  bounds = getLocalBounds();
    float scalar = getHeight() / 189.f;     //same height as pipsequencer, so same scalar.
    auto  headerBounds = bounds.removeFromTop(35.f * scalar);
    auto  bodyBounds = bounds;

    //headeer
    Colour c1 = Colour::fromString("#818BCA").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#818BCA").withAlpha(1.0f);
    drawUIBlock(g, headerBounds, c1, c2, false, true, scalar);

    //body
    c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bodyBounds, c1, c2, false, true, scalar);

    //dotted line to separate the three tailoff knobs
    g.setColour(Colours::darkgrey);
    const float dashPattern[] = { 2.0f * scalar, 4.0f * scalar};    //2px dash, 2px gap
    float midY = bodyBounds.getY() + bodyBounds.getHeight() * 0.5f;
    float startX = static_cast<float>(bodyBounds.getX());
    float endX = static_cast<float>(bodyBounds.getRight());
    g.drawDashedLine({ startX, midY, endX, midY }, dashPattern, 2);
}



void ClickSettingsKnobRack::resized() {
    auto bounds = getLocalBounds();

    float scalar = getHeight() / 189.f;
    //position the title at the top.
    auto titleHeight = 35.f * scalar;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleBounds.removeFromLeft(5.f * scalar);
    titleLabel.setBounds(titleBounds);
    float fontHeight = scalar * 21.f;
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight).withExtraKerningFactor(.05f));

    //help button
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

    //loweringOffset shifts knobs downward, and labelSpacing adds extra space between knobs and labels.
    const int loweringOffset = -10 * scalar;
    const int labelSpacing = 10 * scalar;

    // ----- top row layout (timing, pitch, and A/D Ratio) -----
    int topColumnWidth = topRow.getWidth() / 3;

    auto topCol0 = topRow.removeFromLeft(topColumnWidth);
    clickTimingKnob.setBounds(topCol0.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    timingLabel.setBounds(topCol0.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    timingLabel.setFont(timingLabel.getFont().withHeight(labelFontSize));

    auto topCol1 = topRow.removeFromLeft(topColumnWidth);
    clickPitchKnob.setBounds(topCol1.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    pitchLabel.setBounds(topCol1.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    pitchLabel.setFont(pitchLabel.getFont().withHeight(labelFontSize));

    auto topCol2 = topRow; // remaining area
    adRatioKnob.setBounds(topCol2.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    adRatioLabel.setBounds(topCol2.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    adRatioLabel.setFont(adRatioLabel.getFont().withHeight(labelFontSize));

    // ----- bottom row layout (min Vol Freq, max Vol Freq, and low freq attenuation) -----
    int bottomColumnWidth = bottomRow.getWidth() / 3;

    auto bottomCol0 = bottomRow.removeFromLeft(bottomColumnWidth);
    floorFreqKnob.setBounds(bottomCol0.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    floorFreqLabel.setBounds(bottomCol0.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    floorFreqLabel.setFont(floorFreqLabel.getFont().withHeight(labelFontSize));

    auto bottomCol1 = bottomRow.removeFromLeft(bottomColumnWidth);
    startJitterKnob.setBounds(bottomCol1.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    startJitterLabel.setBounds(bottomCol1.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    startJitterLabel.setFont(startJitterLabel.getFont().withHeight(labelFontSize));

    auto bottomCol2 = bottomRow; //remaining area
    startFadeoutKnob.setBounds(bottomCol2.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    startFadeoutLabel.setBounds(bottomCol2.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
    startFadeoutLabel.setFont(startFadeoutLabel.getFont().withHeight(labelFontSize));
}


void ClickSettingsKnobRack::initializeKnob(AnimatedKnobSlider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment) {
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(UIDrawer::getFontInterRegular());
    label.setColour(Label::textColourId, Colours::black);
    label.setJustificationType(juce::Justification::centred);
    slider.updateOriginalLabelText();
    addAndMakeVisible(label);

    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramName, slider);
}





