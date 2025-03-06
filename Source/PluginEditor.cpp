/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Evaluator.h"


//==============================================================================
BugsoundsAudioProcessorEditor::BugsoundsAudioProcessorEditor(BugsoundsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), clickSettingsRack(p), frequencyEditor("Frequency Editor", false),
    resonatorEditor("Resonator Editor", true), resonatorKnobRack(p, *this)
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
    const int rackHeight = 110;

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
    clickSettingsRack.setBounds(area.removeFromTop(30 + (rackHeight - 30) * 2));
    resonatorKnobRack.setBounds(area.removeFromTop(30 + (rackHeight - 30) * 2)); //30 is title height
}


//if this doesn't work implement a fifo queue for transferring the string w/out locks
void BugsoundsAudioProcessorEditor::freqCodeEditorHasChanged() {
    juce::String freqSongCode = frequencyEditor.getText();
    ErrorInfo errorInfo = {};
	std::map<std::string, float> env;
    std::vector<SongElement> songElements;

    //compile check freq songcode 
    juce::ReferenceCountedObjectPtr<ScriptNode> parsedSong = generateAST(freqSongCode.toStdString(), &errorInfo);
    if (errorInfo.message != "") goto freqError;
    //need to evaluate so we can report errors that only show up when you eval
    songElements = evaluateAST(parsedSong, &errorInfo, &env);
	if (errorInfo.message != "") goto freqError;


    //compile check res songcode
    if (*audioProcessor.apvts.getRawParameterValue("Resonator On")) {
        juce::String resSongCode = resonatorEditor.getText();
        juce::ReferenceCountedObjectPtr<ScriptNode> parsedResSong = generateAST(resSongCode.toStdString(), &errorInfo);
        if (errorInfo.message != "") goto resError;
		songElements = evaluateAST(parsedResSong, &errorInfo, &env);
        if (errorInfo.message != "") goto resError;
        //set resonator AST/error message
        audioProcessor.setResAST(parsedResSong);
        resonatorEditor.setError(nullptr);
    }
    
	//now we can send the freq song, since res compiled
    frequencyEditor.setError(nullptr);
    audioProcessor.setSongAST(parsedSong);
    return;
    
    //two failure states. mostly the same, besides error message handling
freqError:
    frequencyEditor.setError(&errorInfo);
    audioProcessor.setSongAST(nullptr);
	audioProcessor.setResAST(nullptr);
    return;

resError:
	frequencyEditor.setError(nullptr);
	resonatorEditor.setError(&errorInfo);
	audioProcessor.setSongAST(nullptr);
    audioProcessor.setResAST(nullptr);
	return;
}


void BugsoundsAudioProcessorEditor::disableResonatorEditor() {
    resonatorEditor.disableEditor();
}


void BugsoundsAudioProcessorEditor::enableResonatorEditor() {
    resonatorEditor.enableEditor();
}