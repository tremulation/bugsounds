/*
  ==============================================================================

    SynthVoice.h
    Created: 8 Oct 2024 10:20:01pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SynthSound.h"
#include "PluginProcessor.h"
#include "SongCodeCompiler.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        //if cast fails, then we can't play this sound (since it's the wrong type)
        return dynamic_cast<SynthSound*>(sound) != nullptr;
    }

    //===========================================================================


    //currently plays a sine wave that rises in pitch by an octave over the course of the note
    void startNote(int /*midiNote*/, float velocity, juce::SynthesiserSound* /*sound*/, int /*currentPitchWheelPosition*/) override {
        //wait for current note to stop before playing another
        
        if (playing == true) {
            clearCurrentNote();
            return;
        }

        //get the song from the UI code editor
        std::string error; 
        std::map<char, int> linkedRandValues;
        juce::Colour freqStatusColor;
        song = compileSongcode(songString.toStdString(), &error, linkedRandValues, freqStatusColor);
        logCompiledSong(song);
        
        if (song.size() == 0) {
            juce::Logger::writeToLog("song size is " + std::to_string(song.size()));
            clearCurrentNote();
            return;
        }

        //add the default pattern if the first note isn't a pattern
        if (!song.empty() && song[0].type != SongElement::Type::Pattern) {
            song.insert(song.begin(), SongElement(std::vector<uint8_t>{1}));
        }
        
        rng.setSeedRandomly();

        //set up stuff to keep track of the impulse generator wave
        level = velocity * 0.15;
        phase = 0.00;

        //and data for the song state
        curElementIndex = 0;
        setupNextNote(song[0]);
        playing = true;
    }


    void logCompiledSong(std::vector<SongElement> compiledSong)
    {
        juce::String output;
        for (size_t i = 0; i < compiledSong.size(); ++i) {
            const auto& element = compiledSong[i];
            output += "Element " + juce::String(i) + ": ";

            if (element.type == SongElement::Type::Note) {
                output += "Note - Start Freq: " + juce::String(element.startFrequency)
                    + " Hz, End Freq: " + juce::String(element.endFrequency)
                    + " Hz, Duration: " + juce::String(element.duration) + " ms\n";
            }
            else if (element.type == SongElement::Type::Pattern) {
                output += "Pattern - Beats: ";
                for (const auto& beat : element.beatPattern) {
                    output += juce::String(static_cast<int>(beat)) + " ";
                }
                output += "\n";
            }
        }
        juce::Logger::writeToLog(output);
    }

    //===========================================================================

    void stopNote(float /*velocity*/, bool /*allowTailOff*/) override {
        //ignore for now. Nothing can stop playback!!
    }

    //===========================================================================

    void pitchWheelMoved(int /*newPitchWheelValue*/) override {

    }

    //===========================================================================

    void controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) override {

    }

    //===========================================================================

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (!playing) return;

        while (--numSamples >= 0) {
            //should we place an impulse on this sample?
            if (phase >= ((1.0 / patternPhaseDivisor) + timingOffset)) {
                if (beatPattern[patternIndex] != 0) {
                    startNewClick();
                }
                
                phase = 0.0;

                //advance click here
                clicksRemainingInBeat--;
                if (clicksRemainingInBeat <= 0) {
                    patternIndex = (++patternIndex) % beatPattern.size();

                    if (beatPattern[patternIndex] == 0) {
                        clicksRemainingInBeat = 1;
                        patternPhaseDivisor = 1;
                    }
                    else {
                        clicksRemainingInBeat = beatPattern[patternIndex];
                        patternPhaseDivisor = beatPattern[patternIndex];
                    }
                }

                //calculate new random offset for next click's phase
                float offsetScalar = *apvts->getRawParameterValue("Click Timing Random"); //0.0 to 1.0
                float randomOffset = (rng.nextFloat() * timingOffsetMax * 2) - timingOffsetMax; 
                timingOffset = (randomOffset * offsetScalar) * (1.0 / patternPhaseDivisor);
            }
            //add the output of all the active clicks to the channels
            float clickOutput = renderActiveClicks();
            for (auto i = outputBuffer.getNumChannels(); --i >= 0;) {
                outputBuffer.addSample(i, startSample, clickOutput);
            }

            //move impulse generator along
            phase += phaseDelta;
            ++startSample;

            //move the frequency of the clicks towards the next note
            --samplesRemainingInNote;
            phaseDelta += deltaChangePerSample;

            //if the note is finished go the the next one, or DIE
            if (samplesRemainingInNote == 0) {
                ++curElementIndex;
                if (curElementIndex == song.size()) {
                    //song is finished
                    phaseDelta = 0.0;
                    clearCurrentNote();
                    playing = false;
                }
                else {
                    //song not finished. proceed to remaining notes
                    setupNextNote(song[curElementIndex]);
                }
            }
        }
    }

    

    //===========================================================================

    void setSongString(juce::String newSongString)  {
        juce::Logger::writeToLog("song recieved: " + newSongString);
        songString = newSongString;
    }

    void setAPVTS(juce::AudioProcessorValueTreeState* apvtsPtr) {
        apvts = apvtsPtr;
    }

