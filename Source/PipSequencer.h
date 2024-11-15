/*
  ==============================================================================
    PipSequencer.h
    Created: 12 Nov 2024 8:44:02pm
    Author:  Taro
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "PipStructs.h"


class PipBar;
class PaddedPipBar;
class SequenceBox;


const float pipWidth = 30.0f;
const float pipSpacing = 30.0f;
const int scrollBarHeight = 10;
const int buttonRowHeight = 20;
const int pipValueLabelHeight = 20;

//the main UI component that contains all the different elements of the pip sequencer
class PipSequencer : public juce::Component
{
public:
    PipSequencer();
    ~PipSequencer() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    std::vector<Pip> getPips();

    enum EditingMode mode = EditingMode::FREQUENCY;
private:
    juce::Label titleLabel;
    //addbutton should go in the sequenceBox -- at the end
    /*juce::TextButton addButton;*/

    std::unique_ptr<SequenceBox> sequenceBox; //container for pip bars
    std::unique_ptr<juce::TextEditor> inlineEditor; //this is the little text box that appears when you double click
    std::unique_ptr<juce::Viewport> viewport;   //horizontal scrolling on pips when they overflow

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PipSequencer)
};



//a bar representing a single pip. It can be edited to change the properties of the pip it corresponds to.
class PipBar : public juce::Component {
public:
    PipBar();
    ~PipBar() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    enum EditingMode mode;  //set by sequencebox when a pip is created
    struct Pip ourPip;
    

private:
    //inner class that handles all mouse events, and draws the bar
    struct PipBarArea : public juce::Component {
        PipBarArea(PipBar& parent);
        void paint(juce::Graphics&) override;
        void resized() override;
        int barHeight;  //in pixels
        int maxHeight = 0;
        void updateBarHeight();

        //mouse events
        void mouseDrag(const juce::MouseEvent& e);
        void mouseDown(const juce::MouseEvent& e);
    private:
        bool isDragging = false;
        
        PipBar& parentBar;
    };
    PipBarArea pipBarArea;
    
    juce::String getFormattedValue() const;  
};


//this contains all the pip bars, as well as the button for adding more pips
class SequenceBox : public juce::Component 
{
public:
    SequenceBox(PipSequencer& p); //why does c++ do constructors like this. it is so weird!
    ~SequenceBox() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    int getMinimumWidth() const;

    std::vector<std::unique_ptr<PipBar>> pipBars;
private:
    
    juce::TextButton addButton;

    PipSequencer& parent;
    void updatePipPositions();
    void onAddButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequenceBox)
};