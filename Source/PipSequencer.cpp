/*
  ==============================================================================
    PipSequencer.cpp
    Created: 12 Nov 2024 8:44:02pm
    Author:  Taro
  ==============================================================================
*/

#include "PipSequencer.h"


//----------------------------========== Pip Sequencer ==========----------------------------\\

PipSequencer::PipSequencer(BugsoundsAudioProcessor& p) : audioProcessor(p){
    //set up title 
    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Subclick Sequencer", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    //set up sequencebox and viewport
    viewport = std::make_unique<juce::Viewport>();
    sequenceBox = std::make_unique<SequenceBox>(*this);

    viewport->setViewedComponent(sequenceBox.get(), false);
    viewport->setScrollBarsShown(false, true); //only need horizontal scroll bar
    addAndMakeVisible(viewport.get());

    //setup editing mode buttons
    createModeButtons();

    //setup preview button
    previewButton.setButtonText("Preview");
    previewButton.onClick = [this] { audioProcessor.triggerPreviewClick(); };
    addAndMakeVisible(previewButton);

    loadPipsFromProcessor();
    audioProcessor.getPresetManager().addChangeListener(this);
}


PipSequencer::~PipSequencer() {
    audioProcessor.getPresetManager().removeChangeListener(this);
}


void PipSequencer::paint(juce::Graphics& g) {
    //normal stuff -- outline and title underline
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

    //draw border under mode buttons for tabs
    auto modeButtonBounds = contentBounds.removeFromTop(buttonRowHeight - 5);
    g.drawRect(modeButtonBounds, 1.0f);
}


void PipSequencer::resized() {
    auto bounds = getLocalBounds().reduced(5);

    //position title
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    //position preview button to the right of the title
    auto previewButtonWidth = 80;
    auto previewButtonHeight = titleHeight - 10;
    auto previewButtonBounds = titleBounds.removeFromRight(previewButtonWidth).reduced(5, 5);
    previewButton.setBounds(previewButtonBounds);


    //position mode buttons with spacing
    auto buttonRow = bounds.removeFromTop(buttonRowHeight);
    const int horizontalSpacing = 5;  // Space between and around buttons
    const int verticalSpacing = 5;    // Space above and below buttons

    // Remove vertical spacing
    // buttonRow.removeFromTop(verticalSpacing);
    buttonRow.removeFromBottom(verticalSpacing);

    // Calculate button width accounting for all spaces
    int totalSpacing = horizontalSpacing * 5; //space before first, between each (3 spaces), and after last
    int buttonWidth = (buttonRow.getWidth() - totalSpacing) / 4;

    // Position each button with spacing
    juce::Font font(16.0f);
    for (int i = 0; i < 4; i++) {
        int buttonPadding = 10;  //space to the sides of each text block
        juce::String buttonText = modeButtons[i]->getButtonText();
        auto textWidth = font.getStringWidth(buttonText);
        modeButtons[i]->setBounds(buttonRow.removeFromLeft(buttonPadding * 2 + textWidth));
    }

    bounds.removeFromBottom(4); //space under scrollbar

    //position viewport below the buttons
    viewport->setBounds(bounds.reduced(1, 0));

    //set sequence box size, keeping original height
    sequenceBox->setSize(sequenceBox->getMinimumWidth(), bounds.getHeight() - 4);   //second arg-- space btw scroll bar and pips
}



std::vector<Pip> PipSequencer::getPips() {
    //dont actually copy the pipbars, just get pointers
    const std::vector<std::unique_ptr<PipBar>>& bars = sequenceBox->pipBars;
    std::vector<Pip> pips = {};
    pips.reserve(bars.size());

    for (const auto& bar : bars) {
        if (bar) {
            pips.push_back(Pip(bar->ourPip));
        }
    }
    logPips(pips);
    return pips;
}


