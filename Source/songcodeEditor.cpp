/*
  ==============================================================================

    songcodeEditor.cpp
    Created: 6 Oct 2024 3:17:13pm
    Author:  Taro

  ==============================================================================
*/

#include "songcodeEditor.h"

SongcodeEditor::SongcodeEditor(const juce::String& title)
{
    //set up title 
    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText(title, juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    //set up main editor. This is where all the code goes
    mainEditor.setMultiLine(true);
    mainEditor.setReturnKeyStartsNewLine(true);
    mainEditor.setReadOnly(false);
    mainEditor.setScrollbarsShown(true);
    mainEditor.setCaretVisible(true);
    mainEditor.setPopupMenuEnabled(true);
    mainEditor.setText("Enter your songcode here...");
    mainEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::white);


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
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(5);

    //draw outer white border
    g.setColour(juce::Colours::white);
    g.drawRect(contentBounds);

    //draw border under title
    auto titleBounds = contentBounds.removeFromTop(30);
    g.drawLine(titleBounds.getX(),
        titleBounds.getBottom(),
        titleBounds.getRight(),
        titleBounds.getBottom(),
        1.0f);
}

void SongcodeEditor::resized()
{
    auto bounds = getLocalBounds().reduced(5);

    // Position title
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    // Position editor and error label
    auto errorLabelBounds = bounds.removeFromBottom(15);
    errorLabelBounds = errorLabelBounds.withTrimmedBottom(5);
    errorLabel.setBounds(errorLabelBounds);
    bounds.removeFromBottom(5);
    mainEditor.setBounds(bounds);
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