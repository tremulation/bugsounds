/*
  ==============================================================================

    Evaluator.cpp
    Created: 30 Jan 2025 4:02:33pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Evaluator.h"
#include "SongCodeCompiler.h"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>


using namespace std;





//--------------------- LEXER ---------------------\\

enum class TokenType {
Num,
Let,
Id,
Equals,
Rand,
Add,
Sub,
Div,
Mul,
LStart,
LEnd,
Comma,
ParStart,
ParEnd,
Pattern,
Comment
};


struct Token {
    TokenType type;
    int numValue;
    std::string idValue;    //variable name, if a var
    std::string text;       //exact text of the token
    size_t startPos;
    size_t endPos;

    explicit Token(TokenType t, size_t s, size_t e, const string& txt) : 
        type(t), numValue(0), idValue(""), startPos(s), endPos(e), text(txt) {}
    Token(TokenType t, int num, size_t s, size_t e, const string& txt) : 
        type(t), numValue(num), idValue(""), startPos(s), endPos(e), text(txt) {}
    Token(TokenType t, const std::string& id, size_t s, size_t e, const string& txt) : 
        type(t), numValue(0), idValue(id), startPos(s), endPos(e), text(txt) {}
};





class SongCodeLexer {
    std::string input;
    size_t pos = 0;


    bool parseNumber(std::vector<Token>& tokens, ErrorInfo* errorInfo) {
        size_t start = pos;
        //consume consecutive nums
        while (pos < input.size() && std::isdigit(input[pos])) pos++;

        //should never occur
        if (pos == start) {
            setErrorInfo(errorInfo, "Error: Empty number", start, pos, "");
            return false;
        }
        
        std::string numStr = input.substr(start, pos - start);
        try {
            int num = std::stoi(numStr);
            tokens.emplace_back(TokenType::Num, num, start, pos - 1, numStr);
            return true;
        }
        catch (...) {
            setErrorInfo(errorInfo, "Error: Invalid number format " + numStr, start, pos - 1, numStr);
            return false;
        }
    }


public:


    explicit SongCodeLexer(const std::string& input) : input(input) {}

    std::vector<Token> tokenize(ErrorInfo* errorInfo) {
        std::vector<Token> tokens;

        while (pos < input.size()) {
            //skip whitespace
            while (pos < input.size() && std::isspace(input[pos])) pos++;
            if (pos >= input.size()) break;

            const size_t tokenStart = pos;
            char current = input[pos];

            //handle comments
            if (current == '$') {
                size_t start = pos++;
                bool closed = false;

                while (pos < input.size()) {
                    if (input[pos] == '$') {
                        closed = true;
                        pos++;
                        break;
                    }
                    pos++;
                }

                if (!closed) {
                    setErrorInfo(errorInfo, "Error: Unclosed comment", tokenStart, input.size() - 1, "");
                    return {};
                }
                std::string commentText = input.substr(start, pos - start);
                tokens.emplace_back(TokenType::Comment, start, pos - 1, commentText);
                continue;
            }

            //handle numbers
            if (std::isdigit(current)) {
                if (!parseNumber(tokens, errorInfo)) return {};
                continue; //parseNumber implicitly advances pos, so you must skip
            }

            //handle keywords/variables
            else if (std::isalpha(current)) {
                size_t start = pos;

                //gobble up chars
                while (pos < input.size() && std::isalnum(input[pos])) pos++;

                std::string word = input.substr(start, pos - start);
                const size_t tokenEnd = pos - 1;

                //match keyword strings
                if (word == "pattern") {
                    tokens.emplace_back(TokenType::Pattern, start, tokenEnd, word);
                }
                else if (word == "rand") {
                    tokens.emplace_back(TokenType::Rand, start, tokenEnd, word);
                }
                else if (word == "let") {
                    tokens.emplace_back(TokenType::Let, start, tokenEnd, word);
                }
                else {
                    //no match, must be var
                    tokens.emplace_back(TokenType::Id, word, start, tokenEnd, word);
                }

                continue;
            }

            //handle symbols
            else {
                switch (current) {
                    case '=': tokens.emplace_back(TokenType::Equals, pos, pos, "="); break;
                    case '+': tokens.emplace_back(TokenType::Add, pos, pos, "+"); break;
                    case '*': tokens.emplace_back(TokenType::Mul, pos, pos, "*"); break;
                    case '-': tokens.emplace_back(TokenType::Sub, pos, pos, "-"); break;
                    case '/': tokens.emplace_back(TokenType::Div, pos, pos, "/"); break;
                    case '[': tokens.emplace_back(TokenType::LStart, pos, pos, "["); break;
                    case ']': tokens.emplace_back(TokenType::LEnd, pos, pos, "]"); break;
                    case ',': tokens.emplace_back(TokenType::Comma, pos, pos, ","); break;
                    case '(': tokens.emplace_back(TokenType::ParStart, pos, pos, "("); break;
                    case ')': tokens.emplace_back(TokenType::ParEnd, pos, pos, ")"); break;
                    default: {
                        std::string charStr(1, current);
                        setErrorInfo(errorInfo, "Error: Unexpected character: " + charStr, pos, pos, charStr);
                        return {};
                    }
                }
            }
            pos++;
        }

        //remove comments from the final token stream
        tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
            [](const Token& t) {return t.type == TokenType::Comment;  }), tokens.end());

        return tokens;
    }


    std::string tokenTypeToString(TokenType type) {
        switch (type) {
        case TokenType::Num:     return "Num";
        case TokenType::Let:     return "Let";
        case TokenType::Id:      return "Id";
        case TokenType::Equals:  return "Equals";
        case TokenType::Rand:    return "Rand";
        case TokenType::Add:     return "Add";
        case TokenType::Sub:     return "Sub";
        case TokenType::Div:     return "Div";
        case TokenType::Mul:     return "Mul";
        case TokenType::LStart:  return "LStart";
        case TokenType::LEnd:    return "LEnd";
        case TokenType::Comma:   return "Comma";
        case TokenType::ParStart:return "ParStart";
        case TokenType::ParEnd:  return "ParEnd";
        case TokenType::Pattern: return "Pattern";
        default:                 return "Unknown";
        }
    }

    void printTokens(const std::vector<Token>&tokens) {
        for (const auto& token : tokens) {
            std::string str = tokenTypeToString(token.type);
            //add value information where applicable
            switch (token.type) {
            case TokenType::Num:
                str += "(" + std::to_string(token.numValue) + ")";
                break;
            case TokenType::Id:
                str += "('" + token.idValue + "')";
                break;
            default:
                break;
            }
            //add in start/end indices
            str += "Orig: " + token.text + " ";
            str += "(" + std::to_string(token.startPos) + ", " + std::to_string(token.endPos) + ").";
            juce::Logger::writeToLog(str);
        }
    }
};



std::vector<SongElement> evaluateSongString(std::string& songcode,
                                            ErrorInfo* errorInfo,
                                            std::map<std::string, float>* vars) {
    SongCodeLexer lexer(songcode);
    auto lexerToks = lexer.tokenize(errorInfo);
    juce::Logger::writeToLog(errorInfo->message);
    juce::Logger::writeToLog("-------------First pass-------------" + juce::String(lexerToks.size()));
    if (lexerToks.empty()) return {};
    
    lexer.printTokens(lexerToks);

    return {};
}