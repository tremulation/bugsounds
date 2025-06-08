/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 23 May 2025 10:27:03pm
    Author:  Taro

  ==============================================================================
*/

#include "CustomLookAndFeel.h"
#include "ButtonsAndStuff.h"
#include "UIDrawer.h"

//================================================================================================================


CustomLookAndFeel::CustomLookAndFeel() {
    knobLayerStatic   = Drawable::createFromImageData(BinData::samknob_2_static_svg, BinData::samknob_2_static_svgSize);
    knoblayerInnerHighlights = Drawable::createFromImageData(BinData::samknob_2_inner_lighting_svg, BinData::samknob_2_inner_lighting_svgSize);
    helpIcon  = Drawable::createFromImageData(BinData::help_svg, BinData::help_svgSize);
    powerIcon = Drawable::createFromImageData(BinData::power_svg, BinData::power_svgSize);
	saveIcon = Drawable::createFromImageData(BinData::save_svg, BinData::save_svgSize);
	deleteIcon = Drawable::createFromImageData(BinData::trash_svg, BinData::trash_svgSize);
	leftIcon = Drawable::createFromImageData(BinData::arrow_left_svg, BinData::arrow_left_svgSize);
	rightIcon = Drawable::createFromImageData(BinData::arrow_right_svg, BinData::arrow_right_svgSize);
    previewIcon = Drawable::createFromImageData(BinData::preview_svg, BinData::preview_svgSize);
    minimizeIcon = Drawable::createFromImageData(BinData::minimize_svg, BinData::minimize_svgSize);

    setDefaultSansSerifTypeface(UIDrawer::getFontInterRegular().getTypefacePtr());
    setColour(ComboBox::textColourId,    Colour::fromString("#000000").withAlpha(1.0f));
    setColour(PopupMenu::textColourId,   Colour::fromString("#000000").withAlpha(1.0f));
	setColour(AlertWindow::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
}

CustomLookAndFeel::~CustomLookAndFeel() {

}


//================================================================================================================


void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float rotaryStartAngle,
    float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    float scalar = bounds.getWidth() / 60.0f;    
    auto centre = bounds.getCentre();

    float startAngleOffset = 0.10f;  
    float endAngleOffset = 0.10f;  

    float arcYOffset = 5.0f * scalar;
    float arcStartAngle = rotaryStartAngle + startAngleOffset;
    float arcEndAngle = rotaryEndAngle - endAngleOffset;
    float toAngle = arcStartAngle + sliderPosProportional * (arcEndAngle - arcStartAngle);
    auto  arcCentre = centre + juce::Point<float>(0.0f, arcYOffset);

    // draw inner fill arc
    if (slider.isEnabled())
    {
        float innerRadius = 0.0f * scalar;
        float thickness = 21.0f * scalar;
        float midRadius = innerRadius + thickness * 0.5f;

        juce::Path valueArc;
        valueArc.addCentredArc(arcCentre.x, arcCentre.y,
            midRadius, midRadius,
            0.0f,
            arcStartAngle,
            toAngle,
            true);

        auto gradient = juce::ColourGradient(
            juce::Colour::fromString("#FF89A3D2"), // start color
            arcCentre.x + std::cos(arcStartAngle) * midRadius,
            arcCentre.y + std::sin(arcStartAngle) * midRadius,
            juce::Colour::fromString("#FF77ADB3"), // end color
            arcCentre.x + std::cos(toAngle) * midRadius,
            arcCentre.y + std::sin(toAngle) * midRadius,
            false
        );

        g.setGradientFill(gradient);
        g.strokePath(valueArc,
            juce::PathStrokeType(thickness,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::butt));
    }

    // draw static SVG underneath
    if (knobLayerStatic != nullptr)
        knobLayerStatic->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);

    //check for morph progress for lobe animation
    float morphProgress = 0.0f;
    if (auto* ak = dynamic_cast<AnimatedKnobSlider*>(&slider))
        morphProgress = ak->morph.getCurrentValue();

    float knobSize = 30.f * scalar + (30.f * scalar) * .1f * morphProgress;
    drawLobedDial(g, bounds.withSizeKeepingCentre(knobSize, knobSize).translated(0.f, 5.f * scalar), morphProgress, scalar, toAngle);

    // draw rotating indicator notch
    {
        float innerRadius = 10.0f * scalar;
        float notchLength = innerRadius * 0.6f;
        float notchThickness = 3.0f * scalar;
        auto  angleDir = juce::Point<float>(
            std::cos(toAngle - juce::MathConstants<float>::halfPi),
            std::sin(toAngle - juce::MathConstants<float>::halfPi));
        auto  startPt = arcCentre + angleDir * (innerRadius - notchLength);
        auto  endPt = arcCentre + angleDir * innerRadius;
        juce::Path notch;
        notch.startNewSubPath(startPt);
        notch.lineTo(endPt);

        g.setColour(juce::Colours::white);
        g.strokePath(notch,
            juce::PathStrokeType(notchThickness,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::rounded));
    }
}


