/*
  ==============================================================================

    AnimatedLevelSlider.h
    Created: 2 Jun 2025 10:06:20pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


class AnimatedSlider : public juce::Slider, private juce::Timer {
public:
    AnimatedSlider() {
        morph.reset(60.f, 0.1); //24 Hz update rate, 0.1 second1 ramp
        morph.setTargetValue(0.0f);
        setLookAndFeel(&levelSliderLNF);

        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    ~AnimatedSlider() override {
        setLookAndFeel(nullptr);
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


private:
    struct LevelSliderLookandfeel : juce::LookAndFeel_V4
    {
        LevelSliderLookandfeel() {
            // configure colours, thumb, track, etc., here if desired
            setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
            setColour(juce::Slider::trackColourId, juce::Colours::grey);
        }

        //TODO add scaling
        void drawLinearSlider(juce::Graphics& g,
            int x, int y, int width, int height, float sliderPos,
            float /*minSliderPos*/, float /*maxSliderPos*/,
            const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            float scalar = height / 31.f;

            //get morph for animating
            auto* animSlider = dynamic_cast<AnimatedSlider*>(&slider);
            float m = animSlider ? animSlider->morph.getCurrentValue() : 0.0f;

            float thumbWidth = 12.0f * scalar;
            float thumbHeight = static_cast<float>(height) - 4.f * scalar;  //leave space for shadow
            float halfW = thumbWidth / 2.f;
            const float cornerRadius = 2.0f * scalar;

            //center thumb at sliderPos, full height vertically
            float posX = sliderPos - halfW;
            float posY = static_cast<float>(y) + (height - thumbHeight) / 2.f;
            juce::Rectangle<float> r(posX, posY, thumbWidth, thumbHeight);

            //compute how much to bow the vertical sides (max 20% of half-width)
            float maxIndent = halfW * 0.4f; // 20% of full thumbWidth
            float bowAmt = maxIndent * m;

            const float topInner = r.getY() + cornerRadius;
            const float bottomInner = r.getBottom() - cornerRadius;
            const float innerHeight = bottomInner - topInner;
            const float xl = r.getX();
            const float xr = r.getRight();
            const float yt = r.getY();
            const float yb = r.getBottom();

            //------------------build thumb slider path-----------------
            juce::Path thumbPath;

            //start top left just past the corner
            thumbPath.startNewSubPath(xl + cornerRadius, yt);
            //line to top right before corner
            thumbPath.lineTo(xr - cornerRadius, yt);
            //round the top right corner
            thumbPath.quadraticTo(xr, yt, xr, yt + cornerRadius);
            //handle tactile animation curve
            const int steps = 20;
            for (int i = 0; i <= steps; ++i) {
                float t = i / static_cast<float>(steps);
                float y = topInner + t * innerHeight;
                float indent = bowAmt * std::sin(juce::MathConstants<float>::pi * t);
                thumbPath.lineTo(xr - indent, y);
            }
            //round the bottom right corner
            thumbPath.quadraticTo(xr, yb, xr - cornerRadius, yb);
            //line to bottom left
            thumbPath.lineTo(xl + cornerRadius, yb);
            //round the bottom left corner
            thumbPath.quadraticTo(xl, yb, xl, yb - cornerRadius);
            //curve up the top side
            for (int i = steps; i >= 0; --i) {
                float t = i / static_cast<float>(steps);
                float y = topInner + t * innerHeight;
                float indent = bowAmt * std::sin(juce::MathConstants<float>::pi * t);
                thumbPath.lineTo(xl + indent, y);
            }
            //round the top left corner
            thumbPath.quadraticTo(xl, yt, xl + cornerRadius, yt);
            thumbPath.closeSubPath();
            //----------------------------------------------------------

            //draw 2px dropshadow
            {
                auto shadowPath = thumbPath;
                juce::AffineTransform down(1.0f, 0.0f, 0.0f,
                                           0.0f,  1.0f, 2.0f);
                shadowPath.applyTransform(down);
                g.setColour(juce::Colours::black.withAlpha(0.4f));
                g.fillPath(shadowPath);
            }

            //thumb body black fill
            g.setColour(juce::Colour::fromString("#818BCA").withAlpha(1.f));
            g.fillPath(thumbPath);
            
            //3px center line w/ rounded ends
            {
                juce::Path centreLine;
                float centreX = r.getCentreX();

                //compute top/bottom 10% margins within the shrunken thumb
                float topMargin = r.getY() + r.getHeight() * 0.30f;
                float bottomMargin = r.getBottom() - r.getHeight() * 0.30f;

                centreLine.startNewSubPath(centreX, topMargin);
                centreLine.lineTo(centreX, bottomMargin);

                g.setColour(juce::Colours::white);
                g.strokePath(centreLine, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }


            //outline
            g.setColour(Colours::black);
            const auto outlineStroke = juce::PathStrokeType(1.0f * scalar, juce::PathStrokeType::JointStyle::curved, juce::PathStrokeType::EndCapStyle::rounded);
            g.strokePath(thumbPath, outlineStroke);
        }
    } levelSliderLNF;

    void timerCallback() override {
        if (morph.isSmoothing()) {
            morph.getNextValue();
            repaint();
        }

        if (!morph.isSmoothing()) {
            stopTimer();
        }
    }

public:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> morph;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedSlider)
};