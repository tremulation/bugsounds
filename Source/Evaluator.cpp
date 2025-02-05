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





/*
-------------------============ LEXER CLASS ============-------------------
Turns the raw songcode string into a vector of tokens for the parser
*/

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


/* 
-------------------============ PARSER CLASS ============-------------------
Parses the token stream from the lexer into an AST that can be sent to the synth and evaluated on the fly
operates on this context-free-grammar:

    Script 		-> Statement*
    Statement 	-> Note COMMA | Pattern COMMA | Declaration COMMA | Loop COMMA
    Note		-> AdditiveExpr AdditiveExpr
    Pattern		-> PATTERN PARSTART AdditiveExpr* PAREND
    Declaration -> LET ID EQUALS AdditiveExpr
    Loop 		-> BARSTART Statement* BAREND AdditiveExpr
    AdditiveExpr-> MultiplicativeExpr (ADDOPERATOR AdditiveExpr)?
    MultiplicativeExpr -> PrimaryExpr (MULTOPERATOR MultiplicativeExpr)?
    PrimaryExpr -> INT | ID | Random | PARSTART AdditiveExpr PAREND
    Random      -> RAND PARSTART AdditiveExpr AdditiveExpr PAREND

All functions operate on a shared state set in the constructor, to minimize the amount
of arguments I have to pass over and over
*/

//--------------------------------- HELPERS ---------------------------------

//returns next token in the stream. 
//optional offset argument for checking tokens past the next one
std::optional<Token> Parser::lookahead(size_t offset) const {
    if (it + offset < end) {
        return *(it + offset);
    }
    return std::nullopt;
}

//checks if the next token is the expected type. if so, consume it, and return true
//else return false and don't change the tokens
bool Parser::match_token(TokenType expected) {
    if (lookahead().has_value() && lookahead()->type == expected) {
        *it++;
        return true;
    }
    return false;
}


//--------------------------------- PARSING FUNCTIONS -----------------------

//top level parsing function, equivalent to parse_script in the CFG
ScriptPtr Parser::parse() {
    while (it != end) {
        //error is set in lesser parsing functions
        bool successful = parse_statement();
        if (!successful) return {};
    }
    return AST;
}


bool Parser::parse_statement() {
    bool successful = false;
    optional<Token> nextTok = lookahead();
    if (!nextTok.has_value()) return true;

    TokenType ntt = nextTok.value().type;
    if (ntt == TokenType::Num || ntt == TokenType::Id || ntt == TokenType::Rand || ntt == TokenType::ParStart) {
        successful = parse_note();
    }
    else if (ntt == TokenType::Pattern) {
        successful = parse_pattern();
    }
    else if (ntt == TokenType::Let) {
        successful = parse_let();
    }
    else if (ntt == TokenType::LStart) {
        successful = parse_loop();
    }
    else {
        int ntStart = nextTok.value().startPos;
        int ntEnd   = nextTok.value().endPos;
        setErrorInfo(errorInfo, "Error: expected a number, pattern, let, or loop", ntStart, ntEnd, "");
        return false;
    }

    //I don't want to enforce required comma as last character, so:
    //If there are at a couple tokens left, then we need a comma as our direct lookahead
    if (successful) {
        if (lookahead(1).has_value() && !match_token(TokenType::Comma)) {
        nextTok = lookahead();  //has probably changed by now.
        int ntStart = nextTok.value().startPos;
        int ntEnd = nextTok.value().endPos;
        setErrorInfo(errorInfo, "Error: expected a number, pattern, let, or loop", ntStart, ntEnd, "");
        }
    }
}

//start by writing a working parse_note, then do the rest of the symbols
bool Parser::parse_note() {
    return true;
}
bool Parser::parse_pattern() {
    return true;
}
bool Parser::parse_let() {
    return true;
}
bool Parser::parse_loop() {
    return true;
}




namespace {
    std::string statementToString(const StatementPtr& stmt);
    std::string exprToString(const ExprPtr& expr);
}

std::string astToString(const ScriptPtr& script) {
    if (!script) return "Empty AST";

    std::stringstream ss;
    int count = 1;
    for (const auto& stmt : script->statements) {
        ss << count++ << ". " << statementToString(stmt) << "\n";
    }
    return ss.str();
}

namespace {
    std::string statementToString(const StatementPtr& stmt) {
        if (auto note = dynamic_cast<NoteNode*>(stmt.get())) {
            return "Note(freq: " + exprToString(note->frequency) +
                ", dur: " + exprToString(note->duration) + ")";
        }
        if (auto pattern = dynamic_cast<PatternNode*>(stmt.get())) {
            std::string elements;
            for (const auto& expr : pattern->subBeats) {
                if (!elements.empty()) elements += ", ";
                elements += exprToString(expr);
            }
            return "Pattern[" + elements + "]";
        }
        if (auto loop = dynamic_cast<LoopNode*>(stmt.get())) {
            std::string body;
            for (const auto& s : loop->body) {
                body += "\n    " + statementToString(s);
            }
            return "Loop[iterations: " + exprToString(loop->iterations) + "]" + body;
        }
        if (auto let = dynamic_cast<LetNode*>(stmt.get())) {
            return "Let(" + let->id + " = " + exprToString(let->value) + ")";
        }
        return "Unknown Statement";
    }

