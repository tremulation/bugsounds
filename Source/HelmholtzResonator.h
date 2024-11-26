/*
  ==============================================================================

    HelmholtzResonator.h
    Created: 24 Nov 2024 11:46:41pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//digital biquad filter
class HelmholtzResonator {
public:
    HelmholtzResonator() {
        reset();
    }


    void reset() {
        z1 = 0.0f;
        z2 = 0.0f;
    }


    void prepareToPlay(double sampleRate) {
        this->sampleRate = sampleRate;
        reset();
    }


    float processSamples(float input, float freq) {
        freq = juce::jlimit(20.0f, static_cast<float>(sampleRate / 2.0f), freq);

        //calculate coefficient values
        const float w0 = juce::MathConstants<float>::twoPi * freq / static_cast<float>(sampleRate);
        const float sinW0 = std::sin(w0);
        const float cosW0 = std::cos(w0);
        const float alpha = sinW0 / (2.0f * Q);

        //calculate normalized filter coefficients
        const float b0 = alpha / (1.0f + alpha);
        const float b1 = 0.0f;
        const float b2 = -alpha / (1.0f + alpha);
        const float a1 = -2.0f * cosW0 / (1.0f + alpha);
        const float a2 = (1.0f - alpha) / (1.0f + alpha);

        //process sample w/ direct form II
        const float v = input - a1 * z1 - a2 * z2;
        const float output = b0 * v + b1 * z1 + b2 * z2;

        //update state
        z2 = z1; 
        z1 = v;

        return output;
    }

    float Q = 10.0f;    //peak sharpness

private:
    double sampleRate = 44100.0f;

    //two delay elements
    float z1 = 0.0f;
    float z2 = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelmholtzResonator)
};