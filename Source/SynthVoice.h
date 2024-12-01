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
#include "PipStructs.h"
#include "PluginProcessor.h"
#include "SongCodeCompiler.h"
#include "HelmholtzResonator.h"

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
        
        //get the resonator song from the other editor
        if (*apvts->getRawParameterValue("Resonator On")) {
            //don't forget to turn it on!
            resonatorEnabled = true;

            resSong = compileSongcode(resString.toStdString(), &error, linkedRandValues, freqStatusColor);
            if (resSong.size() == 0) {
                resonatorEnabled = false;   //might lead to unexpected behavior
                clearCurrentNote();
                return;
            }
            resCurIndex = 0;
            resFreqDelta = 0;
            resonatorFreq = resSong[0].startFrequency;
            setupNextResNote(resSong[0]);
        }
        else {
            resonatorEnabled = false;
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
            // IMPULSES  --  should we place an impulse on this sample?
            // Explanation of all the factors that determine the target phase:
            //      1.0 -- phase is 0 to 1, so this is the normal place a click would begin
            //      patternPhaseDivisor -- Patterns allow us to create subdivisions of the current click,
            //          and the subdivision of the current max corresponds to that
            //      timingOffset -- a randomly generated offset that scales with the timing randomness parameter
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
                //run second-layer subclick generation on all the clicks in activeClicks
                updateActiveClicks();

                //calculate new random offset for next click's phase
                //offset will be in range (-timingOffestMax * offsetScalar, timingOffsetMax * offsetScalar)
                float offsetScalar = *apvts->getRawParameterValue("Click Timing Random"); //0.0 to 1.0
                float randomOffset = (rng.nextFloat() * timingOffsetMax * 2) - timingOffsetMax; 
                timingOffset = (randomOffset * offsetScalar) * (1.0 / patternPhaseDivisor);
            }


            //calculate the output of all currently-active clicks
            float clickOutput = renderActiveClicks();

            //if the resonator is on, then pass the click audio through it
            if (resonatorEnabled) {
                resonator.Q = *apvts->getRawParameterValue("Resonator Q");
               clickOutput = resonator.processSamples(clickOutput, resonatorFreq);
            }

            //add the clicks to the output buffer
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

            //now do all that stuff for the resonator too
            if (resonatorEnabled) {
                resSamplesRemaining--;
                resonatorFreq += resFreqDelta;

                if (resSamplesRemaining == 0) {
                    ++resCurIndex;
                    if (resCurIndex == resSong.size()) {
                        resFreqDelta = 0.0;
                        resSamplesRemaining = -1;   //i think this will stop res song going past final note
                    }
                    else {
                        setupNextResNote(resSong[resCurIndex]);
                    }
                }
            }
        }
    }

    

    //===========================================================================

    void setSongString(juce::String newSongString)  {
        juce::Logger::writeToLog("song recieved: " + newSongString);
        songString = newSongString;
    }

    void setResonatorString(juce::String newResonatorString) {
        juce::Logger::writeToLog("resonator song received: " + newResonatorString);
        resString = newResonatorString;
    }

    void setPipSequence(std::vector<Pip> pips) {
        pipSequence = pips;
        juce::Logger::writeToLog("Pips received. First freq: " + juce::String(pipSequence[0].frequency));
    }

    void setAPVTS(juce::AudioProcessorValueTreeState* apvtsPtr) {
        apvts = apvtsPtr;
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        resonator.prepareToPlay(sampleRate);
    }

