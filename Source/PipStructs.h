/*
  ==============================================================================

    PipStructs.h
    Created: 6 Nov 2024 7:46:25pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <stdint.h>

struct Pip {
    float frequency;        // Hz
    int  length;        // microseconds
    int  tail;          // microseconds
    float level;            // 0 to 1


    Pip() : frequency(3520.0f), length(1000000), tail(500000), level(0.5f) {}


    Pip(float f, int l, int t, float lv)
        : frequency(f), length(l), tail(t), level(lv) {}


    Pip(const Pip& other) = default;


    Pip& operator=(const Pip& other) = default;
};

namespace PipConstants {
    const float MIN_FREQUENCY = 20.0f;     // Hz - lowest audible frequency
    const float MAX_FREQUENCY = 20000.0f;  // Hz - upper limit of human hearing

    const int MIN_LENGTH = 1;           // microseconds - shortest duration (1µs)
    const int MAX_LENGTH = 100000;   // microseconds - max duration (100ms)

    const int MIN_TAIL = 0;             // microseconds - no tail (0ns)
    const int MAX_TAIL = 100000;     // microseconds - max tail (100ms)

    const float MIN_LEVEL = 0.0f;           // silent
    const float MAX_LEVEL = 1.0f;           // maximum volume
}

enum EditingMode {
    FREQUENCY, 
    LENGTH,
    TAIL,
    LEVEL
};