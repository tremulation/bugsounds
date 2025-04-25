/*
  ==============================================================================

    ChorusKnobRack.h
    Created: 21 Apr 2025 11:46:36pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PowerButtonLAF.h"

class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;





//==============================================================================
class ChorusKnobRack : public juce::Component {
public:
    ChorusKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    ~ChorusKnobRack() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void initializeKnob(juce::Slider& slider, juce::Label& label,
        const juce::String& labelText,
        const juce::String& paramName,
        std::unique_ptr<SliderAttachment>& attachment);

    // Member variables
    const int margin = 5;
    juce::Slider countKnob, spreadKnob, distanceKnob, excitationKnob, correlationKnob;
    juce::Label countLabel, spreadLabel, distanceLabel, excitationLabel, correlationLabel;

    PowerButtonLookAndFeel powerButtonLAF;
    std::unique_ptr<juce::ToggleButton> powerButton;

    std::unique_ptr<SliderAttachment> countAttachment;
    std::unique_ptr<SliderAttachment> spreadAttachment;
    std::unique_ptr<SliderAttachment> distanceAttachment;
    std::unique_ptr<SliderAttachment> excitationAttachment;
    std::unique_ptr<SliderAttachment> correlationAttachment;
    std::unique_ptr<ButtonAttachment> powerButtonAttachment;

    juce::Label titleLabel;

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;
};