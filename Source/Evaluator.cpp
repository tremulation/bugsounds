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
        //TODO does this load them into the loop backwards, or is my print method backwards?
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

int evaluateExpr(const ExprPtr expr, std::map<std::string, float>* env, ErrorInfo* errorInfo) {
    if (!expr) {
        setErrorInfo(errorInfo, "Error: null expression node encountered", 0, 0, "");
        return -1;
    }

    //evaluate additive expression
    if (auto* additive = dynamic_cast<AdditiveExprNode*>(expr.get())) {
        int left_val = evaluateExpr(additive->left, env, errorInfo);
        if (errorInfo->message != "") return -1;
        int right_val = evaluateExpr(additive->right, env, errorInfo);
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
		int left_val = evaluateExpr(multiplicative->left, env, errorInfo);
		if (errorInfo->message != "") return -1;
		int right_val = evaluateExpr(multiplicative->right, env, errorInfo);
		if (errorInfo->message != "") return -1;

		switch (multiplicative->op) {
		case MultiplicativeExprNode::Multiply: return left_val * right_val;
		case MultiplicativeExprNode::Divide: return left_val / right_val;
		default:
			setErrorInfo(errorInfo, "Error: unknown multiplicative operator", 0, 0, "");
			return -1;
		}
	}

    //evaluate primary expressions
     else if (auto* inty = dynamic_cast<PrimaryExprNode*>(expr.get())) {
        if (inty->kind == PrimaryExprNode::Integer) {
            return inty->integerValue;
        }
        if (inty->kind == PrimaryExprNode::Variable) {
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
        int freq = evaluateExpr(note->frequency, env, errorInfo);
        if (errorInfo->message != "") return {};
        int dur = evaluateExpr(note->duration, env, errorInfo);
        if (errorInfo->message != "") return {};
        float oldLastFreq = *lastFreq;
        *lastFreq = freq;
		//return a vector with one note
		return { SongElement(oldLastFreq, freq, dur) };
    }
    //patterns
    else if (auto pattern = dynamic_cast<PatternNode*>(statement.get())) {
        std::vector<uint8_t> patternVec;
        for (auto& subBeat : pattern->subBeats) {
            int subBeatVal = evaluateExpr(subBeat, env, errorInfo);
            if (errorInfo->message != "") return {};
            patternVec.push_back(subBeatVal);
        }
        return { SongElement(patternVec) };
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
}


std::vector<SongElement> evaluateScript(const ScriptPtr script, std::map<std::string, float>* initialEnv, ErrorInfo* errorInfo) {
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
		auto res = evaluateStatement(cur, &env, errorInfo, &lastFreq);
		if (errorInfo->message != "") return {};
		song.insert(song.end(), res.begin(), res.end());
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
    Parser parser(lexerToks, errorInfo);
    ScriptPtr ast = parser.parse();

    juce::Logger::writeToLog("-------------AST-------------" + juce::String(lexerToks.size()));
    juce::Logger::writeToLog(juce::String(astToString(ast)));
    return ast;
}


std::vector<SongElement> evaluateAST(ScriptPtr ast, ErrorInfo* errorInfo, std::map<std::string, float>* vars) {
	auto song = evaluateScript(ast, vars, errorInfo);
	/*for (auto& elem : song) {
		juce::Logger::writeToLog(elem.toString());
	}*/
	/*juce::Logger::writeToLog("------------Error------------");
	juce::Logger::writeToLog(errorInfo->message);*/
	if (!ast) return {};
	return song;
}