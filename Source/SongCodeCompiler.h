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
    enum Type {
        Note,
        Pattern,
        SubclickPattern,
        None
    };


    Type type = Type::None;
    //for notes
    float startFrequency = -1.0;
    float endFrequency = -1.0;
    float duration = -1.0;
    //for patterns (both clickpattern and subclickpattern)
    std::vector<int> beatPattern;

    SongElement() = default;

    SongElement(float start, float end, float dur)
        : type(Type::Note), startFrequency(start), endFrequency(end), duration(dur) {}

    SongElement(Type pType, std::vector<int> pattern) : type(pType), beatPattern(pattern) {}


    std::string toString() {
        if (type == Type::Note) {
            return "Note: (freq:" + std::to_string(static_cast<int>(startFrequency)) + "-" + std::to_string(static_cast<int>(endFrequency)) +
                ", len: " + std::to_string(duration) + ")";
        }
        else if (type == Type::Pattern) {
            std::string patternAcc = "";
            for (auto& i : beatPattern) patternAcc += std::to_string(static_cast<int>(i)) + " ";
            return "Pattern: (" + patternAcc + ")";
        }
        else if (type == Type::SubclickPattern) {
            std::string patternAcc = "";
            for (auto& i : beatPattern) patternAcc += std::to_string(static_cast<int>(i)) + " ";
            return "Pubclick pattern: (" + patternAcc + ")";
        }
        else {
			return "None";
        }
    }
};


//for adding metadata to the song past the list of elements
struct extraSongInfo {
    bool hasSection = false;
    int sectionStartInd = -1;
    int sectionEndInd = -1;
};