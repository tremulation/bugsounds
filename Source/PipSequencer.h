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
#include "PluginProcessor.h"
#include "ButtonsAndStuff.h"
#include "UIDrawer.h"

class PipBar;
struct PipBarArea;
class SequenceBox;
class PipSequencer;

class BugsoundsAudioProcessorBugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;

const float pipWidth = 40.0f;
const float pipSpacing = 20.0f;
const int scrollBarHeight = 10;
const int buttonRowHeight = 30;
const int pipValueLabelHeight = 20;

const int ANIMATION_INTERVAL = 15;  //ms between frames
const float ANIMATION_SPEED = 0.3f; //how much to move target between frames.
const int DEFAULT_TEXT_HEIGHT = 12;

class TabStyleLookAndFeel : public juce::LookAndFeel_V4 {
public:
    TabStyleLookAndFeel(PipSequencer& p);

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight);

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
private:
    PipSequencer& parent;
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
    
    //for animations
    float currentTextHeight;
    float targetTextHeight;

    void setSelected(bool isSelected) { selected = isSelected; repaint(); }
    bool selected = true;
    void changeMode(enum EditingMode newMode);

    juce::Rectangle<int> innerBarRectangle;

    //inner class that handles all mouse events, and draws the bar
    struct PipBarArea : public juce::Component, public juce::Timer, public UIDrawer {
        PipBarArea(PipBar& parent);
        void paint(juce::Graphics&) override;
        void resized() override;
        int barHeight;  //in pixels
        int maxHeight = 0;
        void updateBarHeight();
        float getValue();

        //mouse events
        void mouseDrag(const juce::MouseEvent& e);
        void mouseDown(const juce::MouseEvent& e);
        void mouseDoubleClick(const juce::MouseEvent& e);
        void applyInlineEditorValue(juce::String rawInput);
        void focusLost(FocusChangeType cause) override;

        //animation
        void startHeightAnimation(float newTarget);
        void timerCallback() override;
        bool isAnimating() const { return isTimerRunning();  }

        float currentHeight;
        float targetHeight;
        
    private:
        
        bool isDragging = false;
        bool isInitialized = false;
        PipBar& parentBar;
    };

    PipBarArea pipBarArea;
private:

    juce::String getFormattedValue() const;
};


//MAIN CLASS
//the main UI component that contains all the different elements of the pip sequencer
class PipSequencer : public juce::Component, public juce::ChangeListener, public::UIDrawer
{
public:
    PipSequencer(BugsoundsAudioProcessor& p, BugsoundsAudioProcessorEditor& editor);
    ~PipSequencer() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    std::vector<Pip> getPips();
    void logPips(const std::vector<Pip> pips);
    void createInlineEditor(PipBar::PipBarArea* pba, juce::Point<int> position);
    void updatePipBarModes(EditingMode newMode);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void updateProcessor() {
        auto pips = getPips();
        audioProcessor.setPips(pips);
    }

    enum EditingMode mode = EditingMode::FREQUENCY;
    float scalar = 1.0f;
private:
    void loadPipsFromProcessor();
    juce::Label titleLabel;
    std::unique_ptr<SequenceBox> sequenceBox; //container for pip bars
    std::unique_ptr<juce::TextEditor> inlineEditor; //this is the little text box that appears when you double click
    std::unique_ptr<juce::Viewport> viewport;   //horizontal scrolling on pips when they overflow
    TabStyleLookAndFeel modeButtonLookAndFeel;    //style for buttons
    
    juce::TextButton previewButton;

    std::unique_ptr<HelpButton>         helpButton;

    std::array<std::unique_ptr<juce::TextButton>, 4> modeButtons;
    void createModeButtons();
    void clearModeButtonStates();

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PipSequencer)
public:
};




//this contains all the pip bars, as well as the button for adding more pips
class SequenceBox : public juce::Component, public juce::KeyListener, public juce::MouseListener 
{
public:
    SequenceBox(PipSequencer& p); //why does c++ do constructors like this. it is so weird!
    ~SequenceBox() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    int getMinimumWidth() const;

    void setSelectedPipBar(PipBar* pipBar);
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    std::vector<std::unique_ptr<PipBar>> pipBars;

	void addPips(std::vector<Pip> pips);

    void updatePipPositions();
private:
    PipSequencer& parent;

    //add and delete functionality
    PipBar* selectedPipBar = nullptr;
    void deleteSelectedPipBar();
    juce::TextButton addButton;
    void onAddButtonClicked();

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequenceBox)
};