void PipSequencer::logPips(const std::vector<Pip> pips) {
    juce::String fullString = "";
    for (size_t i = 0; i < pips.size(); i++) {
        const auto& pip = pips[i];
        juce::String pipString = "(";

        if (pip.frequency >= 1000.0f) {
            float kHzValue = pip.frequency / 1000.0f;
            pipString += juce::String(kHzValue, 2) + "kHz, ";
        }
        else {
            int hzValue = static_cast<int>(std::round(pip.frequency));
            pipString += juce::String(hzValue) + "Hz, ";
        }

        // Length (µs/ms)
        if (pip.length < 100) {
            int usValue = static_cast<int>(std::round(pip.length));
            pipString += juce::String(usValue) + "us, ";
        }
        else {
            float msValue = pip.length / 1000.0f;
            pipString += juce::String(msValue, 2) + "ms, ";
        }

        // Tail (µs/ms)
        if (pip.tail < 100) {
            int usValue = static_cast<int>(std::round(pip.tail));
            pipString += juce::String(usValue) + "us, ";
        }
        else {
            int msValue = pip.tail / 1000.0f;
            pipString += juce::String(msValue) + "ms, ";
        }

        // Level (percentage)
        pipString += juce::String(std::round(pip.level * 100)) + "%";

        pipString += ")";
        
        if (i < pips.size() - 1) {
            pipString += ", ";
        }
        fullString += pipString;
    }

    // Log total count
    juce::Logger::writeToLog("Pips: " + fullString);
}


void PipSequencer::createInlineEditor(PipBar::PipBarArea* pba, juce::Point<int> position) {
    //convert pos from pipbar's coordinate space to pipsequencer's coordinate space
    auto editorPos = pba->localPointToGlobal(position);
    editorPos = getLocalPoint(nullptr, editorPos);

    inlineEditor = std::make_unique<juce::TextEditor>();
    addAndMakeVisible(inlineEditor.get());

    //setup inline editor appearance
    inlineEditor->setText(juce::String(pba->getValue(), 2));
    inlineEditor->setJustification(juce::Justification::centred);
    inlineEditor->setColour(juce::TextEditor::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    inlineEditor->setColour(juce::TextEditor::outlineColourId, juce::Colours::white);
    inlineEditor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::white);

    //position above click point
    const int editorWidth = 60;
    const int editorHeight = 20;
    editorPos = editorPos.withY(editorPos.getY() - editorHeight - 5);  //5px gap

    //keep editor within component bounds
    int minX = 0;
    int maxX = getWidth() - editorWidth;
    int minY = 0;
    int maxY = getHeight() - editorHeight;

    if (minX > maxX) {
        std::swap(minX, maxX);
    }
    if (minY > maxY) {
        std::swap(minY, maxY);
    }

    editorPos.setX(juce::jlimit(minX, maxX, editorPos.getX()));
    editorPos.setY(juce::jlimit(minY, maxY, editorPos.getY()));

    inlineEditor->setBounds(editorPos.getX(), editorPos.getY(), editorWidth, editorHeight);
    

    //set up editor behavior
    inlineEditor->setSelectAllWhenFocused(true);
    inlineEditor->setInputRestrictions(0, "0123456789.-");  //only numbs
    inlineEditor->grabKeyboardFocus();

    //enter key
    inlineEditor->onReturnKey = [this, pba] {
        pba->applyInlineEditorValue(inlineEditor->getText());
        inlineEditor = nullptr;
    };

    //focus lost
    inlineEditor->onFocusLost = [this, pba] {
        pba->applyInlineEditorValue(inlineEditor->getText());
        inlineEditor = nullptr;
    };
}


