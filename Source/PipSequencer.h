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
struct PipBarArea;
class SequenceBox;



const float pipWidth = 40.0f;
const float pipSpacing = 20.0f;
const int scrollBarHeight = 10;
const int buttonRowHeight = 30;
const int pipValueLabelHeight = 20;

const int ANIMATION_INTERVAL = 15;  //ms between frames
const float ANIMATION_SPEED = 0.3f; //how much to move target between frames.
const int DEFAULT_TEXT_HEIGHT = 12;

// Add this class to handle square corners
class SquareLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override {
        auto bounds = button.getLocalBounds().toFloat();
        auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || button.getToggleState())
            baseColour = baseColour.darker(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.1f);

        g.setColour(baseColour);
        g.fillRect(bounds); // Use fillRect instead of fillRoundedRectangle

        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.drawRect(bounds, 1.0f); // Use drawRect instead of drawRoundedRectangle
    }
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
    void changeMode(enum EditingMode newMode);

    float currentTextHeight;
    float targetTextHeight;

    //inner class that handles all mouse events, and draws the bar
    struct PipBarArea : public juce::Component, public juce::Timer {
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
class PipSequencer : public juce::Component
{
public:
    PipSequencer();
    ~PipSequencer() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    std::vector<Pip> getPips();
    void logPips(const std::vector<Pip> pips);
    void createInlineEditor(PipBar::PipBarArea* pba, juce::Point<int> position);
    void updatePipBarModes(EditingMode newMode);

    enum EditingMode mode = EditingMode::FREQUENCY;
private:

    juce::Label titleLabel;
    std::unique_ptr<SequenceBox> sequenceBox; //container for pip bars
    std::unique_ptr<juce::TextEditor> inlineEditor; //this is the little text box that appears when you double click
    std::unique_ptr<juce::Viewport> viewport;   //horizontal scrolling on pips when they overflow
    SquareLookAndFeel squareLookAndFeel;    //style for buttons

    std::array<std::unique_ptr<juce::TextButton>, 4> modeButtons;
    void createModeButtons();
    void clearModeButtonStates();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PipSequencer)
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








