/*
  ==============================================================================

    songcodeEditor.cpp
    Created: 6 Oct 2024 3:17:13pm
    Author:  Taro

  ==============================================================================
*/

#include "songcodeEditor.h"

SongcodeEditor::SongcodeEditor()
{
    //set up main editor. This is where all the code goes
    mainEditor.setMultiLine(true);
    mainEditor.setReturnKeyStartsNewLine(true);
    mainEditor.setReadOnly(false);
    mainEditor.setScrollbarsShown(true);
    mainEditor.setCaretVisible(true);
    mainEditor.setPopupMenuEnabled(true);
    mainEditor.setText("Enter your songcode here...");
    addAndMakeVisible(mainEditor);

    // Set up the error label. If there's a parsing error it should be displayed here
    errorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    errorLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(errorLabel);
}

SongcodeEditor::~SongcodeEditor()
{
}

void SongcodeEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SongcodeEditor::resized()
{
    auto area = getLocalBounds();
    errorLabel.setBounds(area.removeFromBottom(20));
    mainEditor.setBounds(area);
}

juce::String SongcodeEditor::getText() const
{
    return mainEditor.getText();
}

void SongcodeEditor::setText(const juce::String& newText)
{
    mainEditor.setText(newText);
}

void SongcodeEditor::setErrorMessage(const juce::String& errorMessage, juce::Colour color)
{
    errorLabel.setText(errorMessage, juce::dontSendNotification);
    errorLabel.setColour(juce::Label::textColourId, color);
}