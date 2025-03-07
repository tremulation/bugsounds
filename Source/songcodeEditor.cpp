/*
  ==============================================================================

    songcodeEditor.cpp
    Created: 6 Oct 2024 3:17:13pm
    Author:  Taro

  ==============================================================================
*/

#include "songcodeEditor.h"




SongcodeEditor::SongcodeEditor(const juce::String& title, BugsoundsAudioProcessor& p) : audioProcessor(p)
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
    mainEditor.setTextToShowWhenEmpty("Enter your songcode here...", juce::Colours::beige);
    mainEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::white);

    //enable text editor listeners
    mainEditor.addListener(this);

    addAndMakeVisible(mainEditor);

    // Set up the error label. If there's a parsing error it should be displayed here
    errorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    errorLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(errorLabel);

    defaultEditorColour = mainEditor.findColour(juce::TextEditor::backgroundColourId);
    defaultBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);

    //i have a feeling this will cause problems laters
    if (title == "Resonator Editor" && audioProcessor.getAPVTS().getParameter("Resonator On")->getValue() == false) {
        //check if the resonator editor is disabled in the audio processor's apvts
        disableEditor();
    }
    else {
        enableEditor();
    }

    //get text stored in the processor
    this->title = title;
	setText(audioProcessor.getUserSongcode(title));

    //set up a listener
    audioProcessor.getPresetManager().addChangeListener(this);
}


SongcodeEditor::~SongcodeEditor(){
    audioProcessor.getPresetManager().removeChangeListener(this);
}


void SongcodeEditor::paint(juce::Graphics& g)
{

    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(5);
    g.fillAll(defaultBackgroundColour);

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

void SongcodeEditor::paintOverChildren(juce::Graphics& g) {
    if (isDisabled) {
        paintOverlay(g);
    }
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


//error handling
void SongcodeEditor::clearErrorHighlight() {
    currentErrorRange = juce::Range<int>(0, 0);
    hasActiveError = false;
    
    mainEditor.setTemporaryUnderlining(currentErrorRange);
}


void SongcodeEditor::changeListenerCallback(juce::ChangeBroadcaster* source){
    if (source == &audioProcessor.getPresetManager()) {
        if (title == "Frequency Editor") {
            setText(audioProcessor.getFreqSong());
        }
        else {
            setText(audioProcessor.getResSong());
        }
    }
}


void SongcodeEditor::setError(ErrorInfo* error) {
    if (error == nullptr) {
        errorLabel.setColour(juce::Label::textColourId, juce::Colours::darkgreen);
        errorLabel.setText(juce::String("Compiled successfully"), juce::NotificationType::dontSendNotification);
    }
    else {
        //set message
        errorLabel.setColour(juce::Label::textColourId, juce::Colours::maroon);
        errorLabel.setText(juce::String(error->message), juce::NotificationType::dontSendNotification);

        //highlight error
        currentErrorRange = juce::Range<int>(error->errorStart, error->errorEnd + 1);
        hasActiveError = true;
        mainEditor.setTemporaryUnderlining(currentErrorRange);
        
        oldTextLength = mainEditor.getText().length();
    }
}


void SongcodeEditor::textEditorTextChanged(juce::TextEditor&) {
    audioProcessor.setUserSongcode(mainEditor.getText(), title);
    if (isDisabled) return;

    if (hasActiveError) {
        auto caretPos = mainEditor.getCaretPosition();
        if (caretPos > currentErrorRange.getStart() && caretPos < currentErrorRange.getEnd()) {
            clearErrorHighlight();
        }
        else if( caretPos < currentErrorRange.getStart()) {
            //recalculate error position
            int newLength = mainEditor.getText().length();
            int lendiff = oldTextLength - newLength;
            oldTextLength = newLength;
   
            currentErrorRange = juce::Range<int>(currentErrorRange.getStart() - lendiff, currentErrorRange.getEnd() - lendiff);
            //juce::Logger::writeToLog("Old length: " + juce::String(oldTextLength) + ", newlength: " + juce::String(newLength));
            mainEditor.setTemporaryUnderlining(currentErrorRange);
        }
    }
}


void SongcodeEditor::paintOverlay(juce::Graphics& g)
{
    auto overlayColour = juce::Colours::black.withAlpha(0.3f);
    g.setColour(overlayColour);
    auto overlayBounds = getLocalBounds().reduced(5);
    g.fillRect(overlayBounds);

    //draw disabled text
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("DISABLED", getLocalBounds(), juce::Justification::centred, true);
    //recompile pls
    10 + 1;
}


void SongcodeEditor::disableEditor() {
    isDisabled = true;
    mainEditor.setReadOnly(true);
    mainEditor.setCaretVisible(false);

    repaint();
}


void SongcodeEditor::enableEditor() {
    isDisabled = false;
    mainEditor.setReadOnly(false);
    mainEditor.setCaretVisible(true);

    repaint();
}