void PipSequencer::createModeButtons() {
    const std::array<const char*, 4> buttonLabels = {
        "Frequency", "Length", "Overlap", "Level"
    };
    const juce::Font buttonFont("Arial", 16.0f, juce::Font::plain);
    for (int i = 0; i < 4; i++) {
        modeButtons[i] = std::make_unique < juce::TextButton>(buttonLabels[i]);
        modeButtons[i]->setClickingTogglesState(true);  //true to show active state
        modeButtons[i]->setRadioGroupId(1); //mutually exclusive buttons
        //setup appearance
        auto sequenceBoxColor = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
        modeButtons[i]->setColour(juce::TextButton::buttonOnColourId, sequenceBoxColor);
        modeButtons[i]->setColour(juce::TextButton::buttonColourId, sequenceBoxColor.darker(0.3f));
        modeButtons[i]->setLookAndFeel(&modeButtonLookAndFeel); //rounded corners
        addAndMakeVisible(modeButtons[i].get());

        //setup click behavior
        modeButtons[i]->onClick = [this, i] {
            clearModeButtonStates();
            mode = static_cast<EditingMode>(i);
            modeButtons[i]->setToggleState(true, juce::dontSendNotification);
            updatePipBarModes(mode);    //propogate mode change to pipBars
            audioProcessor.updatePipBarModes(mode);
        };
    }

    //setup initial state
    modeButtons[0]->setToggleState(true, juce::dontSendNotification);
}


void PipSequencer::loadPipsFromProcessor() {
    std::vector<Pip> storedPips;
    EditingMode storedMode;
    audioProcessor.getPips(storedPips, storedMode);

    //setup remembered mode  
    clearModeButtonStates();
    mode = storedMode;
    modeButtons[mode]->setToggleState(true, juce::dontSendNotification);

    updatePipBarModes(mode);

    //setup remembered pips  
    sequenceBox->pipBars.clear();
    sequenceBox->addPips(storedPips);
}



void PipSequencer::clearModeButtonStates() {
    for (int i = 0; i < 4; i++) {
        if (static_cast<EditingMode>(i) != mode) {
            modeButtons[i]->setToggleState(false, juce::dontSendNotification);
        }
    }
}


void PipSequencer::updatePipBarModes(EditingMode newMode) {
    if (sequenceBox) {
        for (auto& pipBar : sequenceBox->pipBars) {
            if (pipBar) {
                pipBar->changeMode(newMode);
            }
        }
    }
}

void PipSequencer::changeListenerCallback(juce::ChangeBroadcaster* source){
    if (source == &audioProcessor.getPresetManager()) {
        loadPipsFromProcessor();
    }
}



//----------------------------========== Sequence Box ==========----------------------------\\

// SequenceBox Implementation
SequenceBox::SequenceBox(PipSequencer& p) : parent(p) {
    addButton.setButtonText("+");
    addButton.onClick = [this] { onAddButtonClicked(); };
    addAndMakeVisible(addButton);

    // Enable scrolling - using Viewport instead of direct scroll bars
    setWantsKeyboardFocus(true);

    //for deleting, need keyboard input
    addKeyListener(this);
}


SequenceBox::~SequenceBox() = default;


void SequenceBox::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    if (selectedPipBar != nullptr)
    {
        //get the bounds of the selected PipBarArea
        auto pipBarBounds = selectedPipBar->getBounds().toFloat();

        //calculate the base area
        auto highlightBounds = pipBarBounds.withTrimmedLeft((pipSpacing / 2) - 1).withTrimmedRight((pipSpacing / 2) - 1);
        highlightBounds.removeFromTop(selectedPipBar->pipBarArea.maxHeight - selectedPipBar->pipBarArea.currentHeight + pipValueLabelHeight);

        //draw drop shadow
        juce::DropShadow dropShadow(juce::Colours::blue, 5, juce::Point<int>(0, 0));
        dropShadow.drawForRectangle(g, highlightBounds.toNearestInt());

        //draw a subtle outline
        g.setColour(juce::Colours::blue.withAlpha(0.4f));
        g.drawRect(highlightBounds, 1.0f);
    }
}





