/*
  ==============================================================================

    UIDrawer.h
    Created: 23 May 2025 9:54:45pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BinaryData.h"
using namespace juce;

class UIDrawer {
public:
    
    //draws the rectangular window that is the building block for all UI element classes
    //is used to draw both headers and bodies 
    //      c1 and c2 can be used to create a gradient. pass same color twice for no gradient
    //      drawDots draws a field of pop-art inspired dots inside the block
    //      deepbottom shadow sets the shadow at the bottom of the area to be larger or smaller
    void UIDrawer::drawUIBlock(Graphics& g,
        Rectangle<int> area,
        Colour c1, Colour c2,
        bool /*drawDots*/,
        bool deepBottomShadow,
        float scalar)
    {
        auto origArea = area;
        //draw background fill rect (fill with gradient or solid color depending on if c1 == c2)
        if (c1 == c2) {
            g.setColour(c1);
            g.fillRect(area);
        } else {
            ColourGradient grad = juce::ColourGradient::horizontal(c1, c2, area);
            g.setGradientFill(grad);
            g.fillRect(area);  // now uses that gradient brush
        }

        //draw side shadows
        g.setColour(Colour::fromString("#163359").withAlpha(.5f));
        g.fillRect(area.removeFromLeft(4.f * scalar));
        g.setColour(Colour::fromString("#70AAF5").withAlpha(.67f));
        g.fillRect(area.removeFromRight(4.f * scalar));

        //draw bottom shadow
        if (deepBottomShadow) {
            g.setColour(Colour::fromString("#163359").withAlpha(.5f));
            g.fillRect(area.removeFromBottom(4.f * scalar));
        }
        else {
            g.setColour(Colour::fromString("#163359").withAlpha(.8f));
            g.fillRect(area.removeFromBottom(2.f * scalar));
        }

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
        g.drawRect(origArea, (int) (scalar * 2.f));
    }


    static const Font getFontInterBlack() {
        static auto typeface = Typeface::createSystemTypefaceFor(
            BinData::Inter_24ptBlack_ttf, 
            BinData::Inter_24ptBlack_ttfSize);
        return Font(typeface);
    }

    static const Font getFontInterBold() {
        static auto typeface = Typeface::createSystemTypefaceFor(
            BinData::Inter_24ptBold_ttf,
            BinData::Inter_24ptBold_ttfSize);
        return Font(typeface);
    }

    static const Font getFontInterRegular() {
        static auto typeface = Typeface::createSystemTypefaceFor(
            BinData::Inter_24ptRegular_ttf,
            BinData::Inter_24ptRegular_ttfSize);
        return Font(typeface);
    }

    static const Font getFontCode() {
        static auto typeface = Typeface::createSystemTypefaceFor(
            BinData::AzeretMonoRegular_ttf,
            BinData::AzeretMonoRegular_ttfSize);
        return Font(typeface);
    }
};
