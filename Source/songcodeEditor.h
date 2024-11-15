/*
  ==============================================================================

    songcodeEditor.h
    Created: 6 Oct 2024 3:17:13pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SongcodeEditor : public juce::Component
{
    public:
        SongcodeEditor(const juce::String& title);
        ~SongcodeEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        juce::String getText() const;
        void setText(const juce::String& newText);

        void SongcodeEditor::setErrorMessage(const juce::String& errorMessage, juce::Colour color);
    private:
        juce::Label titleLabel;
        juce::TextEditor mainEditor;
        juce::Label errorLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongcodeEditor)
};