/*
  ==============================================================================

    songcodeEditor.cpp
    Created: 6 Oct 2024 3:17:13pm
    Author:  Taro

  ==============================================================================
*/

#include "songcodeEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"



SongcodeEditor::SongcodeEditor(const juce::String& title, const juce::String& helpPage, BugsoundsAudioProcessor& p, BugsoundsAudioProcessorEditor& e) : audioProcessor(p), audioEditor(e)
{
    //set up the rack's title
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f).withExtraKerningFactor(.1f));
    titleLabel.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
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
    mainEditor.setTextToShowWhenEmpty("Enter your songcode here...", juce::Colours::black);
    mainEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xffD9D9D9));
    mainEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xff000000));
    mainEditor.setFont(UIDrawer::getFontCode().withHeight(14.f));

    //enable text editor listeners
    mainEditor.addListener(this);

    addAndMakeVisible(mainEditor);

    // Set up the error label. If there's a parsing error it should be displayed here
    errorLabel.setColour(Label::textColourId, Colours::black);
    errorLabel.setJustificationType(juce::Justification::centredBottom);
    errorLabel.setFont(UIDrawer::getFontInterRegular());
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

    helpButton = std::make_unique<HelpButton>(
        [this, helpPage] { audioEditor.toggleHelpCompendium(helpPage); });
    addAndMakeVisible(helpButton.get());
    helpButton->setName(title + "Help");
}


SongcodeEditor::~SongcodeEditor(){
    audioProcessor.getPresetManager().removeChangeListener(this);
}


void SongcodeEditor::paint(juce::Graphics& g)
{
    auto  bounds = getLocalBounds();
    float scalar = getHeight() / 180.f;
    auto  headerBounds = bounds.removeFromTop(38.f * scalar);
    auto  bodyBounds = bounds;

    //header
    Colour c1 = Colour::fromString("#818BCA").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#818BCA").withAlpha(1.0f);
    drawUIBlock(g, headerBounds, c1, c2, false, true, scalar);

    //body
    c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bodyBounds, c1, c2, false, true, scalar);
}

void SongcodeEditor::paintOverChildren(juce::Graphics& g) {
    if (isDisabled) {
        paintOverlay(g);
    }
} 



void SongcodeEditor::resized()
{
    auto bounds = getLocalBounds();
    float scalar = getHeight()/ 180.f;

    // Position title
    auto titleHeight = 35.f * scalar;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleBounds.removeFromLeft(5.f * scalar);
    titleLabel.setBounds(titleBounds);
    float titleFontHeight = scalar * 21.f;
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(titleFontHeight).withExtraKerningFactor(.05f));

    //add help buton
    auto helpArea = titleBounds.removeFromRight(titleHeight);
    helpButton->setBounds(helpArea);

    // Position editor and error label
    auto errorLabelBounds = bounds.removeFromBottom(20.f * scalar);
    errorLabelBounds = errorLabelBounds.withTrimmedBottom(5.f * scalar);
    const float labelFontSize = 17.f * scalar;
    errorLabel.setBounds(errorLabelBounds);
    errorLabel.setFont(errorLabel.getFont().withHeight(labelFontSize));
    bounds.removeFromBottom(5 * scalar);
    mainEditor.setBounds(bounds.withTrimmedLeft(2.f * scalar).withTrimmedRight(2.f * scalar).withTrimmedTop(5.f * scalar));

    //update the size of the existing text in the editor
    mainEditor.setFont(UIDrawer::getFontCode().withHeight(14.f * scalar));
    auto currentText = mainEditor.getText();
    mainEditor.removeListener(this);
    mainEditor.clear();
    mainEditor.insertTextAtCaret(currentText);
    mainEditor.addListener(this);
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