//================================================================================================================


void CustomLookAndFeel::drawToggleButton(Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool isButtonDown) {
    drawButtonBackground(g, button, findColour(button.getToggleState() ? button.textColourId : button.textColourId),
        shouldDrawButtonAsHighlighted, isButtonDown);
}


//================================================================================================================


void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, 
                                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) {
    //----------------------- scaling. ----------------------- 
	float scale = height / 40.f; //40px default height
    this->scale = scale;
	float padY = 8.f * scale; //8px padding on top and bottom

    //------------------- draw background. ------------------- 
    Rectangle<float> fullArea{ 0.0f, 0.0f,  (float)width, (float)height };
    Rectangle<float> paddedArea = fullArea.withTrimmedTop(padY).withTrimmedBottom(padY);
    g.setColour(Colour::fromString("#D9D9D9").withAlpha(1.0f));
	g.fillRect(paddedArea);

	//--------------- draw border outline. ------------------
    g.setColour(Colour::fromString("#163359").withAlpha(1.0f));
    g.drawRect(paddedArea.toNearestInt(), (int)scale);

	//--------------- draw shadow under border. --------------
	float shadowHeight = 2.0f * scale;
    Rectangle<float> shadowRect(paddedArea.getX(), paddedArea.getBottom(), paddedArea.getWidth(), shadowHeight);
    g.setColour(Colours::black.withAlpha(0.3f));
    g.fillRect(shadowRect);

    //---------------draw the combox arrow.---------------
    auto arrowArea = fullArea.removeFromRight(height * 0.6f);
    float triW = 12.0f * scale;
    float triH = 6.0f * scale;
    float cx = arrowArea.getCentreX();
    float cy = arrowArea.getCentreY();

    // Define the V-shaped arrow path
    Path arrow;
    arrow.startNewSubPath(cx - triW * 0.5f, cy - triH * 0.5f);
    arrow.lineTo(cx, cy + triH * 0.5f);
    arrow.lineTo(cx + triW * 0.5f, cy - triH * 0.5f);

    // Stroke with rounded caps and joints
    float lineThickness = 1.5f * scale;
	PathStrokeType strokeType(
		lineThickness,
		PathStrokeType::curved,
		PathStrokeType::rounded);

    g.setColour(Colour::fromString("#163359").withAlpha(1.f));
    g.strokePath(arrow, strokeType);

}



//================================================================================================================


void CustomLookAndFeel::drawPopupMenuBackground(Graphics& g, int width, int height) {
    UIDrawer drawer;
    Rectangle<int> area = {0, 0, width, height};
    Colour c = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawer.drawUIBlock(g, area, c, c, true, false, scale);
}


//================================================================================================================


