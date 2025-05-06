/*
  ==============================================================================

    HeaderBar.h
    Created: 6 Mar 2025 5:39:20pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <BinaryData.h>
#include "presetPanel.h"
#include "PresetManager.h" 


class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;
class PresetPanel;
class PresetManager;

class HeaderBar : public juce::Component
{
public:
    HeaderBar(BugsoundsAudioProcessor& p);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    BugsoundsAudioProcessor& audioProcessor;
    juce::ImageComponent logoComponent;
    juce::Label title;
    PresetPanel presetPanel;
};