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
        startTimerHz(30);   //30 updates per sec
    }


    void paint(juce::Graphics& g) override {
        //background fill
        auto bounds = getLocalBounds().toFloat();
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::white);

        

        //draw waves around playing voices
        for (auto& wave : waves) {
            g.setColour(juce::Colours::white.withAlpha(wave.alpha));
            float d = wave.radius;
            g.drawEllipse(wave.x - d, wave.y - d, d * 2.0f, d * 2.0f, 2.0f);
        }

        //draw listener position
        g.setColour(juce::Colours::white.withAlpha(1.0f));
        auto center = bounds.getCentre();
        g.fillEllipse(center.x - 5, center.y - 5, 10, 10);
        g.setColour(juce::Colours::skyblue);

        //draw voices
        for (auto& pos : currentPositions) {
            g.setColour(juce::Colours::lightblue);
            float distance = pos.distance;
            float angle = pos.angle;

            const float minDistance = 5.0f;
            const float maxDistance = 15.0f; 
            float normalizedDistance = distance / (maxDistance - minDistance);

            float maxRadius = bounds.getWidth() / 2 - 15;
            float radius = normalizedDistance * maxRadius;

            float x = center.x + std::cos(angle) * radius;
            float y = center.y + std::sin(angle) * radius;

            g.fillEllipse(x - 3, y - 3, 6, 6);
        }
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

        //spawn a ripple for each playing note
        if (++spawnCounter >= spawnIntervalTicks) {
            spawnCounter = 0;
            for (auto& pos : currentPositions) {
                if (pos.isPlaying) {
                    float normD = (pos.distance) / (15.0f - 5.0f);
                    float maxR = bounds.getWidth()/2 - 15.0f;
                    float r0 = normD * maxR;
                    float x = center.x + std::cos(pos.angle) * r0;
                    float y = center.y + std::sin(pos.angle) * r0;

                    waves.push_back({ x, y, 0.0f, 1.0f });
                }
            }
        }
       

        //advance/cull old ripples
        const float speed = 0.5f;    //1/2 pix per tick
        const float fadeAmount = 0.05f; //alpha drop
        
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