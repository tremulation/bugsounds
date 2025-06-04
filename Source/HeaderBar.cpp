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

			logoComponent.onClick = [this] {
				juce::Logger::writeToLog("Logo clicked! Opening credits window...");
				audioEditor.showCreditsWindow(); 
				};
        }
        else {
            juce::Logger::writeToLog("Failed to create image from binary data");
        }
    }
    else {
        juce::Logger::writeToLog("Logo resource not found: " + juce::String(resourceName));
    }

    //title
    title.setText("Bugsounds", juce::dontSendNotification);
    title.setFont(UIDrawer::getFontInterBold().withHeight(25.0f));
    addAndMakeVisible(title);

    //preset selector
    addAndMakeVisible(presetPanel);

    //help button
    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("bugSounds"); });
    helpButton->setName("HeaderHelp");
    addAndMakeVisible(helpButton.get());
}



void HeaderBar::resized() {
    auto bounds = getLocalBounds();
    auto container = getLocalBounds();

    //remove area for shadow on the bottom. otherwise it won't look 3d
    bounds.removeFromBottom((int)getHeight() * 5.f / 640.f);

    //------------------- logo. square bounding box.------------------- 
    //TODO make the logo extend past the bounds of the header to make it look cooler
    const int logoW = (int)(getWidth() * 40.f / 640.f);
    auto logoArea = bounds.removeFromLeft(logoW);
    logoComponent.setBounds(logoArea);
    
    //padding btw logo and title
    const int logoTitleGap = getWidth() * 4.f / 640.f;
    bounds.removeFromLeft(logoTitleGap);

    //------------------- Title.  ------------------- 
    const int titleW = getWidth() * 164.f / 800.f;
    const int titleH = getHeight() ;
    auto titleArea = bounds.removeFromLeft(titleW).withSizeKeepingCentre(titleW, titleH);
    title.setBounds(titleArea);
    //and scale the font
    float scale = getHeight() / 40.0f;   
    float fontHeight = getHeight() - scale * 10.f;
    title.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight));

    //------------------- Preset Panel -------------------
    const int presetW = (int) (getWidth() * 300.f / 640.f);
    const int presetH = (int) getHeight();  //child is responsible for padding
    presetPanel.setBounds(container.withSizeKeepingCentre(presetW, presetH));

    //------------------- Help Button -------------------
    const int scaledHelpW = (int)getHeight();   //40 at start
    auto helpArea = bounds.removeFromRight(scaledHelpW);
    helpButton->setBounds(helpArea);
}


void HeaderBar::paint(juce::Graphics& g) {
    float scalar = getWidth() / 800.f;
    drawUIBlock( g, getLocalBounds(),
        Colour::fromString("#3E28BD").withAlpha(1.0f),
        Colour::fromString("#81CAA5").withAlpha(1.0f),
        false, true,  scalar);
}