/*
  ==============================================================================
    PipSequencer.cpp
    Created: 12 Nov 2024 8:44:02pm
    Author:  Taro
  ==============================================================================
*/

#include "PipSequencer.h"
#include "PluginEditor.h"




//----------------------------========== Pip Sequencer ==========----------------------------\\

PipSequencer::PipSequencer(BugsoundsAudioProcessor& p, BugsoundsAudioProcessorEditor& editor) : audioProcessor(p), audioEditor(editor), modeButtonLookAndFeel(*this) {
    //set up title 
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f * scalar).withExtraKerningFactor(.05f));
    titleLabel.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
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
    previewButton.setName("ClickPreview");
    previewButton.onClick = [this] { audioProcessor.triggerPreviewClick(); };
    addAndMakeVisible(previewButton);

    loadPipsFromProcessor();
    audioProcessor.getPresetManager().addChangeListener(this);

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("subclickSequencer"); });
    helpButton->setName("SequencerHelp");
    addAndMakeVisible(helpButton.get());
}


PipSequencer::~PipSequencer() {
    audioProcessor.getPresetManager().removeChangeListener(this);
}


void PipSequencer::paint(juce::Graphics& g) {
    //normal stuff -- outline and title underline
    auto bounds = getLocalBounds();
    scalar = getHeight() / 189.f;

    auto headerBounds = bounds.removeFromTop(35.f * scalar);
    auto bodyBounds = bounds;

    //headeer
    Colour c1 = Colour::fromString("#818BCA").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#818BCA").withAlpha(1.0f);
    drawUIBlock(g, headerBounds, c1, c2, false, true, scalar);

    //body
    c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bodyBounds, c1, c2, false, true, scalar);

    //draw border under mode buttons
    auto modeButtonBounds = bodyBounds.removeFromTop((buttonRowHeight - 5.f) * scalar);
    g.drawRect(modeButtonBounds, 1.0f * scalar);

}


void PipSequencer::resized() {
    scalar = getHeight() / 189.f;
    auto bounds = getLocalBounds();

    //----------------------- title stuff: buttons and text. ------------------------------
    auto titleHeight = 35.f * scalar;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleBounds.removeFromLeft(5.f * scalar);
    titleLabel.setBounds(titleBounds);
    float fontHeight = scalar * 21.f;
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight).withExtraKerningFactor(.05f));

    //help buton
    auto helpArea = titleBounds.removeFromRight(titleHeight);
    helpButton->setBounds(helpArea);

    //preview button
    titleBounds.removeFromRight(4.f * scalar);
    auto previewButtonBounds = titleBounds.removeFromRight(titleHeight);
    previewButton.setBounds(previewButtonBounds);

    //sequence container scroll bar
    sequenceBox->setSize(sequenceBox->getWidth(), 110 * scalar);

    //position mode buttons with spacing
    auto buttonRow = bounds.removeFromTop(buttonRowHeight * scalar);
    const int horizontalSpacing = 5.f * scalar;  // Space between and around buttons
    const int verticalSpacing = 5.f   * scalar;    // Space above and below buttons

    // Remove vertical spacing
    // buttonRow.removeFromTop(verticalSpacing);
    buttonRow.removeFromBottom(verticalSpacing);

    // Calculate button width accounting for all spaces
    int totalSpacing = horizontalSpacing * 5; //space before first, between each (3 spaces), and after last
    int buttonWidth = (buttonRow.getWidth() - totalSpacing) / 4;

    // Position each button with spacing
    juce::Font font(16.0f * scalar);
    for (int i = 0; i < 4; i++) {
        int buttonPadding = 10.f * scalar;  //space to the sides of each text block
        juce::String buttonText = modeButtons[i]->getButtonText();
        auto textWidth = font.getStringWidth(buttonText);
        modeButtons[i]->setBounds(buttonRow.removeFromLeft((buttonPadding * 2.f + textWidth)));
    }

    bounds.removeFromBottom(4.f * scalar); //space under scrollbar

   
    viewport->setBounds(bounds.reduced(1.f * scalar, scalar));


    auto* content = viewport->getViewedComponent();
    if (content != nullptr)
    {
        content->setTransform(juce::AffineTransform::scale(scalar, scalar));
    }

    //set sequence box size, keeping original height
    //OK to use non-scaling vals here. the transofrm will handle it
    sequenceBox->setSize(sequenceBox->getMinimumWidth(), 110);   //second arg-- space btw scroll bar and pips
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

    //position above click point
    const int editorWidth = 70.f * scalar;
    const int editorHeight = 20.f * scalar;
    editorPos = editorPos.withY(editorPos.getY() - editorHeight - 5);  //5px gap

    //setup inline editor appearance
    inlineEditor->setFont(UIDrawer::getFontCode().withHeight(15 * scalar));
    inlineEditor->setText(juce::String(pba->getValue(), 2));
    inlineEditor->setJustification(juce::Justification::centred);
    inlineEditor->setColour(juce::TextEditor::backgroundColourId, Colour::fromString("#163359").withAlpha(1.0f));
    inlineEditor->setColour(juce::TextEditor::outlineColourId, Colour::fromString("#163359").withAlpha(1.0f));
    inlineEditor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::white);

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


