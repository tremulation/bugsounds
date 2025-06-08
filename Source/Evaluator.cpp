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
#include <random>
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
                while (pos < input.size() && (std::isalnum(input[pos]) || input[pos] == '#')) pos++;

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
                } else if (word == "getScale") {
                    tokens.emplace_back(TokenType::GetScale, start, tokenEnd, word);
                } else if (word == "clampLength") {
                    tokens.emplace_back(TokenType::ClampLength, start, tokenEnd, word);
                } else if (word == "chitter") {
                    tokens.emplace_back(TokenType::Chitter, start, tokenEnd, word);
                } else if (word == "subclick") {
                    tokens.emplace_back(TokenType::Subclick, start, tokenEnd, word);
                } else if (word == "choose") {
                    tokens.emplace_back(TokenType::Choose, start, tokenEnd, word);
                } else {
                    //no match, must be var or a note literal
                    float freqVal = 0.f;
                    if (tryParseNoteLiteralToFrequency(word, freqVal)) {
                        tokens.emplace_back(TokenType::Num, freqVal, start, tokenEnd, word);
                    } else {
                        tokens.emplace_back(TokenType::Id, word, start, tokenEnd, word);
                    }
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
                    case '{': tokens.emplace_back(TokenType::CurlyStart, pos, pos, "{"); break;
                    case '}': tokens.emplace_back(TokenType::CurlyEnd, pos, pos, "}"); break;
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
        case TokenType::CurlyStart:  return "CurlyStart";
        case TokenType::CurlyEnd:    return "CurlyEnd";
        case TokenType::GetScale:    return "GetScale";
        case TokenType::ClampLength: return "ClampLength";
        case TokenType::Subclick:    return "Subclick";
        case TokenType::Chitter:return "Chitter";
        case TokenType::Choose: return "choose";
        default:                return "Unknown";
        }
    }

    void printTokens(const std::vector<Token>&tokens) {
        juce::Logger::writeToLog("-------------------- LEXER TOKEN OUTPUT -------------------");
        for (const auto& token : tokens) {
            std::string str = tokenTypeToString(token.type);
            //add value information where applicable
            int written;
            std::string s(16, '\0');
            switch (token.type) {
            case TokenType::Num:
                /*str += "(" + std::to_string(token.numValue) + ")";*/
                written = std::snprintf(&s[0], s.size(), "%.1f", token.numValue);
                s.resize(written);
                str += " (" + s + ")";
                break;
            case TokenType::Id:
                str += "('" + token.idValue + "')";
                break;
            default:
                break;
            }
            //add in start/end indices
            str += "    Orig: " + token.text + " ";
            str += "(" + std::to_string(token.startPos) + ", " + std::to_string(token.endPos) + ").";
            juce::Logger::writeToLog(str);
        }
    }

    private:


        //try to parse strings like A4, C#9 to a numerical frequency
        static bool tryParseNoteLiteralToFrequency(const std::string& s, float& outFreq) {
            //valid note literals have 2-3 characters
            if (s.size() < 2 || s.size() > 3) return false;

            //first character is A-G
            char letter = std::toupper((char)s[0]);
            int baseSemitone = 0;
            switch (letter) {
            case 'C': baseSemitone = 0;  break;
            case 'D': baseSemitone = 2;  break;
            case 'E': baseSemitone = 4;  break;
            case 'F': baseSemitone = 5;  break;
            case 'G': baseSemitone = 7;  break;
            case 'A': baseSemitone = 9;  break;
            case 'B': baseSemitone = 11; break;
            default: return false;
            }

            //handle accidentals
            int semitone = baseSemitone;
            int octave = 0;
            int charIdx = 1;
            if (s.size() == 3) {
                char acc = s[1];
                if (acc == '#') {
                    semitone += 1;
                } else if (acc == 'b') {
                    semitone -= 1;
                } else {
                    return false;
                }

                //wrap semitone
                if (semitone == 12) {  
                    //B# = C in the next octave
                    semitone = 0;
                    octave = 1;
                } else if (semitone < 0) {
                    //Cb = B in the previous octave
                    semitone = 11;
                    octave = -1;
                }
                //advance to octave part
                charIdx = 2;
            }

            //handle octave
            //maybe handle double digit octaves? probably not useful.
            char octChar = s[charIdx];
            octave += (octChar - '0');   //quick convert from ascii to int
            int midiNumber = (octave + 1) * 12 + semitone;
            outFreq = 440.f * std::pow(2.f, (midiNumber - 69) / 12.f);
            return true;
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

    REVISION 2 CFG
    Script 		-> Statement*
    Statement   -> Note COMMA | Operator COMMA | Declaration COMMA | Loop COMMA | Section COMMA
    Note		-> AdditiveExpr AdditiveExpr
    Operator    -> Pattern | SPattern | getScale | clampLength | Chitter
    Pattern		-> PATTERN PARSTART AdditiveExpr* PAREND
    SPattern    -> SPATTERN PARSTART AdditiveExpr* PAREND
    ClampLength -> CLAMPLENGTH PARSTART Statement* PAREND
    Chitter		-> CHITTER PARSTART Statement* PAREND
    Declaration -> LET ID EQUALS AdditiveExpr
    Section		-> CURLYSTART Statement* CURLYEND
    Loop		-> BARSTART Statement* BAREND AdditiveExpr
    AdditiveExpr-> MultiplicativeExpr (ADDOPERATOR AdditiveExpr)?
    MultiplicativeExpr -> PrimaryExpr (MULTOPERATOR MultiplicativeExpr)?
    PrimaryExpr -> INT | ID | Random | Choose | PARSTART AdditiveExpr PAREND
    Random      -> RAND PARSTART AdditiveExpr AdditiveExpr PAREND
    GetScale	-> GETSCALE PARSTART AdditiveExpr AdditiveExpr AdditiveExpr PAREND
	Choose      -> CHOOSE PARSTART AdditiveExpr COMMA AdditiveExpr * PAREND

All functions operate on a shared state set in the constructor, to minimize the amount
of arguments I have to pass over and over
*/

//--------------------------------- HELPERS ---------------------------------

//returns next token in the stream. 
//optional offset argument for checking tokens past the next one
std::optional<Token> Parser::lookahead(int offset) const {
    if (std::distance(it, end) > static_cast<std::ptrdiff_t>(offset)) {
        return *(it + offset);
    }
    return std::nullopt;
}

//checks if the next token is the expected type. if so, consume it, and return true
//else return false and don't change the tokens
bool Parser::match_token(TokenType expected) {
    auto next = lookahead();
    if (next.has_value() && next->type == expected) {
        ++it;
        return true;
    }
    return false;
}


//--------------------------------- PARSING FUNCTIONS -----------------------

//top level parsing function, equivalent to parse_script in the CFG
ScriptPtr Parser::parse() {
    sectionAlreadyCompiled = false;
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
    if (ntt == TokenType::Num || ntt == TokenType::Id || ntt == TokenType::Rand || ntt == TokenType::ParStart || 
        ntt == TokenType::GetScale || ntt == TokenType::Choose) {
        successful = parse_note();
    }
    else if (ntt == TokenType::Pattern || ntt == TokenType::Subclick || ntt == TokenType::ClampLength || ntt == TokenType::Chitter) {
        successful = parse_operator();
    }
    else if (ntt == TokenType::Let) {
        successful = parse_let();
    }
    else if (ntt == TokenType::LStart) {
        successful = parse_loop();
    } 
    else if (ntt == TokenType::CurlyStart) {
        successful = parse_section();
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
            setErrorInfo(errorInfo, "Error: missing comma", ntStart, ntEnd, "");
        }
    }
    return successful;
}






