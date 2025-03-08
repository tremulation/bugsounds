/*
  ==============================================================================

    ClickPreviewer.h
    Created: 7 Mar 2025 5:07:59pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include "PipStructs.h"

class ClickPreviewer : public juce::AudioSource {

public:
    ClickPreviewer(juce::AudioProcessorValueTreeState& valueTreeState) : apvts(valueTreeState){

    }

    ~ClickPreviewer() = default;

    //audiosource stuff
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        currentSampleRate = sampleRate;
        activeSubClicks.clear();
        currentPipIndex = 0;
        samplesUntilNextSubClick = 0;
        previewActive = false;
        juce::Logger::writeToLog("in prepare to play");
    }

    void releaseResources() override {
        activeSubClicks.clear();
    }


    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        auto* buffer = bufferToFill.buffer;
        auto numSamples = bufferToFill.numSamples;

        // Clear output buffer.
        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
            buffer->clear(channel, bufferToFill.startSample, numSamples);

        // Process sample by sample.
        for (int sample = 0; sample < numSamples; sample++) {
            // Global scheduling of new subclicks.
            if (previewActive) {
                if (samplesUntilNextSubClick <= 0 && currentPipIndex < pips.size()) {
                    const Pip& pip = pips[currentPipIndex];
                    spawnSubClick(pip);
                    int delay = pip.length - pip.tail;  // Ensure pip.length & pip.tail are in samples.
                    samplesUntilNextSubClick = (delay > 0) ? delay : 1;
                    currentPipIndex++;

                    //this is never called. why?
                    juce::Logger::writeToLog("created a new pip");

                    // End preview scheduling when all pips have been spawned.
                    if (currentPipIndex >= static_cast<int>(pips.size())) {
                        previewActive = false;
                    }
                }
                else {
                    samplesUntilNextSubClick--;
                }
            }

            // Render active subclicks.
            float output = renderActiveSubClicks();
            for (int channel = 0; channel < buffer->getNumChannels(); channel++) {
                buffer->addSample(channel, bufferToFill.startSample + sample, output);
            }
        }
    }


    float renderActiveSubClicks() {
        double output = 0.0;
        for (auto& click : activeSubClicks) {
            // Render the click audio only.
            float oscVal = std::sin(click.phase * 2.0 * juce::MathConstants<double>::pi);
            click.phase += click.frequency / currentSampleRate;
            if (click.phase >= 1.0)
                click.phase -= 1.0;

            if (click.curLevel >= click.maxLevel) {
                click.levelChangePerSample = -(click.curLevel / static_cast<double>(click.samplesRemaining));
            }
            click.curLevel += click.levelChangePerSample;
            output += oscVal * click.curLevel;
            click.samplesRemaining--;
        }

        // Remove finished subclicks.
        activeSubClicks.erase(
            std::remove_if(activeSubClicks.begin(), activeSubClicks.end(),
                [](const auto& click) { 
                    //it only prints finishing pip once, meaning only one pip is ever played
                    if(click.samplesRemaining <= 0) juce::Logger::writeToLog("Finishing pip");
                    return click.samplesRemaining <= 0; 
                }),
            activeSubClicks.end()
        );

        return static_cast<float>(output);
    }

    //call to trigger a new click
    void triggerPreviewClick() {
        // Immediately spawn the first subclick.
        if (pips.empty()) {
            return;
        }
        spawnSubClick(pips[0]);
        int delay = pips[0].length - pips[0].tail;
        samplesUntilNextSubClick = (delay > 0) ? delay : 1;
        currentPipIndex = 1;
        // If there's only one pip, the preview will immediately end.
        //this logger correctly reports the number of pips
        /*juce::Logger::writeToLog("There are " + juce::String(pips.size()) + " pips");*/
        previewActive = pips.size() == 1;
        if (pips.size() == 1) {
            previewActive = false;
        } else {
            previewActive = true;
        }
            
    }

    void setPips(std::vector<Pip> newPips) {
        pips = newPips;
    }




private:
    //identical to struct in synthVoice.h
    struct SubClick
    {
        int samplesRemaining;     
        double frequency;              
        double phase;                  
        double maxLevel;               
        double curLevel;               
        double levelChangePerSample;   
    };


    void spawnSubClick(const Pip& pip) {
        SubClick newSubClick;
        float baseFreq = pip.frequency;
        float freqRandomnessAmount = *apvts.getRawParameterValue("Click Pitch Random"); // value from 0 to 1
        float freqRandomOffset = ((rng.nextFloat() * 2.0f) - 1.0f) * freqRandomnessAmount;
        float frequencyMultiplier = std::pow(2.0f, freqRandomOffset);

        float ratioParam = *apvts.getRawParameterValue("Click Atack Decay Ratio");
        int samplesUntilFall = std::round(ratioParam * static_cast<float>(pip.length));

        newSubClick.samplesRemaining = pip.length;
        newSubClick.phase = 0.0;
        newSubClick.frequency = baseFreq * frequencyMultiplier;
        newSubClick.maxLevel = pip.level;
        newSubClick.curLevel = 0.0;
        newSubClick.levelChangePerSample = pip.level / static_cast<double>(samplesUntilFall);

        activeSubClicks.push_back(newSubClick);
    }


    int samplesUntilNextSubClick = 0;
    bool previewActive = false;
    int currentPipIndex = 0;
    std::vector<Pip> pips;
    std::vector<SubClick> activeSubClicks;
    double currentSampleRate = 44100.0;

    juce::Random rng;
    juce::AudioProcessorValueTreeState& apvts;
};