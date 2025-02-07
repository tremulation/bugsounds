/*
  ==============================================================================

    Evaluator.h
    Created: 30 Jan 2025 4:38:46pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include "SongCodeCompiler.h"
#include <JuceHeader.h>
#include <vector>
#include <string>
#include <map>


struct StatementNode;
struct ExprNode;
struct ScriptNode;

using ExprPtr = juce::ReferenceCountedObjectPtr<ExprNode>;
using StatementPtr = juce::ReferenceCountedObjectPtr<StatementNode>;
using ScriptPtr = juce::ReferenceCountedObjectPtr<ScriptNode>;


/* 
Error reporting struct 
Should include everything needed to report errors in the UI. 
set inside the evaluator, and used in the ui thread
*/
struct ErrorInfo {
    std::string message;
    size_t errorStart;
    size_t errorEnd;
    std::string tokenText;
};


inline void setErrorInfo(ErrorInfo* errorInfo, const std::string& message,
    size_t start, size_t end, const std::string& text) {
    errorInfo->message = message;
    errorInfo->errorStart = start;
    errorInfo->errorEnd = end;
    errorInfo->tokenText = text;
}



/* -------------------============ LEXER TOKENS ============-------------------*/
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

    explicit Token(TokenType t, size_t s, size_t e, const std::string& txt) :
        type(t), numValue(0), idValue(""), startPos(s), endPos(e), text(txt) {}
    Token(TokenType t, int num, size_t s, size_t e, const std::string& txt) :
        type(t), numValue(num), idValue(""), startPos(s), endPos(e), text(txt) {}
    Token(TokenType t, const std::string& id, size_t s, size_t e, const std::string& txt) :
        type(t), numValue(0), idValue(id), startPos(s), endPos(e), text(txt) {}
};



/*
-------------------============ AST NODES ============-------------------

The tokens from the lexer are compiled into a tree of nodes of this structure.
The two overarching node types are statement nodes, and expression nodes.

Statement nodes will evaluate directly into a note, or a pattern (with the
exception of variable declarations, and loops). 

Expression nodes always get compiled into numbers. These numbers then form
parts of statements (the subbeats of patterns, or the freq/dur of notes, for
example.


*/



struct StatementNode : public juce::ReferenceCountedObject {
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatementNode)
    StatementNode() = default;
    virtual ~StatementNode() = default;
};

struct ExprNode : public juce::ReferenceCountedObject{
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExprNode)
    ExprNode() = default;
    virtual ~ExprNode() = default;
};

//scriptnode is the main one. keeps track of the context
struct ScriptNode : public juce::ReferenceCountedObject {
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptNode)
    //using a vector of statements instead of a statement list for simplicity here
    std::vector<StatementPtr> statements;
    std::map<std::string, int> variables;
    ScriptNode() = default;
};


//notenode: freq and duration
struct NoteNode : StatementNode {
    ExprPtr frequency;
    ExprPtr duration;

    NoteNode(ExprPtr freq, ExprPtr dur) : frequency(freq), duration(dur) {};
};


//patternNode: list of sub beat expresions
struct PatternNode : StatementNode {
    std::vector<ExprPtr> subBeats;

    PatternNode(const std::vector<ExprPtr>& beats) : subBeats(beats) { }
};


//loopNode: loop body, and iteration count
struct LoopNode : StatementNode {
    std::vector<StatementPtr> body;
    ExprPtr iterations;

    LoopNode(std::vector<StatementPtr> body, ExprPtr iter) : body(std::move(body)), iterations(iter) {}
};


//letNode: variable declaration
struct LetNode : StatementNode {
    std::string id;
    ExprPtr value;

    LetNode(const std::string& id, ExprPtr val) : id(id), value(val) {}
};



//additiveExprNode: left, right, operator
struct AdditiveExprNode : ExprNode {
    ExprPtr left;
    ExprPtr right;
    enum Op{Add, Subtract} op;

    AdditiveExprNode(ExprPtr l, ExprPtr r, Op op) : left(l), right(r), op(op) {}
};


//multiplicativeExprNode: left, right, operator
struct MultiplicativeExprNode : ExprNode {
    ExprPtr left;
    ExprPtr right;
    enum Op { Multiply, Divide } op;

    MultiplicativeExprNode(ExprPtr l, ExprPtr r, Op op) : left(l), right(r), op(op) {}
};


//primaryExprNode: integer, var, or another expr grouped in parenthesis
struct PrimaryExprNode : ExprNode {
    enum Kind { Integer, Variable, Grouped };
    Kind kind;

    int integerValue;
    std::string variableName;
    ExprPtr groupedExpr;

    PrimaryExprNode(int value) : kind(Integer), integerValue(value) {}
    PrimaryExprNode(const std::string& var) : kind(Variable), variableName(var) {}
    PrimaryExprNode(ExprPtr expr) : kind(Grouped), groupedExpr(expr) {}
};


// RandomNode: min and max expressions
struct RandomNode : ExprNode {
    ExprPtr min;
    ExprPtr max;

    RandomNode(ExprPtr min, ExprPtr max) : min(min), max(max) {}
};


/* -------------------============ PARSER CLASS ============------------------- */
class Parser {
public:
    Parser(const std::vector<Token>& toks, ErrorInfo* errorInfo)
        :tokens(toks), errorInfo(errorInfo), it(tokens.begin()), end(tokens.end()) {}

    ScriptPtr parse();
private:

    //all parsing functions share this state state
    const std::vector<Token>& tokens;
    ErrorInfo* errorInfo;
    std::vector<Token>::const_iterator it;
    std::vector<Token>::const_iterator end;
    ScriptPtr AST = new ScriptNode(); //ast being build
    std::map <std::string, int> variables;

    //parsing helpers
    bool match_token(TokenType expected);
    std::optional<Token> lookahead(int offset = 0) const;

    // Parsing methods
    bool parse_statement();
    bool parse_note();
    bool parse_pattern();
    bool parse_let();
    bool parse_loop();

    // Expression parsing
    ExprPtr parse_additive_expr();
    ExprPtr parse_multiplicative_expr();
    ExprPtr parse_primary_expr();
};

//implied nullptr for vars. If you don't provide a pointer to already-initialized variables, then they will be null
std::vector<SongElement> evaluateSongString(std::string& songcode,
                                            ErrorInfo* errorInfo,
                                            std::map<std::string, float>* vars = nullptr);