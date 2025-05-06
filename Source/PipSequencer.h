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

class PipBar;
struct PipBarArea;
class SequenceBox;

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
    TabStyleLookAndFeel() {
        setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto baseColour = backgroundColour;

        // Determine if button is selected
        bool isSelected = button.getToggleState();

        if (isSelected) {
            // Selected tab appearance
            g.setColour(baseColour);
            g.fillRect(bounds);

            // Draw borders on three sides only (left, top, right)
            g.setColour(juce::Colours::white);
            g.drawRect(bounds, 1.0f);

            // Extend the fill slightly below to cover the border of the sequenceBox
            g.setColour(baseColour);
            g.fillRect(bounds.getX() + 1, bounds.getBottom() - 1, bounds.getWidth() - 2, 2.0f);
        }
        else {
            // Unselected tab appearance
            auto darkerColor = baseColour.darker(0.2f);
            auto reducedBounds = bounds.withTrimmedTop(4);
            g.setColour(darkerColor);
            g.fillRect(reducedBounds);

            // Draw all borders for unselected tabs
            g.setColour(juce::Colours::white);
            g.drawRect(reducedBounds, 1.0f);

            // Add subtle inner shadow for recessed effect
            g.setColour(juce::Colours::black.withAlpha(0.1f));
            g.drawHorizontalLine(1, reducedBounds.getX() + 1, reducedBounds.getRight() - 1);
            g.drawVerticalLine(1, reducedBounds.getY() + 1, reducedBounds.getBottom() - 1);
        }
    }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) { return juce::Font(13.f); }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
            : juce::TextButton::textColourOffId)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

        auto yOffset = button.getToggleState() ? 0.0f : 2.0f;  // 2px lower if not selected
        auto textBounds = button.getLocalBounds();
        textBounds.translate(0, (int)yOffset);

        g.drawText(button.getButtonText(), textBounds,
            juce::Justification::centred, false);
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
    
    //for animations
    float currentTextHeight;
    float targetTextHeight;

    void setSelected(bool isSelected) { selected = isSelected; repaint(); }
    bool selected = true;
    void changeMode(enum EditingMode newMode);

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
class PipSequencer : public juce::Component, public juce::ChangeListener
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








