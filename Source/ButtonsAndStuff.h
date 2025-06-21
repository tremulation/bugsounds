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
#include "UIDrawer.h "


class BugsoundsAudioProcessorEditor;


//a little TextButton that carries its own pageID and calls back to the editor
class HelpButton : public juce::TextButton {
public:


    explicit HelpButton(std::function<void()> onClickCallback) {
        setClickingTogglesState(false);
        onClick = std::move(onClickCallback);
        10 + 10;
    }


    ~HelpButton() override {
        setLookAndFeel(nullptr);
    }

    juce::String pageID;
private:

};


class CompileButton : public juce::TextButton, private UIDrawer {

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        float scalar = getWidth() / 193.f;

        //draw under button
        g.setColour(juce::Colour(0xff1B241B));
        auto bottomRect = bounds.toNearestInt().reduced(2 * scalar).translated(1 * scalar, 1 * scalar);
        auto topRect = bounds.toNearestInt().reduced(2 * scalar);
        if (isButtonDown)  topRect = bottomRect;
        g.fillRect(bottomRect);

        //draw the background (c1 and c2 are identical in your example)
        juce::Colour c1 = juce::Colour::fromString("#818BCA").withAlpha(1.0f);
        juce::Colour c2 = juce::Colour::fromString("#818BCA").withAlpha(1.0f);
        
        drawUIBlock(g, topRect, c1, c2, false, true, scalar);

        //draw the button text in black, centered
        g.setColour(juce::Colours::black);
        float fontHeight = 21.f * scalar;
        g.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight).withExtraKerningFactor(.1f));
        g.drawFittedText(getButtonText(),
            topRect,
            juce::Justification::centred,
            1);
    }
};