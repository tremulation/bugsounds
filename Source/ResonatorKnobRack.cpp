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
void PowerButtonLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    //get the full bounds of the button.
    auto bounds = button.getLocalBounds().toFloat();

    //fill the background with green when toggled on, dark grey otherwise.
    g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
    g.fillRect(bounds);

    //draw a white border around the button.
    g.setColour(juce::Colours::white);
    g.drawRect(bounds, 1.0f);

    //define the icon area with minimal reduction so the circle is as big as possible.
    auto iconBounds = bounds.reduced(2);

    //calculate the maximum square that fits within iconBounds.
    float diameter = std::min(iconBounds.getWidth(), iconBounds.getHeight()) * 0.95f; // 95% of available space
    juce::Rectangle<float> ellipseBounds;
    ellipseBounds.setSize(diameter, diameter);
    ellipseBounds.setCentre(iconBounds.getCentre());

    //draw the full circle.
    g.setColour(juce::Colours::white);
    g.drawEllipse(ellipseBounds, 2.0f);


    float centerX = ellipseBounds.getCentreX();
    float lineTop = ellipseBounds.getY() - 1.0f;
    float lineBottom = ellipseBounds.getCentreY();

    //draw the thick line (mask) in the background color.
    g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
    float thickLineThickness = 6.0f;  // Increased thickness to fully cover the top stroke.
    g.drawLine(centerX, lineTop, centerX, lineBottom, thickLineThickness);

    //draw the thinner white line along the same path to create the power symbol's indicator.
    g.setColour(juce::Colours::white);
    float thinLineThickness = 2.0f;
    g.drawLine(centerX, lineTop, centerX, lineBottom, thinLineThickness);
}

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
    initializeKnob(resonatorQKnob, qLabel, "Bandwidth", "Resonator Q", qAttachment);
    initializeKnob(resonatorGainKnob, gainLabel, "Gain", "Resonator Gain", gainAttachment);
    initializeKnob(resonatorOvertoneKnob, overtoneLabel, "Layers", "Resonator Overtone Number", overtoneAttachment);
    // --- Bottom row: Harmonic Emphasis, Overtones, Drive ---

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Resonator On", *powerButton);

    powerButton->onClick = [this] {
        disableResonatorEditor();
        };
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

void ResonatorKnobRack::resized()
{
    const int margin = 5;
    auto bounds = getLocalBounds().reduced(margin);

    // --- Title row with power button ---
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);

    // Position power button on the far left
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight).reduced(5);
    powerButton->setBounds(powerButtonBounds);

    // Position title next to power button
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    // Add spacing after title and separator line
    bounds.removeFromTop(5);

    // --- Divide remaining space into two rows for the knobs ---
    int numColumns = 3;
    int numRows = 1;
    int labelHeight = 20;
    const int upwardOffset = 10;

    // Split the area into top and bottom rows
    auto topRowBounds = bounds.removeFromTop(bounds.getHeight());


    // --- Top Row Knobs ---
    // These are for Bandwidth, Gain, and Mix.
    int knobWidth = topRowBounds.getWidth() / numColumns;
    int knobSize = std::min(knobWidth, topRowBounds.getHeight()) - 20;

    auto bandwidthArea = topRowBounds.removeFromLeft(knobWidth);
    resonatorQKnob.setBounds(bandwidthArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
    qLabel.setBounds(bandwidthArea.removeFromBottom(labelHeight).translated(0, -upwardOffset));

    auto gainArea = topRowBounds.removeFromLeft(knobWidth);
    resonatorGainKnob.setBounds(gainArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
    gainLabel.setBounds(gainArea.removeFromBottom(labelHeight).translated(0, -upwardOffset));

    auto overtoneArea = topRowBounds.removeFromLeft(knobWidth);
    resonatorOvertoneKnob.setBounds(overtoneArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, -upwardOffset));
    overtoneLabel.setBounds(overtoneArea.removeFromBottom(labelHeight).translated(0, -upwardOffset));
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
