/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "SynthVoice.h"
#include "SongCodeCompiler.h"
#include "PipStructs.h"
#include "Evaluator.h"
#include "PresetManager.h"
#include "PipStructs.h"
#include "ClickPreviewer.h"



//==============================================================================
/**
*/
class BugsoundsAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    BugsoundsAudioProcessor();
    ~BugsoundsAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", createParameterLayout()};
  

    //==============================================================================
    
    //MY FUNCTIONS
    void setSongAST(juce::ReferenceCountedObjectPtr<ScriptNode> scriptAST) {
		myVoice->setSongScript(scriptAST);
    }

	void setResAST(juce::ReferenceCountedObjectPtr<ScriptNode> scriptAST) {
		myVoice->setResScript(scriptAST);
	}
    
    void setPipSequence(std::vector<Pip> pips) {
        myVoice->setPipSequence(pips);
    }


    //getter method for pips
    const std::vector<Pip>& getPips() const {
        return pips;
    }

	PresetManager& getPresetManager() {
		return *presetManager;
	}

    //persistence functions
	const juce::String& getUserSongcode(const juce::String& editorTitle);
	void setUserSongcode(const juce::String& songcode, const juce::String& editorTitle); 
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    void setPips(std::vector<Pip> pips) { 
        this->pips = pips; 
        myVoice->setPipSequence(pips);
        clickPreviewer->setPips(pips);
    }
    void updatePipBarModes(EditingMode newMode) { pipMode = newMode; }
    void getPips(std::vector<Pip>& pips, EditingMode& mode) { pips = this->pips;  mode = pipMode;  }
    
    void propogatePresetLoad(juce::String fs, juce::String rs, std::vector<Pip> pipi) {
        freqSong = fs;
        resSong = rs;
        setPips(pipi);
    }


    //for updating the chorus position readout in chorusKnobRack
    struct ChorusVoicePosition {
        float distance, angle;
        bool isPlaying;
    };

    std::vector<ChorusVoicePosition> getChorusVoicePositions() {
        return chorusVoicePositions;
    }

    void rerollChorusVoicePositions() { myVoice->randomizeChorusPositions(); }

    void setChorusVoicePositions(std::vector<ChorusVoicePosition> newPositions) {
        chorusVoicePositions = newPositions;
    }

    //getter methods for freqSong, and resSong
    const juce::String& getFreqSong() const { return freqSong; }
    const juce::String& getResSong() const { return resSong; }
    
    void triggerPreviewClick();
	
private:

    std::vector<ChorusVoicePosition> chorusVoicePositions;

    juce::String freqSong = "";
    juce::String resSong = "";
    std::vector<Pip> pips;
    enum EditingMode pipMode = EditingMode::FREQUENCY;

    juce::Synthesiser mySynth;
    SynthVoice* myVoice;
    double lastSampleRate;
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<ClickPreviewer> clickPreviewer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BugsoundsAudioProcessor)
};
