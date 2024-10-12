/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BugsoundsAudioProcessorEditor::BugsoundsAudioProcessorEditor (BugsoundsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(frequencyEditor);
    testButton.setButtonText("Play song from code");
    testButton.onClick = [this] { freqCodeEditorHasChanged(); };
    addAndMakeVisible(testButton);


    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    
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
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds().reduced(10);
    testButton.setBounds(area.removeFromBottom(30));
    area.removeFromBottom(10);
    frequencyEditor.setBounds(area);
}


//if this doesn't work implement a fifo queue for transferring the string
void BugsoundsAudioProcessorEditor::freqCodeEditorHasChanged() {
    audioProcessor.setFrequencyCodeString(frequencyEditor.getText());
}


//void BugsoundsAudioProcessorEditor::compileAndPlayback()
//{
//    // First, get the text from the frequencyEditor and compile it
//    juce::String songcode = frequencyEditor.getText();
//    std::string freqStatusMsg;
//    std::map<char, int> linkedRandValues;
//    juce::Colour freqStatusColor;
//    std::vector<SongElement> parsedSong = compileSongcode(songcode.toStdString(),
//                                                          &freqStatusMsg,
//                                                          linkedRandValues,
//                                                          freqStatusColor);
//    frequencyEditor.setErrorMessage(freqStatusMsg, freqStatusColor);
//    logCompiledSong(parsedSong);
// 
//    // send it to the audioprocessor so the synth can get to it
//    audioProcessor.playProvidedSong(parsedSong);
//}


