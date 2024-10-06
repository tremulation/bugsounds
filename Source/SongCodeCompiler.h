/*
  ==============================================================================

    songCodeCompiler.h
    Created: 5 Oct 2024 8:43:02pm
    Author:  Taro

  ==============================================================================
*/

#pragma once


#include <vector>
#include <string>
#include <iostream>

struct SongElement {
    enum class Type {
        Note,
        Pattern
    };

    Type type;

    // For notes
    float startFrequency;
    float endFrequency;
    float duration;

    // For patterns
    std::vector<uint8_t> beatPattern;

    SongElement(float start, float end, float dur)
        : type(Type::Note), startFrequency(start), endFrequency(end), duration(dur) {}

    SongElement(const std::vector<uint8_t>& pattern)
        : type(Type::Pattern), beatPattern(pattern) {}
};

void printSongElements(const std::vector<SongElement>& songElements);
std::vector<SongElement> compileSongcode(const std::string& songcode);