void SequenceBox::resized() {
    auto bounds = getLocalBounds();
    bounds.removeFromBottom(scrollBarHeight);   
    bounds.removeFromTop(buttonRowHeight);     
    //position add button
    float buttonSize = 30.0f;
    float xPos = pipBars.size() * (pipWidth + pipSpacing) + pipSpacing + pipSpacing/4;

    addButton.setBounds(xPos,
                        (bounds.getHeight()) / 2,
                        buttonSize,
                        buttonSize);

    updatePipPositions();
}



int SequenceBox::getMinimumWidth() const {
    return (pipBars.size() * (pipWidth + pipSpacing) + pipSpacing * 3 );  // for add button and some padding
}



void SequenceBox::addPips(std::vector<Pip> pips) {
	for (const auto& pip : pips) {
		auto newPip = std::make_unique<PipBar>();
		newPip->mode = parent.mode;
		newPip->ourPip = pip;
		addAndMakeVisible(newPip.get());
		pipBars.push_back(std::move(newPip));
	}

	setSize(getMinimumWidth(), getHeight());
	updatePipPositions();
}

//draws each pip with the proper start coord
//also handles padding to the left of the add button when there are no pips yet
void SequenceBox::updatePipPositions() {
    auto bounds = getLocalBounds();
    bounds.removeFromBottom(scrollBarHeight);  // Reserve space for scrollbar

    for (size_t i = 0; i < pipBars.size(); ++i) {
        pipBars[i]->setBounds(i * (pipWidth + (pipSpacing)) + pipSpacing/2,
            0,
            pipWidth + pipSpacing/2,
            bounds.getHeight());
    }
}



//called by the pipbararea when it is clicked on
void SequenceBox::setSelectedPipBar(PipBar* pipBar)
{
    if (selectedPipBar != pipBar)
    {
        if (selectedPipBar != nullptr)
            selectedPipBar->setSelected(false);

        selectedPipBar = pipBar;

        if (selectedPipBar != nullptr)
            selectedPipBar->setSelected(true);

        repaint();
        resized();
    }
}




void SequenceBox::onAddButtonClicked() {
    auto newPip = std::make_unique<PipBar>();
    newPip->mode = parent.mode;
    addAndMakeVisible(newPip.get());
    pipBars.push_back(std::move(newPip));

    //update size of the container when adding a new pip
    setSize(getMinimumWidth(), getHeight());
    //scroll to show the new pip (if necessary)
    if (auto* viewport = findParentComponentOfClass<juce::Viewport>()) {
        viewport->setViewPosition(getMinimumWidth() - viewport->getWidth(), 0);
    }

    parent.updateProcessor();
}



//handle deleting a pip with backspace or the delete key
bool SequenceBox::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    if (selectedPipBar != nullptr && (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey))
    {
        deleteSelectedPipBar();
        return true;
    }
    return false;
}



void SequenceBox::deleteSelectedPipBar()
{
    if (selectedPipBar != nullptr)
    {
        auto it = std::find_if(pipBars.begin(), pipBars.end(),
            [this](const std::unique_ptr<PipBar>& pipBar) { return pipBar.get() == selectedPipBar; });

        if (it != pipBars.end())
        {
            pipBars.erase(it);
            selectedPipBar = nullptr;
            resized();
            repaint();

            parent.updateProcessor();
        }
    }
}



//----------------------------============ Pip Bars ============----------------------------\\


PipBar::PipBar() : pipBarArea(*this){
    setWantsKeyboardFocus(true);
    addAndMakeVisible(pipBarArea);

    //set up animation
    //when the height of the bar changes, the height of the text should change as well
    //all the animation for text/bar height is handled in the pipBarArea's timerCallback method
    currentTextHeight = DEFAULT_TEXT_HEIGHT;
    targetTextHeight = DEFAULT_TEXT_HEIGHT;
}



PipBar::~PipBar() = default;



