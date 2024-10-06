/*
  ==============================================================================

    songCodeCompiler.cpp
    Created: 5 Oct 2024 10:01:56pm
    Author:  Taro

  ==============================================================================
*/

#include "songCodeCompiler.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <random>
#include <cmath>
#include <cstdint>

using namespace std;


void printSongElements(const std::vector<SongElement>& songElements) {
    for (size_t i = 0; i < songElements.size(); ++i) {
        const auto& element = songElements[i];
        std::cout << "Element " << i << ": ";
        
        if (element.type == SongElement::Type::Note) {
            std::cout << "Note - Start Freq: " << element.startFrequency 
                      << " Hz, End Freq: " << element.endFrequency 
                      << " Hz, Duration: " << element.duration << " ms\n";
        } else if (element.type == SongElement::Type::Pattern) {
            std::cout << "Pattern - Beats: ";
            for (const auto& beat : element.beatPattern) {
                std::cout << static_cast<int>(beat) << " ";
            }
            std::cout << "\n";
        }
    }
}


std::vector<SongElement> parseTokens(const std::vector<std::string>& tokens) {
    std::vector<SongElement> songElements;
    float lastFrequency = 0.0f;

    for (const auto& token : tokens) {
        if (token.substr(0, 7) == "pattern") {
            // Parse pattern
            std::vector<uint8_t> pattern;
            std::istringstream iss(token.substr(8, token.length() - 9)); // Remove "pattern(" and ")"
            std::string num;
            while (std::getline(iss, num, ' ')) {
                pattern.push_back(static_cast<uint8_t>(std::stoi(num)));
            }
            songElements.emplace_back(pattern);
        } else {
            // Parse note
            std::istringstream iss(token);
            float endFreq, duration;
            if (!(iss >> endFreq >> duration)) {
                throw std::runtime_error("Invalid note format: " + token);
            }
            songElements.emplace_back(lastFrequency, endFreq, duration);
            lastFrequency = endFreq;
        }
    }

    return songElements;
}



// Helper function to trim leading and trailing whitespace
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}


