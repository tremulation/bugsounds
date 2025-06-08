/*
  ==============================================================================

    LevelMeter.h
    Created: 2 Jun 2025 6:54:39pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UIDrawer.h"
#include "AnimatedLevelSlider.h"

class BugsoundsAudioProcessor;

class LevelMeter : public juce::Component, private UIDrawer {

public:

    LevelMeter(BugsoundsAudioProcessor& p);

    void paint(juce::Graphics&) override;
    void resized() override;

    void setLevel(const float leftVal, const float rightVal);

private:
    AnimatedSlider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    void drawSingleChannelMeter(juce::Graphics& g, Rectangle<float> bounds, float levelDecibels, float scalar);
    float leftLevel  = -60.f;   //-60 to +6 full scale
    float rightLevel = -60.f;
};

