/*
  ==============================================================================

    HeaderBar.h
    Created: 6 Mar 2025 5:39:20pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "presetPanel.h"

class HeaderBar : public juce::Component
{
public:
	HeaderBar(BugsoundsAudioProcessor& p) : audioProcessor(p), presetPanel(p.getPresetManager())
    {
        //logo
        juce::File logoFile = juce::File::getCurrentWorkingDirectory()
            .getChildFile("../../Graphics/bglogo1.png");

        juce::Image logoImage = juce::ImageFileFormat::loadFrom(logoFile);
        if (!logoImage.isNull())
        {
            logoComponent.setImage(logoImage, juce::RectanglePlacement::centred);
        }
        else
        {
            juce::Logger::writeToLog("Header bar logo image not found");
            juce::Logger::writeToLog(logoFile.getFullPathName());
        }
        addAndMakeVisible(logoComponent);

        //title
        title.setText("bugsounds", juce::dontSendNotification);
        title.setFont(juce::Font(20.0f, juce::Font::bold));
        addAndMakeVisible(title);

        //preset selector
        addAndMakeVisible(presetPanel);
    }

    void resized() override
    {
        const auto container = getLocalBounds().reduced(4);
        auto bounds = container;
        int logoWidth = 40;

        //logo (left)
        auto logoArea = bounds.removeFromLeft(logoWidth).withTrimmedBottom(3);
        logoComponent.setBounds(logoArea);

        //title (left after logo)
        auto titleArea = bounds.removeFromLeft(120);
        title.setBounds(titleArea.reduced(5));

		//preset panel (middle)
        presetPanel.setBounds(container.withSizeKeepingCentre(350, getHeight() * (0.9f)));

    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds().withTrimmedTop(39), 1);
    }

private:
    BugsoundsAudioProcessor& audioProcessor;
    juce::ImageComponent logoComponent;
    juce::Label title;
    PresetPanel presetPanel;
};