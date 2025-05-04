/*
  ==============================================================================

    Spatializer.h
    Created: 22 Apr 2025 2:31:57am
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>



//effects object takes raw mono audio and spatializes it into a stereo signal,
//based on two variables which are set before processing: distance, and angle
class Spatializer {
public:
	using ProcessChain = juce::dsp::ProcessorChain<
		juce::dsp::Gain<float>,			//distance attenuation
		juce::dsp::IIR::Filter<float>,	//high shelf filter
		juce::dsp::Reverb				//reverb
	>;

	//keep the panner separate so the chain stays mono
	juce::dsp::Panner<float> panner;	

	//sets up the processorChain, and sets the parameters for the current voice
	//should be called when a voice is created, and every time its dist/angle is randomized
	void prepare(float distance, float angle, const juce::dsp::ProcessSpec& spec) {
		currentSpec = spec;

		//ensure the spec is mono
		juce::dsp::ProcessSpec monoSpec = spec;
		monoSpec.numChannels = 1;

		chain.prepare(monoSpec);
		panner.prepare(spec);

		//set up distance gains and panning
		isPrepared = true;
		updatePosition(distance, angle);
	}


	void updatePosition(float newDistance, float newAngle) {
		const float minDistance = 5.0f;
		const float maxDistance = 15.0f;

		//GAIN: calculate gain based on inverse square law
		float distanceFactor = minDistance / newDistance;
		float gainFactor = distanceFactor * distanceFactor;
		float gainDB = juce::jlimit(-5.0f, -0.0f, juce::Decibels::gainToDecibels(gainFactor));
		chain.get<0>().setGainDecibels(gainDB);

		//HIGH SHELF FILTER: 
		//get distance from 0 to 1 for scaling params
		float normalizedDistance = (newDistance - minDistance) / (maxDistance - minDistance);
		//higher distance means lower cutoff
		float cutoffFreq = juce::jmap(normalizedDistance, 10000.0f, 400.0f);
		//higher distance is more attenuation. 0 dB close, -48 dB far
		float shelfGainDB = juce::jmap(normalizedDistance, 0.0f, -24.0f);
		float q = 0.7f;
		auto& filter = chain.get<1>();
		filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
			currentSpec.sampleRate,
			cutoffFreq,
			q,
			juce::Decibels::decibelsToGain(shelfGainDB)
		);

		//REVERB: 
		juce::Reverb::Parameters reverbParams;
		reverbParams.wetLevel = juce::jmap(normalizedDistance, 0.1f, 0.8f);
		reverbParams.roomSize = 0.5f;
		reverbParams.damping = 1.0f;
		reverbParams.dryLevel = 1 - reverbParams.wetLevel;
		reverbParams.width = 0.5f;
		chain.get<2>().setParameters(reverbParams);


		//PANNING: map angle (0 to 2pi) to the panners -1 to 1 range
		//we only care about x-axis projection since we're working with stereo
		float normalizedPan = juce::jlimit(-1.0f, 1.0f, std::cos(newAngle));
		panner.setPan(normalizedPan);
	}


	//=====================================================================================


	//processes the block of audio
	//has to be done blockwise, so this isn't by-sample, like click generation
	void processBlock(const juce::AudioBuffer<float>& inBuffer,
				      juce::AudioBuffer<float>& outBuffer,	
		              int outputStartSample, int numSamples) {

		//MONO CHAIN SECTION
		//ensure mono in, stereo out, same numSamples to fill
		jassert(inBuffer.getNumChannels() == 1);
		jassert(outBuffer.getNumChannels() == 2);
		jassert(outputStartSample + numSamples <= outBuffer.getNumSamples());

		//create a temporary mono buffer for processing
		juce::AudioBuffer<float> monoBuffer(1, numSamples);
		monoBuffer.clear();
		monoBuffer.copyFrom(0, 0, inBuffer, 0, 0, numSamples);

		//process through the chain
		juce::dsp::AudioBlock<float> block(monoBuffer);
		juce::dsp::ProcessContextReplacing<float> context(block);
		chain.process(context);


		//STEREOIZATION SECTION
		juce::AudioBuffer<float> stereoBuffer(2, numSamples);

		//copy the processed mono data to both channels
		for (int channel = 0; channel < 2; ++channel) {
			stereoBuffer.copyFrom(channel, 0, monoBuffer, 0, 0, numSamples);
		}

		//apply panning
		juce::dsp::AudioBlock<float> stereoblock(stereoBuffer);
		juce::dsp::ProcessContextReplacing<float> steroContext(stereoblock);
		panner.process(steroContext);

		//add processed stereo output to outBuffer
		for (int channel = 0; channel < 2; ++channel) {
			outBuffer.addFrom(channel, outputStartSample, stereoBuffer, channel, 0, numSamples);
		}
	}


	//=====================================================================================


	void reset() {
		chain.reset();
	}


	bool isPrepared = false;
private:

	ProcessChain chain;
	juce::dsp::ProcessSpec currentSpec;

	//spatialization parameters
	float leftGain, rightGain;
	float distanceFreqCutoff;
	float reverbMix;
};