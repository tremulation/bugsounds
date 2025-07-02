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

        for (size_t i = 0; i < activeSubClicks.size(); ++i) {
            auto& click = activeSubClicks[i];
            //render the click audio only.
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
        //TODO STILL CAUSING CRASHES WHY??>????>?>
        for (auto it = activeSubClicks.begin(); it != activeSubClicks.end();) {
            if (it == activeSubClicks.end() || it < activeSubClicks.begin()) break;
            if (it->samplesRemaining <= 0) {
                //erase returns the next valid iterator
                //check if the iterator is in range, if not -> return

                it = activeSubClicks.erase(it);        
            } else {
                ++it;
            }
        }

        return static_cast<float>(output);
    }


    void setPips(std::vector<Pip> newPips) {
        pips = newPips;
    }


    void generateFullPreview(juce::AudioBuffer<float>& outBuffer) {
        if (pips.empty()) {
            outBuffer.setSize(0, 0);
            return;
        }

        //get the correct size of the buffer so we can tell rendernextblock how many we need
        //tail is the amount of overlap with the next note, so disregard tail of last note

        //if (pips.size() == 1) lengthMicoseconds = pips[0].length;   //just one pip
        //else {
        //    //all but the last: full lengths
        //    for (int i = 0; i < pips.size() - 1; ++i)
        //        lengthMicoseconds += pips[i].length;
        //    // Last pip: subtract the previous pip’s tail
        //    int last = (int)pips.size() - 1;
        //    lengthMicoseconds += pips[last].length
        //        - pips[last - 1].tail;
        //}

        int nextStart = 0.f;
        int lenMS = 0.f;
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
            if (end > lenMS) lenMS = end;
            int advance = length - overlap;
            if (advance < 0) advance = 0;
            nextStart += advance;
        }
        double lengthSeconds = static_cast<double>(lenMS) / 1e6; // 1 second is 1 million microseconds
		int totalSamples = static_cast<int>(lengthSeconds * currentSampleRate);

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