void PipBar::resized() {
    auto bounds = getLocalBounds();
    //reserve space for text at the top
    pipBarArea.setBounds(bounds);
    pipBarArea.maxHeight = pipBarArea.getHeight() - pipValueLabelHeight;
}



void PipBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    /*g.drawRect(bounds, 1.0f);*/
    juce::String valueText = getFormattedValue();
    int textHeight = DEFAULT_TEXT_HEIGHT;

    //calculate the text bounds to ensure it's not clipped
    pipBarArea.updateBarHeight();


    // Calculate text bounds based on current animated heights
    float currentTotalHeight = pipBarArea.isAnimating() ?
        currentTextHeight + pipBarArea.currentHeight :
        textHeight + pipBarArea.barHeight;

    auto textBounds = bounds.removeFromTop(bounds.getHeight() - currentTotalHeight);
    if (textBounds.getHeight() < textHeight) {
        textBounds.setHeight(textHeight);
    }

    // Text
    g.setFont(textHeight);
    g.setColour(juce::Colours::white);
    g.drawText(valueText, textBounds, juce::Justification::centred);
}


//considers the mode and the current value of the pipbar and returns a nicely-formatted string 
juce::String PipBar::getFormattedValue() const {
    switch (mode) {
    case FREQUENCY:
        //switch from Hz to kHz if you feel like it I guess
        if (ourPip.frequency >= 1000.0f) {
            float kHzValue = ourPip.frequency / 1000.0f;
            return juce::String(kHzValue, 2) + "kHz";
        } else {
            int hzValue = static_cast<int>(std::round(ourPip.frequency));
            return juce::String(hzValue) + "Hz";
        }

    case LENGTH:
    case OVERLAP: {
        //same formatting for both time values
        int timeVal = (mode == LENGTH) ? ourPip.length : ourPip.tail;
        if (timeVal < 100) {  
            //show microseconds w/ no decimal place
            return juce::String(timeVal) + " us";
        }
        else {
            //ms with two decimal places
            float msVal = timeVal / 1000.f;
            return juce::String(msVal, 2) + "ms";
        }
    }
    case LEVEL:
        return juce::String(std::round(ourPip.level * 100)) + "%";

    default:
        return "";
    }
}



void PipBar::changeMode(enum EditingMode newMode) {
    mode = newMode;
    repaint();
    pipBarArea.repaint();
}


//----------------------------============ Pip Bar Area ============----------------------------\\


PipBar::PipBarArea::PipBarArea(PipBar& parent) : parentBar(parent) { 
    setWantsKeyboardFocus(true);
}


void PipBar::PipBarArea::resized() {
    auto bounds = parentBar.getLocalBounds();
    bounds.removeFromLeft(pipSpacing / 2);
    bounds.removeFromRight(pipSpacing / 2);
    setBounds(bounds);
}


void PipBar::PipBarArea::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const int outlineThickness = 2;
    const int minHeight = outlineThickness;  //min height so border is always visible

    //calculate bar bounds, ensuring minheight at least
    auto barBounds = juce::Rectangle<int>(
        bounds.getX(),
        bounds.getBottom() - std::max(barHeight, minHeight),
        bounds.getWidth(),
        std::max(barHeight, minHeight)
    );

    //outline
    g.setColour(juce::Colours::blue.darker(0.2f));
    g.fillRect(barBounds);

    //main bar (slightly inset)
    g.setColour(juce::Colours::blue);
    g.fillRect(barBounds.reduced(outlineThickness));
}


