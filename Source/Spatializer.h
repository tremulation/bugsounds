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
		juce::dsp::DelayLine<float>,	//predelay
		juce::dsp::IIR::Filter<float>,	//high shelf filter
		juce::dsp::Reverb				//reverb	
	>;

	//sets up the processorChain, and sets the parameters for the current voice
	//should be called when a voice is created, and every time its dist/angle is randomized
	void prepare(float distance, float angle, const juce::dsp::ProcessSpec& spec) {
		currentSpec = spec;
		chain.prepare(spec);

		//set up distance gains and panning
		updatePosition(distance, angle);

		//calculate predelay
		auto& delay = chain.get<0>();
		delay.setMaximumDelayInSamples((100.0f / 1000.0f) * spec.sampleRate);	//100ms max delay time
		delay.reset();
		const float preDelayMs = juce::jmap(distance, 0.01f, 1.0f, 10.0f, 50.0f); //10ms to 50ms
		delay.setDelay((preDelayMs / 1000.0f) * spec.sampleRate); //convert to samples

		//calculate the highpass filter cutoff frequency
		const float effectiveDistance = std::max(0.01f, distance);
		const float filterCutoffFreq = 6000.0f; //fixed cutoff freq (high end)
		const float maxDistance      = 1.0f; //from 0 to 1
		const float Q                = 0.5f;	//small Q = wide Q
		const float resonanceCut = juce::jmap(effectiveDistance,
			0.01f, 1.f,
			1.f  , 0.3f);	//more resonance further away(more attenuation)

		auto& filter = chain.get<1>();
		auto filterCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
			spec.sampleRate,
			filterCutoffFreq,
			resonanceCut,
			Q); 
		filter.coefficients = *filterCoeffs;
		filter.reset();

		//configure reverb
		//trying to approximate a diffuse, outside sound where waves are bouncing off trees and the ground
		//not really any walls
		auto& reverb = chain.get<2>();
		juce::dsp::Reverb::Parameters reverbParams;
		reverbParams.roomSize = 0.65f; 
		//damping is variable, since sound gets more diffuse the further it has to travel. it spreads out.
		reverbParams.damping = 0.95f;	//strong damping
		reverbParams.wetLevel = juce::jmap(distance, 0.01f, 1.0f, 0.05f, 0.15f);
		reverbParams.dryLevel = 1.0f - reverbParams.wetLevel;
		reverbParams.width = 0.8;
		reverb.setParameters(reverbParams);
	}


	void updatePosition(float newDistance, float newAngle) {
		//calculate distance falloff based on inverse square law. decibels are confuse.
		const float minDistance = 0.01f;	//no divide by 0
		const float effectiveDistance = std::max(minDistance, newDistance);
		const float distanceQuieting = 1.0f / (1 + effectiveDistance * effectiveDistance);

		//calculate left/right gains for panning
		leftGain = std::cos(newAngle * 0.5) * distanceQuieting;
		rightGain = std::sin(newAngle * 0.5) * distanceQuieting;
	}


	//=====================================================================================


	//processes the block of audio
	//has to be done blockwise, so this isn't by-sample, like click generation
	//apply the chain, THEN pan/spatialize the output with leftGain/rightGain
	void processBlock(const juce::AudioBuffer<float>& inBuffer,
				      juce::AudioBuffer<float>& outBuffer,	
		              int outputStartSample, int numSamples) {
		//ensure mono in, stereo out, same numSamples to fill
		jassert(inBuffer.getNumChannels() == 1);
		jassert(outBuffer.getNumChannels() == 2);
		jassert(outputStartSample + numSamples <= outBuffer.getNumSamples());

		//copy mono input to temp buffer
		juce::AudioBuffer<float> tempBuffer (1, numSamples);
		tempBuffer.copyFrom(0,				//destChannel
			                0,				//destStartSample
							inBuffer,		//sourceBuffer
							0,				//sourceChannel
							0,				//sourceStartSample
							numSamples);	//numSamples

		//create context for processing
		juce::dsp::AudioBlock<float> block(tempBuffer);
		juce::dsp::ProcessContextReplacing<float> context(block);
		
		//apply the reverb/filter chain to the context
		chain.process(context);

		//stereoize the output and write it to the buffer
		auto* outL = outBuffer.getWritePointer(0) + outputStartSample;
		auto* outR = outBuffer.getWritePointer(1) + outputStartSample;
		auto* processedOutput = tempBuffer.getReadPointer(0);
		auto* input = inBuffer.getReadPointer(0);
		for (int i = 0; i < numSamples; i++) {
			outL[i] += processedOutput[i] * leftGain;
			outR[i] += processedOutput[i] * rightGain;
		}
	}

	void reset() {
		chain.reset();
	}


private:

	ProcessChain chain;
	juce::dsp::ProcessSpec currentSpec;

	//spatialization parameters
	float leftGain, rightGain;
	float distanceFreqCutoff;
	float reverbMix;
};