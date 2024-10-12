/*
  ==============================================================================

    SynthSound.h
    Created: 8 Oct 2024 10:19:52pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SongCodeCompiler.h"



class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int /*midiNoteNumber*/) {
        return true;
    }

    bool appliesToChannel(int /*midiChannel*/) {
        return true;
    }

};