void PipBar::PipBarArea::updateBarHeight() {
    auto bounds = getLocalBounds();
    
    float heightPercent = 0.0f;
    float tailValue;
    switch (parentBar.mode) {
    case FREQUENCY:
        //log scaling for freq
        heightPercent = (std::log(parentBar.ourPip.frequency) - std::log(PipConstants::MIN_FREQUENCY)) / 
            (std::log(PipConstants::MAX_FREQUENCY) - std::log(PipConstants::MIN_FREQUENCY));
        break;
    case LENGTH:
        //linear for length values
        heightPercent = (std::log(static_cast<float>(parentBar.ourPip.length)) - std::log(static_cast<float>(PipConstants::MIN_LENGTH))) /
            (std::log(static_cast<float>(PipConstants::MAX_LENGTH)) - std::log(static_cast<float>(PipConstants::MIN_LENGTH)));
        break;
    case OVERLAP:
        
        tailValue = std::max(static_cast<float>(parentBar.ourPip.tail), 1.0f);  //ensure we don't take log of 0
        heightPercent = (std::log(tailValue) - std::log(1.0f)) /
            (std::log(static_cast<float>(PipConstants::MAX_TAIL)) - std::log(1.0f));
        if (parentBar.ourPip.tail == 0) heightPercent = 0.0f;  //force to 0 if tail is 0
        break;
    case LEVEL:
        //and linear for volume
        heightPercent = (parentBar.ourPip.level - PipConstants::MIN_LEVEL) /
            (PipConstants::MAX_LEVEL - PipConstants::MIN_LEVEL);
        break;
    }

    float newHeight = std::round(heightPercent * maxHeight);

    if (!isInitialized) {
        barHeight = newHeight;
        currentHeight = newHeight;
        targetHeight = newHeight;
        isInitialized = true;
    }
    else {
        startHeightAnimation(newHeight);
    }
    
    //barHeight = std::round(heightPercent * maxHeight);
}



void PipBar::PipBarArea::mouseDrag(const juce::MouseEvent& e) {
    auto bounds = getLocalBounds();
    
    //convert mouse position on the bar to a percent
    float normalizedValue = 1.0f - ((float)(e.y - 20) / (maxHeight));
    normalizedValue = juce::jlimit(0.0f, 1.0f, normalizedValue);

    //and then convert that position to a concrete value for the currently active parameter
    switch (parentBar.mode) {
        case FREQUENCY: {
            //log scaling for velocity
            float newFreq = std::exp(normalizedValue *
                (std::log(PipConstants::MAX_FREQUENCY) - std::log(PipConstants::MIN_FREQUENCY)) +
                std::log(PipConstants::MIN_FREQUENCY));
            parentBar.ourPip.frequency = newFreq;
            break;
        }

        case LENGTH: {
            //log scaling for length
            float newLength = std::exp(normalizedValue *
                (std::log(static_cast<float>(PipConstants::MAX_LENGTH)) - std::log(static_cast<float>(PipConstants::MIN_LENGTH))) +
                std::log(static_cast<float>(PipConstants::MIN_LENGTH)));
            parentBar.ourPip.length = static_cast<int>(std::round(newLength));
            break;
        }

        case OVERLAP: {
            if (normalizedValue < 0.01f) {
                //handle zero tail
                parentBar.ourPip.tail = 0;
            }
            else {
                //log scaling for non-zero tail values
                float newTail = std::exp(normalizedValue *
                    (std::log(static_cast<float>(PipConstants::MAX_TAIL)) - std::log(1.0f)) +
                    std::log(1.0f));
                parentBar.ourPip.tail = static_cast<int>(std::round(newTail));
            }
            break;
        }

        case LEVEL: {
            //linear for level
            parentBar.ourPip.level = normalizedValue;
            break;
        }
    }

    repaint();
    parentBar.repaint();
    if (auto* sequenceBox = findParentComponentOfClass<SequenceBox>()) {
        if (auto* pipSequencer = sequenceBox->findParentComponentOfClass<PipSequencer>()) {
            pipSequencer->updateProcessor();
        }
    }
}


