/*
  ==============================================================================

    ChorusPositionReadout.h
    Created: 26 Apr 2025 8:32:53pm
    Author:  Taro

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "PluginProcessor.h"

class ChorusPositionReadout : public juce::Component, private juce::Timer {
public:


    ChorusPositionReadout(BugsoundsAudioProcessor& p)
        : processor(p) {
        startTimerHz(30);   
    }


    void paint(juce::Graphics& g) override {
        //background fill
        float scale = getWidth() / 68.f;
        auto bounds = getLocalBounds();
        g.setColour(juce::Colour(0xff1B241B));
        g.fillRect(bounds);

        //draw waves around playing voices
        for (auto& wave : waves) {
            g.setColour(juce::Colours::white.withAlpha(juce::jlimit<float>(0.0, 1.f, wave.alpha)));
            float d = wave.radius * scale;
            g.drawEllipse(wave.x - d * 2, wave.y - d * 2,
                          d * 4.f, d * 4.f,
                          2.0f * scale);
        }

        //draw listener position
        g.setColour(juce::Colours::white.withAlpha(1.0f));
        auto center = bounds.getCentre();
        g.fillEllipse(center.x - 5 * scale, center.y - 5 * scale, 5.f * scale, 5.f * scale);
        g.setColour(juce::Colours::skyblue);

        //draw voices
        for (auto& pos : currentPositions) {
            g.setColour(juce::Colours::lightblue);
            float distance = pos.distance;
            float angle = pos.angle;

            const float minDistance = 5.0f * scale;
            const float maxDistance = 15.0f * scale;
            float normalizedDistance = (distance * scale)/ (maxDistance - minDistance);


            float maxRadius = (bounds.getWidth() / 2.f) - 15.f * scale;
            float radius = normalizedDistance * maxRadius;

            float x = center.x + std::cos(angle) * radius;
            float y = center.y + std::sin(angle) * radius;

            g.fillEllipse(x - 3.f * scale, y - 3.f * scale, 3.f * scale, 3.f * scale);
        }

        //border last
        g.setColour(juce::Colour(0xff555555));
        g.drawRect(bounds, 2.f * scale);
    }


private:
    struct Wave
    {
        float x, y;
        float radius = 0.0f;
        float alpha = 1.0f;
        int flickerTimer = 100;
    };

    void timerCallback() override {
        currentPositions = processor.getChorusVoicePositions();

        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        float scale = getWidth() / 68.f;

        //spawn a ripple for each playing note
        if (++spawnCounter >= spawnIntervalTicks) {
            spawnCounter = 0;
            for (auto& pos : currentPositions) {
                if (pos.isPlaying) {
                    float normD = (pos.distance * scale) / (15.0f * scale  - 5.0f * scale);
                    float maxR = bounds.getWidth()/2 - 15.0f * scale;
                    float r0 = normD * maxR;
                    float x = center.x + std::cos(pos.angle) * r0 - 1.5f * scale;
                    float y = center.y + std::sin(pos.angle) * r0 - 1.5f * scale;

                    waves.push_back({ x, y, 0.0f, 2.0f * scale });
                }
            }
        }
       

        //advance/cull old ripples
        const float speed = 0.3f;    //1/2 pix per tick
        const float fadeAmount = 0.1f; //alpha drop
        
        for (int i = (int)waves.size() - 1; i >= 0; --i)
        {
            waves[i].radius += speed;
            waves[i].alpha -= fadeAmount;

            if (waves[i].alpha <= 0.0f)
                waves.erase(waves.begin() + i);
        }

        repaint();
    }

    int spawnCounter = 0;
    static constexpr int spawnIntervalTicks = 5;

    BugsoundsAudioProcessor& processor;
    std::vector<BugsoundsAudioProcessor::ChorusVoicePosition>  currentPositions;
    std::vector<Wave> waves;
};