/*
  ==============================================================================

    ResonatorKnobRack.cpp
    Created: 4 Feb 2025 4:30:21pm
    Author:  Taro

  ==============================================================================
*/

#include "ResonatorKnobRack.h"
#include "PluginEditor.h"  



//==============================================================================
ResonatorKnobRack::ResonatorKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor)
    : audioProcessor(processor), audioEditor(editor)
{
    powerButton = std::make_unique<juce::ToggleButton>("");
    powerButton->setClickingTogglesState(true);
    powerButton->setLookAndFeel(&powerButtonLAF);
    addAndMakeVisible(powerButton.get());

    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Resonator Settings", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    // --- Top row: Bandwidth, Gain, Mix ---
    initializeKnob(resonatorOvertoneKnob, overtoneLabel, "Overtones", "Resonator Overtone Number", overtoneAttachment);
    initializeKnob(resonatorQKnob, qLabel, "Bandwidth", "Resonator Q", qAttachment);
    initializeKnob(resonatorGainKnob, gainLabel, "Peak Gain", "Resonator Gain", gainAttachment);

    // --- Bottom row: Harmonic Emphasis, Overtones, Drive ---
    initializeKnob(resonatorDecayKnob, oDecayLabel, "Overtone Decay", "Resonator Overtone Decay", oDecayAttachment);
    initializeKnob(resonatorOriginalMixKnob, originalMixLabel, "Original Mix", "Resonator Original Mix", originalMixAttachment);

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Resonator On", *powerButton);

    powerButton->onClick = [this] {
        disableResonatorEditor();
        };

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("resonatorSettings"); });
    addAndMakeVisible(helpButton.get());
}

void ResonatorKnobRack::paint(juce::Graphics& g)
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


void ResonatorKnobRack::paintOverChildren(juce::Graphics& g) {
    bool resonatorOn = audioProcessor.apvts.getRawParameterValue("Resonator On")->load() > 0.5f;
    if (!resonatorOn) {

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

void ResonatorKnobRack::resized()
{
    const int margin = 5;
    auto bounds = getLocalBounds().reduced(margin);

    // --- Title row with power and help buttons---
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);

    // Position buttons
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight).reduced(5);
    powerButton->setBounds(powerButtonBounds);
    auto helpArea = titleBounds.removeFromRight(titleHeight).reduced(5);
    helpButton->setBounds(helpArea);

    // Position title next to power button
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    // Add spacing after title and separator line
    bounds.removeFromTop(5);

    // --- Divide remaining space into two rows for the knobs ---
    int labelHeight = 20;
    // Lower all knobs by a fixed number of pixels:
    const int loweringOffset = -10;
    // New variable to add extra spacing between knobs and labels:
    const int labelSpacing = 5;

    int totalKnobAreaHeight = bounds.getHeight();
    auto topRowBounds = bounds.removeFromTop(totalKnobAreaHeight / 2);
    auto bottomRowBounds = bounds; // The rest of the area

    // --- Define fixed knob size ---
    const int knobSize = 60;

    // --- Top Row Knobs (3 equally spaced knobs) ---
    int topRowColumns = 3;
    int topRowKnobWidth = topRowBounds.getWidth() / topRowColumns;

    auto qKnobArea = topRowBounds.removeFromLeft(topRowKnobWidth);
    resonatorQKnob.setBounds(qKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    qLabel.setBounds(qKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto gainKnobArea = topRowBounds.removeFromLeft(topRowKnobWidth);
    resonatorGainKnob.setBounds(gainKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    gainLabel.setBounds(gainKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto overtoneKnobArea = topRowBounds;
    resonatorOvertoneKnob.setBounds(overtoneKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    overtoneLabel.setBounds(overtoneKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    // --- Bottom Row Knobs (2 equally spaced knobs) ---
    int bottomRowColumns = 2;
    int bottomRowKnobWidth = bottomRowBounds.getWidth() / bottomRowColumns;

    auto decayKnobArea = bottomRowBounds.removeFromLeft(bottomRowKnobWidth);
    resonatorDecayKnob.setBounds(decayKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    oDecayLabel.setBounds(decayKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto mixKnobArea = bottomRowBounds;
    resonatorOriginalMixKnob.setBounds(mixKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    originalMixLabel.setBounds(mixKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
}

void ResonatorKnobRack::initializeKnob(juce::Slider& slider, juce::Label& label,
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

ResonatorKnobRack::~ResonatorKnobRack()
{
    powerButton->setLookAndFeel(nullptr);
}

void ResonatorKnobRack::disableResonatorEditor()
{
    if (powerButton->getToggleState())
        audioEditor.enableResonatorEditor();
    else
        audioEditor.disableResonatorEditor();
}
