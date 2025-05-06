/*
  ==============================================================================

    songcodeEditor.h
    Created: 6 Oct 2024 3:17:13pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Evaluator.h"
#include "ButtonsAndStuff.h"

struct ErrorInfo;


class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;

class SongcodeEditor : public juce::Component,
                       public juce::TextEditor::Listener,
                       public juce::ChangeListener
{
    public:
        SongcodeEditor(const juce::String& title, const juce::String& helpPage, BugsoundsAudioProcessor& p, BugsoundsAudioProcessorEditor& e);
        ~SongcodeEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;
        void paintOverChildren(juce::Graphics& g) override;

        juce::String getText() const;
        void setText(const juce::String& newText);

        //if error is null, then successfully compiled.
        void SongcodeEditor::setError(ErrorInfo* error = nullptr);
        void clearErrorHighlight();
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        void disableEditor();
        void enableEditor();

    private:
        void textEditorTextChanged(juce::TextEditor&) override;
        void paintOverlay(juce::Graphics& g);

        juce::Label titleLabel;
        juce::TextEditor mainEditor;
        juce::Label errorLabel;

        juce::Range<int> currentErrorRange;
        int oldTextLength = 0;
        bool hasActiveError = false;
        juce::LookAndFeel_V4 errorLookAndFeel;
        juce::LookAndFeel_V4* normalLookAndFeel;

        std::unique_ptr<HelpButton>         helpButton;


        juce::Colour defaultEditorColour;
        juce::Colour defaultBackgroundColour;
        juce::Colour disabledEditorColour;
        juce::Colour disabledBackgroundColour;

        bool isDisabled;

        BugsoundsAudioProcessor& audioProcessor;
        BugsoundsAudioProcessorEditor& audioEditor;
        juce::String title;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongcodeEditor)
};