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
    : AudioProcessorEditor(&p), audioProcessor(p), clickSettingsRack(p), frequencyEditor("Frequency Editor", audioProcessor),
	resonatorEditor("Resonator Editor", audioProcessor), resonatorKnobRack(p, *this), headerBar(p), pipSequencer(p), chorusKnobRack(p, *this)
{
    addAndMakeVisible(headerBar);

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
	addAndMakeVisible(chorusKnobRack);
    setSize(baseWidth, baseHeight);

    addAndMakeVisible(helpCompendium);
    helpCompendium.setVisible(false);
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
    const int headerHeight = 40;

    auto fullArea = getLocalBounds();

    if (helpCompendium.isVisible())
    {
        auto helpArea = fullArea.removeFromRight(helpWidth);
        helpCompendium.setBounds(helpArea);
    }
    else
    {
        helpCompendium.setBounds(0, 0, 0, 0); // Collapse it completely if hidden
    }

    // Now fullArea is only the main part

    // Header bar at the top
    headerBar.setBounds(fullArea.removeFromTop(headerHeight));

    // add padding around edges
    auto area = fullArea.reduced(padding);

    // Split into left and right sections
    auto leftArea = area.removeFromLeft(area.getWidth() * (8.f / 12.f));
    leftArea.removeFromRight(padding / 2); // tiny extra padding
    area.removeFromLeft(padding / 2);       // matching padding

    const int editorHeight = (leftArea.getHeight() - pipSequencerHeight - buttonHeight - padding * 2) / 2;

    // Pip Sequencer
    pipSequencer.setBounds(leftArea.removeFromTop(pipSequencerHeight));
    leftArea.removeFromTop(padding);

    // Frequency Editor
    frequencyEditor.setBounds(leftArea.removeFromTop(editorHeight));
    leftArea.removeFromTop(padding);

    // Resonator Editor
    resonatorEditor.setBounds(leftArea.removeFromTop(editorHeight));
    leftArea.removeFromTop(padding);

    // Test Button
    testButton.setBounds(leftArea.removeFromTop(buttonHeight).reduced(padding, 0));

    // Right side (controls)
    clickSettingsRack.setBounds(area.removeFromTop(30 + (rackHeight - 30) * 2));
    resonatorKnobRack.setBounds(area.removeFromTop(30 + (rackHeight - 30) * 2));
    chorusKnobRack.setBounds(area);
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


void BugsoundsAudioProcessorEditor::toggleHelpCompendium(juce::String pageId) {
    if (!helpCompendium.isVisible()) {
        helpCompendium.setVisible(true);
        helpCompendium.setPage(pageId);
        setSize(baseWidth + helpWidth, baseHeight);
    }
    else {
        helpCompendium.setVisible(false);
        helpCompendium.setPage("closed");
        setSize(baseWidth, baseHeight);
    }
}


void BugsoundsAudioProcessorEditor::disableResonatorEditor() {
    resonatorEditor.disableEditor();
}


void BugsoundsAudioProcessorEditor::enableResonatorEditor() {
    resonatorEditor.enableEditor();
}