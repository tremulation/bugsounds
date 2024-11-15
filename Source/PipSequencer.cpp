/*
  ==============================================================================
    PipSequencer.cpp
    Created: 12 Nov 2024 8:44:02pm
    Author:  Taro
  ==============================================================================
*/

#include "PipSequencer.h"


//----------------------------========== Pip Sequencer ==========----------------------------\\

PipSequencer::PipSequencer() {
    //set up title 
    titleLabel.setFont(juce::Font(16.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Pip Sequencer", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    //set up sequencebox and viewport
    viewport = std::make_unique<juce::Viewport>();
    sequenceBox = std::make_unique<SequenceBox>(*this);

    viewport->setViewedComponent(sequenceBox.get(), false);
    viewport->setScrollBarsShown(false, true); //only need horizontal scroll bar
    addAndMakeVisible(viewport.get());
}


PipSequencer::~PipSequencer() = default;


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
}


void PipSequencer::resized() {
    auto bounds = getLocalBounds().reduced(5);

    //position title
    auto titleHeight = 30;
    auto titleBounds = bounds.removeFromTop(titleHeight);
    titleLabel.setBounds(titleBounds.reduced(5, 0));

    //Leave space for buttons at the top by removing buttonRowHeight
    bounds.removeFromTop(buttonRowHeight);
    bounds.removeFromBottom(3); //scrollbar spacing

    //position viewport below the title and buttons
    viewport->setBounds(bounds.reduced(1, 0));

    //set sequence box size, keeping original height
    sequenceBox->setSize(sequenceBox->getMinimumWidth(), bounds.getHeight() - scrollBarHeight);  // Leave room for scrollbar
   
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
    for (size_t i = 0; i < pips.size(); i++) {
        const auto& pip = pips[i];
        juce::String pipString = "Pip " + juce::String(i) + ": ";

        // Frequency (Hz/kHz)
        if (pip.frequency >= 1000.0f) {
            int kHzValue = static_cast<int>(std::round(pip.frequency / 1000.0f));
            pipString += juce::String(kHzValue) + "kHz, ";
        }
        else {
            int hzValue = static_cast<int>(std::round(pip.frequency));
            pipString += juce::String(hzValue) + "Hz, ";
        }

        // Length (탎/ms)
        if (pip.length < 100) {
            int usValue = static_cast<int>(std::round(pip.length));
            pipString += juce::String(usValue) + "탎, ";
        }
        else {
            int msValue = static_cast<int>(std::round(pip.length / 1000.0f));
            pipString += juce::String(msValue) + "ms, ";
        }

        // Tail (탎/ms)
        if (pip.tail < 100) {
            int usValue = static_cast<int>(std::round(pip.tail));
            pipString += juce::String(usValue) + "탎, ";
        }
        else {
            int msValue = static_cast<int>(std::round(pip.tail / 1000.0f));
            pipString += juce::String(msValue) + "ms, ";
        }

        // Level (percentage)
        pipString += juce::String(std::round(pip.level * 100)) + "%";

        juce::Logger::writeToLog(pipString);
    }

    // Log total count
    juce::Logger::writeToLog("Total pips: " + juce::String(pips.size()));
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





//----------------------------========== Sequence Box ==========----------------------------\\

// SequenceBox Implementation
SequenceBox::SequenceBox(PipSequencer& p) : parent(p) {
    addButton.setButtonText("+");
    addButton.onClick = [this] { onAddButtonClicked(); };
    addAndMakeVisible(addButton);

    // Enable scrolling - using Viewport instead of direct scroll bars
    setWantsKeyboardFocus(true);
}


SequenceBox::~SequenceBox() = default;


void SequenceBox::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::grey);
    //g.drawRect(getLocalBounds());   //remove this outline later -- once everything is positioned correctly
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


void SequenceBox::onAddButtonClicked() {
    auto newPip = std::make_unique<PipBar>();
    newPip->mode = parent.mode;
    juce::Logger::writeToLog("The editing mode is: " + std::to_string(parent.mode));
    addAndMakeVisible(newPip.get());
    pipBars.push_back(std::move(newPip));

    //update size of the container when adding a new pip
    setSize(getMinimumWidth(), getHeight());
    //scroll to show the new pip (if necessary)
    if (auto* viewport = findParentComponentOfClass<juce::Viewport>()) {
        viewport->setViewPosition(getMinimumWidth() - viewport->getWidth(), 0);
    }
}


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


//----------------------------============ Pip Bars ============----------------------------\\


PipBar::PipBar() : pipBarArea(*this){
    setWantsKeyboardFocus(true);
    addAndMakeVisible(pipBarArea);
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
    int textHeight = 12;

    //calculate the text bounds to ensure it's not clipped
    pipBarArea.updateBarHeight();
    //fix text bounds to the top
    auto textBounds = bounds.removeFromTop(bounds.getHeight() - pipBarArea.barHeight);
    if (textBounds.getHeight() < textHeight) {
        textBounds.setHeight(textHeight);
    }
    
    //rectangle
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(textBounds);
    
    //text
    g.setFont(textHeight);
    g.setColour(juce::Colours::white);
    //g.drawRect(textBounds, 1.0f); //for testing
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
        if (ourPip.length < 100) {
            int usValue = static_cast<int>(std::round(ourPip.length));
            return juce::String(usValue) + " 탎";
        }
        else {
            int msValue = static_cast<int>(std::round(ourPip.length / 1000.0f));
            return juce::String(msValue) + " ms";
        }
        
    case TAIL:
        if (ourPip.tail < 100) {
            int usValue = static_cast<int>(std::round(ourPip.tail));
            return juce::String(usValue) + " 탎";
        }
        else {
            int msValue = static_cast<int>(std::round(ourPip.tail / 1000.0f));
            return juce::String(msValue) + " ms";
        }

    case LEVEL:
        return juce::String(std::to_string(ourPip.level * 100) + "%");

    default:
        return "";
    }
}


