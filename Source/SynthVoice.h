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
        
        //set up stuff to keep track of the impulses
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
            if (phase >= 1.0) {
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;) {
                    outputBuffer.addSample(i, startSample, level);
                }
                phase = 0.0;
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

private:

    juce::String songString;
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


    void setupNextNote(struct SongElement nextNote) {
        // calculate how much the angleDelta will have to increase/decrease by
        // to hit the end frequency in exactly curElement->duration ms.
        auto startingPhaseChange = nextNote.startFrequency / getSampleRate();
        auto endingPhaseChange   = nextNote.endFrequency / getSampleRate();
        auto noteLengthInSamples = (nextNote.duration / 1000) * getSampleRate();

        phaseDelta = startingPhaseChange;
        samplesRemainingInNote = (int)noteLengthInSamples;
        deltaChangePerSample = (endingPhaseChange - startingPhaseChange) / noteLengthInSamples;
        //juce::Logger::writeToLog("delta increase: " + std::to_string(deltaChangePerSample) + ". samples remaining: " + std::to_string(samplesRemainingInNote));
    }
};