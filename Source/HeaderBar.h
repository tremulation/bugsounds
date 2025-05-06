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
#include "ButtonsAndStuff.h"


class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;
class PresetPanel;
class PresetManager;

class HeaderBar : public juce::Component
{
public:
    HeaderBar(BugsoundsAudioProcessor& p, BugsoundsAudioProcessorEditor& editor);
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;

    juce::ImageComponent logoComponent;
    juce::Label title;
    PresetPanel presetPanel;
    std::unique_ptr<HelpButton> helpButton;
};