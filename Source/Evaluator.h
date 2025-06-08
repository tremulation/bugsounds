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
    Comment,
    //revision 2
    CurlyStart,
    CurlyEnd,
    GetScale,
    ClampLength,
    Subclick,
    Chitter,
    Choose
};


struct Token {
    TokenType type;
    float numValue;
    std::string idValue;    //variable name, if a var
    std::string text;       //exact text of the token
    size_t startPos;
    size_t endPos;

    explicit Token(TokenType t, size_t s, size_t e, const std::string& txt) :
        type(t), numValue(0), idValue(""), startPos(s), endPos(e), text(txt) {}
    Token(TokenType t, float num, size_t s, size_t e, const std::string& txt) :
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


//subbeatPatternNode: list of subclick percentages
struct SubclickPatternNode : StatementNode {
    std::vector<ExprPtr> subclicks; //percentages of the full list of subclicks to use, 0-100

    SubclickPatternNode(const std::vector<ExprPtr>& beats) : subclicks(beats) { }
};


//section node: a loopable section in the song, if looping is enabled. just a statement list
struct SectionNode : StatementNode {
    std::vector<StatementPtr> body;

    SectionNode(std::vector<StatementPtr> body) : body(std::move(body)) {}
};


//chitter operator node: expr* chitter pattern, and then statement list, 
struct ChitterNode : StatementNode {
    std::vector<StatementPtr> body;
    std::vector<ExprPtr> chitterPattern;

    ChitterNode(std::vector<StatementPtr> body, const std::vector<ExprPtr>& pattern) : body(std::move(body)), chitterPattern(pattern) {}
};

//clamplength node: expr length first, followed by statements
struct ClampLengthNode : StatementNode {
	std::vector<StatementPtr> body;
    ExprPtr length; //length in ms

	ClampLengthNode(std::vector<StatementPtr> body, ExprPtr len) : length(len), body(std::move(body)) {}
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

//getscalenode: has a baseFreq, an interval, and a scale type enum
//in code, of the format: getScale(baseFreq, interval, scaleNum). All arguments are ints, so we can randomize them
struct GetScaleNode : ExprNode {
	enum ScaleType { Major, Minor, MelodicMinor, Mixolydian, Dorian, Lydian, Chromatic, Pentatonic, MinorPentatonic, Edo10, Edo22, Edo29,};
	ExprPtr baseFrequency;
	ExprPtr interval;
	ExprPtr scaleType; //scaletype is an expression so we can randomize it/change it easier in code

	GetScaleNode(ExprPtr baseFreq, ExprPtr interval, ExprPtr type)
		: baseFrequency(baseFreq), interval(interval), scaleType(type) {}
};


//choose(expr index, expr* choices): returns the choice at index from the supplied list
//might be useful
struct ChooseNode : ExprNode {
	ExprPtr index; 
	std::vector<ExprPtr> choices;

	ChooseNode(ExprPtr idx, const std::vector<ExprPtr>& choicesList)
		: index(idx), choices(choicesList) {}
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
    bool parse_operator();
    bool parse_spattern();
	bool parse_chitter();
	bool parse_clamplength();
	bool parse_section();
    

    //for making sure there's only one section in the song. 
    //set this at the start of compilation. 
    bool sectionAlreadyCompiled = false;


    // Expression parsing
    ExprPtr parse_getscale();
    ExprPtr parse_additive_expr();
    ExprPtr parse_multiplicative_expr();
    ExprPtr parse_primary_expr();
    ExprPtr parse_choose_expr();
};


//implied nullptr for vars. If you don't provide a pointer to already-initialized variables, then they will be null
ScriptPtr                generateAST(std::string& songcode, ErrorInfo* errorInfo);

std::vector<SongElement> evaluateAST(ScriptPtr ast, ErrorInfo* errorInfo, std::map<std::string, float>* vars);


std::vector<SongElement> evaluateScript(const ScriptPtr script, std::map<std::string, float>* initialEnv, 
    ErrorInfo* errorInfo, extraSongInfo& extraInfo);