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
    void setFrequencyCodeString(const juce::String& newCode) {
        myVoice->setSongString(newCode);
    }

    juce::String getFrequencyCodeString() const {
        return frequencyCodeString;
    }
    
    void setPipSequence(std::vector<Pip> pips) {
        myVoice->setPipSequence(pips);
    }

    void setResonatorCodeString(const juce::String& newCode) {
        myVoice->setResonatorString(newCode);
    }

    juce::String getResonatorCodeString() const {
        return resonatorCodeString;
    }

private:
    juce::String frequencyCodeString;
    juce::String resonatorCodeString;
    juce::Synthesiser mySynth;
    SynthVoice* myVoice;
    double lastSampleRate;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BugsoundsAudioProcessor)
};
