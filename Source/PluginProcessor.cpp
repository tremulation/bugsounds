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
    mySynth.clearVoices();
    myVoice = new SynthVoice();
    mySynth.addVoice(myVoice);
    myVoice->setAPVTS(&apvts);
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
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
    myVoice->prepareToPlay(sampleRate, samplesPerBlock);
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }

    buffer.clear();
    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
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
void BugsoundsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BugsoundsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout BugsoundsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //resonator parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Resonator Q",
        "Resonator Q",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        5.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Resonator Gain",
        "Resonator Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Resonator On",
        "Resonator On",
        false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Resonator Harmonics",
        "Resonator Harmonics",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.5f));

    //click parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Timing Random",
        "Timing Randomness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Volume Random",
        "Volume Randomness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Pitch Random",
        "Pitch Randomness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Click Rise Ratio",
        "Click Rise Ratio",
        juce::NormalisableRange<float>(0.1f, 0.9f, 0.01f, 1.f),
        0.10f  // Default to 1/4 rise, 3/4 fall
    ));

    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BugsoundsAudioProcessor();


}

//==============================================================================
//MY FUNCTIONS

