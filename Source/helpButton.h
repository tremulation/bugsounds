/*
  ==============================================================================

    helpButton.h
    Created: 27 Apr 2025 8:01:48pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>

class BugsoundsAudioProcessorEditor;

//==============================================================================


// A little TextButton that carries its own pageID and calls back to the editor
class HelpButton : public juce::TextButton {
public:


    explicit HelpButton(std::function<void()> onClickCallback){
        setLookAndFeel(&laf);
        setClickingTogglesState(false);
        onClick = std::move(onClickCallback);
        10 + 10;
    }


    ~HelpButton() override {
        setLookAndFeel(nullptr);
    }

    juce::String pageID;
private:


    // LookAndFeel that draws a “?” button
    struct HelpButtonLookAndFeel : public juce::LookAndFeel_V4
    {
        void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool, bool) override {
            auto bounds = b.getLocalBounds().toFloat();
            g.setColour(b.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
            g.fillRect(bounds);
            g.setColour(juce::Colours::white);
            g.drawRect(bounds, 1.0f);
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override {
            g.setColour(juce::Colours::white);
            auto font = juce::Font(20.0f, juce::Font::bold);
            g.setFont(font);
            g.drawText("?", b.getLocalBounds(), juce::Justification::centred);
        }
    };

    HelpButtonLookAndFeel laf;
};