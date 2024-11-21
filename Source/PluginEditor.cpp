/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BugsoundsAudioProcessorEditor::BugsoundsAudioProcessorEditor (BugsoundsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), clickSettingsRack(p), frequencyEditor("Frequency Editor")
{
    addAndMakeVisible(frequencyEditor);
    testButton.setButtonText("Play song from code");
    testButton.onClick = [this] { 
        freqCodeEditorHasChanged(); 
        audioProcessor.setPipSequence(pipSequencer.getPips());
    };
    addAndMakeVisible(testButton);
    addAndMakeVisible(pipSequencer);

    addAndMakeVisible(clickSettingsRack);
    setSize(800, 400);
    
}

BugsoundsAudioProcessorEditor::~BugsoundsAudioProcessorEditor()
{
}

//==============================================================================
void BugsoundsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void BugsoundsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Split into left and right sections first
    //left is for entering custom data like pip settings and songcode
    //right is for knobs that control parameters (things you can save in apvts)
    auto leftArea = area.removeFromLeft(area.getWidth() * (8.f / 12.f));

    // Now reduce both areas equally
    // leftArea = leftArea.reduced(5);

    // Left half for text editors and button
    auto pipSequencerHeight = 200;
    pipSequencer.setBounds(leftArea.removeFromTop(pipSequencerHeight));
    testButton.setBounds(leftArea.removeFromBottom(30));
    leftArea.removeFromBottom(5);
    frequencyEditor.setBounds(leftArea);

    // Right half for controls
    auto rackHeight = 120;
    clickSettingsRack.setBounds(area.removeFromTop(rackHeight));
    

    // Space for additional controls below the rack
    area.removeFromTop(5);  // Add some spacing after the rack
}


//if this doesn't work implement a fifo queue for transferring the string w/out locks
void BugsoundsAudioProcessorEditor::freqCodeEditorHasChanged() {
    juce::String songcode = frequencyEditor.getText();
    std::string freqStatusMsg;
    std::map<char, int> linkedRandValues;
    juce::Colour freqStatusColor;
    std::vector<SongElement> parsedSong = compileSongcode(songcode.toStdString(),
                                                           &freqStatusMsg,
                                                           linkedRandValues,
                                                           freqStatusColor);
    
    frequencyEditor.setErrorMessage(freqStatusMsg, freqStatusColor);
    
    if (freqStatusMsg.substr(0, 5) != "Error") {
        audioProcessor.setFrequencyCodeString(frequencyEditor.getText());
    }
    else {
        audioProcessor.setFrequencyCodeString("");
    }
}


