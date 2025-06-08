/*
  ==============================================================================

    Crebits.h
    Created: 26 May 2025 11:53:49pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UIDrawer.h"


class Crebits : public juce::Component, public UIDrawer
{
public:
	Crebits() {
        //the scaling is set in the plugineditor
        setWantsKeyboardFocus(true);
        setMouseClickGrabsKeyboardFocus(true);

        const juce::String resourceName = "logoCredits_png";
        int dataSize = 0;
        const char* imageData = BinData::getNamedResource(resourceName.toRawUTF8(), dataSize);

        if (imageData != nullptr && dataSize > 0) {
            logoImage = juce::ImageCache::getFromMemory(imageData, dataSize);
        } else {
            juce::Logger::writeToLog("Logo resource not found: " + juce::String(resourceName));
        }
    }


    //modified drawUIBlock to do vertical gradient
    void paint(juce::Graphics& g) override {
		auto area = getLocalBounds().toFloat();
		area.removeFromTop(18.f * scalar); 
        area.removeFromRight(31.f * scalar);
        auto origArea = area;
        auto origArea2 = area;

        //draw background fill rect (fill with gradient or solid color depending on if c1 == c2)
		Colour c1 = Colour::fromString("#3E28BD").withAlpha(1.0f);
        Colour c2 = Colour::fromString("#81CAA5").withAlpha(1.0f);
        ColourGradient grad = juce::ColourGradient::vertical(c1, c2, area);
        g.setGradientFill(grad);
        g.fillRect(area);  // now uses that gradient brush

        //draw side shadows
        g.setColour(Colour::fromString("#163359").withAlpha(.5f));
        g.fillRect(area.removeFromLeft(2.f * scalar));
        g.setColour(Colour::fromString("#70AAF5").withAlpha(.67f));
        g.fillRect(area.removeFromRight(4.f * scalar));

        //draw bottom shadow
        g.setColour(Colour::fromString("#163359").withAlpha(.5f));
        g.fillRect(area.removeFromBottom(4.f * scalar));

        //draw top highlight
        g.setColour(Colour::fromString("#FFFFFF").withAlpha(.76f));
        //draw hightlight with original line thickness 1, starting offset 2 from the left side, and 2 from the top.
        g.fillRect(Rectangle<float>(
            origArea.getX() + 4.f * scalar,
            origArea.getY() + 3.f * scalar,
            origArea.getWidth() - 4.f * scalar,
            1.0f * scalar
        ));

        //draw border rect (inside)
        g.setColour(Colour::fromString("#163359").withAlpha(1.0f));
        float outlineThickness;
        g.drawRect(origArea, (int)scalar);

        //TEXT TIME
        g.setColour(juce::Colours::white.withAlpha(1.0f));
		float fontHeight = 40.0f * scalar; // scale the font height
        g.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight));
        auto titleArea = origArea.removeFromTop(50.f * scalar).withTrimmedTop(30.f * scalar).toNearestInt();
        g.drawFittedText("Bugsounds", titleArea, juce::Justification::centred, 1);

        //acknowledgements
		fontHeight = 20.f * scalar; 
        g.setFont(UIDrawer::getFontInterRegular().withHeight(17.f * scalar));
        auto creditsArea = origArea2.removeFromBottom(180.f * scalar).toNearestInt();
		creditsArea.removeFromLeft(70.f * scalar); 
		creditsArea.removeFromRight(70.f * scalar);
		g.drawFittedText("By Taro\n\nUI work & feedback: Heckware\n\nAdditional input: hbprinter, malum, and flower snekk", 
                          creditsArea, juce::Justification::left, 1);


        //draw logo in the top right corner, mirrored. 
        if (logoImage.isValid()) {
            // compute the target area
            float logoSize = 111.f * scalar;
            Rectangle<float> logoArea{getWidth() - logoSize, 0.0f, logoSize, logoSize };
            g.drawImage(logoImage, logoArea);
        }
    }


    void mouseDown(const juce::MouseEvent& e) override {
        grabKeyboardFocus(); 
        Component::mouseDown(e);
    }

    float scalar = 1.0f;

private:
    juce::Image logoImage;
};




//================================================================================
// for detecting when the credits have been clicked outside of
class ClickBlocker : public juce::Component
{
public:
    std::function<void()> onClickOutside;

    ClickBlocker()
    {
        setInterceptsMouseClicks(true, false);
        setAlpha(0.0f);         // fully transparent
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClickOutside)
            onClickOutside();
    }
};