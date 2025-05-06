/*
  ==============================================================================

    HeaderBar.cpp
    Created: 5 May 2025 8:02:04pm
    Author:  Taro

  ==============================================================================
*/

#include "HeaderBar.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"



HeaderBar::HeaderBar(BugsoundsAudioProcessor& p, BugsoundsAudioProcessorEditor& editor) 
    : audioProcessor(p), audioEditor(editor), presetPanel(p.getPresetManager())
{
    //logo
    const juce::String resourceName = "bglogo1_png";
    int dataSize = 0;
    const char* imageData = BinData::getNamedResource(resourceName.toRawUTF8(), dataSize);

    if (imageData != nullptr && dataSize > 0) {
        juce::Image logoImage = juce::ImageCache::getFromMemory(imageData, dataSize);
        if (logoImage.isValid()) {
            logoComponent.setImage(logoImage, juce::RectanglePlacement::centred);
            addAndMakeVisible(logoComponent);
        }
        else {
            juce::Logger::writeToLog("Failed to create image from binary data");
        }
    }
    else {
        juce::Logger::writeToLog("Logo resource not found: " + juce::String(resourceName));
    }

    //title
    title.setText("bugsounds", juce::dontSendNotification);
    title.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(title);

    //preset selector
    addAndMakeVisible(presetPanel);

    //help button
    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("bugSounds"); });
    addAndMakeVisible(helpButton.get());
}



void HeaderBar::resized() {
    const auto container = getLocalBounds().reduced(4);
    auto bounds = container;

    // 1. Logo (left side)
    const int logoWidth = 40;
    auto logoArea = bounds.removeFromLeft(logoWidth).withTrimmedBottom(3);
    logoComponent.setBounds(logoArea);

    // 2. Title (left of remaining space)
    auto titleArea = bounds.removeFromLeft(juce::jmin(120, bounds.getWidth()));
    title.setBounds(titleArea.reduced(5));

    // 3. Help button (right side)
    const int helpButtonSize = 24;
    auto helpArea = bounds.removeFromRight(container.getHeight()).reduced(3);
    helpButton->setBounds(helpArea);

    // 4. Preset panel (centered, fixed size)
    presetPanel.setBounds(container.withSizeKeepingCentre(350, getHeight() * (0.9f)));
}
void HeaderBar::paint(juce::Graphics& g) {
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds().withTrimmedTop(39), 1);
}