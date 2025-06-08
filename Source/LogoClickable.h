/*
  ==============================================================================

    LogoClickable.h
    Created: 26 May 2025 10:58:03pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LogoClickable : public juce::ImageComponent
{
public:
    std::function<void()> onClick;

    void mouseUp(const juce::MouseEvent&) override {
        if (onClick) onClick();
    }
};