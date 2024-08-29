#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <random>
#include <cmath>

using namespace std;


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
    //extract the two numbers from the rand statement
    size_t randStart = randToken.find("rand(");
    size_t randEnd = randToken.find(")");
    if (randStart == std::string::npos || randEnd == std::string::npos) {
        throw std::invalid_argument("Invalid rand() format");
    }

    std::string range = randToken.substr(randStart + 5, randEnd - randStart - 5);
    size_t separator = range.find(' ');

    if (separator == std::string::npos) {
        throw std::invalid_argument("Invalid rand() format");
    }

    float min = std::stoi(range.substr(0, separator));
    float max = std::stoi(range.substr(separator + 1));
    
    static std::mt19937 gen(std::time(0));
    static std::uniform_real_distribution<> dis(0.0, 1.0);

    // Generate a random float between 0 and 1
    double randomFloat = dis(gen);

    // Scale and translate to the desired range
    return min + static_cast<int>(randomFloat * (max - min + 1));
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

    // Further tokenization: move brackets and loop times to their own lines
    // std::vector<std::string> tokens2;
    // for (const std::string& str : tokens) {
    //     int numClosedBrackets = 0;
    //     int numOpenBrackets = 0;
    //     for (char c : str) {
    //         if (c == '[') numOpenBrackets++;
    //         if (c == ']') numClosedBrackets++;
    //     }
    //     cout << "Closed bracckets: " << numClosedBrackets << endl;
    //     cout << "Open Brackets: " << numOpenBrackets << endl;
    //     if ((numClosedBrackets == 0) && (numOpenBrackets == 0)) {
    //         tokens2.push_back(str);
    //     } else if (numOpenBrackets == 1) {
    //         // Handle open brackets: move them to their own line
    //         size_t openBracketPos = str.find('[');
    //         if (openBracketPos != 0) {
    //             std::cerr << "Error: Mismatched brackets in the input song notation.\n";
    //             return {}; // Return an empty vector to indicate failure
    //         }
    //         tokens2.push_back("[");
    //         tokens2.push_back(str.substr(1, std::string::npos));
    //     } else if (numClosedBrackets > 0) {
    //         // Handle multiple closing brackets using recursion
    //         cout << str << endl;
    //         recursiveSplitClosedBrackets(str, tokens2);
    //     }
    // }
    
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


vector<string> tokenize(const string& song) 
{   
    vector<string> firstPassTokens = firstPassTokenize(song);
    if (firstPassTokens.empty()) return {};  // Error handling: Return an empty song on error
    vector<string> secondPassTokens = secondPassTokenize(firstPassTokens);

    return secondPassTokens; 
}

int main() 
{
    string songInput;
    
    while(false != true) {
        cout << "Enter the notation: ";
        getline(cin, songInput);
    
        vector<string> tokens = tokenize(songInput);
    
        if (!tokens.empty()) {
            cout << "TOKENIZED:\n";
            for (size_t i = 0; i < tokens.size(); ++i) {
                cout << i << ": " << tokens[i] << '\n';
            }
        }
    }
    return 0;
}
