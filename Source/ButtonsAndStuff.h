/*
  ==============================================================================

    ButtonsAndStuff.h
    Created: 5 May 2025 10:12:37pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>


class BugsoundsAudioProcessorEditor;


//a little TextButton that carries its own pageID and calls back to the editor
class HelpButton : public juce::TextButton {
public:


    explicit HelpButton(std::function<void()> onClickCallback) {
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



class PowerButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override {


        //get the full bounds of the button.
        auto bounds = button.getLocalBounds().toFloat();

        //fill the background with green when toggled on, dark grey otherwise.
        g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
        g.fillRect(bounds);

        //draw a white border around the button.
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1.0f);

        //define the icon area with minimal reduction so the circle is as big as possible.
        auto iconBounds = bounds.reduced(2);

        //calculate the maximum square that fits within iconBounds.
        float diameter = std::min(iconBounds.getWidth(), iconBounds.getHeight()) * 0.95f; // 95% of available space
        juce::Rectangle<float> ellipseBounds;
        ellipseBounds.setSize(diameter, diameter);
        ellipseBounds.setCentre(iconBounds.getCentre());

        //draw the full circle.
        g.setColour(juce::Colours::white);
        g.drawEllipse(ellipseBounds, 2.0f);


        float centerX = ellipseBounds.getCentreX();
        float lineTop = ellipseBounds.getY() - 1.0f;
        float lineBottom = ellipseBounds.getCentreY();

        //draw the thick line (mask) in the background color.
        g.setColour(button.getToggleState() ? juce::Colours::green : juce::Colours::darkgrey);
        float thickLineThickness = 6.0f;  // Increased thickness to fully cover the top stroke.
        g.drawLine(centerX, lineTop, centerX, lineBottom, thickLineThickness);

        //draw the thinner white line along the same path to create the power symbol's indicator.
        g.setColour(juce::Colours::white);
        float thinLineThickness = 2.0f;
        g.drawLine(centerX, lineTop, centerX, lineBottom, thinLineThickness);
    }
};