void CustomLookAndFeel::getIdealPopupMenuItemSize(const String& text,
    bool isSeparator,
    int standardMenuItemHeight,
    int& idealWidth,
    int& idealHeight)
{
    if (isSeparator)
    {
        //thinner separator
        idealWidth = 50;
        idealHeight = standardMenuItemHeight > 0
            ? roundToInt(standardMenuItemHeight * 0.1f * scale)
            : roundToInt(10.0f * scale);
    } else {
        // popup-menu font (already scaled)
        auto font = getPopupMenuFont();

        //measure text width
        idealWidth = GlyphArrangement::getStringWidthInt(font, text) + roundToInt((font.getHeight() + 8.0f * scale) * 2.0f);

        //font height + 8px total vertical padding (4px top + 4px bottom)
        float paddingV = 8.0f * scale;
        idealHeight = roundToInt(font.getHeight() + paddingV);

        //if JUCE passed in a standard height bigger than ours, clamp down
        if (standardMenuItemHeight > 0 && idealHeight > standardMenuItemHeight)
            idealHeight = standardMenuItemHeight;
    }
}


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
    const Colour* textColourToUse)
{
    // 1) get the L&F’s current scale factor (set elsewhere)
    float scale = this->scale;

    if (isSeparator)
    {
        auto r = area
            .reduced(roundToInt(5.0f * scale), 0)
            .removeFromTop(1)
            .withY(area.getY() + roundToInt((area.getHeight() * 0.5f) - 0.5f));

        g.setColour(findColour(PopupMenu::textColourId).withAlpha(0.3f));
        g.fillRect(r);
        return;
    }

    // 2) determine text colour
    Colour baseTextColour = textColourToUse != nullptr
        ? *textColourToUse
        : findColour(PopupMenu::textColourId);

    auto r = area.reduced(1);

    // 3) background highlight
    if (isHighlighted && isActive)
    {
        g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
        g.fillRect(r);
        g.setColour(findColour(PopupMenu::highlightedTextColourId));
    }
    else
    {
        g.setColour(baseTextColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
    }

    // 4) horizontal padding
    int pad = jmin(roundToInt(5.0f * scale), area.getWidth() / 20);
    r.reduce(pad, 0);

    // 5) font setup
    auto font = getPopupMenuFont();
    float maxFontH = (float)r.getHeight() / 1.3f;
    if (font.getHeight() > maxFontH)
        font.setHeight(maxFontH);
    g.setFont(font);

    // 6) icon or tick
    int iconSize = roundToInt(maxFontH);
    auto iconArea = r.removeFromLeft(iconSize).toFloat();

    if (icon != nullptr)
    {
        icon->drawWithin(g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        r.removeFromLeft(roundToInt(maxFontH * 0.5f));
    }
    else if (isTicked)
    {
        auto tick = getTickShape(1.0f);
        g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
    }

    // 7) submenu arrow
    if (hasSubMenu)
    {
        float arrowH = 0.6f * font.getAscent();
        auto arrowX = (float)r.removeFromRight(roundToInt(arrowH)).getX();
        float halfY = (float)area.getCentreY();

        Path path;
        path.startNewSubPath(arrowX, halfY - arrowH * 0.5f);
        path.lineTo(arrowX + arrowH * 0.6f, halfY);
        path.lineTo(arrowX, halfY + arrowH * 0.5f);

        g.strokePath(path, PathStrokeType(2.0f * scale));
    }

    // 8) draw main text
    r.removeFromRight(roundToInt(3.0f * scale));
    g.drawFittedText(text, r, Justification::centredLeft, 1);

    // 9) draw shortcut text
    if (shortcutKeyText.isNotEmpty())
    {
        auto f2 = font;
        f2.setHeight(f2.getHeight() * 0.75f);
        f2.setHorizontalScale(0.95f);
        g.setFont(f2);
        g.drawText(shortcutKeyText, r, Justification::centredRight, true);
    }
}


//================================================================================================================

//TODO maybe change this for vertical. works fine for horizontal
void CustomLookAndFeel::drawScrollbar(Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool){
    const Colour thumbFill = scrollbar.findColour(ScrollBar::thumbColourId).withAlpha(0.8f);
    const Colour finalThumb = isMouseOver ? thumbFill.brighter(0.2f) : thumbFill;
    const Colour outlineColour = Colour(0xff163359);

    //compute thumb rectangle depending on orientation
    Rectangle<float> thumb(0.0f, 0.0f, 0.0f, 0.0f);

    if (isScrollbarVertical){
        //for vertical: thumbStartPosition is y offset, thumbSize is height
        float thumbX = float(x + (width - thumbSize) / 2);
        float thumbY = float(y + thumbStartPosition);
        thumb = Rectangle<float>(thumbX,thumbY,float(thumbSize), float(thumbSize));
    } else {
        //for horizontal: thumbStartPosition is x offset, thumbSize is width
        float thumbX = float(x + thumbStartPosition);
        float thumbY = float(y + (height - thumbSize) / 2);
        thumb = Rectangle<float>(thumbX,thumbY,float(thumbSize), float(thumbSize));
    }

    // draw thumb fill
    g.setColour(finalThumb);
    g.fillRect(thumb);

    // draw thumb outline
    g.setColour(outlineColour);
    g.drawRect(thumb, 1.0f);
}



//================================================================================================================


void CustomLookAndFeel::drawButtonBackground(Graphics& g,
    Button& button,
    const Colour& /*backgroundColour*/,
    bool /*isMouseOverButton*/,
    bool isButtonDown)
{
    //------------------- Set up sizes and scaling.  ------------------- 
    //metrics: all buttons start at 35x35px squares. header buttons are slightly larger
    float scale = this->scale;
    float padding = 7.0f * scale;   //5 + 2 for the offset

    //---------------------- bottom rectangle.  ------------------- 
    float offset = 2.0f * scale;
	Rectangle<float> bottomRect = button.getLocalBounds().toFloat().reduced(padding).translated(offset, offset);
    Colour baseColour = Colour::fromString("#1B241B").withAlpha(1.0f);
    g.setColour(baseColour);
    g.fillRect(bottomRect);

    //----------------------- top rectangle.  --------------------- 
	Rectangle<float> topRect = button.getLocalBounds().toFloat().reduced(padding);
    if (isButtonDown) { //align top and bottom rectangles.
        topRect = bottomRect;
    }
    g.setColour(baseColour);
    g.fillRect(topRect);

    //------------------------- dropshadow.  ---------------------- 
    if (!isButtonDown) {
        float shadowHeight = 4.0f * scale;
        Rectangle<float> shadowRect(bottomRect.getX(), bottomRect.getBottom() - 2.0f * scale, bottomRect.getWidth(), shadowHeight);
        g.setColour(Colours::black.withAlpha(0.3f));
        g.fillRect(shadowRect);
    }

    //----------------------------- icon.  ------------------------ 
    Drawable* icon = nullptr;
    if (button.getName().contains("Power")) {
        icon = powerIcon.get();
    } else if (button.getName().contains("Help")) {
        icon = helpIcon.get();
    } else if (button.getName().contains("Delete")) {
		icon = deleteIcon.get();
    } else if (button.getName().contains("Save")) {
		icon = saveIcon.get();
    } else if (button.getName().contains("<")) {
		icon = leftIcon.get();
    } else if (button.getName().contains(">")) {
		icon = rightIcon.get();
    } else if (button.getName().contains("Preview")) {
        icon = previewIcon.get();
    } else if (button.getName().contains("Close")) {
        icon = minimizeIcon.get();
    }

    if (icon != nullptr) {
        auto iconArea = topRect.reduced(padding * 0.5f);
        icon->drawWithin(g,
            iconArea,
            RectanglePlacement::centred,
            1.0f);
    }
}



//================================================================================================================


void CustomLookAndFeel::drawAlertBox(Graphics& g, AlertWindow& alert, const Rectangle<int>& textArea, TextLayout& textLayout) {
    UIDrawer drawer;

    auto width = alert.getWidth();
    auto height = alert.getHeight();
    Rectangle<int> area = { 0, 0, width, height };

    // Colors
    Colour headerColour = Colour::fromString("#163359").withAlpha(1.0f);
    Colour bodyColour = Colour::fromString("#D9D9D9").withAlpha(1.0f);

    // Draw header and body using drawUIBlock
    drawer.drawUIBlock(g, area, bodyColour, bodyColour, true, false, scale);

    // Draw title text (assume first line is title)
    auto titleFont = UIDrawer::getFontInterBold().withHeight(16.0f * scale);
    g.setFont(titleFont);
    g.setColour(Colours::white);

    // Draw main text (message content)
    auto messageFont = UIDrawer::getFontInterRegular().withHeight(14.0f * scale);
    g.setFont(messageFont);
    g.setColour(Colours::black.withAlpha(1.0f));

    textLayout.draw(g, textArea.toFloat());
}


//================================================================================================================


void CustomLookAndFeel::drawLobedDial(Graphics& g,
    Rectangle<float> bounds,
    float morph,
    float scalar,
    float spinAngleRadians)
{
    morph = jlimit(0.0f, 1.0f, morph);
    auto center = bounds.getCentre();
    float fullRadius = jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    Colour fillColor(0xFF222A2E);
    Colour borderColor(0xFF181C1F);
    Colour shadowColor(0x851A2024);
    Colour highlightColor(0x49FFFFFF);

    //build outline and save notches
    Path knobShape;
    knobShape.setUsingNonZeroWinding(false);

    int stepCount = 100;
    int notchCount = 5.f;   //number of knob cutouts
    int stepCounter = 0;
    const int sliceStepSize = stepCount / (notchCount * 2);
    float angleOffset = (MathConstants<float>::pi) * 1.1; //to line up the indicator notch with the indentations correctly
    float maxIndentDepth = fullRadius * 0.15f;
    float cutoutDepth = maxIndentDepth * morph;

    struct Notches { float centerAngle; Path path; };
    std::vector<Notches> notchArray;
    bool inNotch = false;
    float notchStartAngle = 0.0f;
    float notchEndAngle = 0.0f;
    Point<float> notchStartPoint = { 0.0f, 0.0f };
    juce::Path currentNotch;

    float notchPitch = MathConstants<float>::twoPi / notchCount;
    float halfNotchPitch = 0.5f * notchPitch;

    //calculate the shape o the knob
    for (int i = 0; i <= stepCount; ++i)
    {
        
        float angle = (MathConstants<float>::twoPi * i / stepCount);
        float detectionAngle = angle + halfNotchPitch;

        float cosv = std::cos(notchCount * angle);
        // zero out positive portions so only negative swings indent
        float clamped = (cosv < 0.0f ? cosv : 0.0f);
        float r = fullRadius + clamped * cutoutDepth;
        float x = center.x + r * std::cos(angle);
        float y = center.y + r * std::sin(angle);

        if (i == 0)   knobShape.startNewSubPath(x, y);
        else          knobShape.lineTo(x, y);

        //------------------notch stuff-------------------
        
NotchStart:
        if (!inNotch) {
            inNotch = true;
            notchStartAngle = angle;
            currentNotch.clear();
            currentNotch.startNewSubPath(center.x, center.y);
            currentNotch.lineTo(x, y);
            stepCounter++;
        }
        else if (inNotch && stepCounter < sliceStepSize) {
            currentNotch.lineTo(x, y);
            stepCounter++;
        }
        else if ((inNotch && stepCounter == sliceStepSize)) {
            
            float notchEndAngle = angle;
            currentNotch.lineTo(x, y);
            currentNotch.lineTo(center.x, center.y);
            currentNotch.closeSubPath();
            float centerAngle = 0.5f * (notchStartAngle + notchEndAngle);
            notchArray.push_back({ centerAngle, currentNotch });
            stepCounter = 0;
            inNotch = false;
            goto NotchStart;
        }
        //--------------------------------------------------
    }
    knobShape.closeSubPath();

    auto rotationTransform = AffineTransform::rotation(spinAngleRadians + angleOffset, center.x, center.y);
    Path rotatedKnobShape(knobShape);
    rotatedKnobShape.applyTransform(rotationTransform);


    //Path shadowShape(rotatedKnobShape);
    //shadowShape.applyTransform(AffineTransform::translation(-3.0f * scalar, 3.0f * scalar));
    //g.setColour(shadowColor);
    //g.fillPath(shadowShape);

    //draw inner fill
    g.setColour(fillColor);
    g.fillPath(rotatedKnobShape);

    constexpr float lightDirectionAngle = -1.f * MathConstants<float>::pi / 4.f; // 45° light position
    float rotationAngle = spinAngleRadians + angleOffset; // Extract knob's current rotation


    //shade each notch individually
    for (const auto& n : notchArray) {
        Path notchPath = n.path;
        notchPath.applyTransform(rotationTransform);

        float worldCenterAngle = n.centerAngle + rotationAngle;

        //calculate shading factor using dot product between light vector and notch normal
        float angleDiff = lightDirectionAngle - worldCenterAngle;
        float dotProduct = std::cos(angleDiff);
        float lightingFactor = jlimit(0.2f, 1.0f, (dotProduct + 1.0f) * 0.5f); // Map to [0.2, 1.0]

        Colour baseNotchColour(0xFFFFFFFF);
        Colour shadedColour = baseNotchColour
            .withMultipliedBrightness(lightingFactor)
            .withAlpha(0.29f);

        g.setColour(shadedColour);
        g.fillPath(notchPath);
    }

    //draw minor knob ellipse
    float innerR = fullRadius - maxIndentDepth - 1.5f * scalar;
    Path minorKnob;
    minorKnob.addEllipse(center.x - innerR,
        center.y - innerR,
        innerR * 2.0f,
        innerR * 2.0f);
    g.setColour(fillColor);
    g.fillPath(minorKnob);
    g.setColour(borderColor);
    g.strokePath(minorKnob, PathStrokeType(2.0f * scalar));

    //draw the lighting svgs onto the inner knob
    if (knoblayerInnerHighlights != nullptr)
        knoblayerInnerHighlights->drawWithin(g, bounds.reduced(4.f * scalar), juce::RectanglePlacement::centred, 1.0f);

    //draw outer shape ellipse stroke
    g.strokePath(rotatedKnobShape, PathStrokeType(2.0f * scalar));
}


//================================================================================================================
