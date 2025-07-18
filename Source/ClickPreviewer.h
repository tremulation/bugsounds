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
#include "ClickGenerator.h"

class ClickPreviewer : public juce::AudioSource, private ClickGenerator {

public:
    ClickPreviewer(juce::AudioProcessorValueTreeState& valueTreeState) : apvts(valueTreeState)
    {
    }

    ~ClickPreviewer() = default;

    //audiosource stuff
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        currentSampleRate = sampleRate;
        activeSubClicks.clear();
        currentPipIndex = 0;
        samplesUntilNextSubClick = 0;
        previewActive = false;
    }

    void releaseResources() override {
        activeSubClicks.clear();
    }


    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        auto* buffer = bufferToFill.buffer;
        auto numSamples = bufferToFill.numSamples;

        //clear output buffer
        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
            buffer->clear(channel, bufferToFill.startSample, numSamples);

        //process sample by sample
        for (int sample = 0; sample < numSamples; sample++) {
            //global scheduling of new subclicks
            if (previewActive) {
                if (samplesUntilNextSubClick <= 0 && currentPipIndex < pips.size()) {
                    const Pip& pip = pips[currentPipIndex];
                    spawnSubClick(pip);
                    int delay = pip.length - pip.tail;
                    samplesUntilNextSubClick = (delay > 0) ? delay : 1;
                    currentPipIndex++;

                    //end preview scheduling when all pips have been spawned
                    //this doesn't turn off audio. only stops new subclicks from being spawned
                    if (currentPipIndex >= static_cast<int>(pips.size())) {
                        previewActive = false;
                    }
                }
                else {
                    samplesUntilNextSubClick--;
                }
            }
            //render active subclicks
            float output = renderActiveSubClicks();
            for (int channel = 0; channel < buffer->getNumChannels(); channel++) {
                buffer->addSample(channel, bufferToFill.startSample + sample, output);
            }
        }
    }



    //============================================================



    float renderActiveSubClicks() {
        double output = 0.0;

        for (size_t i = 0; i < activeSubClicks.size(); ++i) {
            //get the current subclick
			SubClick& click = activeSubClicks[i];
			output += renderSubclickSample(click);
        }

        //try to remove finished clicks without crashing the whole program
        int activeClickNum = activeSubClicks.size();
        if (activeClickNum == 0) {
            return 0;
        }
        else {
            // Remove finished subclicks. 
            activeSubClicks.erase(
                std::remove_if(activeSubClicks.begin(), activeSubClicks.end(),
                    [](auto& c) { return c.samplesRemaining <= 0; }),
                activeSubClicks.end()
            );

            return static_cast<float>(output);
        }
    }



    void setPips(std::vector<Pip> newPips) {
        pips = newPips;
    }


    void generateFullPreview(juce::AudioBuffer<float>& outBuffer) {
        if (pips.empty()) {
            outBuffer.setSize(0, 0);
            return;
        }

        //get the correct size of the buffer so we can set the buffer correctly
        int nextStart = 0.f;
        int lenSamples = 0;
        for (int i = 0; i < pips.size(); i++) {
            const Pip& pip = pips[i];
            int overlap = pip.tail;
			int length = pip.length;

            //get overlap with next subclick
            if (overlap > length) overlap = length;

            //find the time boundaries of this particular subclick
            int start = nextStart;
            int end = nextStart + length;

            //update the two time markers
            if (end > lenSamples) lenSamples = end;
            int advance = length - overlap;
            if (advance < 0) advance = 0;
            nextStart += advance;
        }
        int totalSamples = lenSamples;

        //allocate buffa
        outBuffer.setSize(1, totalSamples);
        outBuffer.clear();

        //reset previewer’s internal scheduling state
        activeSubClicks.clear();
        currentPipIndex = 0;
        samplesUntilNextSubClick = 0;
        //immediately queue the very first subclick
        if (!pips.empty()){
            spawnSubClick(pips[0]);
            int delay = pips[0].length - pips[0].tail;
            samplesUntilNextSubClick = (delay > 0) ? delay : 1;
            currentPipIndex = 1;
            // if there's more than one pip, keep scheduling
            previewActive = (pips.size() > 1);
        }

        //tell your AudioSource exactly how many samples the host expects
        //(we’re pretending the host is asking for one big block)
        juce::AudioSourceChannelInfo info(&outBuffer, 0, outBuffer.getNumSamples());

        //do the render
        getNextAudioBlock(info);
		juce::Logger::writeToLog("Generated full preview with samples: " + juce::String(outBuffer.getNumSamples()));
        //print out samples from the buffer to verify
		//for (int i = 0; i < outBuffer.getNumSamples(); ++i) {
		//	juce::Logger::writeToLog(juce::String(i) + ": " + juce::String(outBuffer.getSample(0, i)));
		//}
    }




private:

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
    double currentSampleRate = 44100.0;

    juce::Random rng;
    juce::AudioProcessorValueTreeState& apvts;
};