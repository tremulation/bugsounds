/*
  ==============================================================================

    AnimatedKnob.h
    Created: 29 May 2025 6:37:15pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UIDrawer.h"

class AnimatedKnobSlider : public juce::Slider, private juce::Timer {
public:
    AnimatedKnobSlider(juce::Label& associatedLabel) : 
        juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox),
        labelPointer(associatedLabel) 
    {
        morph.reset(60.0, 0.1); //60 Hz update rate, 0.1 second1 ramp
        morph.setTargetValue(0.0f);
    }


    //------------------------ hover effects. ------------------------------
    //hover enter - transition to lobed shape
    void mouseEnter(const juce::MouseEvent& e) override {
        morph.setTargetValue(1.0f);
        startTimerHz(60);
        juce::Slider::mouseEnter(e);
    }

    //hover exit - return to circular shape
    void mouseExit(const juce::MouseEvent& e) override {
        morph.setTargetValue(0.0f);
        startTimerHz(60);
        juce::Slider::mouseExit(e);
    }


    //--------------------- onclick effects. ----------------------------
    void mouseDown(const juce::MouseEvent& e) override {
        isEditing = true;
        startTimerHz(60);
        juce::Slider::mouseDown(e);
    }


    //show final value briefly before reverting
    void mouseUp(const juce::MouseEvent& e) override {
        labelPointer.setText(getValueText(), juce::dontSendNotification);
        revertTimer = 30;  
        isEditing = false;
        startTimerHz(60);
        juce::Slider::mouseUp(e);
    }

    void updateOriginalLabelText() {
        originalLabelText = labelPointer.getText();
    }

    void setValueStyle(const String& valueLabel, int decimals, bool isPercent = false) {
        unitLabel = valueLabel;
        decimalPlaces = decimals;
        this->isPercent = isPercent;
    }


private:
    void timerCallback() override {
        //handle knob morph
        if (morph.isSmoothing()) {
            morph.getNextValue();
            repaint();
        }

        //update value display while editing
        if (isEditing) {
            labelPointer.setText(getValueText(), juce::dontSendNotification);
        }

        //handle label revert delay after editing is finished
        if (revertTimer > 0) {
            revertTimer--;
            if (revertTimer == 0) {
                labelPointer.setText(originalLabelText, juce::dontSendNotification);
            }
        }

        //stop timer when all animations are complete
        const bool animationActive = morph.isSmoothing();
        const bool editingActive = isEditing;
        const bool revertActive = revertTimer > 0;

        if (!animationActive && !editingActive && !revertActive) stopTimer();
    }


    //customize value formatting here
    juce::String getValueText() const {
        if (isPercent) {
            return (String((getValue() * 100.f), decimalPlaces) + unitLabel);
        }
        else {
            return (String(getValue(), decimalPlaces) + unitLabel);
        }
        
    }

    juce::Label& labelPointer;
    juce::String originalLabelText = "";
    juce::String unitLabel = "";
    int decimalPlaces = 2;
    int revertTimer = 0;
    bool isPercent = false;
    bool isEditing = false;
public:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> morph;
};