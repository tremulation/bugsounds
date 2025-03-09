/*
  ==============================================================================

    PresetManager.h
    Created: 6 Mar 2025 9:42:22pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PipStructs.h"


class BugsoundsAudioProcessor;

class PresetManager : public juce::ChangeBroadcaster {
public:
    static const juce::File defaultDir;
    static const juce::String extension;


    PresetManager(juce::AudioProcessorValueTreeState& valueTreeState, juce::String& freqSongRef,
        juce::String& resSongRef, std::vector<Pip>& pipsRef, BugsoundsAudioProcessor& p);

    void savePreset(const juce::String& presetName);
    void exportXml(juce::XmlElement& parentElement);
    void deletePreset(const juce::String& presetName);
    void loadPreset(const juce::String& presetName);
    void importXml(const juce::XmlElement& parentElement);
    int loadNextPreset();
    int loadPreviousPreset();

    juce::StringArray getAllPresets() const;
    juce::String getCurrentPreset() const;
    void setCurrentPresetName(const juce::String& pname);

private:
    static const juce::String defaultPresetXml;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String& freqSong;
    juce::String& resSong;
    std::vector<Pip>& pips;
    BugsoundsAudioProcessor& audioProcessor;

    juce::String currentPreset;
};