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
#include "ResonatorKnobRack.h"
#include "Evaluator.h"
#include "HeaderBar.h"
#include "ChorusKnobRack.h"
#include "HelpCompendium.h"

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

    void disableResonatorEditor();
    void enableResonatorEditor();
    void toggleHelpCompendium(juce::String pageId);

private:

    const int baseWidth = 800;
    const int baseHeight = 640;
    const int helpWidth = 300;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BugsoundsAudioProcessor& audioProcessor;

    PipSequencer pipSequencer;
    SongcodeEditor frequencyEditor;
    SongcodeEditor resonatorEditor;
    juce::TextButton testButton;
    ClickSettingsKnobRack clickSettingsRack; 
    ResonatorKnobRack resonatorKnobRack;
	ChorusKnobRack chorusKnobRack;
    HeaderBar headerBar;
    HelpCompendium helpCompendium;

    void freqCodeEditorHasChanged();
    void resonatorCodeEditorHasChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BugsoundsAudioProcessorEditor)
};
