/*
  ==============================================================================

    CustomLookAndFeel.h
    Created: 23 May 2025 10:26:03pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UIDrawer.h"
#include "BinaryData.h"
#include "AnimatedKnob.h"

using namespace juce;

class CustomLookAndFeel : public LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override;

    //============================== style overrides ============================= 
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle,float rotaryEndAngle,
        juce::Slider& slider) override;

    void drawToggleButton(Graphics& g, ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool isButtonDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

    void drawPopupMenuBackground(Graphics& g, int width, int height) override;

    void CustomLookAndFeel::getIdealPopupMenuItemSize(const String& text,
        bool isSeparator,
        int standardMenuItemHeight,
        int& idealWidth,
        int& idealHeight);

    void CustomLookAndFeel::drawPopupMenuItem(Graphics& g,
        const Rectangle<int>& area,
        bool isSeparator,
        bool isActive,
        bool isHighlighted,
        bool isTicked,
        bool hasSubMenu,
        const String& text,
        const String& shortcutKeyText,
        const Drawable* icon,
		const Colour* textColour)override;

    void drawScrollbar(Graphics& g,
        ScrollBar& scrollbar,
        int x, int y, int width, int height,
        bool isScrollbarVertical,
        int thumbStartPosition,
        int thumbSize,
        bool isMouseOver,
        bool /*isMouseDown*/) override;

    //================================== one liners ==========================
    Font getComboBoxFont(ComboBox& box) override {
        return UIDrawer::getFontInterRegular().withHeight(20.0f * scale);
    }

    Font getPopupMenuFont() override {
        return UIDrawer::getFontInterRegular().withHeight(20.0f * scale);
    }

    int CustomLookAndFeel::getAlertWindowButtonHeight() override {
        return roundToInt(40 * scale);
    }

    //================================== helpers ============================= 
    void drawButtonBackground(Graphics& g, Button& button,
        const Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown) override;

    void drawAlertBox(Graphics& g, AlertWindow& alert, const Rectangle<int>& textArea,
        TextLayout& textLayout) override;

    void drawLobedDial(Graphics& g, Rectangle<float> bounds, float morph, float scalar,
        float spinAngleRadians);

private:

	float scale = 1.0f; //update this when we can tell the scale, and then use this when we can't

    //knob svgs
    std::unique_ptr<Drawable> knobLayerStatic;
    std::unique_ptr<Drawable> knoblayerInnerHighlights;

    //other svg icons
    std::unique_ptr<Drawable> helpIcon;
    std::unique_ptr<Drawable> powerIcon;
    std::unique_ptr<Drawable> saveIcon;
	std::unique_ptr<Drawable> deleteIcon;
    std::unique_ptr<Drawable> leftIcon;
    std::unique_ptr<Drawable> rightIcon;
    std::unique_ptr<Drawable> previewIcon;
    std::unique_ptr<Drawable> minimizeIcon;
};