//start by writing a working parse_note, and a working parse_primary_expr (int only).
//then do the rest of the statements
//then do the rest of the expressions
//then do the evaluator
bool Parser::parse_note() {
    //match frequency
    ExprPtr freqExpr = parse_additive_expr();
	if (!freqExpr) return false;

    //match duration
    ExprPtr durExpr = parse_additive_expr();
    if (!durExpr) return false;
	AST->statements.push_back(new NoteNode(freqExpr, durExpr));
    return true;
}





bool Parser::parse_pattern() {
	//match pattern keyword
    if (!match_token(TokenType::Pattern)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected 'pattern'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

	//match open parenthesis (
    if (!match_token(TokenType::ParStart)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following pattern", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match loop contents
    std::vector<ExprPtr> subBeats;
	while (lookahead().has_value() && lookahead()->type != TokenType::ParEnd) {
		ExprPtr subBeat = parse_additive_expr();
        subBeats.push_back(subBeat);
	}

	//match closing parenthesis )
	if (!match_token(TokenType::ParEnd)) {
		if (lookahead().has_value()) return false;
		setErrorInfo(errorInfo, "Error: expected a closing parenthesis ')'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return false;
	}

    //check pattern has stuff in it
	if (subBeats.empty()) {
		setErrorInfo(errorInfo, "Error: pattern is empty", lookahead(-2)->startPos, lookahead(-2)->endPos, "");
		return false;
	}

    AST->statements.push_back(new PatternNode(subBeats));
    return true;
}


bool Parser::parse_let() {
    //match let
	if (!match_token(TokenType::Let)) {
		if (lookahead().has_value()) return false;
		setErrorInfo(errorInfo, "Error: expected 'let'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return false;
	}

    //match variable id
	std::optional<Token> idToken = lookahead();
	if(!idToken.has_value() || !match_token(TokenType::Id)) {
		setErrorInfo(errorInfo, "Error: expected an identifier after 'let'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
    }

    //match equals
    if (!match_token(TokenType::Equals)) {
		setErrorInfo(errorInfo, "Error: expected '=' after variable name", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
    }

    //match the expression to assign to the variable
	ExprPtr value = parse_additive_expr();
	if (!value) return false;   //error set by recursive call

	AST->statements.push_back(new LetNode(idToken->idValue, value));
	return true;
}


bool Parser::parse_loop() {
    //match open bracket [
    if (!match_token(TokenType::LStart)) {
        setErrorInfo(errorInfo, "Error: expected loop open bracket", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //parse loop body
    //this is gonna be janky. The loop's body is statements, not expressions, but our parse_statement method places
	//parsed statements directly into the AST. So we need to parse statements until we hit the closing bracket,
	//pull them out of the AST, and put them into the loop's body.
    std::vector<StatementPtr> loopBody;
    int bodyElements = 0;
	while (lookahead().has_value() && lookahead()->type != TokenType::LEnd) {
		bool successful = parse_statement();
        bodyElements++;

		if (!successful) return false;
        if (errorInfo->message == "Error: missing comma") {
            //scrub the false error related to not having a comma at the end of a string of statements in the loop 
            setErrorInfo(errorInfo, "", 0, 0, "");
        }

        //all other errors should be real
		if (errorInfo->message != "") return false;
	}

	for (int i = 0; i < bodyElements; i++) {
		loopBody.push_back(AST->statements.back());
		AST->statements.pop_back();
	}

    //match closing bracket ]
	if (!match_token(TokenType::LEnd)) {
		setErrorInfo(errorInfo, "Error: expected loop close bracket", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return false;
	}

    //mmatch loop iteration count
	ExprPtr iterations = parse_additive_expr();
    if (!iterations) {
		setErrorInfo(errorInfo, "Error: missing loop iteration count", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    std::reverse(loopBody.begin(), loopBody.end());
	AST->statements.push_back(new LoopNode(loopBody, iterations));
    return true;
}




//Pattern | SPattern | getScale | clampLength | Chitter
bool Parser::parse_operator() {
    bool successful = false;
    optional<Token> nextTok = lookahead();

    switch (nextTok->type) {
	    case TokenType::Pattern:successful = parse_pattern(); break;
        case TokenType::Subclick: successful = parse_spattern(); break;
		case TokenType::ClampLength: successful = parse_clamplength(); break;
		case TokenType::Chitter: successful = parse_chitter(); break;
        default: 
            setErrorInfo(errorInfo, "Error: unrecognized operator " + nextTok->text, nextTok->startPos, nextTok->endPos, "");
            successful = false;
            break;
    }
	
    return successful;
}



//subclick(expr*) -- same parsing as pattern
bool Parser::parse_spattern() {
	//match subclick keyword
    if (!match_token(TokenType::Subclick)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected 'subclick'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match open parenthesis
    if (!match_token(TokenType::ParStart)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following subclick", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match subbeat pattern contents
    std::vector<ExprPtr> subBeats;
    while (lookahead().has_value() && lookahead()->type != TokenType::ParEnd) {
        ExprPtr subBeat = parse_additive_expr();
        subBeats.push_back(subBeat);
    }

    //match closing parenthesis
    if (!match_token(TokenType::ParEnd)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected a closing parenthesis ')'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

	//check subbeat pattern has stuff in it
    if (subBeats.empty()) {
        setErrorInfo(errorInfo, "Error: subclick pattern is empty", lookahead(-2)->startPos, lookahead(-2)->endPos, "");
        return false;
    }

    AST->statements.push_back(new SubclickPatternNode(subBeats));
    return true;
}





//chitter(expr* , statement*)
bool Parser::parse_chitter() {
    //match chitter
    if (!match_token(TokenType::Chitter)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected 'chitter'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match open parenthesis
    if (!match_token(TokenType::ParStart)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following chitter", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //parse expr statements until we run out
    std::vector<ExprPtr> subBeats;
    while (lookahead().has_value() && lookahead()->type != TokenType::Comma) {
        ExprPtr subBeat = parse_additive_expr();
        subBeats.push_back(subBeat);
    }

    //match comma separating pattern from the statement list to apply it to
	if (!match_token(TokenType::Comma)) {
		if (lookahead().has_value()) return false;
		setErrorInfo(errorInfo, "Error: expected a comma after chitter pattern", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return false;
	}

    //parse the statement list
    std::vector<StatementPtr> chitterBody;
    int bodyElements = 0;
    //idk if this will work with parend instead of bracket or loop or something else special
    while (lookahead().has_value() && lookahead()->type != TokenType::ParEnd) {
        bool successful = parse_statement();
        bodyElements++;

        if (!successful) return false;
        if (errorInfo->message == "Error: missing comma") {
            //scrub the false error related to not having a comma at the end of a string of statements in the loop 
            setErrorInfo(errorInfo, "", 0, 0, "");
        }

        //all other errors should be real
        if (errorInfo->message != "") return false;
    }

    for (int i = 0; i < bodyElements; i++) {
        chitterBody.push_back(AST->statements.back());
        AST->statements.pop_back();
    }

    //match closing paren
    if (!match_token(TokenType::ParEnd)) {
        setErrorInfo(errorInfo, "Error: expected closed parenthesis for chitter", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    std::reverse(chitterBody.begin(), chitterBody.end());
    AST->statements.push_back(new ChitterNode(chitterBody, subBeats));
    return true;
}







//clamplength(expr length, statement*) 
bool Parser::parse_clamplength() {
    //match clamplength keyword
    if (!match_token(TokenType::ClampLength)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected 'clampLength'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match open parenthesis
    if (!match_token(TokenType::ParStart)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following clamplength", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match length expr
	ExprPtr lengthExpr = parse_additive_expr();
    if (!lengthExpr) {
        setErrorInfo(errorInfo, "Error: missing total length (arg 1) for clamplength", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match comma separating length from statement list
	if (!match_token(TokenType::Comma)) {
		if (lookahead().has_value()) return false;
		setErrorInfo(errorInfo, "Error: expected a comma after clamplength length", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return false;
	}

    //parse the statement list, same method as loop
    std::vector<StatementPtr> clampBody;
	int bodyElements = 0;
    while (lookahead().has_value() && lookahead()->type != TokenType::ParEnd) {
        bool successful = parse_statement();
        bodyElements++;

        if (!successful) return false;
		if (errorInfo->message == "Error: missing comma") {
			//scrub the false error related to not having a comma at the end of a string of statements in the loop 
			setErrorInfo(errorInfo, "", 0, 0, "");
		}

		//all other errors should be real
		if (errorInfo->message != "") return false;
    }

	for (int i = 0; i < bodyElements; i++) {
		clampBody.push_back(AST->statements.back());
		AST->statements.pop_back();
	}

    //match closing paren
    if (!match_token(TokenType::ParEnd)) {
        setErrorInfo(errorInfo, "Error: expected closed parenthesis for clamplength", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

	std::reverse(clampBody.begin(), clampBody.end());
	AST->statements.push_back(new ClampLengthNode(clampBody, lengthExpr));
	return true;
}




// { statement* }
bool Parser::parse_section() {
    //there can only be one looping section per songCode song, so we need to fail
    if (sectionAlreadyCompiled) {
		setErrorInfo(errorInfo, "Error: multiple sections not allowed", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //set this here in case people try to nest sections inside sections. they shall be punished for this foolishness.
	sectionAlreadyCompiled = true;

    //match starting curly
	if (!match_token(TokenType::CurlyStart)) {
		if (lookahead().has_value()) return false;
		setErrorInfo(errorInfo, "Error: expected section open curly", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return false;
	}

    //parse section body
    std::vector<StatementPtr> sectionBody;
    int bodyElements = 0;
    while (lookahead().has_value() && lookahead()->type != TokenType::CurlyEnd) {
        bool successful = parse_statement();
        juce::Logger::writeToLog("Here: " + std::to_string(bodyElements));
        bodyElements++;

        if (!successful) return false;
        if (errorInfo->message == "Error: missing comma") {
            //scrub the false error related to not having a comma at the end of a string of statements in the loop 
            setErrorInfo(errorInfo, "", 0, 0, "");
        }

        //all other errors should be real
        if (errorInfo->message != "") return false;
    }

    //empty patterns aren't allowed, because they're impossible to loop
    //TODO: during evaluation, make sure to check that the section also has duration. 
    if (bodyElements == 0) {
        setErrorInfo(errorInfo, "Error: sections have to have at least 1 element to loop", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    //match end curly
    if (!match_token(TokenType::CurlyEnd)) {
        if (lookahead().has_value()) return false;
        setErrorInfo(errorInfo, "Error: expected closed curly brace at the end of the section", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return false;
    }

    for (int i = 0; i < bodyElements; i++) {
        sectionBody.push_back(AST->statements.back());
        AST->statements.pop_back();
    }




    std::reverse(sectionBody.begin(), sectionBody.end());
    AST->statements.push_back(new SectionNode(sectionBody));
    return true;
}




//getScale(baseFreq interval scaleType)
ExprPtr Parser::parse_getscale() {
    //match getscale keyword
    if (!match_token(TokenType::GetScale)) {
        if (lookahead().has_value()) return nullptr;
        setErrorInfo(errorInfo, "Error: expected 'getScale'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //match open parenthesis
    if (!match_token(TokenType::ParStart)) {
        if (lookahead().has_value()) return nullptr;
        setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //mmatch baseFreq expr
    ExprPtr baseFreq = parse_additive_expr();
    if (!baseFreq) {
        setErrorInfo(errorInfo, "Error: missing baseFreq (arg 1) for getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //parse the comma
	if (!match_token(TokenType::Comma)) {
		setErrorInfo(errorInfo, "Error: expected a comma after baseFreq in getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
		return nullptr;
	}

    //mmatch interval expr
    ExprPtr interval = parse_additive_expr();
    if (!interval) {
        setErrorInfo(errorInfo, "Error: missing interval (arg 2) for getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //parse the next comma
    if (!match_token(TokenType::Comma)) {
        setErrorInfo(errorInfo, "Error: expected a comma after interval", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //match scale type
    ExprPtr scaleType = parse_additive_expr();
    if (!scaleType) {
        setErrorInfo(errorInfo, "Error: missing scaleType (arg 3) for getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //match closing paren
    if (!match_token(TokenType::ParEnd)) {
        setErrorInfo(errorInfo, "Error: expected closed parenthesis for getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    return new GetScaleNode(baseFreq, interval, scaleType);
}






ExprPtr Parser::parse_additive_expr() {
	ExprPtr left = parse_multiplicative_expr();
    ExprPtr right = nullptr;
	if (!left) return nullptr;

    //optional right side of the expression
    auto currentToken = lookahead();
    if (!currentToken) return left;

    TokenType opType = currentToken->type;
	if (opType == TokenType::Add || opType == TokenType::Sub) {
        match_token(opType);
		right = parse_additive_expr();
        if (!right) {
			setErrorInfo(errorInfo, "Error: expected an expression after +/-", lookahead()->startPos, lookahead()->endPos, "");
			return nullptr;
        }
        left = new AdditiveExprNode(left, right, opType == TokenType::Add ? AdditiveExprNode::Op::Add : AdditiveExprNode::Op::Subtract);
	}

    return left;
}





ExprPtr Parser::parse_multiplicative_expr() {
    ExprPtr left = parse_primary_expr();
    ExprPtr right = nullptr;
    if (!left) return nullptr;

    auto currentToken = lookahead();
    if (!currentToken) return left; //no mo to ko

    TokenType opType = currentToken->type;
    if (opType == TokenType::Mul || opType == TokenType::Div) {
        match_token(opType);
        right = parse_multiplicative_expr();
        if (!right) {
            setErrorInfo(errorInfo, "Error: expected an expression after operation", lookahead()->startPos, lookahead()->endPos, "");
            return nullptr;
        }
        left = new MultiplicativeExprNode(left, right, opType == TokenType::Mul ? MultiplicativeExprNode::Op::Multiply : MultiplicativeExprNode::Op::Divide);
    }

    return left;
}


ExprPtr Parser::parse_primary_expr() {
    auto next = lookahead();
    //parse ints
    if (next.has_value() && next->type == TokenType::Num) {
        int value = next->numValue; 
        match_token(TokenType::Num);
        return new PrimaryExprNode(value);
    }
    //parse variables
	else if (next.has_value() && next->type == TokenType::Id) {
		std::string varName = next->idValue;
		match_token(TokenType::Id);
		return new PrimaryExprNode(varName);
	}
    //parse rands
    else if (next.has_value() && next->type == TokenType::Rand) {
        //match rand token
		match_token(TokenType::Rand);

		//match open parenthesis (
        if (!match_token(TokenType::ParStart)) {
            setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following 'rand'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
            return nullptr;
        }

        //match rand's min expr
		ExprPtr min = parse_additive_expr();
        if (!min) return nullptr;
        
        //match rand's max expr
		ExprPtr max = parse_additive_expr();
		if (!max) return nullptr;

		//match closing parenthesis )
        if (!match_token(TokenType::ParEnd)) {
            setErrorInfo(errorInfo, "Error: expected a closing parenthesis ')'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
            return nullptr;
        }

		return new RandomNode(min, max);
    }
    //parse get scales
    else if (next.has_value() && next->type == TokenType::GetScale) {
		ExprPtr getScaleNode = parse_getscale();
        if (!getScaleNode) return nullptr;
        return getScaleNode;
    }
    else if (next.has_value() && next->type == TokenType::Choose) {
        ExprPtr chooseNode = parse_choose_expr();
        if (!chooseNode) return nullptr;
        return chooseNode;
    }
	//parse grouped expressions
	else if (next.has_value() && next->type == TokenType::ParStart) {
		//match open parenthesis (
		match_token(TokenType::ParStart);

		//parse the inner expression
		ExprPtr grouped = parse_additive_expr();
		if (!grouped) return nullptr;

		//match closing parenthesis )
		if (!match_token(TokenType::ParEnd)) {
			setErrorInfo(errorInfo, "Error: expected a closing parenthesis ')'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
			return nullptr;
		}

		return new PrimaryExprNode(grouped);
	}
    else {
        if (!next.has_value() && lookahead(-1).has_value()) {
			//when they have a statement with a missing number or expression (for example, a note with no duration)
            setErrorInfo(errorInfo, "Error: statement is missing a number/expression after this", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        }
        else {
            setErrorInfo(errorInfo, "Error: unrecognized expression", next->startPos, next->endPos, "");
        }
        return nullptr;
    }
}

ExprPtr Parser::parse_choose_expr(){
    //match choose
    if (!match_token(TokenType::Choose)) {
        if (lookahead().has_value()) return nullptr;
        setErrorInfo(errorInfo, "Error: expected 'choose'", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    ////match open parenthesis
    if (!match_token(TokenType::ParStart)) {
        if (lookahead().has_value()) return nullptr;
        setErrorInfo(errorInfo, "Error: expected an open parenthesis '(' following choose", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //match the index to choose
    ExprPtr indexExpr = parse_additive_expr();
    if (!indexExpr) {
        setErrorInfo(errorInfo, "Error: missing index to choose (arg 1) for choose()", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    //parse the comma
    if (!match_token(TokenType::Comma)) {
        setErrorInfo(errorInfo, "Error: expected a comma between choose index and things to choose from", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

	//match the expr list to choose from
    std::vector<ExprPtr> choices;
    while (lookahead().has_value() && lookahead()->type != TokenType::ParEnd) {
        ExprPtr c = parse_additive_expr();
        choices.push_back(c);
    }

    //match closing paren
    if (!match_token(TokenType::ParEnd)) {
        setErrorInfo(errorInfo, "Error: expected closed parenthesis for getScale", lookahead(-1)->startPos, lookahead(-1)->endPos, "");
        return nullptr;
    }

    return new ChooseNode(indexExpr, choices);
}

/* 
-------------------============ AST TESTING FUNCTIONS ============-------------------
createRandomAST() and astToString() are the most useful for testing purposes
you could probably put the createRandomAST() function into the evaluator and spam it
and that's pretty much unit testing
*/

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
		if (auto clampLength = dynamic_cast<ClampLengthNode*>(stmt.get())) {
			std::string body;
			for (const auto& s : clampLength->body) {
				body += "\n    " + statementToString(s);
			}
			return "ClampLength(length: " + exprToString(clampLength->length) + ")" + body;
		}
        if (auto chitter = dynamic_cast<ChitterNode*>(stmt.get())) {
            std::string pattern;
            std::string body;

            for (const auto& expr : chitter->chitterPattern) {
                if (!pattern.empty()) pattern += ", ";
                pattern += exprToString(expr);
            }

            for (const auto& s : chitter->body) {
                body += "\n    " + statementToString(s);
            }

            return "chitter(pattern: " + pattern + ", " + body + ")";
        }
        if (auto sc = dynamic_cast<SubclickPatternNode*>(stmt.get())) {
            std::string elements;
            for (const auto& expr : sc->subclicks) {
                if (!elements.empty()) elements += ", ";
                elements += exprToString(expr);
            }
            return "subclick pattern[" + elements + "]";
        }
        if (auto section = dynamic_cast<SectionNode*>(stmt.get())) {
            std::string body;
            for (const auto& s : section->body) {
                body += "\n    " + statementToString(s);
            }
            return "section { " + body + "}";
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
        if (auto getScale = dynamic_cast<GetScaleNode*>(expr.get())) {
            return "getScale(baseFreq: " + exprToString(getScale->baseFrequency) + " interval: " + exprToString(getScale->interval) + "scaleNum: " + exprToString(getScale->scaleType) + ")";
        }
        if (auto choose = dynamic_cast<ChooseNode*>(expr.get())) {
            std::string indexStr = exprToString(choose->index);
            std::string choicesStr;
            for (const auto& expr : choose->choices) {
                if (!choicesStr.empty()) choicesStr += ", ";
                choicesStr += exprToString(expr);
            }
            return "choose(" + indexStr + ", " + choicesStr + ")";
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
        auto op = (rand() % 2 == 0) ? AdditiveExprNode::Add : AdditiveExprNode::Subtract;
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
		} else if (choice == 1) //noteNode
		{
			ExprNode* freq = createExprRecursive();
			ExprNode* dur = createExprRecursive();
			script->statements.push_back(new NoteNode(freq, dur));
		} else if (choice == 2) { //patternNode
            std::vector<ExprPtr> sb = {};
            auto pattern = new PatternNode(sb);
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



/*
* -------------------= ========== EVALUATOR CLASS ============-------------------
* Parses a scriptnode into a list of song elements (notes, patterns)
*/

float evaluateExpr(const ExprPtr expr, std::map<std::string, float>* env, ErrorInfo* errorInfo) {
    if (!expr) {
        setErrorInfo(errorInfo, "Error: null expression node encountered", 0, 0, "");
        return -1;
    }

    //evaluate additive expression
    if (auto* additive = dynamic_cast<AdditiveExprNode*>(expr.get())) {
        float left_val = evaluateExpr(additive->left, env, errorInfo);
        if (errorInfo->message != "") return -1;
        float right_val = evaluateExpr(additive->right, env, errorInfo);
        if (errorInfo->message != "") return -1;

        switch (additive->op) {
        case AdditiveExprNode::Add: return left_val + right_val;
        case AdditiveExprNode::Subtract: return left_val - right_val;
        default:
            setErrorInfo(errorInfo, "Error: unknown additive operator", 0, 0, "");
            return -1;
        }
    }

	//evaluate multiplicative expressions
	else if (auto* multiplicative = dynamic_cast<MultiplicativeExprNode*>(expr.get())) {
        float left_val = evaluateExpr(multiplicative->left, env, errorInfo);
		if (errorInfo->message != "") return -1;
        float right_val = evaluateExpr(multiplicative->right, env, errorInfo);
		if (errorInfo->message != "") return -1;

		switch (multiplicative->op) {
		case MultiplicativeExprNode::Multiply: return left_val * right_val;
		case MultiplicativeExprNode::Divide: return left_val / right_val;
		default:
			setErrorInfo(errorInfo, "Error: unknown multiplicative operator", 0, 0, "");
			return -1;
		}
	}

    //evaluate choose expressions
	else if (auto* choose = dynamic_cast<ChooseNode*>(expr.get())) {
		float index = evaluateExpr(choose->index, env, errorInfo);
		if (errorInfo->message != "") return -1;

		//ensure index is within bounds
		if (index < 0 || index >= static_cast<float>(choose->choices.size())) {
			setErrorInfo(errorInfo, "Error: choose index out of bounds", 0, 0, "");
			return -1;
		}

		//return the chosen expression's evaluated value
		return evaluateExpr(choose->choices[static_cast<int>(index)], env, errorInfo);
	}

    //evaluate getScale expressions
    else if (auto* gscale = dynamic_cast<GetScaleNode*>(expr.get())) {
		float baseNotePitch = evaluateExpr(gscale->baseFrequency, env, errorInfo);
		if (errorInfo->message != "") return -1;
		float interval = evaluateExpr(gscale->interval, env, errorInfo);
		if (errorInfo->message != "") return -1;
		int scaleType = static_cast<int>(evaluateExpr(gscale->scaleType, env, errorInfo));
		if (errorInfo->message != "") return -1;

        //get the scale interval array
		scaleType %= 11; // there are 11 scales, wrap around if we go over
        std::vector<int> intervals;
        switch (static_cast<GetScaleNode::ScaleType>(scaleType)) {
            //normal scales
            case GetScaleNode::Major: intervals = { 2, 2, 1, 2, 2, 2, 1 };  break;
            case GetScaleNode::Minor: intervals = { 2, 1, 2, 2, 1, 2, 2 };  break;
            case GetScaleNode::MelodicMinor: intervals = { 2, 1, 2, 2, 2, 2, 1 }; break;
            case GetScaleNode::Mixolydian: intervals = { 2, 2, 1, 2, 2, 1, 2 }; break;
            case GetScaleNode::Dorian: intervals = { 2, 1, 2, 2, 2, 1, 2 }; break;
            case GetScaleNode::Lydian: intervals = { 2, 2, 2, 1, 2, 2, 1 }; break;
            //chromatic/microtonal scales (experimental)
            case GetScaleNode::Chromatic:intervals = { 1,1,1,1,1,1,1,1,1,1,1,1 }; break;
            case GetScaleNode::Pentatonic:intervals = { 2, 2, 3, 2, 3 }; break;
            case GetScaleNode::MinorPentatonic:intervals = { 3, 2, 2, 3, 2 }; break;
            case GetScaleNode::Edo10:intervals = { 1,1,1,1,1,1,1,1,1,1 };break;
            case GetScaleNode::Edo22: intervals = std::vector<int>(22, 1); break;
            case GetScaleNode::Edo29: intervals = std::vector<int>(29, 1); break;
            default: return static_cast<float>(baseNotePitch);
        }

        //compute semitone offset
		int semitoneOffset = 0;
		int scaleSize = static_cast<int>(intervals.size());
        if (interval >= 0) {
            for (int i = 0; i < interval; i++) {
                //wrap if we go over the size of the scale
                semitoneOffset += intervals[i % scaleSize]; 
            }
        } else {
            //for negative intervals (is this possible?)
            for (int i = 0; i < -interval; i++) {
                int idx = (-i + 1) % scaleSize;
                semitoneOffset -= intervals[(scaleSize - 1) - idx];
            }
        }

        int stepsPerOctave = 0;
        for (auto n : intervals) stepsPerOctave += n;
        float ratio = std::pow(2.f, semitoneOffset / ((float)stepsPerOctave));
        return baseNotePitch * ratio;
    }

    //evaluate primary expressions
     else if (auto* inty = dynamic_cast<PrimaryExprNode*>(expr.get())) {
        if (inty->kind == PrimaryExprNode::Integer) {
            return inty->integerValue;
        }
        if (inty->kind == PrimaryExprNode::Variable) {
            //testing: pront variables
            //for (const auto& pair : *env) std::cout << "  " << pair.first << " = " << pair.second << std::endl;
            auto it = env->find(inty->variableName);
            if (it == env->end()) {
                for (const auto& [key, value] : (*env)) {
					juce::Logger::writeToLog(key + ": " + std::to_string(value));
                }
                setErrorInfo(errorInfo, "Error: variable used before initialization: " + inty->variableName, 0, 0, "");
                return -1;
            }
            return it->second;
        }
        if (inty->kind == PrimaryExprNode::Grouped) {
            return evaluateExpr(inty->groupedExpr, env, errorInfo);
        }
        setErrorInfo(errorInfo, "Error: can't parse primary expression", 0, 0, "");
        return -1;
    }

	//evaluate random expressions
    //this is the only expression that returns an int
	 else if (auto* rand = dynamic_cast<RandomNode*>(expr.get())) {
		int min = evaluateExpr(rand->min, env, errorInfo);
		if (errorInfo->message != "") return -1;
		int max = evaluateExpr(rand->max, env, errorInfo);
		if (errorInfo->message != "") return -1;
        static std::mt19937 gen(std::time(nullptr));
        static std::uniform_int_distribution<> dis(0, std::numeric_limits<int>::max());
        return min + dis(gen) % (max - min + 1);
	}
	 else {
		setErrorInfo(errorInfo, "Error: unknown expression type", 0, 0, "");
		return -1;
	}
	return -1;
}






//usually returns 1 statement, but can return 0 to more than 1 statements (for loops)
std::vector<SongElement> evaluateStatement(StatementPtr statement, std::map<std::string, float>* env, ErrorInfo* errorInfo, float* lastFreq) {
    //notes
    if (auto note = dynamic_cast<NoteNode*>(statement.get())) {
        float freq = evaluateExpr(note->frequency, env, errorInfo);
        if (errorInfo->message != "") return {};
        float dur = evaluateExpr(note->duration, env, errorInfo);
        if (errorInfo->message != "") return {};
        float oldLastFreq = *lastFreq;
        *lastFreq = freq;
		//return a vector with one note
		return { SongElement(oldLastFreq, freq, dur) };
    }
    //patterns
    else if (auto pattern = dynamic_cast<PatternNode*>(statement.get())) {
        std::vector<int> patternVec;
        for (auto& subBeat : pattern->subBeats) {
            int subBeatVal = evaluateExpr(subBeat, env, errorInfo);
            if (errorInfo->message != "") return {};
            patternVec.push_back(subBeatVal);
        }
        return { SongElement(SongElement::Type::Pattern, patternVec) };
    }
    //lets
    else if (auto let = dynamic_cast<LetNode*>(statement.get())) {
        float val = evaluateExpr(let->value, env, errorInfo);
        //bind val to the variable name in the env
        if (errorInfo->message == "") {
			(*env)[let->id] = val;      
            
        }
        return {};
    }
    //loops
    else if (auto loop = dynamic_cast<LoopNode*>(statement.get())) {
        int iterations = evaluateExpr(loop->iterations, env, errorInfo);
        if (errorInfo->message != "") return {};
		std::vector<SongElement> loopContents;
        for (int i = 0; i < iterations; i++) {
			for (auto& stmt : loop->body) {
				auto res = evaluateStatement(stmt, env, errorInfo, lastFreq);
				if (errorInfo->message != "") return {};
                //append the results into the AST in the correct order
				loopContents.insert(loopContents.end(), res.begin(), res.end());
			}
        }
		return loopContents;
    }
    //clamplength
    else if (auto clampLength = dynamic_cast<ClampLengthNode*>(statement.get())) {
        int newTotalLength = evaluateExpr(clampLength->length, env, errorInfo);
        if (errorInfo->message != "") return {};
        if (newTotalLength <= 0) {
            setErrorInfo(errorInfo, "Error: the length arg for clampLength is less than or equal to 0", 0, 0, "");
            return {};
        }

        //evaluate all the statements within clamplength
        std::vector<SongElement> clampContents;
        for (auto& stmt : clampLength->body) {
            auto res = evaluateStatement(stmt, env, errorInfo, lastFreq);
            if (errorInfo->message != "") return {};
            //append the results into the AST in the correct order
            clampContents.insert(clampContents.end(), res.begin(), res.end());
        }

        //compute the total length of body
        int currentTotalLength = 0;
        for (auto& elem : clampContents) {
            //every element with length. just note so far.
            if (elem.duration != -1.f) currentTotalLength += elem.duration;
        }

        //special case: what if we try to apply clampLength to a list of statements with 0 total length?
        //add a 0-freq rest note at the end
        if (currentTotalLength == 0) {
            SongElement& restNote = *(new SongElement(0.f, 0.f, (float)newTotalLength));
            clampContents.push_back(restNote);
            return clampContents;
        }

        float ratio = (float)newTotalLength / (float)currentTotalLength;

        //scale all length-having elements by ratio
        for (auto& elem : clampContents) {
            if (elem.duration != -1.f) elem.duration *= ratio;
        }

        return clampContents;
    }
    //chitter(uint8_t vector, songElement vector)
    else if (auto chitter = dynamic_cast<ChitterNode*>(statement.get())) {
        //eval the pattern
        std::vector<uint8_t> patternVec;
        for (auto& subBeat : chitter->chitterPattern) {
            int subBeatVal = evaluateExpr(subBeat, env, errorInfo);
            if (errorInfo->message != "") return {};
            patternVec.push_back(subBeatVal);
        }

        //eval the statements to apply the chitter pattern to
        std::vector<SongElement> chitterContents;
        for (auto& stmt : chitter->body) {
            auto res = evaluateStatement(stmt, env, errorInfo, lastFreq);
            if (errorInfo->message != "") return {};
            chitterContents.insert(chitterContents.end(), res.begin(), res.end());
        }

        //check we have length elements to apply the chitter pattern to
        float totalLength = 0;
        for (auto& elem : chitterContents) if (elem.duration != -1.f) totalLength += elem.duration;
        if (totalLength == 0) return chitterContents;

        //also make sure the pattern has non zero elements
        int patternLength = 0;
        for (auto unit : patternVec) patternLength += unit;
        if (patternLength == 0) return {};

        std::vector<SongElement> results;
        int elemIdx = 0;
        float elemOffset = 0.0f;
        float remainingTotal = totalLength;
        int patIdx = 0;

        while (remainingTotal > 0.0f && elemIdx < chitterContents.size()) {
            //skip elements with no duration (patterns, subclickpatterns, etc.)
            while (elemIdx < chitterContents.size() && chitterContents[elemIdx].duration < 0.0f)  elemIdx++;
            if (elemIdx >= chitterContents.size()) break;

            float segmentLen = static_cast<float>(patternVec[patIdx % patternVec.size()]);
            if (segmentLen > remainingTotal) segmentLen = remainingTotal;

            bool isPlay = ((patIdx % 2) == 0);
            float remaining = segmentLen;

            if (isPlay) {
                while (remaining > 0.0f && elemIdx < chitterContents.size()) {
                    auto& elem = chitterContents[elemIdx];
                    float avail = elem.duration - elemOffset;
                    float take = std::min(avail, remaining);

                    //emplace note fragment
                    float fracStart = elemOffset / elem.duration;
                    float curFreq = elem.startFrequency + (elem.endFrequency - elem.startFrequency) * fracStart;
                    float fracEnd = (elemOffset + take) / elem.duration;
                    float nextFreq = elem.startFrequency + (elem.endFrequency - elem.startFrequency) * fracEnd;

                    results.emplace_back(curFreq, nextFreq, take );

                    elemOffset += take;
                    remaining -= take;
                    remainingTotal -= take;

                    if (elemOffset >= elem.duration) {
                        elemIdx++;
                        elemOffset = 0.0f;
                    }
                    patIdx++;
                }
            } else {
                float skip = remaining;
                while (skip > 0.0f && elemIdx < chitterContents.size()) {
                    auto& elem = chitterContents[elemIdx];
                    float avail = elem.duration - elemOffset;
                    float take = std::min(avail, skip);

                    //emplace silence fragment
                    float tailoffTime = take / 10.f;
                    float fracStart = elemOffset / elem.duration;
                    float curFreq = elem.startFrequency + (elem.endFrequency - elem.startFrequency) * fracStart;
                    results.emplace_back( curFreq, 0.0f, tailoffTime);
                    results.emplace_back(0.0f, 0.0f, take - tailoffTime);

                    elemOffset += take;
                    skip -= take;
                    remainingTotal -= take;

                    if (elemOffset >= elem.duration) {
                        elemIdx++;
                        elemOffset = 0.0f;
                    }
                }
                patIdx++;
            }
        }
        return results;
    }
    //section { statement* }
    else if (auto section = dynamic_cast<SectionNode*>(statement.get())) {
        //just eval the body and paste it in. setting start/end is handled in evaluateScript()
		std::vector<SongElement> sectionContents;
        for (auto& stmt : section->body) {
            auto res = evaluateStatement(stmt, env, errorInfo, lastFreq);
            if (errorInfo->message != "") return {};
            //append the results into the AST in the correct order
            sectionContents.insert(sectionContents.end(), res.begin(), res.end());
        }
        return sectionContents;
    }
    //subclick patterns
	else if (auto subclickPattern = dynamic_cast<SubclickPatternNode*>(statement.get())) {
		std::vector<int> subclicks;
		for (auto& subclick : subclickPattern->subclicks) {
			int subclickVal = evaluateExpr(subclick, env, errorInfo);
			if (errorInfo->message != "") return {};
			subclicks.push_back(static_cast<int>(subclickVal));
		}
		return { SongElement(SongElement::Type::SubclickPattern, subclicks) };
	}
}


std::vector<SongElement> evaluateScript(const ScriptPtr script, std::map<std::string, float>* initialEnv, ErrorInfo* errorInfo, extraSongInfo& extraInfo) {
    if (!script) return {};
    std::vector<SongElement> song;
    std::map<std::string, float> env;

    //add all the variables from the script and the argument to the shared env
    if (initialEnv) {
        for (auto const& x : *initialEnv) {
            env[x.first] = x.second;
        }
    }

    float lastFreq = 0;

    //go through each statement and evaluate it recursively
    for (auto cur : script->statements) {
        //if this is a section node, then we need to set the extraInfo to reflect it's start/end
        bool currentlyProcessingSection = false;
        if (auto section = dynamic_cast<SectionNode*>(cur.get())) {
            //set start of section index
			extraInfo.hasSection = true;
			extraInfo.sectionStartInd = song.size();   //current position + 1
            //set end after evaluation
            currentlyProcessingSection = true;
        }


		auto res = evaluateStatement(cur, &env, errorInfo, &lastFreq);
		if (errorInfo->message != "") return {};
		song.insert(song.end(), res.begin(), res.end());


        if (currentlyProcessingSection) {
            //now we can set the end index
			extraInfo.sectionEndInd = song.size() - 1;
        }
    }

	//clear initialEnv, then copy env into it
	if (initialEnv) {
		initialEnv->clear();
		for (auto const& x : env) {
			(*initialEnv)[x.first] = x.second;
		}
    }

    return song;
}






/*
-------------------============ MAIN FUNCTIONS ============-------------------
*/


ScriptPtr generateAST(std::string& songcode, ErrorInfo *errorInfo) {
    SongCodeLexer lexer(songcode);
    auto lexerToks = lexer.tokenize(errorInfo);
    lexer.printTokens(lexerToks);
    Parser parser(lexerToks, errorInfo);
    ScriptPtr ast = parser.parse();

    juce::Logger::writeToLog("-------------------- COMPILER AST OUTPUT -------------------" + juce::String(lexerToks.size()));
    juce::Logger::writeToLog(juce::String(astToString(ast)));
    return ast;
    return nullptr;
}


std::vector<SongElement> evaluateAST(ScriptPtr ast, ErrorInfo* errorInfo, std::map<std::string, float>* vars) {
    //for storing the positions of sections, if we use them.
    //return by setting a pointer passed in as an optional argument
    struct extraSongInfo extraInfo = {};

	auto song = evaluateScript(ast, vars, errorInfo, extraInfo);
    juce::Logger::writeToLog("---------------------- EVALUATOR OUTPUT ---------------------" + juce::String(song.size()));
	for (auto& elem : song) {
		juce::Logger::writeToLog(elem.toString());
	}
    if (errorInfo->message != "") {
        juce::Logger::writeToLog("-------------------- ERROR MESSAGE ----------------------\n" + errorInfo->message);
    }


	if (!ast) return {};
	return song;
}