private:



    juce::Random rng;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::String songString;
    juce::String resString;
    std::vector<Pip> pipSequence;

    //song state
    std::vector<SongElement> song = {};
    int curElementIndex = 0;
    bool playing = false;
    int samplesRemainingInNote = 0; 

    //resonator song state
    HelmholtzResonator resonator;
    std::vector<SongElement> resSong = {};
    int resCurIndex = 0;
    int resSamplesRemaining = 0;
    bool resonatorEnabled = false;
    double resonatorFreq = 0.0f;
    double resFreqDelta = 0.0f;

    //first layer impulse state
    double phase = 0.0;
    double phaseDelta = 0.0;
    double deltaChangePerSample = 0.0;
    double level;
    //move randomness timing offset variables to second layer impulse state
    float timingOffset = 0.0;
    float timingOffsetMax = .50;     // 50% of normal click length

    //second layer impulse state
    double pipPhase = 0.0;
    double pipPhaseDelta = 0.0;
    double pipPhaseDeltaChangePerSample = 0.0;

    //click generator state
    struct Click {
        int pos; //position of the next pip in the sequence
        int samplesTilNextClick; //how long until the next pip starts
        int subClicksRemaining;
    };
 
    struct SubClick {
        int samplesRemaining;
        double frequency;
        double phase;
        double maxLevel;
        double curLevel;
        double levelChangePerSample;
    };

    std::vector<Click> activeClicks;
    std::vector<SubClick> activeSubClicks;


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


    void setupNextResNote(struct SongElement nextNote) {
        if (nextNote.type == SongElement::Type::Pattern) {
            //ignore. TODO in the future maybe throw a parsing error if a pattern is detected in res parsing in the future

            //advance to next note
            resCurIndex++;
            setupNextResNote(resSong[resCurIndex]);
        }
        else {
            auto startingFreq = nextNote.startFrequency;
            auto endingFreq = nextNote.endFrequency;
            auto noteLengthInSamples = (nextNote.duration / 1000) * getSampleRate();

            resFreqDelta = (endingFreq - startingFreq) / noteLengthInSamples;
            resSamplesRemaining = (int)noteLengthInSamples;
        }
    }


    float renderActiveClicks() {
        double output = 0.0;
        //process all the clicks the synth is currently playing
        for (auto& click : activeSubClicks) {

            //calculate raw sine val, save its state
            float oscVal = std::sin(click.phase * 2.0f * juce::MathConstants<double>::pi);
            click.phase += click.frequency / getSampleRate();
            if (click.phase >= 1.0f) click.phase -= 1.0f;

            //when curlevel = maxLevel, linearly descend to 0 intensity in remaining samples
            if (click.curLevel >= click.maxLevel) {
                click.levelChangePerSample = -(click.curLevel / static_cast<double>(click.samplesRemaining));
            }
            click.curLevel += click.levelChangePerSample;
             
            output += oscVal * click.curLevel;

            click.samplesRemaining--;
        }

        //remove finished clicks
        activeSubClicks.erase(
            std::remove_if(activeSubClicks.begin(), activeSubClicks.end(),
                [](const SubClick& click) { return click.samplesRemaining <= 0; }),
            activeSubClicks.end()
        );
        return static_cast<float>(output);
    }


    //TODO should I randomize things like pitch/level/length here, or in startNewSubClick
    void startNewClick(){
        Click newClick;

        //create first subclick
        struct Pip firstPip = pipSequence[0];
        startNewSubClick(firstPip.frequency, firstPip.length, firstPip.level);

        //now set up the click state
        newClick.pos = 1;   //already started first pip, go to second
        newClick.samplesTilNextClick = firstPip.length - firstPip.tail;
        if (newClick.samplesTilNextClick <= 0) {
            newClick.samplesTilNextClick == 1;
        }

        activeClicks.push_back(newClick);
    }


    void startNewSubClick(float freq, int samples, float vol) {
        SubClick newClick;
        //use the random number generator and the range from the parameters to change the frequency of this click
        //remember the range of the parameter is 0 (no randomness) to 1 (very random pitch)
        //at randomness = 1, the random value will have a range of (0, 2 * baseFreq)
        // ^ is that range right?
        float baseFreq = freq;
        float freqRandomnessAmount = *apvts->getRawParameterValue("Click Pitch Random");   //val from 0 to 1, representing how random it should be
        float freqRandomOffset = ((rng.nextFloat() * 2.0f) - 1.0f) * freqRandomnessAmount;
        float frequencyMultiplier = std::pow(2.0f, freqRandomOffset);

        float baseLevel = vol;
        float levelRandomnessAmount = *apvts->getRawParameterValue("Click Volume Random");  //0 to 1 again
        float levelOffsetScalar = 1 - (rng.nextFloat() * levelRandomnessAmount);
        float randomizedLevel = baseLevel * levelOffsetScalar;
        
        float ratioParam = *apvts->getRawParameterValue("Click Rise Ratio");
        int samplesUntilFall = std::round(ratioParam * static_cast<float>(samples));

        newClick.samplesRemaining = samples;
 
        newClick.phase = 0.0f;
        newClick.frequency = baseFreq * frequencyMultiplier;

        newClick.maxLevel = randomizedLevel;
        newClick.curLevel = 0.0f;
        newClick.levelChangePerSample = randomizedLevel / static_cast<double>(samplesUntilFall);
        
        activeSubClicks.push_back(newClick);
    }


    //handles turning the active clicks into subclicks
    void updateActiveClicks() {
        //process all the clicks
        for (auto& click : activeClicks) {
            //should we start a new click here?
            if (click.samplesTilNextClick <= 0 && click.pos < pipSequence.size()) {
                //start a new subclick with the next pip in the sequence
                struct Pip nextPip = pipSequence[click.pos];
                startNewSubClick(nextPip.frequency, nextPip.length, nextPip.level);

                //update the click state to the next pip
                click.pos++; 
                if (click.pos < pipSequence.size()) {
                    click.samplesTilNextClick = nextPip.length - nextPip.tail;
                    if (click.samplesTilNextClick <= 0) {
                        click.samplesTilNextClick = 1;
                    }
                }

            }
            else {  //we don't need a new subclick on this sample. Decrement samples remaining til next
                click.samplesTilNextClick--;
            }
        }

        //remove finished clicks with an iterator
        activeClicks.erase(
            std::remove_if(activeClicks.begin(), activeClicks.end(),
                [size = pipSequence.size()](const Click& click) {return click.pos >= size; }),
            activeClicks.end()
        );
    }
};