private:

    juce::Random rng;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String songString;

    //declare random number generator here

    //song state
    std::vector<SongElement> song = {};
    int curElementIndex = 0;
    bool playing = false;
    int samplesRemainingInNote = 0; 

    //impulse state
    double phase = 0.0;
    double phaseDelta = 0.0;
    double deltaChangePerSample = 0.0;
    double level;
    float timingOffset = 0.0;
    float timingOffsetMax = .50;     // 10%

    //click generator state
    struct Click {
        float frequency;
        int samplesRemaining;
        float phase;
        float level;
    };
    std::vector<Click> activeClicks;

    //pattern state
    std::vector <uint8_t> beatPattern = { 1 };
    int patternIndex = 0;
    int patternPhaseDivisor = 1;
    int clicksRemainingInBeat = 1;
    

    void setupNextNote(struct SongElement nextNote) {
        if (nextNote.type == SongElement::Type::Pattern) {
            beatPattern = nextNote.beatPattern;
            patternIndex = 0;

            if (beatPattern[0] == 0) {
                clicksRemainingInBeat = 1;
                patternPhaseDivisor = 1;
            }
            else {
                clicksRemainingInBeat = beatPattern[0];
                patternPhaseDivisor = beatPattern[0];
            }
            
            phase = 1.0;    //trigger click on next sample. not sure this's necessary

            //advance to next note
            curElementIndex++;
            setupNextNote(song[curElementIndex]);
        }
        else {
            // calculate how much the angleDelta will have to increase/decrease by
            // to hit the end frequency in exactly curElement->duration ms.
            auto startingPhaseChange = nextNote.startFrequency / getSampleRate();
            auto endingPhaseChange = nextNote.endFrequency / getSampleRate();
            auto noteLengthInSamples = (nextNote.duration / 1000) * getSampleRate();

            phaseDelta = startingPhaseChange;
            samplesRemainingInNote = (int)noteLengthInSamples;
            deltaChangePerSample = (endingPhaseChange - startingPhaseChange) / noteLengthInSamples;
            //juce::Logger::writeToLog("delta increase: " + std::to_string(deltaChangePerSample) + ". samples remaining: " + std::to_string(samplesRemainingInNote));
        }
    }


    float renderActiveClicks() {
        float output = 0.0;

        //process all the clicks the synth is currently playing
        for (auto& click : activeClicks) {
            output += std::sin(click.phase * 2.0f * juce::MathConstants<float>::pi) * click.level;
            click.phase += click.frequency / getSampleRate();
            if (click.phase >= 1.0f) click.phase -= 1.0f;
            click.samplesRemaining--;
        }

        //remove finished clicks
        activeClicks.erase(
            std::remove_if(activeClicks.begin(), activeClicks.end(),
                [](const Click& click) { return click.samplesRemaining <= 0; }),
            activeClicks.end()
        );
        return output;
    }

    void startNewClick() {
        Click newClick;
        //use the random number generator and the range from the parameters to change the frequency of this click
        //remember the range of the parameter is 0 (no randomness) to 1 (very random pitch)
        //at randomness = 1, the random value will have a range of (0, 2 * baseFreq)
        float baseFreq = 3000.0f;
        float freqRandomnessAmount = *apvts->getRawParameterValue("Click Pitch Random");   //val from 0 to 1, representing how random it should be
        float freqRandomOffset = ((rng.nextFloat() * 2.0f) - 1.0f) * freqRandomnessAmount;
        float frequencyMultiplier = std::pow(2.0f, freqRandomOffset);

        float baseLevel = 1.f;
        float levelRandomnessAmount = *apvts->getRawParameterValue("Click Volume Random");
        float levelOffsetScalar = 1 - (rng.nextFloat() * levelRandomnessAmount);
        float randomizedLevel = baseLevel * levelOffsetScalar;
        
        newClick.frequency = baseFreq * frequencyMultiplier;
        newClick.samplesRemaining = 100;
        newClick.phase = 0.0f;
        newClick.level = randomizedLevel;
        activeClicks.push_back(newClick);
    }
};