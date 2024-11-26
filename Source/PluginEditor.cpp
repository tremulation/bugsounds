/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BugsoundsAudioProcessorEditor::BugsoundsAudioProcessorEditor(BugsoundsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), clickSettingsRack(p), frequencyEditor("Frequency Editor"),
    resonatorEditor("Resonator Editor"), resonatorKnobRack(p)
{
    addAndMakeVisible(frequencyEditor);
    addAndMakeVisible(resonatorEditor);
    testButton.setButtonText("Play song from code");
    testButton.onClick = [this] { 
        freqCodeEditorHasChanged(); 
        audioProcessor.setPipSequence(pipSequencer.getPips());
    };
    addAndMakeVisible(testButton);
    addAndMakeVisible(pipSequencer);

    addAndMakeVisible(clickSettingsRack);
    addAndMakeVisible(resonatorKnobRack);
    setSize(800, 600);
    
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
    const int padding = 5;
    const int pipSequencerHeight = 200;
    const int buttonHeight = 30;
    const int rackHeight = 120;

    //add consistent padding around all edges
    auto area = getLocalBounds().reduced(padding);

    //split into left and right sections
    auto leftArea = area.removeFromLeft(area.getWidth() * (8.f / 12.f));
    leftArea.removeFromRight(padding / 2);  
    area.removeFromLeft(padding / 2);       

    //calculate editor height based on remaining space
    const int editorHeight = (leftArea.getHeight() - pipSequencerHeight - buttonHeight - padding * 2) / 2;

    //left area for ui components that control custom parameters (songcode and pips)
    auto workingArea = leftArea;

    //pip sequencer
    pipSequencer.setBounds(workingArea.removeFromTop(pipSequencerHeight));
    workingArea.removeFromTop(padding);

    //frequency editor
    frequencyEditor.setBounds(workingArea.removeFromTop(editorHeight));
    workingArea.removeFromTop(padding);

    //resonator editor
    resonatorEditor.setBounds(workingArea.removeFromTop(editorHeight));
    workingArea.removeFromTop(padding);

    //test button at the bottom with padding on all sides
    auto buttonArea = workingArea.removeFromTop(buttonHeight);
    testButton.setBounds(buttonArea.reduced(padding, 0));

    //right area for controls
    clickSettingsRack.setBounds(area.removeFromTop(rackHeight));
    resonatorKnobRack.setBounds(area.removeFromTop(rackHeight));
}


//if this doesn't work implement a fifo queue for transferring the string w/out locks
void BugsoundsAudioProcessorEditor::freqCodeEditorHasChanged() {
    juce::String freqSongcode = frequencyEditor.getText();
    std::string freqStatusMsg;
    juce::Colour freqStatusColor;
    std::map<char, int> linkedRandValues;
    std::vector<SongElement> parsedSong = compileSongcode(freqSongcode.toStdString(),
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

    //set up resonator if it is on
    if (*audioProcessor.apvts.getRawParameterValue("Resonator On")) {
        juce::String resSongcode = resonatorEditor.getText();
        std::string resStatusMsg;
        juce::Colour resStatusColor;
        std::vector<SongElement> parsedResSong = compileSongcode(resSongcode.toStdString(),
            &resStatusMsg,
            linkedRandValues,
            resStatusColor);
        resonatorEditor.setErrorMessage(resStatusMsg, resStatusColor);
        if (resStatusMsg.substr(0, 5) != "Error") {
            audioProcessor.setResonatorCodeString(resonatorEditor.getText());
        }
        else {
            audioProcessor.setResonatorCodeString("");
        }
    }
    
}
