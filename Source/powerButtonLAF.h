/*
  ==============================================================================

    powerButtonLAF.h
    Created: 21 Apr 2025 11:52:35pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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
