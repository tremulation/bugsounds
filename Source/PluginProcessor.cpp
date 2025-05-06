/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BugsoundsAudioProcessor::BugsoundsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
	 ) 
#endif
{
    presetManager = std::make_unique<PresetManager>(apvts, freqSong, resSong, pips, *this);
    clickPreviewer = std::make_unique<ClickPreviewer>(apvts);
    mySynth.clearVoices();
    myVoice = new SynthVoice();
    mySynth.addVoice(myVoice);
    myVoice->setAPVTS(&apvts);
    myVoice->setOwner(*this);
    myVoice->beginPeriodicChorusUpdates();
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());

    if (presetManager != nullptr) presetManager->loadPreset("Default");
}

BugsoundsAudioProcessor::~BugsoundsAudioProcessor()
{
}

//==============================================================================
const juce::String BugsoundsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BugsoundsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BugsoundsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BugsoundsAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BugsoundsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BugsoundsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BugsoundsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BugsoundsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BugsoundsAudioProcessor::getProgramName (int index)
{
    return {};
}

void BugsoundsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BugsoundsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(samplesPerBlock);    //clears out unused samples from last key press
    lastSampleRate = sampleRate;
    mySynth.setCurrentPlaybackSampleRate(lastSampleRate);

    if (clickPreviewer != nullptr) {
        clickPreviewer->prepareToPlay(samplesPerBlock, sampleRate);
    }
}

void BugsoundsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BugsoundsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BugsoundsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //clear extra output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //render main synth player output
    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    //render previewer output into a temporary buffer
    juce::AudioBuffer<float> previewBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    previewBuffer.clear();
    juce::AudioSourceChannelInfo previewInfo(&previewBuffer, 0, buffer.getNumSamples());
    if (clickPreviewer != nullptr) clickPreviewer->getNextAudioBlock(previewInfo);

    //mix previewer output into main buffer
    for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
        buffer.addFrom(channel, 0, previewBuffer, channel, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool BugsoundsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BugsoundsAudioProcessor::createEditor()
{
    return new BugsoundsAudioProcessorEditor (*this);
}

//==============================================================================
void BugsoundsAudioProcessor::getStateInformation (juce::MemoryBlock& destData){
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::XmlElement mainElement("StateInfo");
    mainElement.setAttribute("currentPreset", presetManager->getCurrentPreset());
    presetManager->exportXml(mainElement);
    copyXmlToBinary(mainElement, destData);
}

void BugsoundsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr) {
        const juce::String loadedPresetName = xml->getStringAttribute("currentPreset", "Default");
        presetManager->setCurrentPresetName(loadedPresetName);
        presetManager->importXml(*xml);
        presetManager->sendChangeMessage();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout BugsoundsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Resonator On",
        "Resonator On",
        false));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        "Resonator Overtone Number",
        "Resonator Overtone Number",
        1, 16, 0));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"Resonator Q",
		"Resonator Q",
		juce::NormalisableRange<float>(0.0f, 200, 1.0f, 1.0f),
		100.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"Resonator Gain",
		"Resonator Gain",
		juce::NormalisableRange<float>(0.0f, 50.0f, 0.01f, 1.0f),
		25.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Resonator Overtone Decay",
        "Resonator Overtone Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Resonator Original Mix",
        "Resonator Original Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        1.0f));

    //click parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Timing Random",
        "Timing Randomness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Pitch Random",
        "Pitch Randomness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Atack Decay Ratio",
        "Click Atack Decay Ratio",
        juce::NormalisableRange<float>(0.1f, 0.9f, 0.01f, 1.f),
        0.10f  // Default to 1/4 rise, 3/4 fall
    ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Start Fadeout",
        "Click Start Fadeout",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.05f, 1.0f),
        1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"Click Floor Frequency",
		"Click Floor Frequency",
		juce::NormalisableRange<float>(0.0f, 1000.0f, 1.f, 1.0f),
		100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Start Jitter",
        "Click Start Jitter",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.05f, 1.0f),
        0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Volume",
        "Click Volume",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -6.0f  
    ));

    //chorus parameters
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Chorus On", 
        "Chorus On", 
        false));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "Chorus Count",
        "Chorus Count",
        1,
        20, 
        3));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Chorus Stereo Spread",
        "Chorus Stereo Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Chorus Max Distance",
        "Chorus Max Distance",
        juce::NormalisableRange<float>(5.0f, 15.0f),
        5.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Chorus Cooldown Max",
        "Chorus Cooldown Max",
        juce::NormalisableRange<float>(0.0f, 24.0f),
        6.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Chorus Correlation",
        "Chorus Correlation",
        juce::NormalisableRange<float> { -1.0f, 1.0f, 0.01f },
        0.0f  // -1 is dispersed, 0 is random, 1 is correlated
    ));

    return layout;
}

const juce::String& BugsoundsAudioProcessor::getUserSongcode(const juce::String& editorTitle){
	if (editorTitle == "Frequency Editor") {
        return freqSong;
	}
	else if (editorTitle == "Resonator Editor") {
		return resSong;
	}
}

void BugsoundsAudioProcessor::setUserSongcode(const juce::String& songcode, const juce::String& editorTitle){
    juce::Logger::writeToLog("HERE");
    if (editorTitle == "Frequency Editor") {
		freqSong = songcode;
	}
	else if (editorTitle == "Resonator Editor") {
		resSong = songcode;
	}
}

void BugsoundsAudioProcessor::triggerPreviewClick(){
    if (clickPreviewer != nullptr) {
        clickPreviewer->triggerPreviewClick();
    }
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BugsoundsAudioProcessor();


}

//==============================================================================
//MY FUNCTIONS