void SequenceBox::paint(juce::Graphics& g) {
    float scale = parent.scalar;

    if (selectedPipBar != nullptr) {

        /*auto barBounds = juce::Rectangle<int>(
            bounds.getX(),
            bounds.getBottom() - std::max(barHeight * scalar, minHeight * scalar),
            bounds.getWidth(),
            std::max(barHeight * scalar, minHeight * scalar)
        );*/
        //get the bounds of the selected PipBarArea
        Rectangle<int> pipBarBounds = selectedPipBar->innerBarRectangle;

        //calculate the base area
        Rectangle<int> highlightBounds = pipBarBounds;
        float centreX = selectedPipBar->getX() + 10.f;
        highlightBounds.translate(centreX, 0);

        //draw drop shadow
        juce::DropShadow dropShadow(juce::Colours::blue, 5, juce::Point<int>(0, 0));
        dropShadow.drawForRectangle(g, highlightBounds);

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
    float buttonSize = 35.0f;
    float xPos = pipBars.size() * (pipWidth + pipSpacing) + pipSpacing + pipSpacing/4;

    addButton.setBounds(xPos,
                        (bounds.getHeight()) / 2,
                        buttonSize,
                        buttonSize);

    updatePipPositions();
}



int SequenceBox::getMinimumWidth() const {
    return (pipBars.size() * (pipWidth + pipSpacing) + pipSpacing * 3 ) * parent.scalar;  // for add button and some padding
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


void SequenceBox::deleteSelectedPipBar(){
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
    g.setFont(UIDrawer::getFontInterRegular().withHeight(textHeight * 1.2));
    g.setColour(juce::Colours::black);
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

    parentBar.innerBarRectangle = barBounds;

    Colour c1 = Colour::fromString("#7BADBC").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#89AEDE").withAlpha(1.0f);
    drawUIBlock(g, barBounds, c1, c2, false, false, .5f);
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


//----------------------------========== Tab Buttons LNF ==========----------------------------\\

TabStyleLookAndFeel::TabStyleLookAndFeel(PipSequencer& p) : parent(p) {

}


void TabStyleLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    //set up measurements for each part in advance
    bool isSelected = button.getToggleState();
    float scalar = parent.scalar;
    Rectangle<int> boundingBox;
    Rectangle<int> originalBox;
    Rectangle<int> leftShadowBox;
    Rectangle<int> rightShadowBox;
    Rectangle<int> highlightBox;
    if (isSelected) {
        boundingBox = button.getLocalBounds();
    } else {
        boundingBox = button.getLocalBounds().withTrimmedTop(4.f * scalar);
    }
    originalBox = boundingBox;
    leftShadowBox = boundingBox.removeFromLeft(4.f * scalar).withTrimmedRight(1.f * scalar);
    rightShadowBox = boundingBox.removeFromRight(4.f * scalar).withTrimmedRight(1.f * scalar);
    highlightBox = { originalBox.getX() + int(4.f * scalar),
                     originalBox.getY() + int(3.f * scalar),
                     originalBox.getWidth() - int(5.f * scalar),
                     int(1.f * scalar)};

    leftShadowBox.removeFromBottom(1.0f * scalar);
    rightShadowBox.removeFromBottom(2.0f * scalar);
    rightShadowBox.removeFromTop(1.f * scalar);
    if (isSelected) rightShadowBox.removeFromTop(1.0f * scalar);
    //no bottom shadow

     //outline
    g.setColour(Colour::fromString("#163359").withAlpha(1.0f));
    g.drawRect(originalBox, (int)(scalar * 1.f));

    //erase the bottom line of the outline on the selected tab so it looks connected to the main editor
    if (isSelected) {
        g.setColour(Colour::fromString("#d9d9d9").withAlpha(1.f));
        g.fillRect(
            ((float) originalBox.getX() + 3.f * scalar),
            (originalBox.getBottom() - 1 * scalar),
            ((float)originalBox.getWidth() - 4.f * scalar),
            1.f * scalar
        );
    }

    g.setColour(Colour::fromString("#FFFFFF").withAlpha(.76f));
    g.fillRect(highlightBox);

    g.setColour(Colour::fromString("#163359").withAlpha(.5f));
    g.fillRect(leftShadowBox);

    g.setColour(Colour::fromString("#70AAF5").withAlpha(.67f));
    g.fillRect(rightShadowBox.translated(0, 1.f * scalar));


}



juce::Font TabStyleLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight) { return juce::Font(13.f * parent.scalar); }



void TabStyleLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);
    g.setColour(Colour::fromString("#000000").withAlpha(1.0f)
        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    auto yOffset = button.getToggleState() ? 0.0f : 2.0f;  // 2px lower if not selected
    auto textBounds = button.getLocalBounds();
    textBounds.translate(0, (int)yOffset);

    g.drawText(button.getButtonText(), textBounds,
        juce::Justification::centred, false);
}
