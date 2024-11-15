/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "songcodeEditor.h"
#include "SongCodeCompiler.h"
#include "ClickSettingsKnobRack.h"
#include "PipSequencer.h"

//==============================================================================
/**
*/
class BugsoundsAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BugsoundsAudioProcessorEditor (BugsoundsAudioProcessor&);
    ~BugsoundsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BugsoundsAudioProcessor& audioProcessor;

    PipSequencer pipSequencer;
    SongcodeEditor frequencyEditor;
    juce::TextButton testButton;
    ClickSettingsKnobRack clickSettingsRack; 
    

    ////resonator settings
    //juce::Slider resonatorQKnob;
    //juce::Slider resonatorGainKnob;
    //juce::Slider resonatorHarmonicsKnob;
    //juce::ToggleButton resonatorOnButton;

    void freqCodeEditorHasChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BugsoundsAudioProcessorEditor)
};