vector<string> tokenizeByBrackets(const string& str) {
    vector<string> tokens;
    string currentToken;

    for (char ch : str) {
        if (ch == '[' || ch == ']') {
            if (!currentToken.empty()) {
                tokens.push_back(trim(currentToken));
                currentToken.clear();
            }
            tokens.push_back(string(1, ch));
        } else {
            currentToken += ch;
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(trim(currentToken));
    }

    return tokens;
}

// Helper function to resolve rand(min, max)
int resolveRand(const std::string& randToken) {
    // Check if the string starts with "rand(" and ends with ")"
    if (randToken.substr(0, 5) != "rand(" || randToken.back() != ')') {
        throw std::invalid_argument("Invalid rand() format: " + randToken);
    }

    // Extract the range string
    std::string range = randToken.substr(5, randToken.length() - 6);

    // Use stringstream to parse the two numbers
    std::istringstream iss(range);
    int min, max;
    if (!(iss >> min >> max)) {
        throw std::invalid_argument("Invalid rand() format: unable to parse numbers");
    }

    // Check if min is less than or equal to max
    if (min > max) {
        throw std::invalid_argument("Invalid range: min must be less than or equal to max");
    }

    // Use static variables for the random number generator to avoid reinitializing every call
    static std::mt19937 gen(std::time(nullptr));
    static std::uniform_int_distribution<> dis(0, std::numeric_limits<int>::max());

    // Generate a random number within the range
    return min + dis(gen) % (max - min + 1);
}

void recursiveSplitClosedBrackets(const std::string& str, std::vector<std::string>& tokens) {
    size_t end = str.find(']');
    
    if(end == string::npos) {
        tokens.push_back(str);
    } else {
        //recursive case: add everything before end to the token vector, add the close bracket
        //then call recursiveSplit on everything after the close bracket 
        
        tokens.push_back(str.substr(0, end));
        tokens.push_back("]");
        if(end + 2 < str.length()){
            recursiveSplitClosedBrackets(str.substr(end + 2), tokens);    //2 = "]" and the space
        }
    }
}


//splits the song into a list of intermediate tokens. The goal of the first pass is to prepare the 
//list of tokens for expanding the loop. It should split up the loops, and resolve the loop
//iterations if they are a rand. 
//token lists from this function should be guaranteed to contain valid loops (hopefully)

#include <iostream>
#include <vector>
#include <sstream>
#include <regex>

std::vector<std::string> firstPassTokenize(const std::string& song) {
    if (song.length() == 0) {
        return {};
    }

    std::vector<std::string> tokens;
    std::stringstream ss(song);
    std::string token;

    // Tokenize by commas
    while (getline(ss, token, ',')) {
        // Trim spaces
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        // Check for empty tokens
        if (token.empty()) {
            std::cerr << "Error: Empty token found between commas.\n";
            return {}; // Return an empty vector to indicate failure
        }
        tokens.push_back(token);
    }
    
    std::vector<std::string> tokens2;
    for (const std::string& str : tokens) {
        int numClosedBrackets = 0;
        int numOpenBrackets = 0;
        for (char c : str) {
            if (c == '[') numOpenBrackets++;
            if (c == ']') numClosedBrackets++;
        }
        if(numOpenBrackets > 0 || numClosedBrackets > 0){
            auto bracketSeparatedStr = tokenizeByBrackets(str);
            for(string& tok : bracketSeparatedStr){
                tokens2.push_back(tok);
            }
            } else {
                tokens2.push_back(str);
            }
    }
    

    // Check that the tokens are valid for the first pass
    const std::string brackets = R"(\]|\[)";
    const std::string numberNumber = R"(\d+ \d+)";
    const std::string randRand = R"(rand\(\d+ \d+\) rand\(\d+ \d+\))";
    const std::string numberRand = R"(\d+ rand\(\d+ \d+\))";
    const std::string randNumber = R"(rand\(\d+ \d+\) \d+)";
    const std::string justNumber = R"(\d+)";
    const std::string justRand = R"(rand\(\d+ \d+\))";
    const std::string patternBlock = R"(pattern\((?:\d+|rand\(\d+ \d+\))(?: (?:\d+|rand\(\d+ \d+\)))*\))";

    // Unified regex pattern
    const std::regex pattern("^(" + brackets + "|" +
                                    numberNumber + "|" +
                                    randRand + "|" +
                                    numberRand + "|" +
                                    randNumber + "|" +
                                    justNumber + "|" +
                                    justRand + "|" +
                                    patternBlock + ")$");

    for (const auto& t : tokens2) {
        if (!std::regex_match(t, pattern)) {
            // Replace this with something that draws an error message on the UI of the synth later
            std::cerr << "Error: Malformed song component - " << t << std::endl;
            return {};
        }
    }

    std::vector<std::string> tokens3;
    // Resolve the rands in the loop times. Check that all loops have a number of iterations
    for (int i = 0; i < tokens2.size(); i++) {
        const std::string& str = tokens2.at(i);

        if (str == "]") {
            // Check there is an iteration count following the closed bracket
            if (i + 1 == tokens2.size()) {
                std::cerr << "Error: A loop is missing its iteration count. [ ... ] x <--\n";
                return {};
            }

            const std::string& str2 = tokens2.at(i + 1);
            // Valid length elements are justNumber, or justRand
            const std::regex loopIterationPattern("^(" + justNumber + "|" + justRand + ")$");
            if (!std::regex_match(str2, loopIterationPattern)) {
                std::cerr << "Error: A loop has a malformed iteration count. [ ... ] x <-- You have: " << str2 << ". It must be either a number or a rand(x)." << std::endl;
                return {};
            }
            // Loop iteration is valid, handle the two cases
            const std::regex justNumberPattern("^(" + justNumber + ")$");
            if (std::regex_match(str2, justNumberPattern)) {
                tokens3.push_back("]");
                tokens3.push_back(str2);
                i++;
            } else {
                tokens3.push_back("]");
                tokens3.push_back(str2);
                i++;
            }
        } else {
            tokens3.push_back(str);
        }
    }

    return tokens3;
}

//expand all the loops iteratively. Work from the outer to the inner.
vector<string> secondPassTokenize(const vector<string>& tokens) {
    
    vector<string> tempTokens(tokens);

    
    while(true){
        //check to see if there are loops remaining.
        int numLoops = 0;
        for(int i = 0; i < tempTokens.size(); i++){
            if(tempTokens[i] == "["){
                numLoops++;
            }
        }
        
        if(numLoops == 0){
            break;
        }
        
        int loopStartInd = 0;
        int loopEndInd = 0;
        int loopIterationCount = 0;
        
        //find first loop start point
        for(int i = 0; i < tempTokens.size(); i++){
            auto curTok = tempTokens[i];
            if(curTok == "["){
                loopStartInd = i;
                break;
            }
        }
        
        //find the loop end point that matches with the start point
        int innerBracketCounter = 1;
        for(int i = loopStartInd + 1; i < tempTokens.size(); i++){
            auto curTok = tempTokens[i];
            if(curTok == "[") innerBracketCounter++;
            if(curTok == "]") innerBracketCounter--;
            
            if(innerBracketCounter == 0){
                loopEndInd = i;
                break;
            }
        }
        
        //find number of iterations for this loop
        //resolve rands to a number if there is one
        const std::string justNumber = R"(\d+)";
        const std::regex justNumberPattern("^(" + justNumber + ")$");
        auto loopIterationToken = tempTokens[loopEndInd + 1];
        if (regex_match(loopIterationToken, justNumberPattern)) {
            loopIterationCount = stoi(loopIterationToken);
        } else {
            try {
                loopIterationCount = resolveRand(loopIterationToken);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: a rand isn't in the correct format: rand(min max)" << std::endl;
                return {};
            }
        }
        
        //copy everything before the loop starts into the return vector
        vector<string> rvector;
        for(int i = 0; i < loopStartInd; i++){
            rvector.push_back(tempTokens[i]);
        }
        
        //copy over the iterations
        for(int j = loopIterationCount; j > 0; j--){
            for(int i = loopStartInd + 1; i < loopEndInd; i++){
                rvector.push_back(tempTokens[i]);
            }
        }
        
        //copy over the stuff after the loop
        for(int i = loopEndInd + 2; i < tempTokens.size(); i++){
            rvector.push_back(tempTokens[i]);
        }
        
        tempTokens = rvector;
    }
    
    
    return tempTokens;
}

//The tokens are now all of the form 
// number number
// number rand(a b)
// rand(a b) number
// rand(a b) rand(b c)
// pattern( ... )

vector<string> thirdPassTokenize(const vector<string>& tokens)
{
    vector<string> tokens2;
    //loop through each token, resolve the randoms it contains.
    //each token can have an indeterminate amount of rands, since pattern can have many
    for(int i = 0; i < tokens.size(); i++){
        string str = tokens[i];
                
        //iteratively resolve all rands in this token
        string::size_type pos = 0;

        if((pos = str.find("rand", pos)) != string::npos){
            //at least one rand found. Iterate through string and resolve it and any others  
            while ((pos = str.find("rand", pos)) != string::npos) {
                int endPos = str.find(")", pos);
                
                //extract and resolve the rand
                string extractedRand = str.substr(pos, endPos + 1 - pos);
                int resolvedValue =  resolveRand(str.substr(pos, endPos + 1 - pos));
                
                //update the string and move the pos past the value we finished
                string resolvedValueString = to_string(resolvedValue);
                str = str.substr(0, pos) + resolvedValueString + str.substr(endPos + 1, string::npos);
                pos = pos + resolvedValueString.length(); // Move past the new text
            }
            tokens2.push_back(str);
        } else {
            //no rand found
            tokens2.push_back(str);
        }
    }
    return tokens2;
}


vector<string> tokenize(const string& song) 
{   
    vector<string> firstPassTokens = firstPassTokenize(song);
    if (firstPassTokens.empty()) return {};  // Error handling: Return an empty song on error
    vector<string> secondPassTokens = secondPassTokenize(firstPassTokens);
    if(secondPassTokens.empty()) return {};
    vector<string> thirdPassTokens = thirdPassTokenize(secondPassTokens);
    if(thirdPassTokens.empty()) return {};
    return thirdPassTokens; 
}


std::vector<SongElement> compileSongcode(const std::string& songcode) {
    std::vector<std::string> tokens = tokenize(songcode);
    if (tokens.empty()) {
        // Handle error: return an empty vector or throw an exception
        return {};
    }
    return parseTokens(tokens);
}