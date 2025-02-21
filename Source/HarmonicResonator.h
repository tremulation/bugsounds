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
#include <algorithm>

//not a helmholtz resonator anymore. 
//bank of resonators, one on the fundamental, and a couple more on the harmonics.
//based on the max patch in prototypes
class HarmonicResonator {
public:
    HarmonicResonator() {
        reset();
    }

    void reset() {
        for (int i = 0; i < 8; i++) {
            delays[i][0] = 0.0f;
            delays[i][1] = 0.0f;
        }
    }

    void prepareToPlay(double sampleRate) {
        this->sampleRate = sampleRate;
        reset();
    }


    float processSamples(float input, float freq) {
        if (freq <= 0.0f || sampleRate <= 0.0f) return 0.0f;

        float twopi = juce::MathConstants<float>::twoPi;
        float totalOut = 0.0f;
        float sumWeights = 0.0f;

        for (int n = 1; n <= overtoneNum; ++n) {
            float currentFreq = freq * n;
            if (currentFreq >= sampleRate / 2) break;

            float bandwidthRadians = (bandwidth / sampleRate) * twopi;
            float peakRadius = 1.0f - (bandwidthRadians / 2.0f);
            float freqRadians = (currentFreq / sampleRate) * twopi;
            float peakLocation = std::acos((2.0f * peakRadius * std::cos(freqRadians)) / (1.0f + peakRadius * peakRadius));
            float normFactor = (1.0f - peakRadius * peakRadius) * std::sin(peakLocation) * 1.4f;

            float weight = 1.0f / n;
            sumWeights += weight;

            float& h1 = delays[n - 1][0];
            float& h2 = delays[n - 1][1];

            float out_n = (input * normFactor) + (2.0f * peakRadius * std::cos(peakLocation) * h2) - (h1 * peakRadius * peakRadius);

            h1 = h2;
            h2 = out_n;

            totalOut += out_n * weight;
        }

        return totalOut * (gain / sumWeights); // Normalize output
    }

    //parameters. set them in rendernextblock
    float bandwidth = 100.0f; //bandwidth: peak sharpness, in Hz. 0 to 500. 100 default.
    int   overtoneNum = 3;      //number of overtone resonators to add. 1 to 8. default 1
    float gain = 1.0f;   //output gain. 0 to 1, default 0.5
    double sampleRate = 44100.0f;

private:


    //2d array for storing all the delay elements:
    //1st element is the fundamental, with the rest being harmonics above the fundamental
    //2*fundamental, 3*fundamental, 4*fundamental, etc.
    float delays[8][2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonicResonator)
};