/*
  ==============================================================================

    ClickSettingsKnobRack.h
    Created: 29 Oct 2024 12:56:41am
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "helpButton.h"

class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;

class ClickSettingsKnobRack : public juce::Component {

public:
    ClickSettingsKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    void paint(juce::Graphics& g) override;
    void resized();


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

    juce::Slider clickVolumeSlider;
    juce::Label clickVolumeLabel;
    std::unique_ptr<SliderAttachment> clickVolumeAttachment;

    std::unique_ptr<HelpButton>         helpButton;

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;

    void initializeKnob(juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment);
};