void PipBar::PipBarArea::mouseDoubleClick(const juce::MouseEvent& e) {
    //this nonsense is necessary to draw the inline editor box over everything else in the pip sequencer
    PipSequencer* ps = findParentComponentOfClass<PipBar>()->
        findParentComponentOfClass<SequenceBox>()->
        findParentComponentOfClass<PipSequencer>();

    ps->createInlineEditor(this, e.getPosition());
}


float PipBar::PipBarArea::getValue() {
    struct Pip parentPip = parentBar.ourPip;
    switch (parentBar.mode) {
        case FREQUENCY:
            return parentPip.frequency;
        case LENGTH:
            return parentPip.length;
        case OVERLAP:
            return parentPip.tail;
        case LEVEL:
            return parentPip.level;
        default:
            return -99;
    }
}


void PipBar::PipBarArea::applyInlineEditorValue(juce::String rawInput) {
    //parentBar.ourPip;
    float newValue = rawInput.getFloatValue();
    
    switch (parentBar.mode) {
        case FREQUENCY:
            parentBar.ourPip.frequency = juce::jlimit(PipConstants::MIN_FREQUENCY, 
                                                      PipConstants::MAX_FREQUENCY, 
                                                      newValue);
            
        case LENGTH:
            parentBar.ourPip.length = juce::jlimit(PipConstants::MIN_LENGTH,
                                                   PipConstants::MAX_LENGTH,
                                                   juce::roundToInt(newValue));
            goto update;
        case OVERLAP:
            parentBar.ourPip.tail = juce::jlimit(PipConstants::MIN_TAIL,
                                                 PipConstants::MAX_TAIL,
                                                 juce::roundToInt(newValue));
            goto update;
        case LEVEL:
            parentBar.ourPip.level = juce::jlimit(PipConstants::MIN_LEVEL,
                                                  PipConstants::MAX_LEVEL,
                                                  newValue);
            goto update;
        default:
            return;
    }

update:
    repaint();
    parentBar.repaint();
    if (auto* sequenceBox = parentBar.findParentComponentOfClass<SequenceBox>()) {
        if (auto* pipSequencer = sequenceBox->findParentComponentOfClass<PipSequencer>()) {
            pipSequencer->updateProcessor();
        }
    }
    return;
}


void PipBar::PipBarArea::timerCallback() {
    //calculate distance to move this frame
    float barDiff = targetHeight - currentHeight;
    float barStep = barDiff * ANIMATION_SPEED;

    //update parent text height
    float textDiff = parentBar.targetTextHeight - parentBar.currentTextHeight;
    float textStep = textDiff * ANIMATION_SPEED;
    parentBar.currentTextHeight += textStep;

    //update bar position
    currentHeight += barStep;
    barHeight = currentHeight;  //update the actual bar height

    //stop if we're close enough to both targets
    if (std::abs(barDiff) < 0.5f && std::abs(textDiff) < 0.5f) {
        currentHeight = targetHeight;
        barHeight = targetHeight;
        parentBar.currentTextHeight = parentBar.targetTextHeight;
        stopTimer();
    }

    parentBar.repaint();
    repaint();
}


void PipBar::PipBarArea::startHeightAnimation(float newTarget) {
    targetHeight = newTarget;
    if (!isTimerRunning()) {
        currentHeight = barHeight;  // Start from current position
        startTimer(ANIMATION_INTERVAL);
    }
}


void PipBar::PipBarArea::mouseDown(const juce::MouseEvent& e) {
    SequenceBox* sq = parentBar.findParentComponentOfClass<SequenceBox>();
    sq->setSelectedPipBar(&parentBar);
    parentBar.setSelected(true);
    grabKeyboardFocus();
    repaint();
}


void PipBar::PipBarArea::focusLost(FocusChangeType cause) {
    if (parentBar.selected) {
        //deselect in parent
        parentBar.setSelected(false);

        //propogate deselection to the sequence box
        SequenceBox* sq = parentBar.findParentComponentOfClass<SequenceBox>();
        sq->setSelectedPipBar(nullptr);
    }
}
