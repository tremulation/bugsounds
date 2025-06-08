/*
  ==============================================================================

    LevelMeter.cpp
    Created: 2 Jun 2025 6:54:39pm
    Author:  Taro

  ==============================================================================
*/

#include "LevelMeter.h"
#include "PluginProcessor.h"

LevelMeter::LevelMeter(BugsoundsAudioProcessor& p){
    //textReadout.setText("0.0 dB", juce::NotificationType::dontSendNotification);
    //textReadout.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
    //textReadout.setJustificationType(juce::Justification::centred);
    //textReadout.setFont(UIDrawer::getFontInterRegular().withHeight(15.f));
    //addAndMakeVisible(textReadout);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.getAPVTS(), 
        "Click Volume",
        slider);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);
}

void LevelMeter::paint(juce::Graphics& g) {
    float scalar = getHeight() / 39.f;
    auto  bounds = getLocalBounds();
    Colour c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bounds, c1, c2, false, true, scalar);

    auto middleArea = bounds
        .reduced(5.f * scalar)
        .withTrimmedBottom(8.f * scalar)
        .withTrimmedTop(8.f * scalar)
        .withTrimmedLeft(10.f * scalar)   
        .withTrimmedRight(10.f * scalar);
    
    float barHeight = (middleArea.getHeight() / 2.f);

    //dropshadow
    g.setColour(Colours::black.withAlpha(.6f));
    g.fillRoundedRectangle(middleArea.toFloat().translated(0.f, -1.f), 1.f * scalar);

    //anotha one
    g.setColour(Colours::black.withAlpha(.4f));
    g.fillRoundedRectangle(middleArea.toFloat().translated(0.f, 2.f), 4.f * scalar);

    //background black rectangle background
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(middleArea.toFloat(), 2.f * scalar);
    drawSingleChannelMeter(
        g, 
        middleArea.toFloat().removeFromTop(barHeight + 1.f * scalar).reduced(1.f * scalar), 
        leftLevel, scalar);
    drawSingleChannelMeter(
        g, 
        middleArea.toFloat().removeFromBottom(barHeight + 1.f * scalar).reduced(1.f * scalar), 
        rightLevel, scalar);

    //black line dividing the two channels
    Rectangle<float> dividingLineRect = { 
        (float)middleArea.getX(), 
        (float)middleArea.getY() + barHeight, 
        (float)middleArea.getWidth(),
        1.f * scalar 
    };
    g.setColour(juce::Colours::black);
    g.fillRect(dividingLineRect);
}

void LevelMeter::resized() {
    float scalar = getHeight() / 39.f;
    auto sliderBounds = getLocalBounds()
        .withTrimmedTop(5.f)
        .withTrimmedBottom(5.f)
        .withTrimmedLeft(15.f * scalar)
        .withTrimmedRight(15.f * scalar);
    slider.setBounds(sliderBounds);
}


void LevelMeter::setLevel(const float leftVal, const float rightVal) {
    leftLevel = leftVal;
    rightLevel = rightVal;
}

void LevelMeter::drawSingleChannelMeter(juce::Graphics& g, Rectangle<float> bounds, float levelDecibels, float scalar) {
    //draw +0 dB area
    float zeroPos = juce::jmap(0.0f, -60.f, 6.f, 0.f, bounds.getWidth());
    if (zeroPos < bounds.getWidth()) {
        Rectangle<float> overArea(
            bounds.getX() + zeroPos,
            bounds.getY(),
            bounds.getWidth() - zeroPos,
            bounds.getHeight()
        );
        g.setColour(juce::Colour::fromString("#163359").withAlpha(1.f));
        g.fillRect(overArea);
    }

    //draw main gradient fill for level
    float length = juce::jmap(levelDecibels, -60.f, 6.f, 0.f, (float)bounds.getWidth());
    juce::Colour cStart = juce::Colour::fromString("#3e28bd").withAlpha(1.f);
    juce::Colour cEnd = juce::Colour::fromString("#81caa5").withAlpha(1.f);
    float yMid = bounds.getCentreY();
    juce::ColourGradient gradient(
        cStart,                      
        bounds.getX(), yMid,         
        cEnd,                        
        bounds.getRight(), yMid,     
        false                        
    );

    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds.toFloat().removeFromLeft(length), 2.f * scalar);
}