//----------------------------============ Pip Bar Area ============----------------------------\\


PipBar::PipBarArea::PipBarArea(PipBar& parent) : parentBar(parent) { 
    juce::Logger::writeToLog("Max height at initialization: " + std::to_string(maxHeight));
    resized();
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
    //TODO handle different vals being edited 
    float normalizedFreq = (std::log(parentBar.ourPip.frequency) - std::log(PipConstants::MIN_FREQUENCY)) / (std::log(PipConstants::MAX_FREQUENCY) - std::log(PipConstants::MIN_FREQUENCY));
    barHeight = std::round(normalizedFreq * maxHeight);

}


void PipBar::PipBarArea::mouseDown(const juce::MouseEvent& e) {
    //TODO maybe apply a highlight here?
}


void PipBar::PipBarArea::mouseDrag(const juce::MouseEvent& e) {
    auto bounds = getLocalBounds();
    //TODO handle different active vals
    float normalizedValue = 1.0f - ((float)(e.y - 20) / (maxHeight));
    normalizedValue = juce::jlimit(0.0f, 1.0f, normalizedValue);

    //convert normalized value to frequency (logarithmic scale)
    float newFreq = std::exp(normalizedValue * (std::log(PipConstants::MAX_FREQUENCY) - std::log(PipConstants::MIN_FREQUENCY)) + std::log(PipConstants::MIN_FREQUENCY));
    parentBar.ourPip.frequency = newFreq;
    repaint();
    parentBar.repaint();
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
        case TAIL:
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
            repaint();
            parentBar.repaint();
        case LENGTH:
            parentBar.ourPip.length = juce::jlimit(PipConstants::MIN_LENGTH,
                                                   PipConstants::MAX_LENGTH,
                                                   juce::roundToInt(newValue));
            repaint();
            parentBar.repaint();
        case TAIL:
            parentBar.ourPip.tail = juce::jlimit(PipConstants::MIN_TAIL,
                                                 PipConstants::MAX_TAIL,
                                                 juce::roundToInt(newValue));
            repaint();
            parentBar.repaint();
        case LEVEL:
            parentBar.ourPip.level = juce::jlimit(PipConstants::MIN_LEVEL,
                                                  PipConstants::MAX_LEVEL,
                                                  newValue);
            repaint();
            parentBar.repaint();
        default:
            return;
    }
}