    std::string exprToString(const ExprPtr& expr) {
        if (!expr) return "null";

        if (auto add = dynamic_cast<AdditiveExprNode*>(expr.get())) {
            char op = (add->op == AdditiveExprNode::Add) ? '+' : '-';
            return "(" + exprToString(add->left) + " " + op + " " + exprToString(add->right) + ")";
        }
        if (auto mul = dynamic_cast<MultiplicativeExprNode*>(expr.get())) {
            char op = (mul->op == MultiplicativeExprNode::Multiply) ? '*' : '/';
            return "(" + exprToString(mul->left) + " " + op + " " + exprToString(mul->right) + ")";
        }
        if (auto prim = dynamic_cast<PrimaryExprNode*>(expr.get())) {
            switch (prim->kind) {
            case PrimaryExprNode::Integer:
                return std::to_string(prim->integerValue);
            case PrimaryExprNode::Variable:
                return prim->variableName;
            case PrimaryExprNode::Grouped:
                return "(" + exprToString(prim->groupedExpr) + ")";
            }
        }
        if (auto rand = dynamic_cast<RandomNode*>(expr.get())) {
            return "rand(" + exprToString(rand->min) + ", " + exprToString(rand->max) + ")";
        }
        return "?";
    }
}


ExprNode* createExprRecursive(int depth = 0) {
    const int maxDepth = 3;

    //base case: return random int
    if (depth >= maxDepth || (rand() % 2 == 0)){
        int value = rand() % 100; // integer literal between 0 and 99
        return new PrimaryExprNode(value);
    }

    //recursive case: randomly choose expr type and descend
    int choice = rand() % 3;
    if (choice == 0){
        //additive expression: (left + right) or (left - right)
        ExprNode* left = createExprRecursive(depth + 1);
        ExprNode* right = createExprRecursive(depth + 1);
        auto op = (rand() % 2 == 0) ? AdditiveExprNode::Add : AdditiveExprNode::subtract;
        return new AdditiveExprNode(left, right, op);
    } else if (choice == 1){
        //multiplicative expression: (left * right) or (left / right)
        ExprNode* left = createExprRecursive(depth + 1);
        ExprNode* right = createExprRecursive(depth + 1);
        auto op = (rand() % 2 == 0) ? MultiplicativeExprNode::Multiply : MultiplicativeExprNode::Divide;
        return new MultiplicativeExprNode(left, right, op);
    } else {
        //parenthesized expression
        ExprNode* inner = createExprRecursive(depth + 1);
        return new PrimaryExprNode(inner);
    }
}


ScriptPtr createRandomAST() {
	auto script = new ScriptNode();
	int numStatements = rand() % 4 + 3;

	for (int i = 0; i < numStatements; ++i) {
		int choice = rand() % 4; // 0: Let, 1: Note, 2: Pattern, 3: Loop
		if (choice == 0) { //letNode 
			ExprNode* expr = createExprRecursive();
			std::string varName = "var" + std::to_string(i);
			script->statements.push_back(new LetNode(varName, expr));   
			script->variables[varName] = rand() % 100;
		} else if (choice == 1) //noteNode
		{
			ExprNode* freq = createExprRecursive();
			ExprNode* dur = createExprRecursive();
			script->statements.push_back(new NoteNode(freq, dur));
		} else if (choice == 2) { //patternNode
			auto pattern = new PatternNode();
			int numBeats = rand() % 4 + 2;
			for (int j = 0; j < numBeats; ++j) {
				pattern->subBeats.push_back(createExprRecursive());
			}
			script->statements.push_back(pattern);
		} else { //loopNode
			std::vector<StatementPtr> loopBody;
			int loopLen = rand() % 3 + 1;
			for (int j = 0; j < loopLen; ++j) {
				//only notenodes rn
				loopBody.push_back(new NoteNode(createExprRecursive(), createExprRecursive()));
			}
			//use a recursively generated expression for the iteration count.
			script->statements.push_back(new LoopNode(loopBody, createExprRecursive()));
		}
	}
	return script;
}


std::vector<SongElement> evaluateSongString(std::string& songcode,
                                            ErrorInfo* errorInfo,
                                            std::map<std::string, float>* vars) {
    SongCodeLexer lexer(songcode);
    auto lexerToks = lexer.tokenize(errorInfo);
    juce::Logger::writeToLog(errorInfo->message);
    juce::Logger::writeToLog("-------------First pass-------------" + juce::String(lexerToks.size()));
    juce::Logger::writeToLog(juce::String(astToString(createRandomAST())));
    if (lexerToks.empty()) return {};
    
    lexer.printTokens(lexerToks);

    return {};
}