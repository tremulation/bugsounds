/*
  ==============================================================================

    songCodeCompiler.h
    Created: 5 Oct 2024 8:43:02pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <vector>
#include <string>
#include <iostream>
#include <map>

struct SongElement {
    enum class Type {
        Note,
        Pattern
    };

    Type type;

    // For notes
    float startFrequency = -1.0;
    float endFrequency = -1.0;
    float duration = -1.0;

    std::string toString() {
        if (type == Type::Note) {
            return "Note: (freq:" + std::to_string(static_cast<int>(startFrequency)) + "-" + std::to_string(static_cast<int>(endFrequency)) +
                ", len: " + std::to_string(duration) + ")";
        }
        else {
            std::string patternAcc = "";
			for (auto& i : beatPattern) {
				patternAcc += std::to_string(static_cast<int>(i)) + " ";
			}
			return "Pattern: (" + patternAcc + ")";
        }
    }

    // For patterns
    std::vector<uint8_t> beatPattern;

    SongElement(float start, float end, float dur)
        : type(Type::Note), startFrequency(start), endFrequency(end), duration(dur) {}

    SongElement(const std::vector<uint8_t>& pattern)
        : type(Type::Pattern), beatPattern(pattern) {}
};