/*
  ==============================================================================

    ClickSettingsKnobRack.h
    Created: 29 Oct 2024 12:56:41am
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ButtonsAndStuff.h"
#include "UIDrawer.h"
#include "AnimatedKnob.h"

class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;

class ClickSettingsKnobRack : public juce::Component, public UIDrawer {

public:
    ClickSettingsKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    void paint(juce::Graphics& g) override;
    void resized();


private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    const int margin = 5;
    juce::Label titleLabel, timingLabel, pitchLabel, adRatioLabel, startFadeoutLabel, startJitterLabel, floorFreqLabel;
    AnimatedKnobSlider clickTimingKnob{ timingLabel };
    AnimatedKnobSlider clickPitchKnob{ pitchLabel };
    AnimatedKnobSlider adRatioKnob{ adRatioLabel };
    AnimatedKnobSlider startFadeoutKnob{ startFadeoutLabel };
    AnimatedKnobSlider startJitterKnob{ startJitterLabel };
    AnimatedKnobSlider floorFreqKnob{ floorFreqLabel };

    std::unique_ptr<SliderAttachment> timingAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<SliderAttachment> adRatioAttachment;
    std::unique_ptr<SliderAttachment> lowFreqAttenuationAttachment;
    std::unique_ptr<SliderAttachment> minVolFreqAttachment;
    std::unique_ptr<SliderAttachment> maxVolFreqAttachment;

    std::unique_ptr<HelpButton>         helpButton;

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;

    void initializeKnob(AnimatedKnobSlider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment);
};