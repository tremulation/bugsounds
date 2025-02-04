/*
  ==============================================================================

    ResonatorKnobRack.cpp
    Created: 4 Feb 2025 4:30:21pm
    Author:  Taro

  ==============================================================================
*/

#include "ResonatorKnobRack.h"
#include "PluginEditor.h"  

// ==============================================================================
void PowerButtonLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
    g.fillRect(bounds);
    g.setColour(juce::Colours::white);
    g.drawRect(bounds, 1.0f);

    auto iconBounds = bounds.reduced(4);
    g.drawLine(iconBounds.getCentreX(), iconBounds.getY(),
        iconBounds.getCentreX(), iconBounds.getBottom(), 2.0f);
    g.drawEllipse(iconBounds.reduced(5), 2.0f);
}

// ==============================================================================
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

    initializeKnob(resonatorQKnob, qLabel, "Q", "Resonator Q", qAttachment);
    initializeKnob(resonatorGainKnob, gainLabel, "Gain", "Resonator Gain", gainAttachment);
    initializeKnob(resonatorHarmonicsKnob, harmonicsLabel, "Harmonics", "Resonator Harmonics", harmonicsAttachment);

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Resonator On", *powerButton);

    powerButton->onClick = [this] {
        disableResonatorEditor();
        };
}


void ResonatorKnobRack::paint(juce::Graphics& g)
{
    //fill background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(margin);

    //draw outer white border
    g.setColour(juce::Colours::white);
    g.drawRect(contentBounds);

    //draw border under title
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

    //position title row with power button
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);

    //position power button on the far left
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight).reduced(5);
    powerButton->setBounds(powerButtonBounds);

    //position title next to power button
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    //add spacing after title and separator line
    bounds.removeFromTop(5);

    //calculate knob layout within the rack
    auto knobSection = bounds;
    auto numKnobs = 3;
    auto knobWidth = knobSection.getWidth() / numKnobs;
    auto knobSize = std::min(knobWidth, knobSection.getHeight()) - 20;
    auto labelHeight = 20;

    //define upward offset for knobs
    const int upwardOffset = 10;

    //position knobs and labels with upward offset
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


void ResonatorKnobRack::initializeKnob(juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment) {
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
    if (powerButton->getToggleState()) {
        audioEditor.enableResonatorEditor();
    }
    else {
        audioEditor.disableResonatorEditor();
    }
}