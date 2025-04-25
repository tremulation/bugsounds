/*
  ==============================================================================

    HarmonicResonator.h
    Created: 24 Nov 2024 11:46:41pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <vector>
#include <algorithm>

//not a helmholtz resonator anymore. 
//based on the max patch in prototypes
class HarmonicResonator {
public:

    HarmonicResonator() : fourierf((int)std::log2(windowSize)), fourieri((int)std::log2(windowSize)),
        windowFunction(windowSize, juce::dsp::WindowingFunction<float>::hann, false, 0.0f) {
        reset();
        
    }
	//==============================================================================================

    //takes in the input samples from the synth, buffers it, and returns a sample from the processed buffer
    float processSample(float sample, float freq) {
        //retrieve one processed sample
		float outputSample = bufout[writehead];
        bufout[writehead] = 0.0f;

        //push the input sample to the circular buffer
        buf[playhead] = sample;

        //increment pointers and hop counter
        playhead++;
		writehead++;
		window++;

        if (window == windowSize / 4) {
            window = 0;
            //do the processing. load the resulting processed block into bufout
            processBlock(freq);
        }
        
		if (playhead == windowSize) {
			playhead = 0;
		}
		if (writehead == windowSize) {
			writehead = 0;
		}

        return outputSample;
    }

    //do this only once a block, to ensure efficiency
    void loadParams() {
        //load parameters
        overtoneDecay = *apvts->getRawParameterValue("Resonator Overtone Decay");
        n = *apvts->getRawParameterValue("Resonator Overtone Number");
        q = *apvts->getRawParameterValue("Resonator Q");
        gain = *apvts->getRawParameterValue("Resonator Gain");
        originalScalar = *apvts->getRawParameterValue("Resonator Original Mix");
    }


    void processBlock(int funFreq) {
        //temporary buffers for processing
        float segmented[windowSize * 2];
		float mag[windowSize], phase[windowSize];
        std::fill(segmented, segmented + (windowSize * 2), 0.0f);
        
        //extract a window-sized segment from the circular buffer
		//need 2 loops to handle the wrap-around
        int counter = 0;
		for (int i = playhead; i < windowSize; i++) {
			segmented[counter] = buf[i];
			counter++;
		}
        for (int i = 0; i < playhead; i++) {
            segmented[counter] = buf[i];
            counter++;
        }

		//apply windowing function
        windowFunction.multiplyWithWindowingTable(segmented, windowSize);

        //perform  forward FFT
		fourierf.performRealOnlyForwardTransform(segmented);
        for (int i = 0; i < windowSize; i++) {
            std::complex<float> temp;
			temp.real(segmented[i * 2]);
			temp.imag(segmented[i * 2 + 1]);

			mag[i] = std::abs(temp);
			phase[i] = std::arg(temp);
        }

		//calculate the amplitude scaling for each frequency bin here. see prototype max patch for reference
		//this is the part of the resonator that actually does the resonating
        for (int i = 0; i < windowSize / 2; i++) {
            float binFreq = (float)i * samplerate / windowSize;

            float highestScalar = 0.0f;  //store the max peak gain per frequency bin


            for (int harmonic = 1; harmonic <= n; harmonic++) {  //fundamental, followed by harmonics
                float overtoneScaler = (harmonic == 1) ? 1.0f : std::pow(overtoneDecay, harmonic - 1);

                float scaledOffset = (2 * (binFreq - funFreq * harmonic)) / q;
                float thisPeakGain = originalScalar + (1.0f / (1.0f + (scaledOffset * scaledOffset))) * gain * overtoneScaler;

                //highestScalar = std::max(highestScalar, thisPeakGain);  //keep the highest gain
                highestScalar += thisPeakGain;
            }

            mag[i] *= highestScalar;
            //scaling the phase too causes an extremely strange, atmospheric effect that I can't describe. 
            //use this for the standalone resonator plugin
            //phase[i] *= highestScalar;
        }

        //perform inverse FFT to get the original signal
        for (int i = 0; i < windowSize; i++) {
            float real = std::cos(phase[i]) * mag[i];
            float imag = std::sin(phase[i]) * mag[i];

            segmented[i * 2] = real;
            segmented[i * 2 + 1] = imag;
        }

        fourieri.performRealOnlyInverseTransform(segmented);

        //apply windowing again (i think this is optional but idk)
		windowFunction.multiplyWithWindowingTable(segmented, windowSize);

        //add the output to the circular output buffer
        for (int i = 0; i < windowSize; i++) {
            if (i < (windowSize / 4) * 3) { //75% overlap. 
				bufout[(writehead + i) % windowSize] += segmented[i] / (2.f / 3.f);
            }
            else {
                bufout[(writehead + i) % windowSize] = segmented[i] / (2.f / 3.f);
            }
        }
    }


    //==============================================================================================

    void prepareToPlay(int samplerate) {
        this->samplerate = samplerate;
        reset();
    }

    void reset() {
		writehead = 0;
		playhead = 0;
	}

    void setAPVTS(juce::AudioProcessorValueTreeState* apvtsPtr) {
        apvts = apvtsPtr;
    }

	//================================================================================================

    //filter parameters
    int samplerate = 44100;

    //FFT parameters
    static constexpr int windowSize = 512;
    static constexpr int overlapFactor = 4;
    int hopSize = windowSize / overlapFactor;
	float bufout[windowSize], buf[windowSize];
    int writehead = 0, playhead = 0, window = 0;
    int hopCounter = 0;

private:
    //objects
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    juce::dsp::FFT fourierf, fourieri;
    juce::dsp::WindowingFunction<float> windowFunction;

    /// APVT paramers
    float overtoneDecay = 0.0f;
    float n = 0.0f;
    float q = 0.0f;
    float gain = 0.0f;
    float originalScalar = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicResonator)
};